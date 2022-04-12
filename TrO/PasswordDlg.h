#pragma once
#include "MainLogic.h"
#include "Password.h"

// PasswordDlg dialog

class PasswordDlg : public CDialog
{
	DECLARE_DYNAMIC(PasswordDlg)

public:
	Password myPass;
	MainLogic * pAllData;
	PasswordDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~PasswordDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PASSWD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
