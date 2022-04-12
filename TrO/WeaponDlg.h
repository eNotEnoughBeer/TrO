#pragma once
#include <atlstr.h>
#include "VirtualGridCtrl.h"
#include "MainLogic.h"
#include "WeaponNum.h"

class WeaponDlg : public CDialog
{
	DECLARE_DYNAMIC(WeaponDlg)

public:
	MainLogic* pLogic;
	WeaponNums pTmpNums;
	WeaponDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~WeaponDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEAPON_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CVirtualGridCtrl m_grid;
	CVirtualGridCtrl m_gridNum;
	void OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResect(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnChangeGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedOk();
	void OnGetDispinfoGridNum(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGridNum(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResectNum(LPNMHDR lpNMHDR, LRESULT* pResult);

	int WeaponQuantityForType(int typeId);
	afx_msg void OnBnClickedDelnum();
	afx_msg void OnBnClickedAddnum();
};
