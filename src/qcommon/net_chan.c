//
// net_chan.c
//
// Copyright 1998 Raven Software
//

#pragma region ========================== NETWORK PACKET INFO ==========================

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

#pragma endregion

#include "qcommon.h"

static cvar_t* showpackets;
static cvar_t* showdrop;
static cvar_t* qport;

// H2:
cvar_t* net_sendrate;
cvar_t* net_receiverate;
cvar_t* net_latency;

netadr_t net_from;
sizebuf_t net_message;
byte net_message_buffer[MAX_MSGLEN];

void Netchan_Init(void)
{
	// Pick a port value that should be nice and random.
	// YQ2: The original code used Sys_Milliseconds() as base.
	// It worked because the original Sys_Milliseconds included some amount of random data (Windows) or was dependend on seconds since epoche (Unix).
	// Our Sys_Milliseconds() always starts at 0, so there's a very high probability for something like "./quake2 +connect example.com" - that two or more clients end up with the same qport.
	const int port = time(NULL) & 0xffff;

	showpackets = Cvar_Get("showpackets", "0", 0);
	showdrop = Cvar_Get("showdrop", "0", 0);
	qport = Cvar_Get("qport", va("%i", port), CVAR_NOSET);

	// H2:
	net_sendrate = Cvar_Get("net_sendrate", "1", 0);
	net_receiverate = Cvar_Get("net_receiverate", "1", 0);
	net_latency = Cvar_Get("net_latency", "0", 0);
}

// Q2 counterpart
// Sends an out-of-band datagram.
static void Netchan_OutOfBand(const int net_socket, const netadr_t* adr, const int length, const byte* data) //mxd. Changed 'adr' type to netadr_t*
{
	sizebuf_t send;
	byte send_buf[MAX_MSGLEN];

	// Write the packet header.
	SZ_Init(&send, send_buf, sizeof(send_buf));

	MSG_WriteLong(&send, -1); // -1 sequence means out of band.
	SZ_Write(&send, data, length);

	// Send the datagram.
	NET_SendPacket(net_socket, send.cursize, send.data, adr);
}

// Q2 counterpart
// Sends a text message in an out-of-band datagram.
void Netchan_OutOfBandPrint(const int net_socket, const netadr_t* adr, const char* format, ...)
{
	static char string[MAX_MSGLEN - 4]; //mxd. Keep 4 bytes for out of band marker
	va_list argptr;

	va_start(argptr, format);
	vsprintf_s(string, sizeof(string), format, argptr); //mxd. vsprintf -> vsprintf_s
	va_end(argptr);

	Netchan_OutOfBand(net_socket, adr, (int)strlen(string), (byte*)string);
}

// Q2 counterpart
// Called to open a channel to a remote system.
void Netchan_Setup(const netsrc_t sock, netchan_t* chan, const netadr_t* adr, const int port)
{
	memset(chan, 0, sizeof(*chan));
	memcpy(&chan->remote_address, adr, sizeof(netadr_t)); //mxd

	chan->sock = sock;
	chan->qport = port;
	chan->last_received = curtime;
	chan->incoming_sequence = 0;
	chan->outgoing_sequence = 1;

	SZ_Init(&chan->message, chan->message_buf, sizeof(chan->message_buf));
	chan->message.allowoverflow = true;
}

// Q2 counterpart
qboolean Netchan_NeedReliable(const netchan_t* chan)
{
	// If the remote side dropped the last reliable message, resend it.
	if (chan->incoming_acknowledged > chan->last_reliable_sequence && chan->incoming_reliable_acknowledged != chan->reliable_sequence)
		return true;

	// If the reliable transmit buffer is empty, copy the current message out.
	if (chan->reliable_length == 0 && chan->message.cursize > 0)
		return true;

	return false;
}

