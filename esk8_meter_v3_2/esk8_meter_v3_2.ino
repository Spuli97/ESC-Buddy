#include "src/VESC_UART/VescUart.h"
#include "src/functions.h"
#include "src/MEMORY/fram.h"
#include "src/MEMORY/eeprom.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <Wire.h>

// _v2 -- search tag //

// EEPROM:
// 0 config reboot
// 1-4 PPM ranges (middle, maxed)
// 5   reversed
// 6-13 calculation values (TAC_PER_KM_tmep, ERPM_PER_KMPH_tmep)
// 14 cell count
// 15-16 capacity
// 17-18 lv alarm

#define SAMPLE_RATE 70 // s*10 // not geht lower than 3s!! and not more than 20s!!

#define VESC_SLAVE_ID 1

#define DRIVING_SPD 0.585 //0.085

#define REQUEST_CONFIRMATION 150
#define REQUEST_TO_BEEP      600

#define DIGIT                200
#define ZERO_DIGIT           200
#define DIGIT_DELAY           70
#define BETWEEN_DIGIT        500
#define DIGIT_FREQ            F2
#define ZERO_FREQ             F4

#define FRAM_DATA_LEN         40 // max. 42

const PROGMEM uint16_t batData60m[] = { 41467, 41382, 41296, 41210, 41124, 41039, 40953, 40867, 40781, 40695,   // idle voltages*10000: 100-1% state of charge 01h after load
                                        40610, 40524, 40438, 40352, 40266, 40181, 40095, 40009, 39923, 39837, 
                                        39752, 39666, 39580, 39494, 39408, 39323, 39237, 39151, 39065, 38979, 
                                        38894, 38808, 38722, 38636, 38550, 38465, 38379, 38293, 38207, 38121, 
                                        38036, 37950, 37864, 37778, 37693, 37607, 37521, 37435, 37349, 37264, 
                                        37178, 37092, 37006, 36920, 36835, 36749, 36663, 36577, 36491, 36406, 
                                        36320, 36234, 36148, 36062, 35977, 35891, 35805, 35719, 35633, 35548, 
                                        35462, 35376, 35290, 35204, 35119, 35033, 34947, 34861, 34775, 34690, 
                                        34604, 34518, 34432, 34346, 34261, 34175, 34089, 34003, 33918, 33832, 
                                        33746, 33660, 33574, 33489, 33403, 33317, 33231, 33145, 33060, 32974 }; 

const PROGMEM uint16_t batData10s[] = { 41436, 41342, 41248, 41154, 41060, 40966, 40873, 40779, 40685, 40591,   // idle voltages*10000: 100-1% state of charge 10s after load
                                        40497, 40403, 40309, 40215, 40122, 40028, 39934, 39840, 39746, 39652, 
                                        39558, 39464, 39370, 39277, 39183, 39089, 38995, 38901, 38807, 38713, 
                                        38619, 38526, 38432, 38338, 38244, 38150, 38056, 37962, 37868, 37775, 
                                        37681, 37587, 37493, 37399, 37305, 37211, 37117, 37024, 36930, 36836, 
                                        36742, 36648, 36554, 36460, 36366, 36273, 36179, 36085, 35991, 35897,
                                        35803, 35709, 35615, 35521, 35428, 35334, 35240, 35146, 35052, 34958, 
                                        34864, 34770, 34677, 34583, 34489, 34395, 34301, 34207, 34113, 34019, 
                                        33926, 33832, 33738, 33644, 33550, 33456, 33362, 33268, 33175, 33081, 
                                        32987, 32893, 32799, 32705, 32611, 32517, 32424, 32330, 32236, 32142 };  
                                     

volatile unsigned long msc;

// the data saved in FRAM
volatile uint32_t erpmTrip = 0;
volatile uint64_t erpmAbs  = 0;  
volatile uint32_t WhDAbs   = 0;   
volatile uint32_t WhCAbs   = 0;   
volatile uint32_t dTime    = 0;     // sek
volatile float WhDTrip     = 0.0;    
volatile float WhCTrip     = 0.0;    
volatile uint32_t spdTop   = 0;     // 10*km/h _v3 unused!! (but still in FRAM etc.
volatile uint32_t powCycs  = 0;  

