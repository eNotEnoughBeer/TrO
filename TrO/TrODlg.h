#pragma once
#include "VirtualGridCtrl.h"
#include "MainLogic.h"

#include "PositDlg.h"
#include "RankDlg.h"
#include "WeaponDlg.h"
#include "DivizionDlg.h"
#include "PersonDlg.h"
// CTrODlg dialog
class CTrODlg : public CDialogEx
{
// Construction
public:
	CMenu menu;
	CMenu popupmenu;
	MainLogic* pLogic;
	CTrODlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRO_DIALOG };
#endif

protected:
	void MenuHandler(UINT id);
	void MenuHandlerPerson(UINT id);
	void MenuHandlerExport(UINT id);
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	void addPosition();
	void addRank();
	void addWeapon();
	void addDivizion();
	void addPerson();
	void editPerson();
	void deletePerson();
	CString positionId2positionName(int id);
	CString positionId2rankMaxName(int id);
	CString rankId2rankName(int id);
	CString weaponId2fullName(int id);
	CString relatives2str(int personId);
	void totalExcelExpert();
	std::vector<ID_NAME> divizionSort();

public:
	CVirtualGridCtrl m_grid;
	void OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
};
