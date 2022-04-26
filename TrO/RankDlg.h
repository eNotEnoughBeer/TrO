#pragma once
#include "VirtualGridCtrl.h"
#include "MainLogic.h"
// RankDlg dialog

class RankDlg : public CDialog
{
	DECLARE_DYNAMIC(RankDlg)

public:
	MainLogic* pLogic;
	RankDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~RankDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RANK_DLG };
#endif

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
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedOk();
};
