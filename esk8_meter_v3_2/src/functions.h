#include <Arduino.h>

/// FLAGS for Menu
#define REQUEST B10000000 /// numbers from 0-11 0==NONE, 1-4 left, 5-8 right, 9 maxL, 10 maxR, 11 specialState, 12 overflow reset
#define TRIGGER B01000000 /// numbers from 0- 5 0==NONE, 1-3, 5==DIRECTION_CHANGE Left and Right indifferent
#define REQUEST_PRESERVE B00100000
#define TRIGGER_PRESERVE B00010000

///frequencies for Buzzer
#define F0 B00000101 //61Hz
#define F1 B00000100 //244Hz
  #define F2 B10000011 //488Hz /9b
#define F3 B00000011 //976Hz
    #define F4 B11000010 //1953 10b
#define F5 B00000010 //7812Hz
///internal and external buzzer definitions
#define INTERNAL B00000001
#define EXTERNAL B00000010
#define BOTH     B00000011
#define NONE     B00000000

#define SPEC_LATCH 180 //200 - 80


class Buzzer
{
public:
    Buzzer(void);
    void setFreq(uint8_t newFreq);
    void setTime(uint8_t newTime);
    void setDutyC(uint8_t newDuty);
    void init(void);
    void frameUpdate(void);
    void setChannel(uint8_t _channel);

private:
    uint8_t freq;
    uint8_t time;
    uint8_t channels;
};

class PPM
{
public:
    PPM(void);
    void init(uint8_t ppmPin);
    uint16_t getDuty(void);

private:
    uint8_t pin;
};

class Menu
{
public:
    Menu(void);
    void init(uint16_t senseCenter, uint16_t senseMax, uint8_t senseLR);
    void doTheMenu(uint16_t duty);
    uint8_t getMenuAction(uint8_t type);
    uint16_t getCenter(void);
    uint16_t getMaxLevel(void);
    void hardReset(void);

private:
    uint8_t lr;

    uint16_t center =     1400; //uS absolute
    uint16_t hystCenter =   80; //uS absolute //60
    uint16_t minLevel =    220;  //uS relative
    uint16_t maxLevel =    440;  //uS relative

    const uint16_t hystLatch =    8; //S/10 until latch //10
    const uint16_t hystMaxLatch = 25; //S/10 until latch //30

    const uint16_t untilTrigger = 1; //S/10 until latch

    const uint16_t minDuty =     200; //invalid PPM if lower
    const uint16_t maxDuty =    3000; //invalid PPM if higher

    uint16_t triggersL;
    uint16_t triggersR;
    uint16_t framesTriggeredL;
    uint16_t framesTriggeredR;
    uint16_t framesCentered;
    uint16_t framesMaxedL;
    uint16_t framesMaxedR;
    uint16_t framesSpecial;

    uint8_t pendingRequest;
    uint8_t newTriggerVal;
    uint8_t maxTriggerStatus;
};

