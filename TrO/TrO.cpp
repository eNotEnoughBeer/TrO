#include "pch.h"
#include "framework.h"
#include "TrO.h"
#include "TrODlg.h"
#include "PasswordDlg.h"
#include "MainLogic.h"

//#include "PositDlg.h"
//#include "RankDlg.h"
//#include "WeaponDlg.h"
//#include "DivizionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTrOApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CTrOApp::CTrOApp(){}
CTrOApp theApp;

BOOL CTrOApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	CShellManager *pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	MainLogic pData;
	PasswordDlg dlg;
	dlg.pAllData = &pData;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK){
		// ��������� ��� �� � ����������
		pData.connectXLS();
		pData.Read();
		CTrODlg dlg;
		dlg.pLogic = &pData;
		m_pMainWnd = &dlg;
		nResponse = dlg.DoModal();
		// �������� ������ � �����, ���� ��������� ������ OK
		//if (IDYES != ::MessageBox(0,_T("������� ���i � ��?"),_T("�����"),MB_YESNO))
		pData.Write();
		pData.freeXLS();
	}
	else if (nResponse == IDCANCEL){
		//...
	}

	if (pShellManager != nullptr){
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