// data for computation fusing saved and current data
volatile uint32_t dt  = 0;
volatile uint64_t da  = 0;
volatile uint32_t wda = 0;
volatile uint32_t wca = 0;
volatile uint32_t tim = 0;  
volatile float wdt    = 0.0;
volatile float wct    = 0.0;
//volatile uint32_t spt = 0;
//volatile uint32_t pwc = 0;  

volatile float wut    = 0.0;
volatile float cv     = 0.0;
// for trip reset
volatile float curWhDOnReset     = 0.0;
volatile float curWhCOnReset     = 0.0;
volatile uint32_t curErpmOnReset = 0;

volatile float old_wut     = 0.0;
volatile uint8_t energyDir = 0;

// menu PPM
// MUST INITIALIZE TO ZERO! valueConfig() add to this with +=
uint16_t middle = 0;
uint16_t maxed  = 0;
uint8_t reverse = 0;

// calculation data for user output
volatile float TAC_PER_KM      = 1.0; //173623.5743;
volatile float ERPM_PER_KMPH   = 1.0; //434.058935636;
volatile float CELL_CNT        = 1.0; //8.0;
volatile uint16_t CAPACITY     = 1; //1200
volatile float LV_ALARM        = 1.0;

VescUart UART;
Buzzer     piezo;
PPM        remote;
Menu       _menu;
FRAM4_ST   _fram;
EEPROM_ST  _eeprom;

// in the main file
void WDT_Init(void);
//ISR(WDT_vect);

//in SYSTEM_functions.ino
void initialConfig(void);
void valueConfig(void);
void sortNumbersAscending(uint16_t number[], uint8_t count);
uint32_t estimateRange(void);
void lvAlarm(void);

// in IO_functions.ino
void beepOutInt (uint32_t number, uint8_t fixedDigits, uint8_t indZero); // max. 9 digits // max. float 32 // fixedDigits 0 if whole number chell be beeped out // reqires "piezo" Buzzer object
void beepOutFloat (float number, uint8_t decimals); // max. 3 decimals //max. float32 // reqires "piezo" Buzzer object
void menuSounds(uint8_t menuAction); // reqires "piezo" Buzzer object
void resetSound(void);
void vescCommErrLoop(void);
void waitingForReboot(void);
void justBuzz(uint16_t t, uint8_t f);
void loopBuzz(uint16_t t, uint8_t f, uint16_t times, uint16_t pause);
void noDataSound(void);

// in MEMORY_functions.ino
uint8_t readFRAM(uint8_t cluster);
void writeFRAM(uint8_t cluster);

void WDT_Init(void)
{
  cli();
  wdt_reset();
  
  //set up WDT interrupt
  WDTCSR = (1<<WDCE)|(1<<WDE);
  //Start watchdog timer with 8s prescaller
  WDTCSR = (1<<WDIE)|(1<<WDP3)|(1<<WDP0);
  
  sei();
}

ISR(WDT_vect) //Watchdog timeout ISR
{
  cli();
  wdt_reset();
  WDTCSR = (1<<WDCE)|(1<<WDE);
  wdt_disable();
  sei();
  // error, beep until manual reset
  
  piezo.setChannel(BOTH);

  piezo.frameUpdate();
  while(1) // WDT ERROR TONE
  {
    loopBuzz(200, F4, 4, 200); /// ----  ---- ---- ----
    
    delay(500);
  }
}


