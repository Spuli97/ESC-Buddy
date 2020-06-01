#include "Arduino.h"
#include "eeprom.h"

EEPROM_ST::EEPROM_ST()
{
     // constructor
}

void EEPROM_ST::eepromWrite(uint16_t ucAddress, unsigned char ucData)
{
    if (eepromRead(ucAddress) == ucData)
        return;

    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));
    /* Set Programming mode */
    EECR = (0<<EEPM1)|(0>>EEPM0);
    /* Set up address and data registers */
    EEAR = ucAddress;
    EEDR = ucData;
    /* Write logical one to EEMPE */
    EECR |= (1<<EEMPE);
    /* Start eeprom write by setting EEPE */
    EECR |= (1<<EEPE);
}

unsigned char EEPROM_ST::eepromRead(uint16_t ucAddress)
{
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));
    /* Set up address register */
    EEAR = ucAddress;
    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);
    /* Return data from data register */
    return EEDR;
}

void EEPROM_ST::init(uint16_t _sizeByte)
{
    sizeByte = _sizeByte;
}

void EEPROM_ST::setPtrAddr(uint16_t _ptrAddr)
{
    ptrAddr = _ptrAddr;
}

uint16_t EEPROM_ST::getPtrAddr(void)
{
    return ptrAddr;
}

void EEPROM_ST::appendByte(uint8_t _data)
{
    eepromWrite(ptrAddr, (unsigned char)_data);
    ptrAddr ++;
}

void EEPROM_ST::appendBuff(uint8_t* _data, uint8_t _len) // _len max. = 255
{
    uint8_t i;
    for(i=0; i<_len; i++)
    {
        appendByte(_data[i]);
    }
}

uint8_t EEPROM_ST::readByte(void)
{
    ptrAddr ++;
    return eepromRead(ptrAddr-1);
}

void EEPROM_ST::readBuff(uint8_t* _data, uint8_t _len) // _len max. = 255
{
    uint8_t i;
    for(i=0; i<_len; i++)
    {
        _data[i] = readByte();
    }
}

// ------------------------------------

void EEPROM_ST::deleteByte(void)
{
    appendByte(0);
}

void EEPROM_ST::deleteAll(void)
{
    uint16_t tempPtrAddr = ptrAddr;
    ptrAddr = 0;

    uint16_t i;
    for (i=0; i<sizeByte; i++)
    {
        appendByte(0);
    }

    ptrAddr = tempPtrAddr;
}

// ------------------------------------

void EEPROM_ST::append8Bit(uint8_t _data)
{
    appendByte(_data);
}

void EEPROM_ST::append16Bit(uint16_t _data)
{
    // xxxxxxxx xxxxxxxx  <--inputData
    // --by.1-- --by.0--
    //0--by.0-- --by.1--E  <--bytes in EEPROM
    uint8_t splitData[2];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);

    appendBuff(splitData, 2);
}

void EEPROM_ST::append32Bit(uint32_t _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in EEPROM
    uint8_t splitData[4];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);
    splitData[2] = (uint8_t)(_data >> 16);
    splitData[3] = (uint8_t)(_data >> 24);

    appendBuff(splitData, 4);
}

void EEPROM_ST::append64Bit(uint64_t _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in EEPROM
    uint8_t splitData[8];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);
    splitData[2] = (uint8_t)(_data >> 16);
    splitData[3] = (uint8_t)(_data >> 24);
    splitData[4] = (uint8_t)(_data >> 32);
    splitData[5] = (uint8_t)(_data >> 40);
    splitData[6] = (uint8_t)(_data >> 48);
    splitData[7] = (uint8_t)(_data >> 56);

    appendBuff(splitData, 8);
}

void EEPROM_ST::appendFloat32(float _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in EEPROM
    uint32_t bin_data = *(uint32_t *)&_data;

    append32Bit(bin_data);
}

void EEPROM_ST::appendDouble64(double _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in EEPROM
    uint64_t bin_data = *(uint64_t *)&_data;

    append64Bit(bin_data);
}

// ------------------------------------

uint8_t EEPROM_ST::read8Bit(void)
{
    return readByte();
}

uint16_t EEPROM_ST::read16Bit(void)
{
    // xxxxxxxx xxxxxxxx  <--inputData
    // --by.1-- --by.0--
    //0--by.0-- --by.1--E  <--bytes in EEPROM
    uint8_t splitData[2];
    uint16_t _data = 0;

    readBuff(splitData, 2);

    uint8_t i;
    for(i=0; i<2; i++)
    {
        uint16_t temp = 0;
        temp = splitData[i];
        temp = temp << (i*8);
        _data += temp;
    }

    return _data;
}

uint32_t EEPROM_ST::read32Bit(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in EEPROM
    uint8_t splitData[4];
    uint32_t _data = 0;

    readBuff(splitData, 4);

    uint8_t i;
    for(i=0; i<4; i++)
    {
        uint32_t temp = 0;
        temp = splitData[i];
        temp = temp << (i*8);
        _data += temp;
    }

    return _data;
}

uint64_t EEPROM_ST::read64Bit(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in EEPROM
    uint8_t splitData[8]; //brutaler bug
    uint64_t _data = 0;

    readBuff(splitData, 8);

    uint8_t i;
    for(i=0; i<8; i++)
    {
        uint64_t temp = 0;
        temp = splitData[i];
        temp = temp << (i*8);
        _data += temp;
    }

    return _data;
}

float EEPROM_ST::readFloat32(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in EEPROM
    uint8_t splitData[4];
    uint32_t _data = 0;

    readBuff(splitData, 4);

    uint8_t i;
    for(i=0; i<4; i++)
    {
        uint32_t temp = 0;
        temp = splitData[i];
        temp = temp << (i*8);
        _data += temp;
    }

    float data;
    data = *(float *)&_data;

    return data;
}


