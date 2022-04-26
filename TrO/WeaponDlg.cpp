#include "pch.h"
#include "TrO.h"
#include "WeaponDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(WeaponDlg, CDialog)

WeaponDlg::WeaponDlg(CWnd* pParent /*=nullptr*/): CDialog(IDD_WEAPON_DLG, pParent){}
WeaponDlg::~WeaponDlg(){}

void WeaponDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID, m_grid);
	DDX_Control(pDX, IDC_GRIDNUM, m_gridNum);
}


BEGIN_MESSAGE_MAP(WeaponDlg, CDialog)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID, OnGetDispinfoGrid)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRID, OnSetDispinfoGrid)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRID, OnKeyDownGridResect)
	ON_NOTIFY(VGN_CHANGE, IDC_GRID, OnChangeGrid)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRIDNUM, OnGetDispinfoGridNum)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRIDNUM, OnSetDispinfoGridNum)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRIDNUM, OnKeyDownGridResectNum)
	ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DEL, OnBnClickedDel)
	ON_BN_CLICKED(IDC_DONE, OnBnClickedOk)
	ON_BN_CLICKED(IDC_DELNUM, &WeaponDlg::OnBnClickedDelnum)
	ON_BN_CLICKED(IDC_ADDNUM, &WeaponDlg::OnBnClickedAddnum)
END_MESSAGE_MAP()

BOOL WeaponDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	int iColumn = m_grid.AddColumn(_T(""), 185);
	CGridHeaderSection* pSection = m_grid.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Тип озброєння"));
	m_grid.GetHeader()->SetSectionHeight(24);
	m_grid.SetAllowEdit();
	m_grid.SetRowCount((int)pLogic->pWeapons.pWeapons.size());
	m_grid.SetAlwaysSelected();

	iColumn = m_gridNum.AddColumn(_T(""), 215);
	pSection = m_gridNum.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Серiйний номер"));
	m_gridNum.GetHeader()->SetSectionHeight(24);
	m_gridNum.SetAllowEdit();
	int count = pLogic->pWeapons.pWeapons.empty() ? 0 : (int)pLogic->pWeapons.pWeapons[0].pWeaponNumbers.size();
	m_gridNum.SetRowCount(count);
	m_gridNum.SetAlwaysSelected();
	delIdsTypes.clear();
	delIdsNums.clear();
	return TRUE;
}

void WeaponDlg::OnChangeGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	std::vector<WeaponType>::const_iterator it = pLogic->pWeapons.pWeapons.begin();
	std::advance(it, pDispInfo->item.iRow);
	m_gridNum.SetRowCount((int)it->pWeaponNumbers.size());
	m_grid.InvalidateGrid();
	m_gridNum.InvalidateGrid();
}

void WeaponDlg::OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pWeapons.pWeapons.size() == 0)
		return;
	std::vector<WeaponType>::const_iterator it = pLogic->pWeapons.pWeapons.begin();
	std::advance(it, pDispInfo->item.iRow);

	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:	
		wcscpy(szBuf, it->name.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

void WeaponDlg::OnSetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pWeapons.pWeapons.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<WeaponType>::iterator it; it = pLogic->pWeapons.pWeapons.begin();
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

void WeaponDlg::OnBnClickedAdd()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	CGridCell cell = m_grid.GetCellFocused();
	WeaponType pNewOne(-1, _T(""));
	if (m_grid.GetRowCount() == 0) {
		pLogic->pWeapons.pWeapons.push_back(pNewOne);
	}
	else {
		std::vector<WeaponType>::iterator it = pLogic->pWeapons.pWeapons.begin();
		std::advance(it, cell.m_iRow);
		pLogic->pWeapons.pWeapons.insert(it, pNewOne);
	}
	m_grid.SetRowCount(m_grid.GetRowCount() + 1);
	m_gridNum.SetRowCount((int)pNewOne.pWeaponNumbers.size());
	m_gridNum.InvalidateGrid();
}

void WeaponDlg::OnKeyDownGridResect(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_grid.GetRowCount())return;
	CGridCell cell = m_grid.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_grid.GetRowCount() - 1)
	{
		WeaponType pNewOne(-1, _T(""));
		pLogic->pWeapons.pWeapons.push_back(pNewOne);
		m_grid.SetRowCount(m_grid.GetRowCount() + 1);
		m_gridNum.SetRowCount((int)pNewOne.pWeaponNumbers.size());
		m_gridNum.InvalidateGrid();
	}
}

