// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "endpoint.h"

class PacketSender;
class PacketReceiver;
class MemoryStream;
class InterfaceConnect;

/*
	网络模块
	处理连接、收发数据
*/
class  NetworkInterface
{
public:
	NetworkInterface();
	virtual ~NetworkInterface();

public:
	KBESOCKET socket() {
		return endpoint.socket();
	}

	void process();

	void reset();
	void close();
	bool valid();

	bool connectTo(const std::string& addr, uint16 port, InterfaceConnect* callback, int userdata);
	bool send(MemoryStream* pMemoryStream);

	void destroy() {
		isDestroyed_ = true;
	}

	EndPoint& getEndPoint()
	{
		return endpoint;
	}

private:
	void tickConnecting();

protected:
	EndPoint endpoint;
	PacketSender* pPacketSender_;
	PacketReceiver* pPacketReceiver_;

	InterfaceConnect* connectCB_;
	std::string connectIP_;
	uint16 connectPort_;
	int connectUserdata_;
	double startTime_;

	bool isDestroyed_;
};
