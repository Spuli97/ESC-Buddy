// memory related functions

uint8_t readFRAM(uint8_t cluster)
{
  _fram.setPtrAddr(cluster*FRAM_DATA_LEN*2);
  
  erpmTrip = _fram.read32Bit();
  erpmAbs  = _fram.read64Bit();
  WhDAbs   = _fram.read32Bit();
  WhCAbs   = _fram.read32Bit();
  dTime    = _fram.read32Bit();
  WhDTrip  = _fram.readFloat32();
  WhCTrip  = _fram.readFloat32();
  spdTop   = _fram.read32Bit();
  powCycs  = _fram.read32Bit();
  
  uint8_t checker = 1;
  
  if (erpmTrip != _fram.read32Bit())
    checker = 0;
  
  if (erpmAbs != _fram.read64Bit())
    checker = 0;
  
  if (WhDAbs != _fram.read32Bit())
    checker = 0;
  
  if (WhCAbs != _fram.read32Bit())
    checker = 0;
  
  if (dTime != _fram.read32Bit())
    checker = 0;
  
  if (WhDTrip != _fram.readFloat32())
    checker = 0;
  
  if (WhCTrip != _fram.readFloat32())
    checker = 0;
  
  if (spdTop != _fram.read32Bit())
    checker = 0;
  
  if (powCycs != _fram.read32Bit())
    checker = 0;
  
  if (checker == 0)
  {
    uint8_t ct;
    for (ct=0; ct<5; ct++)
    {
      loopBuzz(300, F3, cluster+1, 200);
      delay(600);
      wdt_reset();
    }
  }
  
  return checker;
}

void writeFRAM(uint8_t cluster) // max. 18ms
{
  if (cluster)
    _fram.setPtrAddr(cluster*FRAM_DATA_LEN*2);
  else
    _fram.setPtrAddr(0);
   
  uint8_t i;
  for (i=0; i<2; i++)
  {
    // do something with the return value (error code)?
    _fram.append32Bit(dt);
    _fram.append64Bit(da);
    _fram.append32Bit(wda);
    _fram.append32Bit(wca);
    _fram.append32Bit(tim);
    _fram.appendFloat32(wdt);
    _fram.appendFloat32(wct);
    _fram.append32Bit(spdTop);  // spt
    _fram.append32Bit(powCycs); // pwc
  }
}