void WeaponDlg::OnBnClickedDel()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	if (pLogic->pWeapons.pWeapons.size() < 1) {
		return;
	}

	std::vector<WeaponType>::iterator it;	it = pLogic->pWeapons.pWeapons.begin();
	std::advance(it, m_grid.GetCellFocused().m_iRow);
	// проверка на наличие табельнх номеров, если пусто - можно удалить
	bool canDelete = true;
	if (!it->pWeaponNumbers.empty()) {
		canDelete = false;
	}
	
	if (canDelete) {
		delIdsTypes.push_back(it->id);
		pLogic->pWeapons.pWeapons.erase(it);
		m_grid.SetRowCount(m_grid.GetRowCount() - 1);
		m_grid.InvalidateGrid();
		int count = pLogic->pWeapons.pWeapons.empty() ? 0 : (int)pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.size();
		m_gridNum.SetRowCount(count);
		m_gridNum.InvalidateGrid();
	}
	else {
		AfxMessageBox(_T("За даним типом озброєння закрiплено кiлька одиниць зброї."));
	}
}

void WeaponDlg::OnGetDispinfoGridNum(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if ((int)pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.size() == 0)
		return;
	std::vector<WeaponNumber>::const_iterator it = pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.begin();
	std::advance(it, pDispInfo->item.iRow);

	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:	
		wcscpy(szBuf, it->name.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

void WeaponDlg::OnSetDispinfoGridNum(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if ((int)pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<WeaponNumber>::iterator it; it = pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.begin();
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

void WeaponDlg::OnKeyDownGridResectNum(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_gridNum.GetRowCount())return;
	CGridCell cell = m_gridNum.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_gridNum.GetRowCount() - 1)
	{
		WeaponNumber pNewOne(-1, _T(""));
		pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.push_back(pNewOne);
		m_gridNum.SetRowCount(m_gridNum.GetRowCount() + 1);
	}
}

void WeaponDlg::OnBnClickedOk()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();

	int res = pLogic->pWeapons.WriteSQL(pLogic->encriptionKey);
	res += pLogic->pWeapons.DeleteSQLType(delIdsTypes);
	res += pLogic->pWeapons.DeleteSQLNumber(delIdsNums);
	if (res > 0) {
		AfxMessageBox(_T("Виникли проблеми із каталогом зброї"));
	}
	
	pLogic->pWeapons.ReadSQL(pLogic->encriptionKey);
	CDialog::OnOK();
}

void WeaponDlg::OnBnClickedAddnum()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	if (pLogic->pWeapons.pWeapons.empty()) {
		AfxMessageBox(_T("Спочатку потрібно додати тип озброєння"));
		return;
	}
	CGridCell cell = m_gridNum.GetCellFocused();
	std::vector<WeaponType>::iterator itType;	itType = pLogic->pWeapons.pWeapons.begin();
	std::advance(itType, m_grid.GetCellFocused().m_iRow);

	WeaponNumber pNewOne(-1, _T(""));
	if (m_gridNum.GetRowCount() == 0) {
		itType->pWeaponNumbers.push_back(pNewOne);
	}
	else {
		std::vector<WeaponNumber>::iterator it = pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.begin();
		std::advance(it, cell.m_iRow);
		pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.insert(it, pNewOne);
	}
	m_gridNum.SetRowCount(m_gridNum.GetRowCount() + 1);
}

void WeaponDlg::OnBnClickedDelnum()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	int count = pLogic->pWeapons.pWeapons.empty() ? 0 : (int)pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.size();
	if (count < 1) {
		return;
	}

	std::vector<WeaponNumber>::iterator it;	it = pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.begin();
	std::advance(it, m_gridNum.GetCellFocused().m_iRow);
	// проверка на использование в PERSON. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pPersons.persons.size(); i++) {
		for (int j = 0; j < (int)pLogic->pPersons.persons[i].weapons.size(); j++) {
			if (it->id == pLogic->pPersons.persons[i].weapons[j]) {
				canDelete = false;
				break;
			}
		}
	}
	if (canDelete) {
		delIdsNums.push_back(it->id);
		pLogic->pWeapons.pWeapons[m_grid.GetCellFocused().m_iRow].pWeaponNumbers.erase(it);
		m_gridNum.SetRowCount(m_gridNum.GetRowCount() - 1);
		m_gridNum.Invalidate();
	}
	else {
		AfxMessageBox(_T("Зброя закріплена за одним з бійців."));
	}
}

