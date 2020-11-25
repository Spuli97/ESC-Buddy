// SYSTEM functions for back end

void initialConfig(void)
{
  _eeprom.setPtrAddr(0);
  
  if (_eeprom.read8Bit() != 0) // new config
  {
    uint16_t val[3] = {0};
    
    // get wheel threshholds
    justBuzz(3500, F1);          wdt_reset();
    val[0] = remote.getDuty();
    loopBuzz(200, F1, 4, 400);   wdt_reset();

    justBuzz(3500, F2);          wdt_reset();
    val[1] = remote.getDuty();  
    loopBuzz(200, F2, 4, 400);  wdt_reset();

    justBuzz(3500, F3);         wdt_reset();
    val[2] = remote.getDuty();
    loopBuzz(200, F3, 4, 400);  wdt_reset();

    // middle is center
    sortNumbersAscending(val, 3);

    // check for valid range 400-3000uS
    uint8_t i;
    for (i=0; i<3; i++)
    {
      if (val[i]<400 || val[i]>3000)
        waitingForReboot();
    }
    // check for minimal difference 300uS
    if (val[0]+300 > val[1] || val[1]+300 > val[2])
      waitingForReboot();

    // save middle (eeprom bit 1-2)
    _eeprom.append16Bit(val[1]);
    
    // get max. allowed and save it (eeprom bit 3-4)
    if (val[1]-val[0] < val[2]-val[1])
    {
      // lower is min distance
      _eeprom.append16Bit(val[0]);
    }
    else
    {
      // higher is min distance
      _eeprom.append16Bit(val[2]);
    }
    
    wdt_reset();
  }
}

void valueConfig(void)
{
  // EEPROM:
  // 0 config reboot
  // 1-4 PPM ranges (middle, maxed)
  // 5   reversed
  // 6-13 calculation values (TAC_PER_KM_tmep, ERPM_PER_KMPH_tmep)
  // 14 cell count
  // 15-16 capacity
  // 17-18 lv alarm
  

  _eeprom.setPtrAddr(0);
  
  // new config y/n?
  if (_eeprom.read8Bit() != 0)
  {
    _eeprom.setPtrAddr(0);
    _eeprom.append8Bit(0); // config done

    waitingForReboot();
  }
  
  // always: read the config:
  
  middle        = _eeprom.read16Bit();
  maxed         = _eeprom.read16Bit();
  reverse       = _eeprom.read8Bit();
  
  TAC_PER_KM    = _eeprom.readFloat32(); //173623.57812500;
  ERPM_PER_KMPH = _eeprom.readFloat32(); //434.05892944;
  
  CELL_CNT      = (float)_eeprom.read8Bit(); //8.0;
  CAPACITY      = _eeprom.read16Bit(); //1150;
  LV_ALARM      = (float)_eeprom.read16Bit(); LV_ALARM /= 1000.0; //2650.0;
}

void sortNumbersAscending(uint16_t number[], uint8_t count)
{
   uint16_t temp;
   uint8_t i, j, k;
   for (j = 0; j < count; ++j)
   {
      for (k = j + 1; k < count; ++k)
      {
         if (number[j] > number[k])
         {
            temp = number[j];
            number[j] = number[k];
            number[k] = temp;
         }
      }
   }
}

uint32_t estimateRange(void)
{
  int32_t codeVoltage = (int32_t)(cv*10000.0); //pgm_read_word_near(batData + 10);
  int32_t bestEstimateV = 32700;
  uint8_t vIter;
  float batPercent = 0.0;
  
  for (vIter=0; vIter<100; vIter++)
  {
    int32_t deltaV;
    if (energyDir)
      deltaV = (int32_t)pgm_read_word_near(batData10s + vIter) - codeVoltage;
    else
      deltaV = (int32_t)pgm_read_word_near(batData60m + vIter) - codeVoltage;
      
    if (deltaV < 0)
      deltaV *= (-1);
      
    if (deltaV <= bestEstimateV)
    {
      bestEstimateV = deltaV;
      batPercent = (float)(100-vIter);
    }
  }

  //beepOutInt((uint32_t)batPercent, 0, 1); delay(1000); // debug line
  float rangeEstimate = ( ((float)CAPACITY) * (batPercent/100.0)) / (wut / (float)( ((double)dt) / ((double)TAC_PER_KM) ));
  
  return (uint32_t)round(rangeEstimate);
}

void lvAlarm(void)
{
  if (LV_ALARM == 0.0)
    return;
    
  if (cv <= LV_ALARM || cv > 4250) // added high V alarm: eg. 4250mV
  {
    _menu.hardReset();
    
    wdt_reset();
    
    uint8_t i;

    uint8_t disabledA = 0;
    uint8_t disabledB = 0;
      
    for(i=0; i<80; i++)
    {
      wdt_reset();
      
      if (i%2 == 0 && cv <= LV_ALARM) // lv: alternating tone; hv: same tone
        justBuzz(40, F3);
      else
        justBuzz(40, F4);
      
      delay(40);

      if(i%10 == 0)
      {
        if (remote.getDuty() < _menu.getCenter() - _menu.getMaxLevel())
          disabledA = 1;
        if (remote.getDuty() > _menu.getCenter() + _menu.getMaxLevel())
          disabledB = 1;
      }

      if (disabledA && disabledB)
      {
        LV_ALARM = 0.0;
        loopBuzz(80, F1, 5, 80); wdt_reset(); loopBuzz(80, F2, 5, 80); delay(800);
        return;
      }
    }
  }
}




