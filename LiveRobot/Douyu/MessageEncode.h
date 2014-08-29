#pragma once
class MessageEncode
{
public:
    MessageEncode();
    ~MessageEncode();

    void AddItem(CStringA _key, CStringA _value);
    void AddItem(CStringA _key, int _value);

    CStringA GetString();

private:
    CStringA Scan(CStringA _str);

private:
    CStringA m_encode;
};
