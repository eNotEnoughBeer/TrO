#pragma once
#include "VirtualGridCtrl.h"
#include "MainLogic.h"
#include "Person.h"
#include "Relative.h"

// PersonDlg dialog
class _ID_NAME {
public:
	int id;
	CString name;
	_ID_NAME(int pId) {
		id = pId;
		name = _T("");
	}
};

class PersonDlg : public CDialog
{
	DECLARE_DYNAMIC(PersonDlg)

public:
	MainLogic* pLogic;
	Person* pPerson;
	std::vector<Relative> * rels;

	PersonDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~PersonDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PERSON_DLG };
#endif
private:
	std::vector<_ID_NAME> weaponArr;
	CString index2name(int index);
	int name2index(CString name);
	std::vector<_ID_NAME> parseDivision();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comboPosit;
	CComboBox m_comboRank;
	CComboBox m_comboDiviz;
	void FillComboPosit();
	void FillComboRank();
	void FillComboDiviz();
	CVirtualGridCtrl m_gridRel;
	CVirtualGridCtrl m_gridWep;
	void OnGetDispinfoGridRel(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGridRel(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResectRel(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnGetDispinfoGridWep(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnSetDispinfoGridWep(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnKeyDownGridResectWep(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnGetEditStyleGridWep(LPNMHDR lpNMHDR, LRESULT* pResult);
	void OnGetEditListGridWep(LPNMHDR lpNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedAddrel();
	afx_msg void OnBnClickedDelrel();
	afx_msg void OnBnClickedAddwep();
	afx_msg void OnBnClickedDelwep();
	afx_msg void OnBnClickedCready();
};
