#pragma once
#include "VirtualGridCtrl.h"
#include "MainLogic.h"

class PositDlg : public CDialog
{
	DECLARE_DYNAMIC(PositDlg)

public:
	MainLogic* pLogic;
	PositDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~PositDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POSIT_DLG };
#endif
private:
	CString index2name(int index);
	int name2index(CString name);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	std::vector<int> delIds;
	CVirtualGridCtrl m_grid;
	void OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResect(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnGetEditStyleGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnGetEditListGrid(LPNMHDR lpNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedOk();
};
