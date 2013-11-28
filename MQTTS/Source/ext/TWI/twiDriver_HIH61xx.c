/*
Copyright (c) 2011-2013 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.github.com

BSD New License
See LICENSE.txt file for license details.
*/

// TWI Driver Honeywell - HIH6130/HIH6131/HIH6120/HIH6121,  Humidity & Temperature

// Outs
// Tw9984       Temperature Counter(TC)
// TB9985       Humidity Counter(HC)
// T�C = (TC * 55 / 5461) - 40
//  A*55/5461-40
// RH% = HC * 20 / 51
//  A*20/51

#include "../../config.h"

#if (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_HIH61XX)

#include "../twim.h"
#include "twiDriver_HIH61xx.h"

#define HIH61XX_TWI_ADDR            0x27

//#define HIH61XX_T_MIN_DELTA         6     // use hysteresis for temperature
//#define HIH61XX_H_MIN_DELTA         1     // use hysteresis for humidity

extern volatile uint8_t twim_access;           // access mode & busy flag

static uint8_t  hih61xx_stat;
static uint8_t  hih61xx_oldhumi;
static uint16_t hih61xx_oldtemp;
uint8_t         hih61xx_buf[4];

uint8_t twi_HIH61xx_Read(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Read Humidity
    {
        *pLen = 1;
        *pBuf = hih61xx_oldhumi;
        // Return Humidity %
//        *pBuf = ((uint16_t)hih61xx_oldhumi*25)>>6;
    }
    else                                // Read Temperature
    {
        *pLen = 2;
        *(uint16_t *)pBuf = hih61xx_oldtemp;
/*
        // Return T 0.1�C
        uint16_t temp = ((uint32_t)hih61xx_oldtemp * 825)>>13;
        temp -= 400;
        *(uint16_t *)pBuf = temp;
*/
    }
    return MQTTS_RET_ACCEPTED;
}

uint8_t twi_HIH61xx_Pool1(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t temp;
#ifdef ASLEEP
  if(sleep != 0)
  {
    hih61xx_stat = (0xFF-(POOL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP
    if(twim_access & TWIM_ERROR)   // Bus Error
    {
        if(hih61xx_stat != 0)
            hih61xx_stat = 0x40;
        return 0;
    }
    
    if(twim_access & (TWIM_BUSY | TWIM_READ | TWIM_WRITE))      // Bus Busy
        return 0;

    switch(hih61xx_stat)
    {
        case 0:         // Start Conversion
            hih61xx_stat = 1;
        case 1:
            twimExch_ISR(HIH61XX_TWI_ADDR, TWIM_WRITE, 0, 0, NULL, NULL);
            break;
        case 4:         // !! The measurement cycle duration is typically 36.65 ms
            twimExch_ISR(HIH61XX_TWI_ADDR, TWIM_READ, 0, 4, hih61xx_buf, NULL);
            break;
        case 5:
            if((hih61xx_buf[0] & 0xC0) != 0)   // data invalid
            {
                hih61xx_stat--;
                return 0;
            }
            temp = ((((uint16_t)hih61xx_buf[2])<<6) | (hih61xx_buf[3]>>2)) & 0x3FFF;
#if (defined HIH61XX_T_MIN_DELTA) && (HIH61XX_T_MIN_DELTA > 0)
            if((temp > hih61xx_oldtemp ? temp - hih61xx_oldtemp : hih61xx_oldtemp - temp) > HIH61XX_T_MIN_DELTA)
#else
            if(temp != hih61xx_oldtemp)
#endif  //  HIH61XX_T_MIN_DELTA
            {
                hih61xx_oldtemp = temp;
                hih61xx_stat++;
                return 1;
            }
            break;
    }

    hih61xx_stat++;
    return 0;
}

uint8_t twi_HIH61xx_Pool2(subidx_t * pSubidx, uint8_t _unused)
{
  uint8_t tmp;

  if(hih61xx_stat == 7)
  {
    hih61xx_stat++;
    tmp = (hih61xx_buf[0]<<2) | (hih61xx_buf[1]>>6);
#if (defined HIH61XX_H_MIN_DELTA) && (HIH61XX_H_MIN_DELTA > 0)
    if((tmp > hih61xx_oldhumi ? tmp - hih61xx_oldhumi : hih61xx_oldhumi - tmp) > HIH61XX_H_MIN_DELTA)
#else
    if(tmp != hih61xx_oldhumi)
#endif  //  HIH61XX_H_MIN_DELTA
    {
      hih61xx_oldhumi = tmp;
      return 1;
    }
  }
  return 0;
}

uint8_t twi_HIH61xx_Config(void)
{
    if(twimExch(HIH61XX_TWI_ADDR, TWIM_WRITE, 0, 0, NULL) != TW_SUCCESS)    // Communication error
        return 0;

    hih61xx_stat = 0;
    hih61xx_oldhumi = 0;
    hih61xx_oldtemp = 0;

    // Register variables
    indextable_t * pIndex1;
    pIndex1 = getFreeIdxOD();
    if(pIndex1 == NULL)
        return 0;

    pIndex1->cbRead  =  &twi_HIH61xx_Read;
    pIndex1->cbWrite =  NULL;
    pIndex1->cbPool  =  &twi_HIH61xx_Pool1;
    pIndex1->sidx.Place = objTWI;                   // Object TWI
    pIndex1->sidx.Type =  objInt16;                 // Variables Type -  UInt16
    pIndex1->sidx.Base = (HIH61XX_TWI_ADDR<<8);     // Device addr

    indextable_t * pIndex2;
    pIndex2 = getFreeIdxOD();
    if(pIndex2 == NULL)
    {
        pIndex1->Index = 0xFFFF;                    // Free Index
        return 0;
    }

    pIndex2->cbRead  =  &twi_HIH61xx_Read;
    pIndex2->cbWrite =  NULL;
    pIndex2->cbPool  =  &twi_HIH61xx_Pool2;
    pIndex2->sidx.Place = objTWI;                   // Object TWI
    pIndex2->sidx.Type =  objUInt8;
    pIndex2->sidx.Base = (HIH61XX_TWI_ADDR<<8) + 1; // Device addr

    return 2;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_HIH61XX)
