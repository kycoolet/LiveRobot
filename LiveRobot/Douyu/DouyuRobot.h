#pragma once

#include <boost/thread/thread.hpp>
#include <map>
#include <vector>

#include "Robot.h"
#include "../HPSocket/Src/TcpAgent.h"
#include "../HPSocket/Src/SocketInterface.h"
#include "../Common/Src/bufferptr.h"

struct SERVER
{
	CString host;
	USHORT port;
};

enum CONN_STATUS
{
    CS_ERROR = 0,
    CS_CONNECTED = 1,
    CS_LOGGED = 2,
    CS_CLOSE = 3
};

class DouyuRobot : public Robot, private CTcpAgentListener
{
public:
    DouyuRobot();
	~DouyuRobot();

    virtual bool Login(const CString &_room_url, UINT _robot_total);
	virtual void Logout();

    virtual UINT GetLoggedTotal();

private:
	virtual EnHandleResult OnConnect(CONNID dwConnID);
	virtual EnHandleResult OnReceive(CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(CONNID dwConnID);
	virtual EnHandleResult OnError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);

    CByteBufferPtr MakeLoginPackage(UINT _roomid);
	CByteBufferPtr MakeDouyuMessagePackage(const CStringA _msg);
    bool GetRoomArgs(const CString &_room_url, UINT *_room_id, std::vector<SERVER> *_server_list);
    bool ParseServerList(const CString &_server_code, std::vector<SERVER> *_server_list);
    void KeepLiveThread();
    void KillKeepliveThread();

private:
    CTcpAgent* m_agent;
	CByteBufferPtr m_login_package;
    std::vector<SERVER> m_server_list;
    boost::thread *m_keeplive_thread;
};

