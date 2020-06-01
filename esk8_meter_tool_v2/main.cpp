#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <conio.h>

class BUFF_ST
{
  public:
    BUFF_ST();
    uint8_t init(unsigned char* _buffer, uint16_t _sizeByte);
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
    double readDouble64(void);

    void deleteByte(void);
    void deleteAll(void); // this can take some time

  private:
    uint16_t ptrAddr  = 0;
    uint16_t sizeByte = 0;

    unsigned char* buffer;

    void eepromWrite(uint16_t ucAddress, unsigned char ucData);
    unsigned char eepromRead(uint16_t ucAddress);

    void appendByte(uint8_t _data);
    void appendBuff(uint8_t* _data, uint8_t _len); // _len max. = 255
    uint8_t readByte(void);
    void readBuff(uint8_t* _data, uint8_t _len); // _len max. = 255
};

BUFF_ST::BUFF_ST()
{
     // constructor
}

void BUFF_ST::eepromWrite(uint16_t ucAddress, unsigned char ucData)
{
   buffer[ucAddress] = ucData;
}

unsigned char BUFF_ST::eepromRead(uint16_t ucAddress)
{
    return (unsigned char)buffer[ucAddress];
}

uint8_t BUFF_ST::init(unsigned char* _buffer, uint16_t _sizeByte)
{
    buffer = _buffer;
    sizeByte = _sizeByte;
}

void BUFF_ST::setPtrAddr(uint16_t _ptrAddr)
{
    ptrAddr = _ptrAddr;
}

uint16_t BUFF_ST::getPtrAddr(void)
{
    return ptrAddr;
}

void BUFF_ST::appendByte(uint8_t _data)
{
    eepromWrite(ptrAddr, (unsigned char)_data);
    ptrAddr ++;
}

void BUFF_ST::appendBuff(uint8_t* _data, uint8_t _len) // _len max. = 255
{
    uint8_t i;
    for(i=0; i<_len; i++)
    {
        appendByte(_data[i]);
    }
}

uint8_t BUFF_ST::readByte(void)
{
    ptrAddr ++;
    return eepromRead(ptrAddr-1);
}

void BUFF_ST::readBuff(uint8_t* _data, uint8_t _len) // _len max. = 255
{
    uint8_t i;
    for(i=0; i<_len; i++)
    {
        _data[i] = readByte();
    }
}

// ------------------------------------

void BUFF_ST::deleteByte(void)
{
    appendByte(0);
}

void BUFF_ST::deleteAll(void)
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

void BUFF_ST::append8Bit(uint8_t _data)
{
    appendByte(_data);
}

void BUFF_ST::append16Bit(uint16_t _data)
{
    // xxxxxxxx xxxxxxxx  <--inputData
    // --by.1-- --by.0--
    //0--by.0-- --by.1--E  <--bytes in EEPROM
    uint8_t splitData[2];
    splitData[0] = (uint8_t)_data;
    splitData[1] = (uint8_t)(_data >> 8);

    appendBuff(splitData, 2);
}

void BUFF_ST::append32Bit(uint32_t _data)
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

void BUFF_ST::append64Bit(uint64_t _data)
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

void BUFF_ST::appendFloat32(float _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3--E  <--bytes in EEPROM
    uint32_t bin_data = *(uint32_t *)&_data;

    append32Bit(bin_data);
}

void BUFF_ST::appendDouble64(double _data)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in EEPROM
    uint64_t bin_data = *(uint64_t *)&_data;

    append64Bit(bin_data);
}

// ------------------------------------

uint8_t BUFF_ST::read8Bit(void)
{
    return readByte();
}

uint16_t BUFF_ST::read16Bit(void)
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

uint32_t BUFF_ST::read32Bit(void)
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

