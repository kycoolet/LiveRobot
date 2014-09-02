#include "stdafx.h"
#include "helper.h"
#include <time.h>

CString GetLocalFormatTime(const CString &_fmt)
{
    time_t t = time(0);
    tm time_struct;
    localtime_s(&time_struct, &t);
    CString time_buff;
    _tcsftime(time_buff.GetBuffer(128), 128, _fmt, &time_struct);
    time_buff.ReleaseBuffer();
    return time_buff;
}
