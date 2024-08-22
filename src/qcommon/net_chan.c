//
// net_chan.c
//
// Copyright 1998 Raven Software
//

/*
packet header
-------------
31	sequence
1	does this message contain a reliable payload
31	acknowledge sequence
1	acknowledge receipt of even/odd message
16	qport

The remote connection never knows if it missed a reliable message, the local side detects that it has been dropped
by seeing a sequence acknowledge higher than the last reliable sequence, but without the correct even/odd bit for the reliable set.

If the sender notices that a reliable message has been dropped, it will be retransmitted.
It will not be retransmitted again until a message after the retransmit has been acknowledged and the reliable still failed to get there.

If the sequence number is -1, the packet should be handled without a netcon.

The reliable message can be added to at any time by doing MSG_Write* (&netchan->message, <data>).

If the message buffer is overflowed, either by a single message, or by multiple frames worth piling up
while the last reliable transmit goes unacknowledged, the netchan signals a fatal error.

Reliable messages are always placed first in a packet, then the unreliable message is included if there is sufficient room.

To the receiver, there is no distinction between the reliable and unreliable parts of the message,
they are just processed out as a single larger message.

Illogical packet sequence numbers cause the packet to be dropped, but do not kill the connection.
This, combined with the tight window of valid reliable acknowledgement numbers provides protection against malicious address spoofing.

The qport field is a workaround for bad address translating routers that sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the channel matches even if the IP port differs.
The IP port should be updated to the new value before sending out any replies.

If there is no information that needs to be transferred on a given frame, such as during the connection stage while waiting for the client to load,
then a packet only needs to be delivered if there is something in the unacknowledged reliable.
*/

#include "qcommon.h"

static cvar_t* showpackets;
static cvar_t* showdrop;
static cvar_t* qport;

// New in H2:
static cvar_t* net_sendrate;
static cvar_t* net_receiverate;
static cvar_t* net_latency;

void Netchan_Init(void)
{
	// Pick a port value that should be nice and random.
	const int port = Sys_Milliseconds() & 0xffff;

	showpackets = Cvar_Get("showpackets", "0", 0);
	showdrop = Cvar_Get("showdrop", "0", 0);
	qport = Cvar_Get("qport", va("%i", port), CVAR_NOSET);

	// New in H2:
	net_sendrate = Cvar_Get("net_sendrate", "1", 0);
	net_receiverate = Cvar_Get("net_receiverate", "1", 0);
	net_latency = Cvar_Get("net_latency", "0", 0);
}