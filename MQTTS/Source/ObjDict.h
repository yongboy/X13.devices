/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef _OBJ_DICT_H_
#define _OBJ_DICT_H_

#define OD_DEV_TYP_LEN  6

// Predefined variables
enum
{
  // Global Settings
  objNodeName     = 0xFF00, // _sName<String>
  objTAsleep      = 0xFF01, // cfg/XD_SleepTime<UIint16>
  // RF Node
  objRFNodeId     = 0xFF10, // cfg/XD_DeviceAddr<UInt8>
  objRFGroup      = 0xFF11, // cfg/XD_GroupID<UInt16>
  objRFChannel    = 0xFF12, // cfg/XD_Channel<UInt8>
  objRSSI         = 0xFF13, // cfg/XD_RSSI<UInt8>
  // Lan Node 
  objMACAddr      = 0xFF20, // cfg/XD_MACAddr - Array - Len 6
  objIPAddr       = 0xFF21, // cfg/XD_IPAddr<UInt32>
  objIPMask       = 0xFF22, // cfg/XD_IPMask<UInt32>
  objIPRouter     = 0xFF23, // cfg/XD_IPRouter<UInt32>
  objIPBroker     = 0xFF24, // cfg/XD_IPBroker<UInt32>
  // Read Only Variables
  objDeviceTyp    = 0xFFC0  // _declarer<String>
}eObjList;

enum
{
    objPROGMEM  = 'F',  // FLASH
    objEEMEM    = 'E',  // EEPROM
    objAin      = 'A',  // Analogue Input
    //    objAout     = 'D',  // Analog Output
    objDin      = 'I',  // Digital Input
    objDout     = 'O',  // Digital Output
    objPWM      = 'P',  // LED PWM out
    objSer      = 'S',  // Serial port, S.0-5 (1200-38400)
    objTWI      = 'T',  // TWI
    objUsrExt   = 'X',  // User extensions
}eObjPlace;

enum
{
    objBool     = 'z',  // bool
    objInt8     = 'b',  // int8
    objUInt8    = 'B',  // uint8
    objInt16    = 'w',  // int16
    objUInt16   = 'W',  // uint16
    objInt32    = 'd',  // int32
    objUInt32   = 'D',  // uint32
    objInt64    = 'q',  // int64
    objString   = 's',  // UTF8 '0' - terminated string
    objArray    = 'a',  // array
    objUnknown  = 'U',  // unknown
// Port's
    objPinPNP   = 'p',  // Pin PNP
    objPinNPN   = 'n',  // Pin NPN ( Input - PullUp)
    objActPNP   = 'A',  // Node Active  = 1, sleep = 0
    objActNPN   = 'a',  // Node Active = 0, sleep = 1;
// Analog In
    objArefVcc  = 'v',  // Ref = Vcc(3,3V)
    objArefExt  = 'e',  // Ref = External(not connected)
    objArefInt  = 'i',  // Ref = Internal(1,1V)
//    objArefInt  = 'I',  // Ref = Internal(2,56V)
// Serial port
    ObjSerRx    = 'r',  // Com to RF
    ObjSerTx    = 't',  // RF to COM, Object type - array
}eObjTyp;

// Variable description
typedef struct
{
    uint8_t     Place;
    uint8_t     Type;
    uint16_t    Base;
}subidx_t;

typedef uint8_t (*cbRead_t)(subidx_t * pSubidx, uint8_t *pLen, uint8_t *pBuf);  // Callback Read
typedef uint8_t (*cbWrite_t)(subidx_t * pSubidx, uint8_t Len, uint8_t *pBuf);   // Callback Write
typedef uint8_t (*cbPoll_t)(subidx_t * pSubidx, uint8_t sleep);                  // Callback Poll

// Struct for creating entries
typedef struct
{
    subidx_t    sidx;
    uint16_t    Index;
    cbRead_t    cbRead;
    cbWrite_t   cbWrite;
    cbPoll_t    cbPoll;
}indextable_t;

void InitOD(void);
void CleanOD(void);

//uint8_t RegistIntOD(indextable_t * pIdx);
indextable_t * getFreeIdxOD(void);
uint8_t RegisterOD(MQ_t *pBuf);
void RegAckOD(uint16_t index);

uint8_t ReadOD(uint16_t Id, uint8_t Flags, uint8_t *pLen, uint8_t *pBuf);
uint8_t WriteOD(uint16_t Id, uint8_t Flags, uint8_t Len, uint8_t *pBuf);

uint16_t PollOD(uint8_t sleep);

#endif
 