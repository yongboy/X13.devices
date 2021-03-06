/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

// Global configuration settings

#ifndef _CONFIG_H
#define _CONFIG_H

/****   Available Options
// Board configuration
#define F_CPU                   8000000UL   // Main frequency
#define USE_RTC_OSC             1       // Use low power - 32,768kHz oscillator, !! used Timer1

#define UNODE                   1       // Select Board uNode Vers 1.0
#define JEENODE                 1       // Select Board JeeNode(Arduino + RFM12 Shield)
#define PANSTAMP                1       // Select Board panSTamp( Arduino + CC1101 Shield)
#define ENC28J60                1       // Select Board ATMega328p + ENC28J60
#define WIZNET                  1       // Select Board ATMega328p + WIZNET Chip on SPI
#define DUMMY                   1       // Select Board without RF Interface

#define RF_NODE                 1       // used RF Interconnection Interface
#define LAN_NODE                1       // used LAN Interconnection Interface

#define GATEWAY                 1       // Build Gateway, otherwise build Node

#define ASLEEP                  1       // Use ASleep mode, !! Not compatible with GATEWAY

// Memory Manager
#define MQMEM_SIZEOF_QUEUE      10      // Allocated memory buffers

// MQTT-S Section
#define MQTTS_MSG_SIZE          30      // Size of payload(base for all buffers)
#define MQTTS_SIZEOF_SEND_FIFO  4       // Size of MQTTS Send Buffer
#define MQTTS_SIZEOF_POLL_FIFO  4       // Size of MQTTS Poll Buffer

// Object Dictionary
#define OD_MAX_INDEX_LIST       12      // Size of identificators list
#define OD_DEFAULT_TASLEEP      0       // Sleep Time, default - always online.

// RF
#define RF_BASE_FREQ            868300000UL // Set RF Frequency
#define RF_USE_RSSI             1       // Get RSSI info, only for CC1101
#define OD_DEFAULT_GROUP        0x2DD4  // Set RF default group(Synchro word)
#define OD_DEFAULT_ADDR         0x07    // if defined, then use this default address, else: 0xFF - DHCP

// Extensions
#define EXTDIO_USED             1       // Use Digital Inputs/Outputs
#define EXTAI_USED              1       // Use Analogue Inputs
#define EXTPWM_USED             1       // enable HW PWM with TIMER0, dependency on EXTDIO_USED
#define EXTSER_TX_USED          1       // enable Serial Output, dependency on EXTDIO_USED
#define EXTSER_RX_USED          1       // enable Serial Input, dependency on EXTDIO_USED
#define TWI_USED                1       // enable TWI, dependency on EXTDIO_USED
****/

// Memory Manager
#define MQMEM_SIZEOF_QUEUE      10      // Allocated memory buffers

// MQTT-S Section
#define MQTTS_MSG_SIZE          30      // Size of payload(base for all buffers)
#define MQTTS_SIZEOF_SEND_FIFO  4       // Size of MQTTS Send Buffer
#define MQTTS_SIZEOF_POLL_FIFO  4       // Size of MQTTS Poll Buffer

// Object Dictionary
#define OD_MAX_INDEX_LIST       12      // Size of identificators list
#define OD_DEFAULT_TASLEEP      0       // Sleep Time, default - always online.

#define OD_DEV_SWVERSH          '2'     // Software Version
#define OD_DEV_SWVERSM          '6'
#define OD_DEV_SWVERSL          '0'

// RF Section
//#define RF_BASE_FREQ            433920000UL
#define RF_BASE_FREQ            868300000UL
//#define RF_BASE_FREQ            869000000UL
//#define RF_BASE_FREQ            915000000UL

// Gateway UART Section
#define UART_TX_QUEUE_SIZE      4       // send buffers

// TWI Section
// TWI Drivers
//#define TWI_USE_SMARTDRV        1       // Smart Driver for smart devices.
#define TWI_USE_BLINKM          1       // BlinkM - Blinky RGB Driver
#define TWI_USE_EXPANDER        1       // 16bit IO Expander, PCA9535/TCA9535/PCA9555/MCP23016
#define TWI_USE_HIH61XX         1       // Honeywell HIH-61xx - Temperature/Humidity
#define TWI_USE_CC2D            1       // GE Sensing CC2Dxx[s]  - Temperature/Humidity
#define TWI_USE_SHT21           1       // Sensirion SHT21 - Temperature/Humidity
#define TWI_USE_LM75            1       // LM75 - Temperature
#define TWI_USE_BMP180          1       // Bosh BMP180/BMP085 - Temperature/Pressure
//#define TWI_USE_DUMMY           1       // DUMMY, checks all addresses
//End TWI Section

#if (defined UNODE)
  #include "Phy/HWconfigUN.h"           // Hardware uNode vers. 1.0 + 1.1
#elif (defined UNODE20)
  #include "Phy/HWconfigUP.h"           // Hardware uNode vers. 2.0
#elif (defined JEENODE)
  #include "Phy/HWconfigJN.h"           // Hardware JeeNode & Arduino
#elif (defined PANSTAMP)
  #include "Phy/HWconfigPS.h"           // Hardware panSTamp
#elif (defined ENC28J60)
  #include "Phy/HWconfigENC.h"          // Hardware MEGA328P + ENC28J60 on SPI
#elif (defined WIZNET)
  #include "Phy/HWconfigWIZ.h"          // Hardware MEGA328P + WIZNET on SPI
#elif (defined DUMMY)
  #include "Phy/HWconfigDM.h"           // Hardware Dummy
#else
  #error Hardware configuration is not defined
#endif

#ifdef GATEWAY
#ifdef ASLEEP
#error Incompatible options 'GATEWAY' & 'ASLEEP'
#endif  //  ASLEEP
#endif  //  GATEWAY

#include "mqtts.h"
#include "mqMEM.h"
#include "objdict.h"
#include "phy.h"
#include "util.h"

#endif  // _CONFIG_H
