#include "stdafx.h"
#include "DouyuRobot.h"

#include <iostream>
#include <time.h>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "../Common/WinHttpClient.h"
#include "../Common/urlcodeing.h"
#include "MessageEncode.h"

using namespace std;

DouyuRobot::DouyuRobot() : m_keeplive_thread(nullptr)
{
    m_agent = new CTcpAgent(this);
	srand((int)time(0));
}


DouyuRobot::~DouyuRobot()
{
	Logout();
    delete m_agent;
}

bool DouyuRobot::Login(const CString &_room_url, UINT _robot_total)
{
	if (!_robot_total) return false;
    UINT room_id = 0;
    if (!GetRoomArgs(_room_url, &room_id, &m_server_list)) return false;
    std::cout << "获取房间信息完成……" << std::endl;
    m_login_package = MakeLoginPackage(room_id);

	m_agent->SetFreeSocketObjPool(_robot_total / 3);
	m_agent->SetFreeSocketObjHold(_robot_total);
	m_agent->SetFreeBufferObjPool(_robot_total);
	m_agent->SetFreeBufferObjHold(_robot_total * 2);
	m_agent->SetReuseAddress(TRUE);
    std::cout << "设置连接池完毕……" << std::endl;

    std::cout << std::endl << "-----------------------开始连接-----------------------" << std::endl;
	SERVER *server_info = nullptr;

	if (m_agent->Start())
	{
        for (UINT i = 0; i < _robot_total; i++)
        {
            server_info = &m_server_list[rand() % (m_server_list.size())];
            if (!m_agent->Connect(server_info->host, server_info->port))
            {
                std::cout << "连接失败……错误代码：" << GetLastError() << "，当前是第" << i << "个连接" << std::endl;
                ATLTRACE(_T("连接失败\n"));
            }
        }
	}
    else
    {
        std::cout << "客户端Start失败……" << std::endl;
    }

    KillKeepliveThread();
    m_keeplive_thread = new boost::thread(boost::bind(&DouyuRobot::KeepLiveThread, this));
	return true;
}

void DouyuRobot::Logout()
{
    KillKeepliveThread();
	DWORD conn_count = m_agent->GetConnectionCount();
	if (conn_count > 0)
	{
		CONNID *ids = new CONNID[conn_count];
		if (m_agent->GetAllConnectionIDs(ids, conn_count))
		{
			MessageEncode encode;
			encode.AddItem("type", "logout");
			CByteBufferPtr msg_package = MakeDouyuMessagePackage(encode.GetString());
			for (DWORD i = 0; i < conn_count; i++)
			{
				m_agent->Send(ids[i], msg_package, msg_package.Size());
			}
		}
	}
	
	m_agent->Stop();
}

CByteBufferPtr DouyuRobot::MakeDouyuMessagePackage(const CStringA _msg)
{
	struct HEAD
	{
		int len;
		short unknow_short;
		char unknow_char1;
		char unknow_char2;
	};

	CByteBufferPtr body;
	body.Cat(reinterpret_cast<const unsigned char*>(_msg.GetString()), _msg.GetLength() + 1);

	CByteBufferPtr head;
	int len = body.Size() + sizeof(HEAD);
	HEAD head_struct = { len, 689, 0, 0 };
	head.Cat(reinterpret_cast<const unsigned char*>(&head_struct), sizeof(head_struct));

	CByteBufferPtr buff;
	buff.Cat(reinterpret_cast<const unsigned char*>(&len), 4);
	buff.Cat(head);
	buff.Cat(body);

	return buff;
}

CByteBufferPtr DouyuRobot::MakeLoginPackage(UINT _roomid)
{
	CStringA room_id_str;
	room_id_str.Format("%u", _roomid);
	MessageEncode encode;
	encode.AddItem("type", "loginreq");
	encode.AddItem("username", "");
	encode.AddItem("password", "");
	encode.AddItem("roompass", "");
	encode.AddItem("roomid", room_id_str);
	return MakeDouyuMessagePackage(encode.GetString());
}

EnHandleResult DouyuRobot::OnConnect(CONNID dwConnID)
{
    m_agent->SetConnectionExtra(dwConnID, reinterpret_cast<PVOID>(CS_CONNECTED));
	//ATLTRACE(_T("[%d] 连接成功\n"), dwConnID);
	m_agent->Send(dwConnID, m_login_package, m_login_package.Size());
    //m_agent->SetConnectionExtra(dwConnID, reinterpret_cast<PVOID>(CS_LOGGED));
	return HR_OK;
}

EnHandleResult DouyuRobot::OnReceive(CONNID dwConnID, const BYTE* pData, int iLength)
{
    int len = *(reinterpret_cast<const int*>(pData));
    if (len > 12 && len + 4 <= iLength)
    {
        int message_len = len - 8;
        CStringA msg(reinterpret_cast<const char*>(&pData[12]), message_len);
        //ATLTRACE("收到数据: %s\n", msg);
        if ("type@=loginres/" == msg.Left(15))
        {
            m_agent->SetConnectionExtra(dwConnID, reinterpret_cast<PVOID>(CS_LOGGED));
        }
    }
	return HR_OK;
}

EnHandleResult DouyuRobot::OnClose(CONNID dwConnID)
{
    m_agent->SetConnectionExtra(dwConnID, reinterpret_cast<PVOID>(CS_CLOSE));
	//ATLTRACE(_T("[%d] 被断开\n"), dwConnID);
    SERVER *server_info = &m_server_list[rand() % (m_server_list.size())];
    m_agent->Connect(server_info->host, server_info->port);
	return HR_OK;
}

