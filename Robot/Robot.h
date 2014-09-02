#pragma once

class Robot
{
public:
    virtual bool Login(const CString &_room_url, UINT _robot_total) = 0;
    virtual void Logout() = 0;
    virtual UINT GetLoggedTotal() = 0;
};