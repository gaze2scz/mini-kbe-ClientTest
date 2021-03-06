
#include "KBEngine.h"
#include "KBEngineArgs.h"
//#include "Entity.h"
#include "NetworkInterface.h"
#include "Bundle.h"
#include "MemoryStream.h"
#include "KBDebug.h"
#include "Message.h"
#include "KBEvent.h"

KBEngineApp::KBEngineApp() :
	pArgs_(NULL),
	pNetworkInterface_(NULL),
	username_(KBETEXT("")),
	password_(KBETEXT("")),
	baseappIP_(KBETEXT("")),
	baseappPort_(0),
	lastTickTime_(0.0),
	lastTickCBTime_(0.0),
	lastUpdateToServerTime_(0.0)
{
	//INFO_MSG("KBEngineApp::KBEngineApp(): hello!\n");
}

KBEngineApp::KBEngineApp(KBEngineArgs* pArgs):
	pArgs_(NULL),
	pNetworkInterface_(NULL),
	username_(KBETEXT("")),
	password_(KBETEXT("")),
	baseappIP_(KBETEXT("")),
	baseappPort_(0),
	lastTickTime_(0.0),
	lastTickCBTime_(0.0),
	lastUpdateToServerTime_(0.0)
{
	INFO_MSG("KBEngineApp::KBEngineApp(): hello!");
	initialize(pArgs);
}

KBEngineApp::~KBEngineApp()
{
	destroy();
	INFO_MSG("KBEngineApp::~KBEngineApp(): destructed!");
}

KBEngineApp& KBEngineApp::getSingleton() 
{
	static KBEngineApp* pKBEngineApp = NULL;

	if (!pKBEngineApp)
		pKBEngineApp = new KBEngineApp();

	return *pKBEngineApp;
}

bool KBEngineApp::initialize(KBEngineArgs* pArgs)
{
	if (isInitialized())
		return false;

	pArgs_ = pArgs;
	reset();
	return true;
}

void KBEngineApp::destroy()
{
	reset();
	resetMessages();

	KBE_SAFE_RELEASE(pArgs_);
	KBE_SAFE_RELEASE(pNetworkInterface_);
}

void KBEngineApp::resetMessages()
{
	Messages::getSingleton().clear();
//	Entity::clear();

	INFO_MSG("KBEngineApp::resetMessages(): done!");
}

void KBEngineApp::reset()
{
	lastTickTime_ = getTimeSeconds();
	lastTickCBTime_ = getTimeSeconds();
	lastUpdateToServerTime_ = getTimeSeconds();
	
	//initNetwork();
	if (pNetworkInterface_)
		pNetworkInterface_->reset();
	else
		initNetwork();
}

bool KBEngineApp::initNetwork()
{
	if (pNetworkInterface_)
		delete pNetworkInterface_;

	pNetworkInterface_ = new NetworkInterface();
	return true;
}

void KBEngineApp::_closeNetwork()
{
	if (pNetworkInterface_)
		pNetworkInterface_->close();
}

void KBEngineApp::process()
{
	// 处理网络
	if (pNetworkInterface_)
		pNetworkInterface_->process();

	// 向服务端发送心跳以及同步角色信息到服务端
	sendTick();
}

void KBEngineApp::sendTick()
{
	if (!pNetworkInterface_ || !pNetworkInterface_->valid())
		return;

	double span = getTimeSeconds() - lastTickTime_;

	if (span > pArgs_->serverHeartbeatTick)
	{
		span = lastTickCBTime_ - lastTickTime_;

		// 如果心跳回调接收时间小于心跳发送时间，说明没有收到回调
		// 此时应该通知客户端掉线了
		if (span < 0)
		{
			SCREEN_ERROR_MSG("KBEngineApp::sendTick(): Receive appTick timeout!");
			pNetworkInterface_->close();
			return;
		}

		Message** Loginapp_onClientActiveTickMsgFind = NULL;// Messages::getSingleton().messages.Find("Loginapp_onClientActiveTick");
		Message** Baseapp_onClientActiveTickMsgFind = NULL;// Messages::getSingleton().messages.Find("Baseapp_onClientActiveTick");

		if (currserver_ == KBETEXT("loginapp"))
		{
			if (Loginapp_onClientActiveTickMsgFind)
			{
				Bundle* pBundle = Bundle::createObject();
				pBundle->newMessage(*Loginapp_onClientActiveTickMsgFind);
				pBundle->send(pNetworkInterface_);
			}
		}
		else
		{
			if (Baseapp_onClientActiveTickMsgFind)
			{
				Bundle* pBundle = Bundle::createObject();
				pBundle->newMessage(*Baseapp_onClientActiveTickMsgFind);
				pBundle->send(pNetworkInterface_);
			}
		}

		lastTickTime_ = getTimeSeconds();
	}
}


