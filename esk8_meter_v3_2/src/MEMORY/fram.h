#include "Arduino.h"

#define ERROR 1
#define OK    0

#define TIMEOUT 100 //ms

/// all write functions return 0 (OK) on success; 1 (ERROR) on failure in regard to I2C, data has to be double checked
/// all read functions return data that has to double checked!

/// PtrAddr will advance automatically the appropriate number of bytes depending on the performed read/ write operation
/// deleteByte will also advance PtrAddr, deleteAll will not change PtrAddr
/// PtrAddr will also advance if write terminates with ERROR

/// user is responsible for keeping 0 >= PtrAddr < PtrAddr

class FRAM4_ST
{
  public:
    FRAM4_ST();
    uint8_t init(uint8_t _framAddr);
    void setPtrAddr(uint16_t _ptrAddr);
    uint16_t getPtrAddr(void);

    uint8_t append8Bit(uint8_t _data);   // 0.32ms
    uint8_t append16Bit(uint16_t _data); // 0.42ms
    uint8_t append32Bit(uint32_t _data); // 0.62ms
    uint8_t append64Bit(uint64_t _data); // 1.2ms 
    uint8_t appendFloat32(float _data);  //~0.62ms

    uint8_t read8Bit(void);   // 0.48ms
    uint16_t read16Bit(void); // 0.58ms
    uint32_t read32Bit(void); // 0.8ms
    uint64_t read64Bit(void); // 1.2ms
    float readFloat32(void);  // 0.8ms

    uint8_t deleteByte(void);
    uint8_t deleteAll(void); // this can take some time

  private:
    uint8_t  framAddr = 0;
    uint16_t ptrAddr  = 0;
    const uint8_t sizeByte = 255;

    uint8_t appendByte(uint8_t _data);
    uint8_t appendBuff(uint8_t* _data, uint8_t _len); // _len max. = 29
    uint8_t readByte(void);
    void readBuff(uint8_t* _data, uint8_t _len); // _len max. = 29
};
