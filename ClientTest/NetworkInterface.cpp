
#include "NetworkInterface.h"
#include "PacketReceiver.h"
#include "PacketSender.h"
#include "MemoryStream.h"
#include "KBDebug.h"
#include "address.h"
#include "interfaces.h"
#include "KBEvent.h"

NetworkInterface::NetworkInterface():
	//endpoint(),
	pPacketSender_(NULL),
	pPacketReceiver_(NULL),
	connectCB_(NULL),
	connectIP_(KBETEXT("")),
	connectPort_(0),
	connectUserdata_(0),
	startTime_(0.0),
	isDestroyed_(false)
{
	EndPoint::initNetwork();
	endpoint.socket(SOCK_STREAM);
}

NetworkInterface::~NetworkInterface()
{
	close();
}

void NetworkInterface::reset()
{
	//close();
	if (endpoint.good())
	{
		endpoint.close();
	}
	endpoint.socket(SOCK_STREAM);
	connectCB_ = NULL;
	connectIP_ = KBETEXT("");
	connectPort_ = 0;
	connectUserdata_ = 0;
	startTime_ = 0.0;
}

void NetworkInterface::close()
{
	if (endpoint.good())
	{
		endpoint.close();
		//INFO_MSG("NetworkInterface::close(): network closed!\n");
	}
	KBE_SAFE_RELEASE(pPacketSender_);
	KBE_SAFE_RELEASE(pPacketReceiver_);

	connectCB_ = NULL;
	connectIP_ = KBETEXT("");
	connectPort_ = 0;
	connectUserdata_ = 0;
	startTime_ = 0.0;
}

bool NetworkInterface::valid()
{
	return endpoint.good();
}

bool NetworkInterface::connectTo(const std::string& addr, uint16 port, InterfaceConnect* callback, int userdata)
{
	INFO_MSG("NetworkInterface::connectTo(): will connect to %s:%d ...\n", addr.c_str(), port);

	reset();
	//uint32 OutIP = 0;
	Address address(addr, port);
	endpoint.addr(address);
	//endpoint.connect(true);
	bool result = true;
	if (endpoint.connect(true) != 0)
	{
		//ERROR_MSG("NetworkInterface::connectTo(): socket could't be created!");
		
		result = false;
		//return false;
	}
	else
	{
		connectIP_ = addr;
		connectPort_ = port;
		connectUserdata_ = userdata;
		startTime_ = getTimeSeconds();
		
	}
	//connectCB_ = callback;
	
	if (callback)
	{
		callback->onConnectCallback(addr, port, result, connectUserdata_);
	}
	return result;
}

bool NetworkInterface::send(MemoryStream* pMemoryStream)
{
	if (!valid())
	{
		return false;
	}

	if (!pPacketSender_)
		pPacketSender_ = new PacketSender(this);

	return pPacketSender_->send(pMemoryStream);
}

void NetworkInterface::process()
{
	if (!valid())
		return;

	if (connectCB_)
	{
		tickConnecting();
		return;
	}

	if (!pPacketReceiver_)
		pPacketReceiver_ = new PacketReceiver(this);

	pPacketReceiver_->process();

	if (isDestroyed_)
	{
		delete this;
		return;
	}
}

void NetworkInterface::tickConnecting()
{
	//ESocketConnectionState state = socket_->GetConnectionState();

	//if (state == SCS_Connected)
	//{
	//	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	//	socket_->GetPeerAddress(*addr);

	//	INFO_MSG("NetworkInterface::tickConnecting(): connect to %s success!", *addr->ToString(true));
	//	connectCB_->onConnectCallback(connectIP_, connectPort_, true, connectUserdata_);
	//	connectCB_ = NULL;

	//	UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
	//	pEventData->success = true;
	//	pEventData->address = std::string::Printf(KBETEXT("%s:%d"), *connectIP_, connectPort_);
	//	KBENGINE_EVENT_FIRE("onConnectionState", pEventData);
	//}
	//else
	//{
	//	// 如果连接超时则回调失败
	//	double currTime = getTimeSeconds();
	//	if (state == SCS_ConnectionError || currTime - startTime_ > 30)
	//	{
	//		ERROR_MSG("NetworkInterface::tickConnecting(): connect to %s:%d timeout!", *connectIP_, connectPort_);
	//		connectCB_->onConnectCallback(connectIP_, connectPort_, false, connectUserdata_);
	//		connectCB_ = NULL;

	//		UKBEventData_onConnectionState* pEventData = NewObject<UKBEventData_onConnectionState>();
	//		pEventData->success = false;
	//		pEventData->address = std::string::Printf(KBETEXT("%s:%d"), *connectIP_, connectPort_);
	//		KBENGINE_EVENT_FIRE("onConnectionState", pEventData);
	//	}
	//}
}
