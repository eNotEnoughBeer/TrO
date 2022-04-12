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
	m_grid.SetRowCount((int)pLogic->pWeaponTypes.weaponTypes.size());
	m_grid.SetAlwaysSelected();

	iColumn = m_gridNum.AddColumn(_T(""), 215);
	pSection = m_gridNum.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Серiйний номер"));
	m_gridNum.GetHeader()->SetSectionHeight(24);
	m_gridNum.SetAllowEdit();
	int typeId = pLogic->pWeaponTypes.weaponTypes.empty() ? -1 : pLogic->pWeaponTypes.weaponTypes[0].id;
	m_gridNum.SetRowCount(WeaponQuantityForType(typeId));
	m_gridNum.SetAlwaysSelected();
	return TRUE;
}

int WeaponDlg::WeaponQuantityForType(int typeId)
{
	pTmpNums.weaponNums.clear();
	if (typeId == -1) return 0;
	int count = 0;
	for (int i = 0; i < (int)pLogic->pWeaponNums.weaponNums.size(); i++) {
		if (pLogic->pWeaponNums.weaponNums[i].typeId == typeId) {
			pTmpNums.weaponNums.push_back(pLogic->pWeaponNums.weaponNums[i]);
			count++;
		}
	}
	return count;
}

void WeaponDlg::OnChangeGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	std::vector<WeaponType>::const_iterator it = pLogic->pWeaponTypes.weaponTypes.begin();
	std::advance(it, pDispInfo->item.iRow);
	m_gridNum.SetRowCount(WeaponQuantityForType(it->id));
	m_grid.InvalidateGrid();
	m_gridNum.InvalidateGrid();
}

void WeaponDlg::OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pWeaponTypes.weaponTypes.size() == 0)
		return;
	std::vector<WeaponType>::const_iterator it = pLogic->pWeaponTypes.weaponTypes.begin();
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

void WeaponDlg::OnSetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pWeaponTypes.weaponTypes.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<WeaponType>::iterator it; it = pLogic->pWeaponTypes.weaponTypes.begin();
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
	WeaponType pNewOne(pLogic->ids.weptype_id++, _T(""));
	if (m_grid.GetRowCount() == 0) {
		pLogic->pWeaponTypes.weaponTypes.push_back(pNewOne);
	}
	else {
		std::vector<WeaponType>::iterator it = pLogic->pWeaponTypes.weaponTypes.begin();
		std::advance(it, cell.m_iRow);
		pLogic->pWeaponTypes.weaponTypes.insert(it, pNewOne);
	}
	m_grid.SetRowCount(m_grid.GetRowCount() + 1);
	m_gridNum.SetRowCount(WeaponQuantityForType(pNewOne.id));
	m_gridNum.InvalidateGrid();
}

void WeaponDlg::OnKeyDownGridResect(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_grid.GetRowCount())return;
	CGridCell cell = m_grid.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_grid.GetRowCount() - 1)
	{
		WeaponType pNewOne(pLogic->ids.weptype_id++, _T(""));
		pLogic->pWeaponTypes.weaponTypes.push_back(pNewOne);
		m_grid.SetRowCount(m_grid.GetRowCount() + 1);
		m_gridNum.SetRowCount(WeaponQuantityForType(pNewOne.id));
		m_gridNum.InvalidateGrid();
	}
}

void WeaponDlg::OnBnClickedDel()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	if (pLogic->pWeaponTypes.weaponTypes.size() < 1) {
		return;
	}

	std::vector<WeaponType>::iterator it;	it = pLogic->pWeaponTypes.weaponTypes.begin();
	std::advance(it, m_grid.GetCellFocused().m_iRow);
	// проверка на использование в WEPNUM. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pWeaponNums.weaponNums.size(); i++) {
		if (it->id == pLogic->pWeaponNums.weaponNums[i].typeId) {
			canDelete = false;
			break;
		}
	}
	if (canDelete) {
		pLogic->pWeaponTypes.weaponTypes.erase(it);
		m_grid.SetRowCount(m_grid.GetRowCount() - 1);
		m_grid.InvalidateGrid();
		m_gridNum.SetRowCount(WeaponQuantityForType(pLogic->pWeaponTypes.weaponTypes[m_grid.GetCellFocused().m_iRow].id));
		m_gridNum.InvalidateGrid();
	}
	else {
		AfxMessageBox(_T("За даним типом озброєння закрiплено кiлька одиниць зброї."));
	}
}