std::string KBEngineApp::serverErr(uint16 id)
{
	//FKServerErr e = serverErrs_.FindRef(id);
	return "";// std::string::Printf(KBETEXT("%s[%s]"), *e.name, *e.descr);
}

void KBEngineApp::Client_onAppActiveTickCB()
{
	lastTickCBTime_ = getTimeSeconds();
}

void KBEngineApp::hello()
{
	Bundle* pBundle = Bundle::createObject();
	//if (currserver_ == KBETEXT("loginapp"))
	//	pBundle->newMessage(Messages::getSingleton().messages[KBETEXT("Loginapp_hello")]);
	//else
		pBundle->newMessage(99, 1);

	//(*pBundle) << clientVersion_;
	//(*pBundle) << clientScriptVersion_;
	//pBundle->appendBlob(encryptedKey_);
	(*pBundle) << (uint32)0;
	pBundle->send(pNetworkInterface_);
}

void KBEngineApp::sendMsg(uint8 mainCmd, uint8 subCmd, const std::vector<uint8> buffer)
{
	printf("KBEngineApp::sendMsg maninCmd:%d, subCmd:%d \n", mainCmd, subCmd);
	Bundle* pBundle = Bundle::createObject();
	pBundle->newMessage(mainCmd, subCmd);
	pBundle->appendBlob(buffer);
	pBundle->send(pNetworkInterface_);
}

void KBEngineApp::Client_onHelloCB(MemoryStream& stream)
{
	if (currserver_ == KBETEXT("baseapp"))
	{
		onLogin_baseapp();
	}
	else
	{
		onLogin_loginapp();
	}
}

void KBEngineApp::Client_onVersionNotMatch(MemoryStream& stream)
{
	
}

void KBEngineApp::Client_onScriptVersionNotMatch(MemoryStream& stream)
{
	
}

void KBEngineApp::Client_onKicked(uint16 failedcode)
{
	
}

void KBEngineApp::onConnectCallback(std::string ip, uint16 port, bool success, int userdata)
{
	if (userdata == 0)
	{
		onConnectTo_loginapp_login_callback(ip, port, success);
	}
	else if (userdata == 1)
	{
		onConnectTo_loginapp_create_callback(ip, port, success);
	}
	else if (userdata == 2)
	{
		onConnectTo_baseapp_callback(ip, port, success);
	}
	else if (userdata == 3)
	{
		onReloginTo_baseapp_callback(ip, port, success);
	}
	else if (userdata == 4)
	{
		onConnectTo_resetpassword_callback(ip, port, success);
	}
	else if (userdata == 5)
	{
		//onConnectTo_resetpassword_callback(ip, port, success);
	}
	else
	{
		KBE_ASSERT(false);
	}
}

bool KBEngineApp::login(const std::string& username, const std::string& password, const std::vector<uint8>& datas)
{
	if (username.length() == 0)
	{
		ERROR_MSG("KBEngineApp::login(): username is empty!");
		return false;
	}

	if (password.length() == 0)
	{
		ERROR_MSG("KBEngineApp::login(): password is empty!");
		return false;
	}

	username_ = username;
	password_ = password;

	return login_loginapp(true);
}

