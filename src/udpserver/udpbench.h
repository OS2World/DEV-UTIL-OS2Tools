/*****************************************************
 * UDP Benchmark Tool                                *
 * Benchmark remote hosts on TCP/IP via UDP          *
 * (c) 1999    Patrick Haller Systemtechnik          *
 *****************************************************/

#ifndef _UDPBENCH_H_
#define _UDPBENCH_H_


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

/* use 4.0 headaers if tcp/ip stack 5.x is installed */
#define TCPV40HDRS

#include <types.h>
#include <sys/socket.h>
#include <netdb.h>
//#include <unistd.h>
#include <netinet/in.h>
#include <nerrno.h>
//#include <arpa/inet.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct tagPacketBench
{
  ULONG      ID;         /* sequenced identifier to identify lost packets    */
  PERFSTRUCT psSent;     /* timestamp when packet has been put on the socket */
  PERFSTRUCT psReceived; /* timestamp when packet has been received again    */
} PACKETBENCH, *PPACKETBENCH;


/*****************************************************************************
 * Prototypes and Classes                                                    *
 *****************************************************************************/

#endif /* _UDPBENCH_H_ */
