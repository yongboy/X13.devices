/*
Copyright (c) 2011-2014 <comparator@gmx.de>

This file is part of the X13.Home project.
http://X13home.org
http://X13home.net
http://X13home.github.io/

BSD New License
See LICENSE file for license details.
*/

#ifndef IP_ARP_UDP_H
#define IP_ARP_UDP_H 1

#include "ip_config.h"

void sec_tick_lan(void);
uint8_t packetloop_lan(uint8_t *buf, uint16_t plen);

#ifdef UDP_client
void send_udp_prepare(uint8_t *buf,uint16_t sport, const uint8_t *dip, uint16_t dport,const uint8_t *dstmac);
void send_udp_transmit(uint8_t *buf,uint16_t datalen);
#endif  //  UDP_client

#ifdef DHCP_client
// The function returns 1 once we have a valid IP. 
// At this point you must not call the function again.
uint8_t packetloop_dhcp_initial_ip_assignment(uint8_t *buf,uint16_t plen);
#endif  //  DHCP_client

#endif /* IP_ARP_UDP_H */

//@}
