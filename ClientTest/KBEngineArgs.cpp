
#include "KBEngineArgs.h"
#include "KBDebug.h"

KBEngineArgs::KBEngineArgs():
	ip(("127.0.0.1")),
	port(20013),
	clientType(EKCLIENT_TYPE::CLIENT_TYPE_WIN),
	serverHeartbeatTick(15),
	SEND_BUFFER_MAX(TCP_PACKET_MAX),
	RECV_BUFFER_MAX(TCP_PACKET_MAX)
{
}

KBEngineArgs::~KBEngineArgs()
{
}

int KBEngineArgs::getRecvBufferSize()
{
	return (int)RECV_BUFFER_MAX;
}

int KBEngineArgs::getSendBufferSize()
{
	return (int)SEND_BUFFER_MAX;
}