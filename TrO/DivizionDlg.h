#pragma once
#include "VirtualGridCtrl.h"
#include "MainLogic.h"
#include "Division.h"

class ID_NAME {
public:
	int id;
	CString name;
	ID_NAME(int pId, CString pName) {
		id = pId;
		name = pName;
	}
};

class DivizionDlg : public CDialog
{
	DECLARE_DYNAMIC(DivizionDlg)

public:
	MainLogic* pLogic;
	std::vector<ID_NAME> rota;
	std::vector<ID_NAME> vzvod;
	std::vector<ID_NAME> otdel;
	void parseDivision(int rotaId, int vzvodId);
	DivizionDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~DivizionDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIVIZION_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CVirtualGridCtrl m_gridR;
	void OnGetDispinfoGridR(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGridR(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResectR(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnChangeGridR(LPNMHDR lpNMHDR, LRESULT* pResult);
	CVirtualGridCtrl m_gridV;
	void OnGetDispinfoGridV(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGridV(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResectV(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnChangeGridV(LPNMHDR lpNMHDR, LRESULT* pResult);
	CVirtualGridCtrl m_gridO;
	void OnGetDispinfoGridO(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGridO(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResectO(LPNMHDR lpNMHDR, LRESULT* pResult);

	afx_msg void OnBnClickedDone2();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedAddv();
	afx_msg void OnBnClickedDelv();
	afx_msg void OnBnClickedAddo();
	afx_msg void OnBnClickedDelo();
};
