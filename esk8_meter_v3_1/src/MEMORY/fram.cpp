#include "Arduino.h"
#include "fram.h"
#include "Wire.h"

FRAM4_ST::FRAM4_ST()
{
    // constructor
}

uint8_t FRAM4_ST::init(uint8_t _framAddr)
{
    framAddr = _framAddr;

    Wire.begin(framAddr);
    Wire.beginTransmission(framAddr);

    if (Wire.endTransmission() != 0 || sizeByte == 0)
    {
        return ERROR;
    }

    return OK;
}

void FRAM4_ST::setPtrAddr(uint16_t _ptrAddr)
{
    ptrAddr = _ptrAddr;
}

uint16_t FRAM4_ST::getPtrAddr(void)
{
    return ptrAddr;
}

uint8_t FRAM4_ST::appendByte(uint8_t _data)
{
    Wire.beginTransmission(framAddr);
    //Wire.write(ptrAddr >> 8);
    Wire.write(ptrAddr);

    ptrAddr ++;

    Wire.write(_data);

    if (Wire.endTransmission() != 0)
    {
        return ERROR;
    }

    return OK;
}

uint8_t FRAM4_ST::appendBuff(uint8_t* _data, uint8_t _len)
{
    Wire.beginTransmission(framAddr);
    //Wire.write(ptrAddr >> 8);
    Wire.write(ptrAddr);

    uint8_t i;
    for (i=0; i<_len; i++)
    {
        Wire.write(_data[i]);
        ptrAddr++;
    }

    if (Wire.endTransmission() != 0)
    {
        return ERROR;
    }

    return OK;
}

uint8_t FRAM4_ST::readByte(void)
{
    Wire.beginTransmission(framAddr);
    //Wire.write(ptrAddr >> 8);
    Wire.write(ptrAddr);

    ptrAddr ++;

    Wire.endTransmission();

    Wire.requestFrom((int)framAddr, (int)1);

    long waitBegin = millis();
    while(Wire.available() != 1)
    {
        if (millis() > waitBegin + TIMEOUT)
            break;
    }

    return (Wire.read());
}

void FRAM4_ST::readBuff(uint8_t* _data, uint8_t _len)
{
    Wire.beginTransmission(framAddr);
    //Wire.write(ptrAddr >> 8);
    Wire.write(ptrAddr);

    Wire.endTransmission();

    Wire.requestFrom((int)framAddr, (int)_len);

    long waitBegin = millis();
    while(Wire.available() != _len)
    {
        if (millis() > waitBegin + TIMEOUT)
            break;
    }

    uint8_t i;
    for (i=0; i<_len; i++)
    {
        _data[i] = Wire.read();
        ptrAddr ++;
    }
}

// ------------------------------------

uint8_t FRAM4_ST::deleteByte(void)
{
    return appendByte(0);
}

uint8_t FRAM4_ST::deleteAll(void)
{
    uint16_t tempPtrAddr = ptrAddr;
    ptrAddr = 0;

    uint16_t i;
    for (i=0; i<sizeByte; i++)
    {
        if (appendByte(0) == ERROR)
            return ERROR;
    }

    ptrAddr = tempPtrAddr;

    return OK;
}

// ------------------------------------

uint8_t FRAM4_ST::append8Bit(uint8_t _data)
{
    return appendByte(_data);
}

uint8_t FRAM4_ST::append16Bit(uint16_t _data)
{
    // xxxxxxxx xxxxxxxx  <--inputData
    // --by.1-- --by.0--
    //0--by.0-- --by.1--E  <--bytes in FRAM
    uint8_t splitData[2];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);

    return appendBuff(splitData, 2);
}

uint8_t FRAM4_ST::append32Bit(uint32_t _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in FRAM
    uint8_t splitData[4];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);
    splitData[2] = (uint8_t)(_data >> 16);
    splitData[3] = (uint8_t)(_data >> 24);

    return appendBuff(splitData, 4);
}

uint8_t FRAM4_ST::append64Bit(uint64_t _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in FRAM
    uint8_t splitData[8];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);
    splitData[2] = (uint8_t)(_data >> 16);
    splitData[3] = (uint8_t)(_data >> 24);
    splitData[4] = (uint8_t)(_data >> 32);
    splitData[5] = (uint8_t)(_data >> 40);
    splitData[6] = (uint8_t)(_data >> 48);
    splitData[7] = (uint8_t)(_data >> 56);

    return appendBuff(splitData, 8);
}

uint8_t FRAM4_ST::appendFloat32(float _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in FRAM
    uint32_t bin_data = *(uint32_t *)&_data;

    return append32Bit(bin_data);
}

// ------------------------------------

uint8_t FRAM4_ST::read8Bit(void)
{
    return readByte();
}

uint16_t FRAM4_ST::read16Bit(void)
{
    // xxxxxxxx xxxxxxxx  <--inputData
    // --by.1-- --by.0--
    //0--by.0-- --by.1--E  <--bytes in FRAM
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

uint32_t FRAM4_ST::read32Bit(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in FRAM
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

uint64_t FRAM4_ST::read64Bit(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in FRAM
    uint8_t splitData[8];
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

float FRAM4_ST::readFloat32(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in FRAM
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




