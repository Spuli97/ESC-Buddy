#include "Arduino.h"

#define TIMEOUT 100 //ms

/// all data has to be double checked

/// PtrAddr will advance automatically the appropriate number of bytes depending on the performed read/ write operation
/// deleteByte will also advance PtrAddr, deleteAll will not change PtrAddr

/// user is responsible for keeping 0 >= PtrAddr < PtrAddr

class EEPROM_ST
{
  public:
    EEPROM_ST();
    void init(uint16_t _sizeByte);
    void setPtrAddr(uint16_t _ptrAddr);
    uint16_t getPtrAddr(void);

    void append8Bit(uint8_t _data);
    void append16Bit(uint16_t _data);
    void append32Bit(uint32_t _data);
    void append64Bit(uint64_t _data);
    void appendFloat32(float _data);
    void appendDouble64(double _data);

    uint8_t read8Bit(void);
    uint16_t read16Bit(void);
    uint32_t read32Bit(void);
    uint64_t read64Bit(void);
    float readFloat32(void);

    void deleteByte(void);
    void deleteAll(void); // this can take some time

  private:
    uint16_t ptrAddr  = 0;
    uint16_t sizeByte = 0;

    void eepromWrite(uint16_t ucAddress, unsigned char ucData);
    unsigned char eepromRead(uint16_t ucAddress);

    void appendByte(uint8_t _data);
    void appendBuff(uint8_t* _data, uint8_t _len); // _len max. = 255
    uint8_t readByte(void);
    void readBuff(uint8_t* _data, uint8_t _len); // _len max. = 255
};
