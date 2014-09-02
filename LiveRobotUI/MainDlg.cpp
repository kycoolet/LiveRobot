// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

#include <time.h>

#include "..\Robot\DouyuRobot.h"
#include "helper.h"

CMainDlg::CMainDlg() : m_robot(nullptr)
{

}

CMainDlg::~CMainDlg()
{
    if (m_robot)
    {
        delete m_robot;
    }
}


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}


LRESULT CMainDlg::OnLink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CString url;
    GetDlgItemText(IDC_EDIT_URL, url.GetBuffer(256), 256);
    url.ReleaseBuffer();
    if (url.IsEmpty())
    {
        GetDlgItem(IDC_EDIT_URL).SetFocus();
        AddLog(_T("请设置房间地址。"));
        return 0;
    }

    CString number_text;
    GetDlgItemText(IDC_EDIT_NUMBER, number_text.GetBuffer(256), 256);
    number_text.ReleaseBuffer();
    int number = _tstoi(number_text);
    if (number <= 0)
    {
        GetDlgItem(IDC_EDIT_NUMBER).SetFocus();
        AddLog(_T("请设置模拟人数。"));
        return 0;
    }

    if (nullptr == m_robot)
    {
        m_robot = new DouyuRobot;
    }
    if (m_robot->Login(url, number))
    {
        AddLog(_T("连接启动成功。"));
        GetDlgItem(IDC_EDIT_URL).EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_NUMBER).EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_LINK).EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_UNLINK).EnableWindow(TRUE);
        this->SetTimer(1, 2000);
    }
    else
    {
        DWORD err = GetLastError();
        if (err = DouyuRobot::ERR_GET_ROOM_ARGS_FAIL)
        {
            AddLog(_T("获取房间参数失败。"));
        }
    }
    return 0;
}

LRESULT CMainDlg::OnUnlink(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_robot)
    {
        delete m_robot;
        m_robot = nullptr;
        AddLog(_T("连接断开。"));
    }
    GetDlgItem(IDC_EDIT_URL).EnableWindow(TRUE);
    GetDlgItem(IDC_EDIT_NUMBER).EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_LINK).EnableWindow(TRUE);
    GetDlgItem(IDC_BUTTON_UNLINK).EnableWindow(FALSE);
    this->KillTimer(1);
    return 0;
}

void CMainDlg::AddLog(const CString &_log)
{
    static CEdit log_edit = GetDlgItem(IDC_EDIT_LOG);
    if (log_edit.IsWindow())
    {
        CString fmt_time = GetLocalFormatTime(_T("%X"));
        log_edit.AppendText(fmt_time + _T(" - ") + _log + _T("\n"));
    }
}

LRESULT CMainDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (1 == wParam)
    {
        UINT connect_total = m_robot->GetLoggedTotal();
        CString msg;
        msg.Format(_T("当前已模拟%d人进入房间。"), connect_total);
        this->AddLog(msg);
    }
    return 0;
}