/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#include "config.h"

static volatile uint8_t iPoll;
#define IPOLL_USR   0x01
#define IPOLL_CALIB 0x02

#ifdef ASLEEP
#define IPOLL_SLEEP 0x04
static void goToSleep(void);
static void wakeUp(void);
#endif  //  ASLEEP

#ifdef GATEWAY
#include "uart.h"
#endif  //  GATEWAY

__attribute__((OS_main)) int main(void) 
{
    uint8_t * pPBuf;            // Publish Buffer
#ifdef GATEWAY
    MQ_t *  pUBuf;              // USART Buffer
    MQ_t *  pUbBuf;             // USART Backup buffer
    pUbBuf = NULL;

    s_Addr  iAddr;
#endif  // GATEWAY
    uint8_t bTmp;
    uint16_t pollIdx = 0xFFFF;

// Watchdog Stop
    cli();
    wdt_reset();
    wdt_disable();
    // Configure Power Reduction
    CONFIG_PRR();
    // Initialize Memory manager
    mqInit();
    // Initialize Object's Dictionary
    InitOD();
#ifdef GATEWAY
    InitUART(USART_BAUD);   //  Buad = 38400, fosc = 16M/ (16 * baud)  - 1
#endif  // GATEWAY
    // Initialise MQTTS
    MQTTS_Init();
    // Initialise PHY
    PHY_Init();
    // Initialise  variables
    iPoll = 0;
    
    // Initialize Task Planer
    InitTimer();
    // configure Sleep controller & enable interrupts
    set_sleep_mode(SLEEP_MODE_IDLE);    // Standby, Idle
    sei();                              // Enable global interrupts

    while(1)
    {
#ifdef GATEWAY
        pUBuf = (MQ_t *)uGetBuf();
        if(pUBuf != NULL)
        {
          iAddr = pUBuf->addr;
          if(iAddr == rf_GetNodeID())
          {
            if(MQTTS_Parser(pUBuf) == 0)
              mqRelease(pUBuf);
          }
          else if(((iAddr != AddrBroadcast) || (MQTTS_Parser(pUBuf) == 0)) &&
                   (MQTTS_GetStatus() == MQTTS_STATUS_CONNECT))
          {
            if(pUbBuf)
              mqRelease(pUbBuf);
            pUbBuf = pUBuf;
          }
        }

        if(pUbBuf && PHY_CanSend())
        {
          PHY_Send(pUbBuf);
          pUbBuf = NULL;
        }

        MQ_t *  pRBuf;              // RF Buffer
        pRBuf = PHY_GetBuf();
        if(pRBuf != NULL)
        {
            if(MQTTS_GetStatus() == MQTTS_STATUS_CONNECT)
                uPutBuf((uint8_t *)pRBuf);
            else
                mqRelease(pRBuf);
        }
        
        if(MQTTS_DataRdy())
          uPutBuf((uint8_t *)MQTTS_Get());
#else   // NODE
        MQ_t *  pRBuf;              // RF Buffer
        pRBuf = PHY_GetBuf();
        if((pRBuf != NULL) && (MQTTS_Parser(pRBuf) == 0))
            mqRelease(pRBuf);
            
        if(MQTTS_DataRdy() && PHY_CanSend())
          PHY_Send(MQTTS_Get());
#endif  //  GATEWAY
        if(iPoll & IPOLL_USR)
        {
            iPoll &= ~IPOLL_USR;
            
            PHY_Poll();

            bTmp = MQTTS_GetStatus();
            if(bTmp == MQTTS_STATUS_CONNECT)
            {
                if(pollIdx == 0xFFFF)
                    pollIdx = PollOD(0);
            
                if(pollIdx != 0xFFFF)
                {
                    // Publish
                    pPBuf = (uint8_t *)mqAssert();
                    if(pPBuf != NULL)
                    {
                        bTmp = (MQTTS_MSG_SIZE - MQTTS_SIZEOF_MSG_PUBLISH);

                        ReadOD(pollIdx, MQTTS_FL_TOPICID_NORM | 0x80, &bTmp, pPBuf);
                        MQTTS_Publish(pollIdx, MQTTS_FL_QOS1, bTmp, pPBuf);
                        mqRelease((MQ_t *)pPBuf);
                        pollIdx = 0xFFFF;
                    }
                }
            }
#ifdef ASLEEP
            else if(bTmp == MQTTS_STATUS_AWAKE)
            {
                if(pollIdx == 0xFFFF)
                    pollIdx = PollOD(0);
            }
#endif  //  ASLEEP

            bTmp = MQTTS_Poll(pollIdx != 0xFFFF);
#ifdef ASLEEP
            if(bTmp == MQTTS_POLL_STAT_ASLEEP)       // Sweet dreams
            {
                PollOD(1);
                goToSleep();
            }
            else if(bTmp == MQTTS_POLL_STAT_AWAKE)   // Wake UP
                wakeUp();
#endif  //  ASLEEP
        }
        sleep_mode();
    }
}

ISR(TIMER_ISR)
{
#ifdef USE_RTC_OSC
#define BASE_TICK       (F_CPU/8/POLL_TMR_FREQ)
#define BASE_TICK_MIN   (uint16_t)(BASE_TICK/1.005)
#define BASE_TICK_MAX   (uint16_t)(BASE_TICK*1.005)

    uint16_t tmp;

//  Calibrate internal RC Osc
// !!!! for ATMEGA xx8P only, used Timer 1
    if(iPoll & IPOLL_CALIB)
    {
        tmp = TCNT1;
        TCCR1B = 0;

        if(tmp < BASE_TICK_MIN)         // Clock is running too slow
            OSCCAL++;
        else if(tmp > BASE_TICK_MAX)    // Clock is running too fast
            OSCCAL--;

        iPoll &= ~IPOLL_CALIB;
    }
    else 
#ifdef ASLEEP
    if(!(iPoll & IPOLL_SLEEP))
#endif  //  ASLEEP
    {
        TCNT1 = 0;
        TCCR1B = (2<<CS10);
        iPoll |= IPOLL_CALIB;
    }
#else   //  !USE_RTC_OSC
#ifdef ASLEEP
    if(!(iPoll & IPOLL_SLEEP))
#endif  //  ASLEEP
#endif  //  USE_RTC_OSC
        iPoll |= IPOLL_USR;
}

#ifdef ASLEEP
#ifndef USE_RTC_OSC
ISR(WDT_vect)
{
    wdt_reset();
    if(iPoll & IPOLL_SLEEP)
        iPoll |= IPOLL_USR;
}
#endif  //  !USE_RTC_OSC

static void goToSleep(void)
{
    iPoll = IPOLL_SLEEP;
    rf_SetState(RF_TRVASLEEP);
#ifdef USE_RTC_OSC
    config_sleep_rtc();
#else   // Use watchdog
    config_sleep_wdt();
#endif  //  USE_RTC_OSC
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);    // Standby, Power Save
}

static void wakeUp(void)
{
    set_sleep_mode(SLEEP_MODE_IDLE);        // Standby, Idle
    rf_SetState(RF_TRVWKUP);
#ifdef USE_RTC_OSC
    InitTimer();
#else   // Use watchdog
    wdt_reset();
    wdt_disable();
#endif  //  USE_RTC_OSC
    iPoll = 0;
}
#endif  //  ASLEEP