bool KBEngineApp::login_loginapp(bool noconnect)
{
	if (noconnect)
	{
		reset();
		return pNetworkInterface_->connectTo(pArgs_->ip, pArgs_->port, this, 0);
	}
	else
	{
		INFO_MSG("KBEngineApp::login_loginapp(): send login! username=%s", username_.c_str());
		//Bundle* pBundle = Bundle::createObject();
		//pBundle->newMessage(Messages::getSingleton().messages[KBETEXT("Loginapp_login"]));
		//(*pBundle) << (uint8)pArgs_->clientType;
		//pBundle->appendBlob(clientdatas_);
		//(*pBundle) << username_;
		//(*pBundle) << password_;
		//pBundle->send(pNetworkInterface_);
	}
	return true;
}

void KBEngineApp::onConnectTo_loginapp_login_callback(std::string ip, uint16 port, bool success)
{
	if (!success)
	{
		//ERROR_MSG("KBEngineApp::onConnectTo_loginapp_login_callback(): connect %s:%d is error!", ip.c_str(), port);
		KBEvent::fire("onconnectfail", NULL);
		return;
	}

	currserver_ = KBETEXT("loginapp");
	KBEvent::fire("onconnect", NULL);
	//INFO_MSG("KBEngineApp::onConnectTo_loginapp_login_callback(): connect %s:%d is success!", ip.c_str(), port);

	//hello();
}

void KBEngineApp::onLogin_loginapp()
{
	lastTickCBTime_ = getTimeSeconds();


	if (currserver_ == KBETEXT("loginapp"))
	{
		

		//if (currstate_ == KBETEXT("login"))
		{
			login_loginapp(false);
		}
		
		//else if (currstate_ == KBETEXT("createAccount"))
		{
			createAccount_loginapp(false);
		}
	}
	else
	{
		{
			login_baseapp(false);
		}
	}
}

void KBEngineApp::Client_onLoginFailed(MemoryStream& stream)
{
	
}

void KBEngineApp::Client_onLoginSuccessfully(MemoryStream& stream)
{
	//std::string accountName;
	//stream >> accountName;
	//username_ = accountName;
	//stream >> baseappIP_;
	//stream >> baseappPort_;

	login_baseapp(true);
}

