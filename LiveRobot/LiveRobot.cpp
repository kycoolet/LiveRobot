// LiveRobot.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>

#include "Robot.h"
#include "Douyu/DouyuRobot.h"


bool g_exit = false;

using namespace std;

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc < 4)
    {
        cout << "参数说明:" << endl;
        cout << "    LiveRobot 平台ID(douyu) 房间ID 模拟人数" << endl;
        return 0;
    }

    CString platform_id = argv[1];
    CString room_url = argv[2];
    UINT robot_total = _ttoi(argv[3]);
    Robot* robot = nullptr;

    if (platform_id == _T("douyu"))
    {
        robot = new DouyuRobot;
    }
    
    if (nullptr != robot && !room_url.IsEmpty() && robot_total > 0)
    {
        SetConsoleCtrlHandler(HandlerRoutine, TRUE);
        cout << "按Ctrl + C即可终止执行" << endl;

        if (robot->Login(room_url, robot_total))
        {
            DWORD last_output_time = 0, now = 0;
            while (false == g_exit)
            {
                Sleep(100);
                now = GetTickCount();
                if (now - last_output_time > 3000)
                {
                    last_output_time = now;
                    cout << "当前已登录机器人数：" << robot->GetLoggedTotal() << endl;
                }
            }
            cout << "用户终止..." << endl;
            robot->Logout();
        }
        else
        {
            cout << "登录失败！" << endl;
        }
        delete robot;
    }
    else
    {
        cout << "参数有误." << endl;
    }

    system("pause");
	return 0;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
    g_exit = true;
    return TRUE;
}