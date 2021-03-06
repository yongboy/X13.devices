/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
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

uint8_t twi_HIH61xx_Write(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf)
{
    if(pSubidx->Base & 1)               // Renew Humidity
        hih61xx_oldhumi = *pBuf;
    else                                // Renew Temperature
        hih61xx_oldtemp = *(uint16_t *)pBuf;
    return MQTTS_RET_ACCEPTED;
}

uint8_t twi_HIH61xx_Poll1(subidx_t * pSubidx, uint8_t sleep)
{
  uint16_t temp;
#ifdef ASLEEP
  if(sleep != 0)
  {
    hih61xx_stat = (0xFF-(POLL_TMR_FREQ/2));
    return 0;
  }
#endif  //  ASLEEP
  if(hih61xx_stat == 0)
  {
    if(twim_access != 0)
      return 0;
    // Start Conversion
    twimExch_ISR(HIH61XX_TWI_ADDR, TWIM_WRITE, 0, 0, NULL, NULL);
  }
  else if((hih61xx_stat < 10) && (hih61xx_stat > 3))
  {
    // !! The measurement cycle duration is typically 36.65 ms
    if(twim_access & TWIM_ERROR)   // Bus Error
    {
      hih61xx_stat = 13;
      return 0;
    }
    
    if(hih61xx_stat & 1)
    {
      if((hih61xx_buf[0] & 0xC0) == 0)  // data valid
      {
        hih61xx_stat = 9;

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
      }
    }
    else
    {
      if(twim_access != 0)
        return 0;
      twimExch_ISR(HIH61XX_TWI_ADDR, TWIM_READ, 0, 4, hih61xx_buf, NULL);
    }
  }

  hih61xx_stat++;
  return 0;
}

uint8_t twi_HIH61xx_Poll2(subidx_t * pSubidx, uint8_t _unused)
{
  uint8_t tmp;

  if(hih61xx_stat == 12)
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
    indextable_t * pIndex2;
    
    pIndex1 = getFreeIdxOD();
    if(pIndex1 == NULL)
        return 0;

    pIndex2 = getFreeIdxOD();
    if(pIndex2 == NULL)
    {
        pIndex1->Index = 0xFFFF;                    // Free Index
        return 0;
    }

    pIndex1->cbRead  =  &twi_HIH61xx_Read;
    pIndex1->cbWrite =  NULL;
    pIndex1->cbPoll  =  &twi_HIH61xx_Poll1;
    pIndex1->sidx.Place = objTWI;                   // Object TWI
    pIndex1->sidx.Type =  objInt16;                 // Variables Type -  UInt16
    pIndex1->sidx.Base = (HIH61XX_TWI_ADDR<<8);     // Device addr

    pIndex2->cbRead  =  &twi_HIH61xx_Read;
    pIndex2->cbWrite =  NULL;
    pIndex2->cbPoll  =  &twi_HIH61xx_Poll2;
    pIndex2->sidx.Place = objTWI;                   // Object TWI
    pIndex2->sidx.Type =  objUInt8;
    pIndex2->sidx.Base = (HIH61XX_TWI_ADDR<<8) + 1; // Device addr

    return 2;
}

#endif  //  (defined EXTDIO_USED) && (defined TWI_USED) && (defined TWI_USE_HIH61XX)