void WeaponDlg::OnGetDispinfoGridNum(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pTmpNums.weaponNums.size() == 0)
		return;
	std::vector<WeaponNum>::const_iterator it = pTmpNums.weaponNums.begin();
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

void WeaponDlg::OnSetDispinfoGridNum(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pTmpNums.weaponNums.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<WeaponNum>::iterator it; it = pTmpNums.weaponNums.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->name = szBuf;
		// а теперь утянуть изменения в настоящий массив
		for (int i = 0; i < (int)pLogic->pWeaponNums.weaponNums.size(); i++) {
			if (it->id == pLogic->pWeaponNums.weaponNums[i].id) {
				pLogic->pWeaponNums.weaponNums[i].name = szBuf;
				break;
			}
		}
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
		std::vector<WeaponType>::iterator it;	it = pLogic->pWeaponTypes.weaponTypes.begin();
		std::advance(it, m_grid.GetCellFocused().m_iRow);

		WeaponNum pNewOne(pLogic->ids.wepnum_id++, it->id, _T(""));
		pLogic->pWeaponNums.weaponNums.push_back(pNewOne);
		pTmpNums.weaponNums.push_back(pNewOne);
		m_gridNum.SetRowCount(m_gridNum.GetRowCount() + 1);
	}
}

void WeaponDlg::OnBnClickedOk()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	CDialog::OnOK();
}

void WeaponDlg::OnBnClickedAddnum()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	if (pLogic->pWeaponTypes.weaponTypes.empty()) {
		AfxMessageBox(_T("Спочатку потрібно додати тип озброєння"));
		return;
	}
	CGridCell cell = m_gridNum.GetCellFocused();
	std::vector<WeaponType>::iterator itType;	itType = pLogic->pWeaponTypes.weaponTypes.begin();
	std::advance(itType, m_grid.GetCellFocused().m_iRow);

	WeaponNum pNewOne(pLogic->ids.wepnum_id++, itType->id, _T(""));
	if (m_gridNum.GetRowCount() == 0) {
		pTmpNums.weaponNums.push_back(pNewOne);
		pLogic->pWeaponNums.weaponNums.push_back(pNewOne);
	}
	else {
		std::vector<WeaponNum>::iterator it = pTmpNums.weaponNums.begin();
		std::advance(it, cell.m_iRow);
		// а теперь в основной массив
		for (int i = 0; i < (int)pLogic->pWeaponNums.weaponNums.size(); i++) {
			if (it->id == pLogic->pWeaponNums.weaponNums[i].id) {
				std::vector<WeaponNum>::iterator itInsert = pLogic->pWeaponNums.weaponNums.begin();
				std::advance(itInsert, i);
				pLogic->pWeaponNums.weaponNums.insert(itInsert, pNewOne);
				break;
			}
		}
		// ну и во временный массив
		pTmpNums.weaponNums.insert(it, pNewOne);
	}
	m_gridNum.SetRowCount(m_gridNum.GetRowCount() + 1);
}

void WeaponDlg::OnBnClickedDelnum()
{
	m_grid.PressReturn();
	m_gridNum.PressReturn();
	if (pTmpNums.weaponNums.size() < 1) {
		return;
	}

	std::vector<WeaponNum>::iterator it;	it = pTmpNums.weaponNums.begin();
	std::advance(it, m_gridNum.GetCellFocused().m_iRow);
	// проверка на использование в WEPNUM. и если свободен, удалить
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
		for (int i = 0; i < (int)pLogic->pWeaponNums.weaponNums.size(); i++) {
			if (it->id == pLogic->pWeaponNums.weaponNums[i].id) {
				std::vector<WeaponNum>::iterator itDel = pLogic->pWeaponNums.weaponNums.begin();
				std::advance(itDel, i);
				pLogic->pWeaponNums.weaponNums.erase(itDel);
				break;
			}
		}

		pTmpNums.weaponNums.erase(it);
		m_gridNum.SetRowCount(m_gridNum.GetRowCount() - 1);
		m_gridNum.Invalidate();
	}
	else {
		AfxMessageBox(_T("За даним типом озброєння закрiплено кiлька одиниць зброї."));
	}
}