// Tries to send an unreliable message to a connection, and handles the transmission / retransmission of the reliable messages.
// A 0 length will still generate a packet and deal with the reliable messages.
int Netchan_Transmit(netchan_t* chan, const int length, const byte* data) // H2: int return type.
{
	byte send_buf[MAX_MSGLEN];
	sizebuf_t send;

	// Check for message overflow
	if (chan->message.overflowed)
	{
		chan->fatal_error = true;
		Com_Printf("%s:Outgoing message overflow\n", NET_AdrToString(&chan->remote_address));

		return 0; // H2
	}

	const qboolean send_reliable = Netchan_NeedReliable(chan);

	if (chan->reliable_length == 0 && chan->message.cursize > 0)
	{
		memcpy(chan->reliable_buf, chan->message_buf, chan->message.cursize);
		chan->reliable_length = chan->message.cursize;
		chan->message.cursize = 0;
		chan->reliable_sequence ^= 1;
	}

	// Write the packet header.
	SZ_Init(&send, send_buf, sizeof(send_buf));

	const int w1 = (chan->outgoing_sequence & ~(1 << 31)) | (send_reliable << 31);
	const int w2 = (chan->incoming_sequence & ~(1 << 31)) | (chan->incoming_reliable_sequence << 31);

	chan->outgoing_sequence++;
	chan->last_sent = curtime;

	MSG_WriteLong(&send, w1);
	MSG_WriteLong(&send, w2);

	// Send the qport if we are a client.
	if (chan->sock == NS_CLIENT)
		MSG_WriteShort(&send, (int)qport->value);

	// Copy the reliable message to the packet first.
	if (send_reliable)
	{
		SZ_Write(&send, chan->reliable_buf, chan->reliable_length);
		chan->last_reliable_sequence = chan->outgoing_sequence;
	}

	// Add the unreliable part if space is available.
	if (send.maxsize - send.cursize >= length)
		SZ_Write(&send, data, length);
	else
		Com_Printf("Netchan_Transmit: dumped unreliable\n");

	// Send the datagram.
	NET_SendPacket(chan->sock, send.cursize, send.data, &chan->remote_address);

	if ((int)showpackets->value)
	{
		if (send_reliable)
		{
			Com_Printf("send %4i : s=%i reliable=%i ack=%i rack=%i\n", 
				send.cursize,
				chan->outgoing_sequence - 1, 
				chan->reliable_sequence, 
				chan->incoming_sequence,
				chan->incoming_reliable_sequence);
		}
		else
		{
			Com_Printf("send %4i : s=%i ack=%i rack=%i\n", 
				send.cursize, 
				chan->outgoing_sequence - 1,
				chan->incoming_sequence, 
				chan->incoming_reliable_sequence);
		}
	}

	return send.cursize; // H2
}

// Q2 counterpart
// Called when the current net_message is from remote_address.
// Modifies net_message so that it points to the packet payload.
qboolean Netchan_Process(netchan_t* chan, sizebuf_t* msg)
{
	// Get sequence numbers.
	MSG_BeginReading(msg);
	uint sequence = MSG_ReadLong(msg);
	uint sequence_ack = MSG_ReadLong(msg);

	// Read the qport if we are a server.
	if (chan->sock == NS_SERVER)
		MSG_ReadShort(msg);

	const uint reliable_message = sequence >> 31;
	const uint reliable_ack = sequence_ack >> 31;

	sequence &= ~(1 << 31);
	sequence_ack &= ~(1 << 31);

	if ((int)showpackets->value)
	{
		if (reliable_message)
			Com_Printf("recv %4i : s=%i reliable=%i ack=%i rack=%i\n", 
				msg->cursize, sequence, chan->incoming_reliable_sequence ^ 1, sequence_ack, reliable_ack);
		else
			Com_Printf("recv %4i : s=%i ack=%i rack=%i\n", msg->cursize, sequence, sequence_ack, reliable_ack);
	}

	// Discard stale or duplicated packets.
	if (sequence <= (uint)chan->incoming_sequence)
	{
		if ((int)showdrop->value)
			Com_Printf("%s:Out of order packet %i at %i\n", NET_AdrToString(&chan->remote_address), sequence, chan->incoming_sequence);

		return false;
	}

	// Dropped packets don't keep the message from being used.
	chan->dropped = (int)sequence - (chan->incoming_sequence + 1);
	if (chan->dropped > 0 && (int)showdrop->value)
		Com_Printf("%s:Dropped %i packets at %i\n", NET_AdrToString(&chan->remote_address), chan->dropped, sequence);

	// If the current outgoing reliable message has been acknowledged, clear the buffer to make way for the next.
	if (reliable_ack == (uint)chan->reliable_sequence)
		chan->reliable_length = 0; // It has been received

	// If this message contains a reliable message, bump incoming_reliable_sequence.
	chan->incoming_sequence = (int)sequence;
	chan->incoming_acknowledged = (int)sequence_ack;
	chan->incoming_reliable_acknowledged = (int)reliable_ack;
	if (reliable_message > 0)
		chan->incoming_reliable_sequence ^= 1;

	// The message can now be read from the current message pointer.
	chan->last_received = curtime;

	return true;
}