void setup()
{
  delay(1000); // give the vesc time so start up //5000
  
  WDT_Init();  // watchdog timer 8s
  
  piezo.init();
  piezo.setChannel(INTERNAL); 
  piezo.setDutyC(127);
  
  remote.init(6); // on PD6 , pin10

  if(_fram.init(80))
    resetSound();

  
  
  _eeprom.init(1024);

  wdt_reset();
  
  // code for initializer programm
  /*
  writeFRAM(0);
  writeFRAM(1);
  writeFRAM(2);
  waitingForReboot();
  */
  //end
  
  /*
  // code for fram to eeprom
  _fram.setPtrAddr(0);
  _eeprom.setPtrAddr(0);
  
  uint8_t i;
  for(i=0; i<B11111111; i++)
  {
    _eeprom.append8Bit(_fram.read8Bit());
  }
  waitingForReboot();
  */
  //end
  
  // code for eeprom to fram
  /*
  _fram.setPtrAddr(0);
  _eeprom.setPtrAddr(0);
  
  uint8_t i;
  for(i=0; i<B11111111; i++)
  {
    _fram.append8Bit(_eeprom.read8Bit());
  }
  waitingForReboot();
  */
  //end

  initialConfig();
  valueConfig();

  _menu.init(middle, maxed, reverse); // center, max throw, left right inverting ( if set to 1, 0 else)
  
  wdt_reset();

  //serial port setup
  Serial.begin(115200);
  while (!Serial) {;}
  UART.setSerialPort(&Serial);

  wdt_reset();

  piezo.setChannel(BOTH); 
  delay(10);
  
  if (readFRAM(0) == 0)
    if (readFRAM(1) == 0)
      if (readFRAM(2) == 0)
        waitingForReboot();

  piezo.setChannel(INTERNAL); 
        
  powCycs ++;
  
  wdt_reset();
}

