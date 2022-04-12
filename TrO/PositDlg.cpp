#include "pch.h"
#include "TrO.h"
#include "PositDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(PositDlg, CDialog)
PositDlg::PositDlg(CWnd* pParent /*=nullptr*/): CDialog(IDD_POSIT_DLG, pParent){}
PositDlg::~PositDlg(){}

void PositDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID, m_grid);
}

BEGIN_MESSAGE_MAP(PositDlg, CDialog)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID, OnGetDispinfoGrid)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRID, OnSetDispinfoGrid)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRID, OnKeyDownGridResect)
	ON_NOTIFY(VGN_GETEDITSTYLE, IDC_GRID, OnGetEditStyleGrid)
	ON_NOTIFY(VGN_GETEDITLIST, IDC_GRID, OnGetEditListGrid)
	ON_BN_CLICKED(IDC_ADD, &PositDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DEL, &PositDlg::OnBnClickedDel)
	ON_BN_CLICKED(IDC_DONE, &PositDlg::OnBnClickedOk)
END_MESSAGE_MAP()


CString PositDlg::index2name(int index)
{
	CString res = _T("");
	for (int i = 0; i < (int)pLogic->pRanks.ranks.size(); i++) {
		if (index == pLogic->pRanks.ranks[i].id) {
			res = pLogic->pRanks.ranks[i].name;
			break;
		}
	}
	return res;
}

int PositDlg::name2index(CString name)
{
	int res = -1;
	for (int i = 0; i < (int)pLogic->pRanks.ranks.size(); i++) {
		if (name.Compare(pLogic->pRanks.ranks[i].name.GetString()) == 0) {
			res = pLogic->pRanks.ranks[i].id;
			break;
		}
	}
	return res;
}

BOOL PositDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	m_grid.AddColumn(_T(""), 275);
	m_grid.AddColumn(_T(""), 170);
	//CGridColumn* pColumn;
	CGridHeaderSection* pSection = m_grid.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Назва посади"));

	pSection = m_grid.GetHeaderSection(1, 0);
	pSection->SetCaption(_T("Військове звання"));
	pSection->SetAlignment(LVCFMT_CENTER);

	m_grid.GetHeader()->SetSectionHeight(24);
	m_grid.SetAllowEdit();
	m_grid.SetRowCount((int)pLogic->pPositions.positions.size());
	return TRUE;
}

void PositDlg::OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pPositions.positions.size() == 0)
		return;
	std::vector<Position>::const_iterator it = pLogic->pPositions.positions.begin();
	std::advance(it, pDispInfo->item.iRow);

	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:	// 
		wcscpy(szBuf, it->name.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 1:	// 
		wcscpy(szBuf, index2name(it->rankId).GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

void PositDlg::OnSetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pPositions.positions.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<Position>::iterator it; it = pLogic->pPositions.positions.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:	
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->name = szBuf;
		break;
	case 1:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->rankId = name2index(szBuf);
		break;
	}
	m_grid.InvalidateGrid();
}

void PositDlg::OnGetEditStyleGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pDispInfo->item.iColumn == 1)
		pDispInfo->item.style = gePickList;
}

void PositDlg::OnGetEditListGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	pDispInfo->item.pickList.RemoveAll();
	// тут сформировать массив строк "ТИП Серийный_номер"
	for (int i = 0; i < (int)pLogic->pRanks.ranks.size(); i++) {
		pDispInfo->item.pickList.Add(pLogic->pRanks.ranks[i].name.GetString());
	}

	*pResult = 0;
}

void PositDlg::OnBnClickedAdd()
{
	m_grid.PressReturn();
	CGridCell cell = m_grid.GetCellFocused();
	Position pNewOne(pLogic->ids.position_id++, _T(""),-1);
	if (m_grid.GetRowCount() == 0) {
		pLogic->pPositions.positions.push_back(pNewOne);
	}
	else {
		std::vector<Position>::iterator it = pLogic->pPositions.positions.begin();
		std::advance(it, cell.m_iRow);
		pLogic->pPositions.positions.insert(it, pNewOne);
	}
	m_grid.SetRowCount(m_grid.GetRowCount() + 1);
}

void PositDlg::OnKeyDownGridResect(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_grid.GetRowCount())return;
	CGridCell cell = m_grid.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_grid.GetRowCount() - 1)
	{
		Position pNewOne(pLogic->ids.position_id++, _T(""),-1);
		pLogic->pPositions.positions.push_back(pNewOne);
		m_grid.SetRowCount(m_grid.GetRowCount() + 1);
	}
}

void PositDlg::OnBnClickedDel()
{
	m_grid.PressReturn();
	if (pLogic->pPositions.positions.size() < 1){
		return;
	}

	std::vector<Position>::iterator it;	it = pLogic->pPositions.positions.begin();
	std::advance(it, m_grid.GetCellFocused().m_iRow);
	// проверка на использование в PERSON. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pPersons.persons.size();i++){
		if (it->id == pLogic->pPersons.persons[i].positionId) {
			canDelete = false;
			break;
		}
	}
	if (canDelete) {
		pLogic->pPositions.positions.erase(it);
		m_grid.SetRowCount(m_grid.GetRowCount() - 1);
		m_grid.Invalidate();
	}
	else {
		AfxMessageBox(_T("Дана посада закрiплена за одним з бiйцiв."));
	}
}

void PositDlg::OnBnClickedOk()
{
	m_grid.PressReturn();
	CDialog::OnOK();
}
