//**************************************************
//File: Packet.h
//Author: GaoJiongjiong
//Function: ÍøÂç°ü½á¹¹
//**************************************************

#ifndef __PACKET_H__
#define __PACKET_H__

#define psize_t USHORT
#define ptype_t USHORT

struct Packet
{
    psize_t size;
    ptype_t type;
};

#endif // __PACKET_H__