EnHandleResult DouyuRobot::OnError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
    m_agent->SetConnectionExtra(dwConnID, reinterpret_cast<PVOID>(CS_ERROR));
    //cout << "[" << dwConnID << "] 发生错误 code = " << iErrorCode << endl;
    SERVER *server_info = &m_server_list[rand() % (m_server_list.size())];
    m_agent->Connect(server_info->host, server_info->port);
	ATLTRACE(_T("[%d] 发生错误 code = %d\n"), dwConnID, iErrorCode);
	return HR_OK;
}

UINT DouyuRobot::GetLoggedTotal()
{
    UINT total = 0;
    DWORD conn_total = m_agent->GetConnectionCount();
    CONNID* conn_arry = new CONNID[conn_total];
    ATLTRACE(_T("当前连接数: %d\n"), conn_total);
    m_agent->GetAllConnectionIDs(conn_arry, conn_total);
    UINT conn_status = 0;
    for (UINT i = 0; i < conn_total; i++)
    {
        if (m_agent->GetConnectionExtra(conn_arry[i], reinterpret_cast<PVOID*>(&conn_status)))
        {
            if (conn_status == CS_LOGGED)
            {
                total++;
            }
        }
        
    }
    return total;
}

void DouyuRobot::KeepLiveThread()
{
    try
    {
        while (true)
        {
            boost::this_thread::sleep(boost::posix_time::seconds(30));
            MessageEncode encode;
            encode.AddItem("type", "keeplive");
            encode.AddItem("tick", rand() % 101);
            CByteBufferPtr keeplive_package = this->MakeDouyuMessagePackage(encode.GetString());
            DWORD conn_total = m_agent->GetConnectionCount();
            CONNID* conn_arry = new CONNID[conn_total];
            m_agent->GetAllConnectionIDs(conn_arry, conn_total);
            UINT conn_status = 0;
            for (UINT i = 0; i < conn_total; i++)
            {
                if (m_agent->GetConnectionExtra(conn_arry[i], reinterpret_cast<PVOID*>(&conn_status)))
                {
                    if (CS_LOGGED == conn_status)
                    {
                        m_agent->Send(conn_arry[i], keeplive_package, keeplive_package.Size());
                    }
                }
                boost::this_thread::interruption_point();
            }
            ATLTRACE(_T("KeepLiveThread 完成\n"));
        }
    }
    catch (boost::thread_interrupted &)
    {
        ATLTRACE(_T("KeepLive捕获到thread_interrupted异常\n"));
    }
}

void DouyuRobot::KillKeepliveThread()
{
    if (m_keeplive_thread)
    {
        m_keeplive_thread->interrupt();
        m_keeplive_thread->join();
        delete m_keeplive_thread;
        m_keeplive_thread = nullptr;
    }
}

bool DouyuRobot::GetRoomArgs(const CString &_room_url, UINT *_room_id, std::vector<SERVER> *_server_list)
{
    WinHttpClient http_client(_room_url.GetString());
    if (!http_client.SendHttpRequest()) return 0;
    std::cout << "访问房间页面成功" << std::endl;
    CString room_html = http_client.GetHttpResponse().c_str();
    if (room_html.IsEmpty()) return false;
    std::cout << "获取页面内容成功" << std::endl;

    boost::wcmatch reg_ret;
    boost::wregex roomid_reg(LR"({"room_id":(\d+?),)");
    if (!boost::regex_search(room_html.GetString(), reg_ret, roomid_reg)) return false;
    *_room_id = boost::lexical_cast<UINT>(reg_ret[1].str());
    std::cout << "获取房间ID成功，房间ID：" << *_room_id << std::endl;
    
    boost::wregex server_reg(LR"("server_config":"(.+?)\")");
    if (!boost::regex_search(room_html.GetString(), reg_ret, server_reg)) return false;
    CString server_code = reg_ret[1].str().c_str();
    std::cout << "服务器代码成功，服务器代码：" << boost::locale::conv::from_utf(server_code.GetString(), "gbk") << std::endl;
    if (!ParseServerList(server_code, _server_list)) return false;
    std::cout << "解析服务器列表成功，服务器数量：" << m_server_list.size() << std::endl;
    return true;
}

bool DouyuRobot::ParseServerList(const CString &_server_code, std::vector<SERVER> *_server_list)
{
    strCoding urlencode;
    string server_str = urlencode.UrlUTF8Decode(boost::locale::conv::from_utf(_server_code.GetString(), "gbk"));

    try
    {
        boost::property_tree::ptree server_tree;
        std::stringstream server_stream(server_str);
        boost::property_tree::read_json(server_stream, server_tree);

        CString server_ip;
        USHORT server_port = 0;
        _server_list->clear();
        for (auto it = server_tree.begin(); it != server_tree.end(); it++)
        {
            server_ip = boost::locale::conv::to_utf<wchar_t>(it->second.get<string>("ip"), "gbk").c_str();
            server_port = it->second.get<USHORT>("port");
            if (!server_ip.IsEmpty() && server_port > 0)
            {
                _server_list->push_back({ server_ip, server_port });
            }
        }
    }
    catch (...)
    {
        std::cout << "解析服务器列表失败！" << std::endl;
        return false;
    }
    return false == _server_list->empty();
}