uint64_t BUFF_ST::read64Bit(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in EEPROM
    uint8_t splitData[2];
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

float BUFF_ST::readFloat32(void)
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

double BUFF_ST::readDouble64(void)
{
    // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
    // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--
    //0--by.0-- --by.1-- --by.2-- --by.3-- --by.4-- --by.5-- --by.6-- --by.7--E  <--bytes in EEPROM
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

    double data;
    data = *(double *)&_data;

    return data;
}


#define MAXCHAR 1024

void printEEPROM(unsigned char buffer[], int len);
void printEEPROMline(unsigned char buffer[], int len, int line);
uint8_t getByte(uint64_t inputData, uint8_t byteNum);
uint8_t ask(char zero, char one);
double getERPM_per_KMPH(double diameter_mm, double ERPM_per_RPM);
double getTAC_per_KM(double diameter_mm, double TAC_per_REV);
double corectionStep(double realD, double odoD, float curSPD);

void read(void);
void save(void);
void show(void);

unsigned char buffer[MAXCHAR];
BUFF_ST _file;

uint16_t middle = 0;
uint16_t maxed = 0;
uint8_t reverse = 0;
float tac = 0.0;
float spd = 0.0;
uint8_t setup = 0;
uint16_t cells = 0;
uint16_t capacity = 0;
uint16_t lvAlarm = 0;

double diameter = 0.0;
double erpm_per_rpm = 0.0;
double tac_steps = 0.0;

double correctDist_km = 0.0;
double actualDist_km = 0.0;

void read(void)
{
    /// EEPROM:
    /// 0 config reboot
    /// 1-4 PPM ranges (middle, maxed)
    /// 5   reversed
    /// 6-13 calculation values (TAC_PER_KM_tmep, ERPM_PER_KMPH_tmep)
    /// 14 cell count
    /// 15-16 capacity
    /// 17-18 lv alarm

    _file.setPtrAddr(0);

    setup = _file.read8Bit();
    middle = _file.read16Bit();
    maxed = _file.read16Bit();
    reverse = _file.read8Bit();
    tac = _file.readFloat32();
    spd = _file.readFloat32();
    cells = _file.read8Bit();
    capacity = _file.read16Bit();
    lvAlarm = _file.read16Bit();
}

void save(void)
{
    /// EEPROM:
    /// 0 config reboot
    /// 1-4 PPM ranges (middle, maxed)
    /// 5   reversed
    /// 6-13 calculation values (TAC_PER_KM_tmep, ERPM_PER_KMPH_tmep)
    /// 14 cell count
    /// 15-16 capacity
    /// 17-18 lv alarm

    _file.setPtrAddr(0);

    _file.append8Bit(setup);
    _file.append16Bit(middle);
    _file.append16Bit(maxed);
    _file.append8Bit(reverse);
    _file.appendFloat32(tac);
    _file.appendFloat32(spd);
    _file.append8Bit(cells);
    _file.append16Bit(capacity);
    _file.append16Bit(lvAlarm);
}

void show(void)
{
    printf("---> read RAW data:");
    printEEPROMline(buffer, MAXCHAR, 0);
    printEEPROMline(buffer, MAXCHAR, 16);

    printEEPROM(buffer, MAXCHAR);
    printf("\n\n");

    printf("Showing your current data, please check:\n");
    if (setup == 0)
    {
        printf("----------PPM----------\ncenter position: %04d\nmax. throw     : %04d\n", middle, maxed);
    }
    else
    {
         printf("----------PPM----------\nsetup on next reboot.\n");
    }
    printf("reversed        :");
    if (reverse)
        printf(" YES\n");
    else
        printf("  NO\n");

    printf("\n");
    printf("----------ODO----------\n");
    printf("ERPM_per_KMPH : %10.8f\n", spd);
    printf("TACOM._per_KM : %10.8f\n", tac);

    printf("\n");
    printf("----------BAT----------\n");
    printf("capacity         (Wh): %4d\n", capacity);
    printf("low cell voltage (mV): %4d\n", lvAlarm);
    printf("cell count           : %4d\n", cells);
}


uint8_t getByte(uint64_t inputData, uint8_t byteNum)
{
  // xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  <--inputData
  // --by.7-- --by.6-- --by.5-- --by.4-- --by.3-- --by.2-- --by.1-- --by.0--

  inputData  = inputData >> (8*byteNum);
  return((uint8_t)inputData);
}

void printEEPROM(unsigned char buffer[], int len)
{
    int i;
    for(i = 0; i<len; i++)
    {
        if (i%16 == 0)
        {
            printf("\n B%04d| ", i);

        }

        int n = buffer[i];
        int j;
        for(j=0; j<8; j++)
        {
            if (n & 128)
                printf("1");
            else
                printf("0");

            n <<= 1;
        }
        printf(" ");
    }
}

void printEEPROMline(unsigned char buffer[], int len, int line)
{
    int i;
    for(i = 0; i<len; i++)
    {
        if (i>=line && i<line+16)
        {
            if (i%16 == 0)
            {
                printf("\n B%04d| ", i);

            }

            int n = buffer[i];
            int j;
            for(j=0; j<8; j++)
            {
                if (n & 128)
                    printf("1");
                else
                    printf("0");

                n <<= 1;
            }
            printf(" ");
        }
    }
}

uint8_t ask(char zero, char one)
{
    char ch = getch();
    while(ch != zero && ch != zero-32 && ch != one && ch != one-32)
        ch = getch();
    printf("%c", ch);
    if (ch == one || ch == one-32)
        return 1;

    return 0;
}

double getERPM_per_KMPH(double diameter_mm, double ERPM_per_RPM)
{
    return ( (25.0)/(3.0*3.14159265359*((diameter_mm/2.0)/1000.0)) )*ERPM_per_RPM;
}

double getTAC_per_KM(double diameter_mm, double TAC_per_REV)
{
    return (  1.0/( (diameter_mm/1000000.0) *3.14159265359) )*TAC_per_REV;
}

double corectionStep(double realD, double odoD, float curSPD)
{
    return ( (double)curSPD )*( odoD / realD );
}


int main()
{
    /// get data --------------------------------------------------------------------
    // this checks if system is compatible
    if (sizeof(float) != 4 && CHAR_BIT == 8)
        return 1;

    FILE *ptr;
    ptr = fopen("read_data", "rb");

    if (NULL == fread(buffer, sizeof(buffer), 1, ptr))
    {
        printf("There is no file named 'read_data' in this folder.\n");
        printf("\nPress 'q' or 'e' to quit!");
        ask('q', 'e');
        return 2;
    }

    _file.init(buffer, MAXCHAR);

    read();

    /// show data --------------------------------------------------------------------

    show();

    /// GUI --------------------------------------------------------------------

    printf("Do you want to calibrate your remote's PPM ranges on next startup?\ny/n: ");
   if(ask('n', 'y'))
    {
        setup = 1;
        printf("\n");
    }
    else
    {
        setup = 0;
        printf("\n");
        printf("Do you want to keep your old settings for the PPM ranges (k), or do you want to enter one now (e)?\nIt is best to keep your old settings, or answer question 1 with YES, except you really know what you are doing!\nk/e: ");
        if (ask('k', 'e'))
        {
            printf("\n");
            printf("Please enter your remotes middle position PPM pulse length in uS without decimal digits. Confirm with ENTER (not SPACE):\n");
            scanf("%d", &middle);
            printf("Please enter your remotes maxed out position PPM pulse length in uS without decimal digits.\n You can also use the minimal position. It is advised to use either one that is the least distance from your middle position.\nConfirm with ENTER (not SPACE):\n");
            scanf("%d", &maxed);
        }
        else
        {
            printf("\n");
        }
    }

    printf("Do you want to reverse your remote input?\ny/n: ");
    if(ask('n', 'y'))
        reverse = 1;
    else
        reverse = 0;
    printf("\n");

    printf("Have you already entered wheel diameter, motor pole count and tachometer step values, but the odometer is inaccurate?\ny/n: ");
    if(ask('n', 'y'))
    {
        printf("\n");
        printf("Please enter the distance in km you actually traveled (GPS), use the '.' as decimal point. Confirm with ENTER (not SPACE):\n");
        scanf("%lf", &correctDist_km);
        printf("Please enter the distance in km the odometer told you, use the '.' as decimal point. Confirm with ENTER (not SPACE):\n");
        scanf("%lf", &actualDist_km);
        tac = (float)corectionStep(correctDist_km, actualDist_km, tac);
    }
    else
    {
        printf("\n");
        printf("Do you want to set new wheel diameter, motor pole count or tachometer step values?\ny/n: ");
        if(ask('n', 'y'))
        {
            printf("\n");
            printf("Please enter your wheel diameter in mm. you can enter decimal digits, use the '.' as decimal point. Confirm with ENTER (not SPACE):\n");
            scanf("%lf", &diameter);
            printf("Please enter your motor pole-pair count (ERPM per RPM value). Confirm with ENTER (not SPACE):\n");
            scanf("%lf", &erpm_per_rpm);
            spd = (float)getERPM_per_KMPH(diameter, erpm_per_rpm);
            printf("Please enter your VESC's tachometer steps per motor revolution. Confirm with ENTER (not SPACE):\n");
            scanf("%lf", &tac_steps);
            tac = (float)getTAC_per_KM(diameter, tac_steps);
        }
    }
    printf("\n");
    printf("How many cells (s) does your battery have? Confirm with ENTER (not SPACE):\n");
    scanf("%d", &cells);

    printf("\n");
    printf("What is the capacity of your battery (Wh)? Confirm with ENTER (not SPACE):\n");
    scanf("%d", &capacity);

    printf("\n");
    printf("At what cell voltage do you want to low voltage alarm to trigger (mV)? Confirm with ENTER (not SPACE):\n");
    scanf("%d", &lvAlarm);

     /// show data --------------------------------------------------------------------

    show();

    /// write data --------------------------------------------------------------------

    save();

    FILE *write_ptr;
    write_ptr = fopen("output_file", "wb");  // w for write, b for binary
    fwrite(buffer, sizeof(buffer), 1, write_ptr); // write 10 bytes from our buffer

    printf("\n\n---> wrote RAW data:");
    printEEPROMline(buffer, MAXCHAR, 0);
    printEEPROMline(buffer, MAXCHAR, 16);

    printf("\nFile 'output_data' generated/ updated!\nPress 'q' or 'e' to quit!");
    ask('q', 'e');


    return 0;
}

