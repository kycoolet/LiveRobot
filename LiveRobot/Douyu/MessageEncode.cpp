#include "stdafx.h"
#include "MessageEncode.h"



MessageEncode::MessageEncode()
{
    
}


MessageEncode::~MessageEncode()
{
}

/*
public function AddItem(_arg1:String, _arg2:String):void
{
var _local3 = "";
_local3 = (((_arg1)!=null) ? (this.scan_str(_arg1) + "@=") : "");
this.SttString = (this.SttString + ((_local3 + this.scan_str(_arg2)) + "/"));
}
*/

void MessageEncode::AddItem(CStringA _key, CStringA _value)
{
    CStringA encode_key;
    encode_key = _key.IsEmpty() ? "" : this->Scan(_key) + "@=";
    m_encode = m_encode + encode_key + this->Scan(_value) + "/";
}

/*
public function AddItem_int(_arg1:String, _arg2:Number):void
{
var _local3 = "";
_local3 = (((_arg1)!=null) ? (this.scan_str(_arg1) + "@=") : "");
this.SttString = (this.SttString + ((_local3 + this.scan_str(_arg2.toString())) + "/"));
}

*/

void MessageEncode::AddItem(CStringA _key, int _value)
{
    CStringA encode_key;
    encode_key = _key.IsEmpty() ? "" : this->Scan(_key) + "@=";
    CStringA value_text;
    value_text.Format("%i", _value);
    m_encode = m_encode + encode_key + this->Scan(value_text) + "/";
}

// private function scan_str(_arg1:String) :String
// {
//     var _local2 = "";
//     var _local3 : int;
//     while (_local3 < _arg1.length)
//     {
//         if (_arg1.charAt(_local3) == "/")
//         {
//             _local2 = (_local2 + "@S");
//         }
//         else
//         {
//             if (_arg1.charAt(_local3) == "@")
//             {
//                 _local2 = (_local2 + "@A");
//             }
//             else
//             {
//                 _local2 = (_local2 + _arg1.charAt(_local3));
//             };
//         };
//         _local3++;
//     };
//     return (_local2);
// }

CStringA MessageEncode::Scan(CStringA _str)
{
    CStringA temp;
    int scan_pos = 0;
    while (scan_pos < _str.GetLength())
    {
        if (_str.GetAt(scan_pos) == '/')
        {
            temp = temp + "@S";
        }
        else
        {
            if (_str.GetAt(scan_pos) == '@')
            {
                temp = temp + "@A";
            } 
            else
            {
                temp = temp + _str.GetAt(scan_pos);
            }
        }
        scan_pos++;
    }
    return temp;
}

CStringA MessageEncode::GetString()
{
    return m_encode;
}