void KBEngineApp::login_baseapp(bool noconnect)
{
	if (noconnect)
	{
		pNetworkInterface_->destroy();
		pNetworkInterface_ = NULL;
		initNetwork();
		pNetworkInterface_->connectTo(baseappIP_, baseappPort_, this, 2);
	}
	else
	{
		//Bundle* pBundle = Bundle::createObject();
		//pBundle->newMessage(Messages::getSingleton().messages[KBETEXT("Baseapp_loginBaseapp"]));
		//(*pBundle) << username_;
		//(*pBundle) << password_;
		//pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onConnectTo_baseapp_callback(std::string ip, uint16 port, bool success)
{
	lastTickCBTime_ = getTimeSeconds();

	if (!success)
	{
		ERROR_MSG("KBEngineApp::onConnectTo_baseapp_callback(): connect %s:%d is error!", ip.c_str(), port);
		return;
	}

	currserver_ = KBETEXT("baseapp");
	//currstate_ = KBETEXT("");

	DEBUG_MSG("KBEngineApp::onConnectTo_baseapp_callback(): connect %s:%d is successfully!", ip.c_str(), port);

	hello();
}

void KBEngineApp::onLogin_baseapp()
{
	lastTickCBTime_ = getTimeSeconds();
}

void KBEngineApp::reloginBaseapp()
{
	if(pNetworkInterface_->valid())
		return;
	pNetworkInterface_->connectTo(baseappIP_, baseappPort_, this, 3);
}

void KBEngineApp::onReloginTo_baseapp_callback(std::string ip, uint16 port, bool success)
{
	if (!success)
	{
		ERROR_MSG("KBEngineApp::onReloginTo_baseapp_callback(): connect %s:%d is error!", ip.c_str(), port);
		return;
	}

	INFO_MSG("KBEngineApp::onReloginTo_baseapp_callback(): connect %s:%d is success!", ip.c_str(), port);

	//Bundle* pBundle = Bundle::createObject();
	//pBundle->newMessage(Messages::getSingleton().messages[KBETEXT("Baseapp_reloginBaseapp"]));
	//(*pBundle) << username_;
	//(*pBundle) << password_;
	//(*pBundle) << entity_uuid_;
	//(*pBundle) << entity_id_;
	//pBundle->send(pNetworkInterface_);

	lastTickCBTime_ = getTimeSeconds();
}

void KBEngineApp::Client_onLoginBaseappFailed(uint16 failedcode)
{
}

void KBEngineApp::Client_onReloginBaseappFailed(uint16 failedcode)
{
}

void KBEngineApp::Client_onReloginBaseappSuccessfully(MemoryStream& stream)
{
}

void KBEngineApp::resetPassword(const std::string& username)
{
	username_ = username;
	resetpassword_loginapp(true);
}

void KBEngineApp::resetpassword_loginapp(bool noconnect)
{
	if (noconnect)
	{
		reset();
		pNetworkInterface_->connectTo(pArgs_->ip, pArgs_->port, this, 4);
	}
	else
	{
		INFO_MSG("KBEngineApp::resetpassword_loginapp(): send resetpassword! username=%s", username_.c_str());
		//Bundle* pBundle = Bundle::createObject();
		//pBundle->newMessage(Messages::getSingleton().messages[KBETEXT("Loginapp_reqAccountResetPassword"]));
		//(*pBundle) << username_;
		//pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onOpenLoginapp_resetpassword()
{

}

void KBEngineApp::onConnectTo_resetpassword_callback(std::string ip, uint16 port, bool success)
{
	onOpenLoginapp_resetpassword();
}

void KBEngineApp::Client_onReqAccountResetPasswordCB(uint16 failcode)
{
	
}

bool KBEngineApp::createAccount(const std::string& username, const std::string& password, const std::vector<uint8>& datas)
{
	if (username.length() == 0)
	{
		ERROR_MSG("KBEngineApp::createAccount(): username is empty!");
		return false;
	}

	if (password.length() == 0)
	{
		ERROR_MSG("KBEngineApp::createAccount(): password is empty!");
		return false;
	}

	username_ = username;
	password_ = password;

	createAccount_loginapp(true);
	return true;
}

void KBEngineApp::createAccount_loginapp(bool noconnect)
{
	if (noconnect)
	{
		reset();
		pNetworkInterface_->connectTo(pArgs_->ip, pArgs_->port, this, 1);
	}
	else
	{
		INFO_MSG("KBEngineApp::createAccount_loginapp(): send create! username=%s", username_.c_str());
		//Bundle* pBundle = Bundle::createObject();
		//pBundle->newMessage(Messages::getSingleton().messages[KBETEXT("Loginapp_reqCreateAccount"]));
		//(*pBundle) << username_;
		//(*pBundle) << password_;
		//pBundle->appendBlob(clientdatas_);
		//pBundle->send(pNetworkInterface_);
	}
}

void KBEngineApp::onOpenLoginapp_createAccount()
{
	DEBUG_MSG("KBEngineApp::onOpenLoginapp_createAccount(): successfully!");

	currserver_ = KBETEXT("loginapp");
	lastTickCBTime_ = getTimeSeconds();
}

void KBEngineApp::onConnectTo_loginapp_create_callback(std::string ip, uint16 port, bool success)
{
	lastTickCBTime_ = getTimeSeconds();

	if (!success)
	{
		ERROR_MSG("KBEngineApp::onConnectTo_loginapp_create_callback(): connect %s:%d is error!", ip.c_str(), port);
		return;
	}

	INFO_MSG("KBEngineApp::onConnectTo_loginapp_create_callback(): connect %s:%d is success!", ip.c_str(), port);

	onOpenLoginapp_createAccount();
}

void KBEngineApp::Client_onCreateAccountResult(MemoryStream& stream)
{
	uint16 retcode;
	stream >> retcode;

	std::vector<uint8> datas;
	stream.readBlob(datas);
}

bool KBEngineApp::connectTo(const char* ip, uint16 port)
{
	reset();
	return pNetworkInterface_->connectTo(ip, port, this, 0);
}