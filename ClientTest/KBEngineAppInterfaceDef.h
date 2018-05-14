// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.



#include "KBECommon.h"
#include "NetworkInterfaceDef.h"
#include "Message.h"

NETWORK_INTERFACE_DECLARE_BEGIN(KBEngineApp)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onHelloCB,									NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

