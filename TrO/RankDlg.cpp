#include "pch.h"
#include "TrO.h"
#include "RankDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(RankDlg, CDialog)

RankDlg::RankDlg(CWnd* pParent /*=nullptr*/): CDialog(IDD_RANK_DLG, pParent){}
RankDlg::~RankDlg(){}

void RankDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID, m_grid);
}

BEGIN_MESSAGE_MAP(RankDlg, CDialog)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID, OnGetDispinfoGrid)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRID, OnSetDispinfoGrid)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRID, OnKeyDownGridResect)
	ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DEL, OnBnClickedDel)
	ON_BN_CLICKED(IDC_DONE, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL RankDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	int iColumn = m_grid.AddColumn(_T(""), 275);
	//CGridColumn* pColumn;
	CGridHeaderSection* pSection = m_grid.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Вiйськове звання"));
	m_grid.GetHeader()->SetSectionHeight(24);
	m_grid.SetAllowEdit();
	m_grid.SetRowCount((int)pLogic->pRanks.ranks.size());
	return TRUE;
}

void RankDlg::OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pRanks.ranks.size() == 0)
		return;
	std::vector<Rank>::const_iterator it = pLogic->pRanks.ranks.begin();
	std::advance(it, pDispInfo->item.iRow);

	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:	// запрос
		wcscpy(szBuf, it->name.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

void RankDlg::OnSetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pRanks.ranks.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<Rank>::iterator it; it = pLogic->pRanks.ranks.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->name = szBuf;
		break;
	}
	m_grid.InvalidateGrid();
}

void RankDlg::OnBnClickedAdd()
{
	m_grid.PressReturn();
	CGridCell cell = m_grid.GetCellFocused();
	Rank pNewOne(pLogic->ids.rank_id++, _T(""));
	if (m_grid.GetRowCount() == 0) {
		pLogic->pRanks.ranks.push_back(pNewOne);
	}
	else {
		std::vector<Rank>::iterator it = pLogic->pRanks.ranks.begin();
		std::advance(it, cell.m_iRow);
		pLogic->pRanks.ranks.insert(it, pNewOne);
	}
	m_grid.SetRowCount(m_grid.GetRowCount() + 1);
}

void RankDlg::OnKeyDownGridResect(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_grid.GetRowCount())return;
	CGridCell cell = m_grid.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_grid.GetRowCount() - 1)
	{
		Rank pNewOne(pLogic->ids.rank_id++, _T(""));
		pLogic->pRanks.ranks.push_back(pNewOne);
		m_grid.SetRowCount(m_grid.GetRowCount() + 1);
	}
}

void RankDlg::OnBnClickedDel()
{
	m_grid.PressReturn();
	if (pLogic->pRanks.ranks.size() < 1) {
		return;
	}

	std::vector<Rank>::iterator it;	it = pLogic->pRanks.ranks.begin();
	std::advance(it, m_grid.GetCellFocused().m_iRow);
	// проверка на использование в PERSON. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pPersons.persons.size(); i++) {
		if (it->id == pLogic->pPersons.persons[i].rankId) {
			canDelete = false;
			break;
		}
	}
	for (int i = 0; i < (int)pLogic->pPositions.positions.size(); i++) {
		if (it->id == pLogic->pPositions.positions[i].rankId) {
			canDelete = false;
			break;
		}
	}
	if (canDelete) {
		pLogic->pRanks.ranks.erase(it);
		m_grid.SetRowCount(m_grid.GetRowCount() - 1);
		m_grid.Invalidate();
	}
	else {
		AfxMessageBox(_T("Звання закрiплене за одним з бiйцiв або за посадою."));
	}
}

void RankDlg::OnBnClickedOk()
{
	m_grid.PressReturn();
	CDialog::OnOK();
}