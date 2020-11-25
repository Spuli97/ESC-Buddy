// IO functions for the user

void beepOutInt (uint32_t number, uint8_t fixedDigits, uint8_t indZero) // resets WDT internally all <1s
{
    // process number

    uint16_t arr[11] = {0};
    int16_t i;

    for (i=10; i>=0; i--)
    {
        arr[i] = number % 10;
        number /= 10;
    }

    //now beep
    piezo.setFreq(DIGIT_FREQ);

    // skip leading zeros
    if (fixedDigits == 0)
    {
      for (i=0; i<11; i++) 
      {
        if (arr[i] != 0)
          break;
      }

      if (i == 11 && indZero) // number is 0
      {
        //sound of the decimal point
        piezo.setTime(3);
        piezo.setFreq(F2);
        //delay(20);
        piezo.frameUpdate();
        piezo.setFreq(F3);
        delay(40);
        piezo.frameUpdate();
        piezo.setFreq(F4);
        delay(80);
        piezo.frameUpdate();
        piezo.setFreq(F5);
        delay(40);
        piezo.frameUpdate();
        piezo.setFreq(DIGIT_FREQ);
      }
    }
    else
    {
      i = 11 - fixedDigits;
    }

    //beep out all digits
    for (; i<11; i++)
    {
        uint8_t j;

        if (arr[i] != 0)
        {
          // non zero, beep the digits number of times
          for(j=0; j<arr[i]; j++)
          {
              piezo.setTime(1);
              piezo.frameUpdate();
              delay(DIGIT);
              piezo.frameUpdate();
              delay(DIGIT_DELAY);
          }
        }
        else // this tone for zero
        {
          piezo.setTime(1);
          piezo.setFreq(ZERO_FREQ);
          piezo.frameUpdate();
          delay(ZERO_DIGIT);
          piezo.setFreq(DIGIT_FREQ);
          piezo.frameUpdate();
        }
        delay(BETWEEN_DIGIT);
        wdt_reset();
    }
}

void beepOutFloat (float number, uint8_t decimals) // resets WDT internally all <1s
{
  // interpret as 2 ints, before and after decimal point
  uint32_t num;
  uint32_t dec;

  num = ((uint32_t)number);

  // bugfix:
  float temp = number - round(number);
  if (abs(temp) < 0.06)
  {
    beepOutInt((uint32_t)round(number), 0, 0);
    //sound of the decimal point
    piezo.setTime(3);
    piezo.setFreq(F2);
    //delay(20);
    piezo.frameUpdate();
    piezo.setFreq(F3);
    delay(40);
    piezo.frameUpdate();
    piezo.setFreq(F4);
    delay(80);
    piezo.frameUpdate();
    piezo.setFreq(F5);
    delay(40);
    piezo.frameUpdate();
    piezo.setFreq(DIGIT_FREQ);
    delay(BETWEEN_DIGIT);
    beepOutInt(0, decimals, 0);

    return;
  }
  
  number -= float(num);

  char str[6];
  dtostrf (number, 5, 3, str);
  str[1] = '0';
  dec = atoi(str);

  if (decimals == 2)
    dec /= 10;
  else if (decimals == 1)
    dec /= 100;

  // inaccurrate!!
  //number *= (float)pow((float)10, (float)decimals);
  //dec = (uint32_t)number;

  beepOutInt(num, 0, 0);

  //sound of the decimal point
  piezo.setTime(3);
  piezo.setFreq(F2);
  //delay(20);
  piezo.frameUpdate();
  piezo.setFreq(F3);
  delay(40);
  piezo.frameUpdate();
  piezo.setFreq(F4);
  delay(80);
  piezo.frameUpdate();
  piezo.setFreq(F5);
  delay(40);
  piezo.frameUpdate();
  piezo.setFreq(DIGIT_FREQ);

  delay(BETWEEN_DIGIT);
  beepOutInt(dec, decimals, 0);
}

void menuSounds(uint8_t menuAction)
{
  switch(menuAction)
  {
  case 0: // not triggered
   break;
  case 1: // triggered 1 time
    piezo.setTime(1);
    piezo.setFreq(F1);
    break;
  case 2: // triggered 2 times
    piezo.setTime(1);
    piezo.setFreq(F2);
    break;
  case 3: // triggered 3 times
    piezo.setTime(1);
    piezo.setFreq(F3);
    break;
  case 4: // triggered 4 times
    piezo.setTime(1);
    piezo.setFreq(F4);
    break;
  case 5: case 12: // reversed or overflow reset
    piezo.setTime(3);
    piezo.setFreq(F2);
    //delay(20); // not needed, allready time waited at the end of the last frame
    piezo.frameUpdate();
    piezo.setFreq(F3);
    delay(20);
    piezo.frameUpdate();
    piezo.setFreq(F4);
    delay(20);
    piezo.frameUpdate();
    piezo.setFreq(F5);
    delay(20);
    piezo.frameUpdate();
    break;
  }
}

void resetSound(void)
{
  piezo.frameUpdate();
  piezo.setTime(3);
  piezo.setFreq(F5);
  delay(20);
  piezo.frameUpdate();
  piezo.setFreq(F4);
  delay(60);
  piezo.frameUpdate();
  piezo.setFreq(F3);
  delay(200);
  piezo.frameUpdate();
  piezo.setFreq(F2);
  delay(80);
  piezo.frameUpdate();
}

void vescCommErrLoop(void)
{
  // just escalate
  cli();
  wdt_reset();
  WDTCSR = (1<<WDCE)|(1<<WDE);
  wdt_disable();
  sei();
  
  piezo.setChannel(BOTH);

  piezo.frameUpdate();
  while(1) // beep until manual reset // VESC COMMUNICATION ERROR TONE
  {
    loopBuzz(200, F2, 2, 200); // -- -- -- --
    delay(500);
  }
}

void waitingForReboot(void)
{
  cli();
  wdt_reset();
  WDTCSR = (1<<WDCE)|(1<<WDE);
  wdt_disable();
  sei();

  piezo.frameUpdate();
  while(1) // beep ubtil manual reset // VESC COMMUNICATION ERROR TONE
  {
    piezo.setTime(1);
    piezo.setFreq(F0);
    piezo.frameUpdate();
    delay(100);
    piezo.frameUpdate();
    delay(800);
  }
}

void justBuzz(uint16_t t, uint8_t f)
{
  piezo.setTime(1);
  piezo.setFreq(f);
  piezo.frameUpdate();
  delay(t);
  piezo.frameUpdate();
}

void loopBuzz(uint16_t t, uint8_t f, uint16_t times, uint16_t pause)
{
  piezo.frameUpdate();
  piezo.setFreq(f);
  
  uint16_t i;
  for (i=0; i<times; i++)
  {
    piezo.setTime(1);
    piezo.frameUpdate();
    delay(t);
    piezo.frameUpdate();
    delay(pause);
  }
}

void noDataSound(void)
{
  delay(800); loopBuzz(50, F3, 5, 100); wdt_reset(); loopBuzz(50, F2, 5, 100); delay(800); // ----- pitch down -----
}

