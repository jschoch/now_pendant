#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_
#include "enow.h"


void udp_server_task(void *pvParameters);
extern int num_peers;
extern peer_info_t peers[];

#endif //_UDP_SERVER_H
