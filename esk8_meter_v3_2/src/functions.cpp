#include <Arduino.h>
#include "functions.h"


/// -----------BUZZER-----------

Buzzer::Buzzer(void)
{
    //constructor
    freq = B00000101; // default
    channels = INTERNAL;
    time = 0;
}

void Buzzer::setFreq(uint8_t newFreq)
{
    freq = newFreq;

    ///B00000111 61Hz
    ///B00000110 244Hz
    ///B00000101 488Hz
    ///B00000100 976Hz
    ///B00000011 1953Hz
    ///B00000010 7812Hz
}

void Buzzer::setTime(uint8_t newTime)
{
    time = newTime;
    /// time == calls of Buzzer::frameUpdate()
}

void Buzzer::setDutyC(uint8_t newDuty)
{
  OCR1A = 254-newDuty;
  OCR1B = 254-newDuty;
}

void Buzzer::frameUpdate(void)
{
  if (freq == F0 || freq == F1 || freq == F3 || freq == F5)
  {
     uint8_t freq_T = freq;
     
    freq_T &= ~B11111000; // make safe for deployment
    TCCR1B &= ~B00000111;
    
    // change Fast PWM mode 8bit
    TCCR1A |= (1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    
    TCCR1B |=  freq_T;
  }
  else if (freq == F4)
  {
    uint8_t freq_T = freq;
     
    freq_T &= ~B11111000; // make safe for deployment
    TCCR1B &= ~B00000111;
    
    // change Fast PWM mode 10bit
    TCCR1A |= (1 << WGM10);
    TCCR1A |= (1 << WGM11);
    
    TCCR1B |=  freq_T;
  }
  else
  {
    uint8_t freq_T = freq;

    freq_T &= ~B11111000; // make safe for deployment
    TCCR1B &= ~B00000111;
    
    // change Fast PWM mode 9bit
    TCCR1A |= (1 << WGM11);
    TCCR1A &= ~(1 << WGM10);
    
    TCCR1B |=  freq_T;
  }

  if (time > 0)
  {
    if (channels & EXTERNAL)
      DDRB |= (1 << PB1);
    if (channels & INTERNAL)
      DDRB |= (1 << PB2);
  }
  else
  {
    DDRB &= ~(1 << PB1);
    DDRB &= ~(1 << PB2);
  }

  if (time > 0)
      time --;
}

void Buzzer::setChannel(uint8_t _channel)
{
    channels = _channel;
}

void Buzzer::init(void)
{
  // satrt switched off
  DDRB &= ~(1 << PB1);
  DDRB &= ~(1 << PB2);

  ICR1 = 0xFFFF; // set TOP to 16bit

  // set PWM for 50% duty
  OCR1A = 128;
  OCR1B = 128;

  TCCR1A |= (1 << COM1A1)|(1 << COM1B1); // set none-inverting mode
  
  // set Fast PWM mode 8bit
  TCCR1A |= (1 << WGM10);
  TCCR1B |= (1 << WGM12);
  
  TCCR1B |= (1 << CS12); // START the timer with no prescaler
}

/// -----------PPM-----------

PPM::PPM(void)
{
    //constructor
}

void PPM::init(uint8_t ppmPin)
{
    pin = ppmPin;
    pinMode(pin, INPUT);
}

uint16_t PPM::getDuty(void)
{
    uint16_t probeA = pulseIn(pin, HIGH, 60000); // timeout for 50Hz PPM: 60ms 16 HZ --> too low
    //uint16_t probeB = pulseIn(pin, HIGH, 60000); // takes too long

    if ( probeA == 0) // if ( (probeA+probeB) == 0)
        return 1;

    return probeA; // return(uint16_t)( (probeA+probeB)/2);
}

/// -----------MENU-----------

Menu::Menu(void)
{
    //constructor
}

void Menu::init(uint16_t senseCenter, uint16_t senseMax, uint8_t senseLR)
{
    if (senseLR != 0)
        lr = 1;
    else
        lr = 0;

    triggersL = 0;
    triggersR = 0;
    framesTriggeredL = 0;
    framesTriggeredR = 0;
    framesCentered = 0;
    framesMaxedL = 0;
    framesMaxedR = 0;
    framesSpecial = 0;

    pendingRequest = 0;
    newTriggerVal = 0;
    maxTriggerStatus = 0;

    /// use standard?
    if (senseCenter<400 || senseCenter>3000 || senseMax<400 || senseMax>3000)
        return;
    else
    {
        if (senseCenter > senseMax)
        {
            if (senseCenter-senseMax < 300)
                return;
        }
        else
        {
            if (senseMax-senseCenter < 300)
                return;
        }
    }

    uint16_t maxRange;

    if (senseCenter > senseMax)
        maxRange = senseCenter-senseMax;
    else
        maxRange = senseMax-senseCenter;

    center =      senseCenter;
    maxLevel =   (uint16_t)( ( (float)maxRange )*0.73333 );
    minLevel =   (uint16_t)( ( (float)maxRange )*0.36666 );
    hystCenter = (uint16_t)( ( (float)maxRange )*0.13333 );
}

void Menu::doTheMenu(uint16_t duty)
{
    /// if PPM invalid, don't do the menu!
    if (duty < minDuty || duty > maxDuty)
        return;

    /// let or right here
    if (duty < center - minLevel)
    {
        if (lr == 0)
            framesTriggeredL ++;
        else
            framesTriggeredR ++;
    }
    else if (duty > center + minLevel)
    {
        if (lr == 0)
            framesTriggeredR ++;
        else
            framesTriggeredL ++;
    }
    else if ( (duty < (center + hystCenter)) && (duty > (center - hystCenter)) )
    {
        framesCentered ++;

        framesTriggeredL = 0;
        framesTriggeredR = 0;

        framesMaxedL = 0;
        framesMaxedR = 0;
        maxTriggerStatus = 0;

        framesSpecial = 0;
    }

    if (duty < center - maxLevel)
    {
        if (lr == 0)
        {
            framesMaxedL ++;
        }
        else
        {
            framesMaxedR ++;
            framesSpecial ++;
        }
    }
    else if (duty > center + maxLevel)
    {
        if (lr == 0)
        {
            framesMaxedR ++;
            framesSpecial ++;
        }
        else
        {
            framesMaxedL ++;
        }
    }

    if (framesTriggeredL > untilTrigger && framesCentered > 0)
    {
        ///one trigger L
        triggersL ++;
        newTriggerVal = triggersL;

        framesCentered = 0; // no more triggers
    }
    else if (framesTriggeredR > untilTrigger && framesCentered > 0)
    {
        ///one trigger R
        triggersR ++;
        newTriggerVal = triggersR;

        framesCentered = 0; // no more triggers
    }

    if ( (triggersL != 0 && triggersR != 0) || (framesTriggeredL != 0 && framesTriggeredR != 0) )
    {
        //direction change
        // maybe with skip over center
        //reset!
        triggersL = 0;
        triggersR = 0;
        newTriggerVal = 5; // direction changed indicator, override newTriggerVal

        framesTriggeredL = 0;
        framesTriggeredR = 0;
        framesCentered = 0;
    }
                                                                            // vv <--- at least wait until the wheel returns
    if (framesCentered > hystLatch) // || ( (triggersL >= 4 || triggersR >= 4) && framesCentered > 0) )
    {
        //if (triggersL >= 4 || triggersR >= 4) // simulate latch time
        //    delay((hystLatch-2)*100);

        pendingRequest = triggersL;
        if (triggersR != 0)
        {
            pendingRequest = 4 + triggersR; // offset
        }
        triggersL = 0;
        triggersR = 0;
    }
    else if (framesMaxedL > hystMaxLatch && maxTriggerStatus == 0)
    {
        pendingRequest = 9;
        triggersL = 0;
        triggersR = 0;

        maxTriggerStatus = 1;
    }
    else if (framesMaxedR > hystMaxLatch && maxTriggerStatus == 0)
    {
        pendingRequest = 10;
        triggersL = 0;
        triggersR = 0;

        maxTriggerStatus = 1;
    }
    else if (framesSpecial > SPEC_LATCH) /// special state -- 20s //200 ------------------------------------------
    {
        framesSpecial = 0;
        pendingRequest = 11;
    }
    else if (triggersL > 4 || triggersR > 4) // reset menu
    {
        hardReset();
        newTriggerVal = 12;
    }


}

uint8_t Menu::getMenuAction(uint8_t type)
{
    switch(type)
    {
        uint8_t temp;

    case TRIGGER:
        temp = newTriggerVal;
        newTriggerVal = 0;
        return temp;
    case REQUEST:
        temp = pendingRequest;
        pendingRequest = 0;
        return temp;
    case TRIGGER_PRESERVE:
        return newTriggerVal;
    case REQUEST_PRESERVE:
        return pendingRequest;
    }

    return 0;
}

uint16_t Menu::getCenter(void)
{
    return center;
}

uint16_t Menu::getMaxLevel(void)
{
    return maxLevel;
}

void Menu::hardReset(void)
{
    triggersL = 0;
    triggersR = 0;
    framesTriggeredL = 0;
    framesTriggeredR = 0;
    framesCentered = 0;
    framesMaxedL = 0;
    framesMaxedR = 0;
    framesSpecial = 0;

    pendingRequest = 0;
    newTriggerVal = 0;
    maxTriggerStatus = 0;
}