void loop()
{
  uint8_t loopCnt = 0;
  long backupTimeA = 0;
  long backupTimeB = 0;
  long loopTime = millis();
  long movTime = 0;
  
  uint8_t volume = 0;
  
  while(1)
  {
    msc = millis(); // start new frame time (max. 8s or call a beep out function at max. 7s of passed frame time)
    loopCnt ++;
    piezo.frameUpdate();

    _menu.doTheMenu(remote.getDuty());
    menuSounds(_menu.getMenuAction(TRIGGER));
    
    if (loopCnt >= SAMPLE_RATE || _menu.getMenuAction(REQUEST_PRESERVE) != 0)  // get (and save) the vesc values every 10sek or on request
    {
      loopCnt = 0;
      
      float inpVoltageTmp;
      float wattHoursTmp;
      float wattHoursChargedTmp;
      long  tachometerAbsTmp;
      long  spd;
      float amps;
      
      // if request triggered: beep!
      unsigned long beeper;
      if ( _menu.getMenuAction(REQUEST_PRESERVE) != 0)
      {
        loopCnt = 50; // faster update after beeping _v2 // attention to SAMPLE_RATE
        beeper = millis();
        piezo.frameUpdate(); piezo.setTime(1); piezo.setFreq(F0); piezo.frameUpdate();
      }
      
      // now get the newest vesc_values from the vesc over UART
      if ( UART.getVescValues(0) )
      {
        // save the data
        inpVoltageTmp          = UART.data.inpVoltage;
        wattHoursTmp           = UART.data.wattHours;
        wattHoursChargedTmp    = UART.data.wattHoursCharged;
        tachometerAbsTmp       = UART.data.tachometerAbs;
        spd                    = UART.data.erpm;
        amps                   = UART.data.avgInputCurrent; // _v2
        
        delay(2); //bugfix: extremly rare misscimmunication, give the VESC more time
        if( !UART.getVescValues(VESC_SLAVE_ID) ) // now update vesc values with 2nd vesc's data
          vescCommErrLoop();
          
        // prepare variables for the calculations
        dt=0; da=0; wda=0; wca=0; cv=0; wut=0; wdt=0; wct=0; //spd=0; // _v3 bugfix spd

        // combine data
        inpVoltageTmp         = (inpVoltageTmp + UART.data.inpVoltage) / 2.0;
        wattHoursTmp         += UART.data.wattHours;
        wattHoursChargedTmp  += UART.data.wattHoursCharged;
        tachometerAbsTmp      = (tachometerAbsTmp + UART.data.tachometerAbs) / 2;
        spd                   = (spd + UART.data.erpm) / 2;
        amps                 += UART.data.avgInputCurrent; // _v2

        // do the maths
        dt = erpmTrip - curErpmOnReset + (uint32_t)tachometerAbsTmp;
        da = erpmAbs + (uint64_t)tachometerAbsTmp;
        wda = WhDAbs + (uint32_t)round(wattHoursTmp);
        wca = WhCAbs + (uint32_t)round(wattHoursChargedTmp);
        cv = inpVoltageTmp / CELL_CNT;
        
        wdt = WhDTrip + wattHoursTmp - curWhDOnReset;
        wct = WhCTrip + wattHoursChargedTmp - curWhCOnReset;
        wut = wdt - wct;
        
        tim = dTime + movTime/1000;
          

        // for range estimation
        if (wut - old_wut > 0.03)
          energyDir = 1;
        else if (wut - old_wut < -0.03)
          energyDir = 0;

        old_wut = wut;

        // for volume and dTime
        if (((float)spd) / ERPM_PER_KMPH > DRIVING_SPD) // driving
        {
          piezo.setChannel(EXTERNAL);
          
          movTime += (millis() - loopTime);
        }
        else
        {
          if (volume == 0) // only if low volume is selected
            piezo.setChannel(INTERNAL);
        }
        loopTime = millis();
              
        writeFRAM(0); // save newly calculates vlaues in main cluster
        backupTimeA = millis(); // set timer for backup
        
        wdt_reset();
      }
      else
      {
        vescCommErrLoop();
      }

      if ( _menu.getMenuAction(REQUEST_PRESERVE) != 0) // stop beeping (if request triggered)
      {
        while (beeper + REQUEST_CONFIRMATION >= millis());
        piezo.frameUpdate(); piezo.frameUpdate();
        delay(REQUEST_TO_BEEP);
      }
      else // if NOT request triggered: do the voltage alarm
      {
        lvAlarm(); // _v2
      }

      switch(_menu.getMenuAction(REQUEST)) // _v2 
      {
      case 0: // nothing requested
        break;
        
      case 1: //dt --------------------------------------------------------------------------------------------------------------------------------------------
        if (dt > (uint32_t)(TAC_PER_KM/15.0)) // at least 67 meters
        {
          beepOutFloat( (float)( ((double)dt) / ((double)TAC_PER_KM) ), 1);
        }
        else
        {
          noDataSound();
        }
        break;
          
      case 2: //da --------------------------------------------------------------------------------------------------------------------------------------------
        if (da > (uint64_t)(TAC_PER_KM/15.0)) // at least 67 meters
        {
          beepOutInt( (uint32_t)( da / ((uint64_t)round(TAC_PER_KM)) ), 0, 1); // decimal digits are not relevant
        }
        else
        {
          noDataSound(); 
        }
        break;
          
      case 3: //power OR range estimate ------------------------------------------------------------------------------------------------------------------------
        if (((float)spd) / ERPM_PER_KMPH < DRIVING_SPD) // do range estimation if not moving
        {
          if (wut > 5.0 && dt > (uint32_t)(TAC_PER_KM/15.0)) // at least gether this much data
          {
            beepOutInt(estimateRange(), 0, 1);
          }
          else
          {
            noDataSound();
          }
        }
        else // show power use
        {
          if (amps >= 0.0)
          {
            beepOutInt( (uint32_t)round(amps * inpVoltageTmp), 0, 1);
          }
          else
          {
            loopBuzz(20, F3, 10, 20); wdt_reset();  delay(500);
            float regen = round(amps * inpVoltageTmp);
            regen *= (-1.0);
            beepOutInt( (uint32_t)regen, 0, 1); // sub zero (regen)
          }
        }
        break;
        
      case 4: //wda AND wca -----------------------------------------------------------------------------------------------------------------------------------
        beepOutInt(wda, 0, 1);
        delay(800); loopBuzz(50, F2, 5, 100); wdt_reset(); loopBuzz(50, F3, 5, 100); delay(800); // intermediate sound
        beepOutInt(wca, 0, 1);
        break;
        
      case 5: //cv --------------------------------------------------------------------------------------------------------------------------------------------
        beepOutFloat(cv, 2);
        break;
        
      case 6: //wut --------------------------------------------------------------------------------------------------------------------------------------------
        if (wut > 0.0)
        {
          beepOutFloat(wut, 1);
        }
        else // sub zero
        {
          noDataSound();
        }
        break;
        
      case 7: //wdt AND wct ------------------------------------------------------------------------------------------------------------------------------------
        beepOutFloat(wdt, 1);
        delay(800); loopBuzz(50, F2, 5, 100); wdt_reset(); loopBuzz(50, F3, 5, 100); delay(800); // intermediate sound //_v3_2
        beepOutFloat(wct, 1); //_v3_2
        break;
        
      case 8: //Wh/km ------------------------------------------------------------------------------------------------------------------------------------------  //_v3_2 (all)
        if (wut > 5.0 && dt > (uint32_t)(TAC_PER_KM/15.0)) // at least gether this much data && not div by 0
        {
          beepOutFloat(wut / (float)( ((double)dt) / ((double)TAC_PER_KM) ), 1);
        }
        else
        {
          noDataSound(); // no data or sub zero
        }
        break;
        
      case 9: //reset trip --------------------------------------------------------------------------------------------------------------------------------------
        erpmTrip = 0; dt = 0;
        WhDTrip = 0; wut = 0; old_wut = 0; wdt = 0;
        WhCTrip = 0; wct = 0;
        curWhDOnReset = wattHoursTmp;
        curWhCOnReset = wattHoursChargedTmp;
        curErpmOnReset = (uint32_t)tachometerAbsTmp;
        writeFRAM(0);
        resetSound();
        writeFRAM(1);
        backupTimeA = millis();
        break;
        
      case 10: //speed OR change volume ------------------------------------------------------------------------------------------------------------------------- //_v3_2
        if (((float)spd) / ERPM_PER_KMPH > DRIVING_SPD) // if moving with at least DRIVING_SPD
        {
          beepOutFloat( (float)( ((double)spd) / ((double)ERPM_PER_KMPH) ), 1); // show speed
        }
        else // change volume
        {
          if (volume == 0)
          {
            volume = 1;
            piezo.setChannel(EXTERNAL); 
          }
          else
          {
            volume = 0;
            piezo.setChannel(INTERNAL);
          }
        }
        break;
        
      case 11: //show all the special data and change volume ---------------------------------------------------------------------------------------------------
        loopBuzz(100, F3, 10, 200); wdt_reset(); loopBuzz(100, F4, 10, 200);

        // _deb
        /// offline debug code, also comment out serial setup and all of getVescValues in the vesc uart cpp, maybe set special latch to 80
        //UART.data.inpVoltage          += 4.5;
        //UART.data.wattHours           += 22.2;
        //UART.data.wattHoursCharged    += 11.1;
        //UART.data.tachometerAbs       += 173624; //4416;
        //UART.data.erpm                += 435;
        //UART.data.avgInputCurrent     += 5.0;

        delay(REQUEST_TO_BEEP);

        //total moving hours -----------------------------------------------------------------------------------------------------------------------------ok 
        if ((float)tim/3600.0 >= 0.1) 
        {
          if (tim < 64790) // < 17.9h
            beepOutFloat((float)tim/3600.0, 1);
          else
            beepOutInt(tim/3600, 0, 1);
        }
        else
        {
          noDataSound();
        }

        delay(800); loopBuzz(50, F2, 5, 100); wdt_reset(); loopBuzz(50, F3, 5, 100); delay(800);

        //total Wh/km -----------------------------------------------------------------------------------------------------------------------------------ok 
        if ((int64_t)wda-(int64_t)wca > (int64_t)10 && da > (uint64_t)(TAC_PER_KM*2)) // at least gether this much data && not div by 0
        {
          beepOutInt((int64_t)(wda-wca) / (int64_t)( da / ((uint64_t)round(TAC_PER_KM)) ), 0, 1);
        }
        else
        {
          noDataSound(); // no data or sub zero
        }
        
        delay(800); loopBuzz(50, F2, 5, 100); wdt_reset(); loopBuzz(50, F3, 5, 100); delay(800);
        
        //power cycles ----------------------------------------------------------------------------------------------------------------------------------ok
        beepOutInt(powCycs, 0, 1);
        break;
        
      default:
        break;
      }
    }
    else if (backupTimeA != 0 && millis() > backupTimeA+300) // never wait longer than SAMPLE_RATE/2
    {
      // write redundent data set backupTimeA later (=300ms (3 frames) later)
      backupTimeA = 0;
      writeFRAM(1);
      backupTimeB = millis();
    }
    else if (backupTimeB != 0 && millis() > backupTimeB+3500) // never wait longer than SAMPLE_RATE/2
    {
      // write redundent data set 30 (3s) frames later
      backupTimeB = 0;
      writeFRAM(2);
    }
    
    wdt_reset();
    while (msc + 100 >= millis()); // wait until next frame (10fps)
  }
}


