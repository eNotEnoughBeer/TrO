#include "pch.h"
#include "TrO.h"
#include "DivizionDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(DivizionDlg, CDialog)

DivizionDlg::DivizionDlg(CWnd* pParent /*=nullptr*/): CDialog(IDD_DIVIZION_DLG, pParent){}
DivizionDlg::~DivizionDlg(){}

void DivizionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID, m_gridR);
	DDX_Control(pDX, IDC_GRIDV, m_gridV);
	DDX_Control(pDX, IDC_GRIDO, m_gridO);
}


BEGIN_MESSAGE_MAP(DivizionDlg, CDialog)
	ON_BN_CLICKED(IDC_DONE2, &DivizionDlg::OnBnClickedDone2)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID, OnGetDispinfoGridR)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRID, OnSetDispinfoGridR)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRID, OnKeyDownGridResectR)
	ON_NOTIFY(VGN_CHANGE, IDC_GRID, OnChangeGridR)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRIDV, OnGetDispinfoGridV)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRIDV, OnSetDispinfoGridV)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRIDV, OnKeyDownGridResectV)
	ON_NOTIFY(VGN_CHANGE, IDC_GRIDV, OnChangeGridV)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRIDO, OnGetDispinfoGridO)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRIDO, OnSetDispinfoGridO)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRIDO, OnKeyDownGridResectO)
	ON_BN_CLICKED(IDC_ADD, &DivizionDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DEL, &DivizionDlg::OnBnClickedDel)
	ON_BN_CLICKED(IDC_ADDV, &DivizionDlg::OnBnClickedAddv)
	ON_BN_CLICKED(IDC_DELV, &DivizionDlg::OnBnClickedDelv)
	ON_BN_CLICKED(IDC_ADDO, &DivizionDlg::OnBnClickedAddo)
	ON_BN_CLICKED(IDC_DELO, &DivizionDlg::OnBnClickedDelo)
END_MESSAGE_MAP()

void DivizionDlg::OnBnClickedDone2()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	CDialog::OnOK();
}

void DivizionDlg::parseDivision(int rotaId, int vzvodId)
{
	rota.clear();
	vzvod.clear();
	otdel.clear();
	// выбираем все роты
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (pLogic->pDivizions.divisions[i].rotaId == -1 && pLogic->pDivizions.divisions[i].vzvodId == -1) {
			ID_NAME pOne(pLogic->pDivizions.divisions[i].id, pLogic->pDivizions.divisions[i].name);
			rota.push_back(pOne);
		}
	}
	// выбираем все взводы для конкретной роты. если rotaId==-1, выбираем взводы для первой роты в списке rota 
	if (rotaId == -1) {
		if (!rota.empty()) {
			rotaId = rota[0].id;
		}
		else {
			return;
		}
	}
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (pLogic->pDivizions.divisions[i].rotaId == rotaId && pLogic->pDivizions.divisions[i].vzvodId == -1) {
			ID_NAME pOne(pLogic->pDivizions.divisions[i].id, pLogic->pDivizions.divisions[i].name);
			vzvod.push_back(pOne);
		}
	}
	// выбираем все отделения для взвода. если vzvodId==-1, выбираем отделения для первого взвода в списке vzvod
	if (vzvodId == -1) {
		if (!vzvod.empty()) {
			vzvodId = vzvod[0].id;
		}
		else {
			return;
		}
	}
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (pLogic->pDivizions.divisions[i].rotaId == rotaId && pLogic->pDivizions.divisions[i].vzvodId == vzvodId) {
			ID_NAME pOne(pLogic->pDivizions.divisions[i].id, pLogic->pDivizions.divisions[i].name);
			otdel.push_back(pOne);
		}
	}
}

BOOL DivizionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();
	parseDivision(-1, -1);
	int iColumn = m_gridR.AddColumn(_T(""), 185);
	CGridHeaderSection* pSection = m_gridR.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Рота"));
	m_gridR.GetHeader()->SetSectionHeight(24);
	m_gridR.SetAllowEdit();
	m_gridR.SetRowCount((int)rota.size());
	m_gridR.SetAlwaysSelected();

	iColumn = m_gridV.AddColumn(_T(""), 185);
	pSection = m_gridV.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Взвод"));
	m_gridV.GetHeader()->SetSectionHeight(24);
	m_gridV.SetAllowEdit();
	m_gridV.SetRowCount((int)vzvod.size());
	m_gridV.SetAlwaysSelected();

	iColumn = m_gridO.AddColumn(_T(""), 185);
	pSection = m_gridO.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Відділення"));
	m_gridO.GetHeader()->SetSectionHeight(24);
	m_gridO.SetAllowEdit();
	m_gridO.SetRowCount((int)otdel.size());
	m_gridO.SetAlwaysSelected();
	return TRUE;
}

void DivizionDlg::OnGetDispinfoGridR(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (rota.size() == 0)
		return;
	std::vector<ID_NAME>::const_iterator it = rota.begin();
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

void DivizionDlg::OnSetDispinfoGridR(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (rota.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<ID_NAME>::iterator it; it = rota.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->name = szBuf;
		// обновить pLogic->pDivizions.divisions
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (it->id == pLogic->pDivizions.divisions[i].id) {
				pLogic->pDivizions.divisions[i].name = szBuf;
				break;
			}
		}
		break;
	}
	m_gridR.InvalidateGrid();
}

void DivizionDlg::OnKeyDownGridResectR(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_gridR.GetRowCount())return;
	CGridCell cell = m_gridR.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_gridR.GetRowCount() - 1)
	{
		Divizion pNewOne(pLogic->ids.divizion_id++, _T(""),-1,-1);// создаем роту
		pLogic->pDivizions.divisions.push_back(pNewOne);
		parseDivision(pNewOne.id, -1);
		m_gridR.SetRowCount((int)rota.size());
		m_gridO.SetRowCount((int)otdel.size());
		m_gridV.SetRowCount((int)vzvod.size());
		m_gridR.InvalidateGrid();
		m_gridV.InvalidateGrid();
		m_gridO.InvalidateGrid();
	}
}

void DivizionDlg::OnChangeGridR(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	std::vector<ID_NAME>::const_iterator it = rota.begin();
	std::advance(it, pDispInfo->item.iRow);
	parseDivision(it->id, -1);

	m_gridR.SetRowCount((int)rota.size());
	m_gridO.SetRowCount((int)otdel.size());
	m_gridV.SetRowCount((int)vzvod.size());
	m_gridR.InvalidateGrid();
	m_gridV.InvalidateGrid();
	m_gridO.InvalidateGrid();
}

void DivizionDlg::OnGetDispinfoGridV(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (vzvod.size() == 0)
		return;
	std::vector<ID_NAME>::const_iterator it = vzvod.begin();
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

void DivizionDlg::OnSetDispinfoGridV(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (vzvod.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<ID_NAME>::iterator it; it = vzvod.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->name = szBuf;
		// обновить pLogic->pDivizions.divisions
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (it->id == pLogic->pDivizions.divisions[i].id) {
				pLogic->pDivizions.divisions[i].name = szBuf;
				break;
			}
		}
		break;
	}
	m_gridV.InvalidateGrid();
}

void DivizionDlg::OnKeyDownGridResectV(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_gridV.GetRowCount())return;
	CGridCell cell = m_gridV.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_gridV.GetRowCount() - 1)
	{
		std::vector<ID_NAME>::const_iterator itR = rota.begin();
		std::advance(itR, m_gridR.GetCellFocused().m_iRow);
		Divizion pNewOne(pLogic->ids.divizion_id++, _T(""), itR->id, -1);// создаем взвод
		pLogic->pDivizions.divisions.push_back(pNewOne);
		parseDivision(itR->id,pNewOne.id);

		m_gridR.SetRowCount((int)rota.size());
		m_gridO.SetRowCount((int)otdel.size());
		m_gridV.SetRowCount((int)vzvod.size());
		m_gridR.InvalidateGrid();
		m_gridV.InvalidateGrid();
		m_gridO.InvalidateGrid();
	}
}

void DivizionDlg::OnChangeGridV(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	std::vector<ID_NAME>::const_iterator itR = rota.begin();
	std::advance(itR, m_gridR.GetCellFocused().m_iRow);
	std::vector<ID_NAME>::const_iterator it = vzvod.begin();
	std::advance(it, pDispInfo->item.iRow);
	parseDivision(itR->id, it->id);
	m_gridR.SetRowCount((int)rota.size());
	m_gridO.SetRowCount((int)otdel.size());
	m_gridV.SetRowCount((int)vzvod.size());
	m_gridR.InvalidateGrid();
	m_gridV.InvalidateGrid();
	m_gridO.InvalidateGrid();
}

void DivizionDlg::OnGetDispinfoGridO(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (otdel.size() == 0)
		return;
	std::vector<ID_NAME>::const_iterator it = otdel.begin();
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

void DivizionDlg::OnSetDispinfoGridO(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (otdel.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<ID_NAME>::iterator it; it = otdel.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->name = szBuf;
		// обновить pLogic->pDivizions.divisions
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (it->id == pLogic->pDivizions.divisions[i].id) {
				pLogic->pDivizions.divisions[i].name = szBuf;
				break;
			}
		}
		break;
	}
	m_gridV.InvalidateGrid();
}

void DivizionDlg::OnKeyDownGridResectO(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_gridO.GetRowCount())return;
	CGridCell cell = m_gridO.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_gridO.GetRowCount() - 1)
	{
		std::vector<ID_NAME>::const_iterator itR = rota.begin();
		std::advance(itR, m_gridR.GetCellFocused().m_iRow);
		std::vector<ID_NAME>::const_iterator itV = vzvod.begin();
		std::advance(itV, m_gridV.GetCellFocused().m_iRow);
		Divizion pNewOne(pLogic->ids.divizion_id++, _T(""), itR->id, itV->id);// создаем отделение
		pLogic->pDivizions.divisions.push_back(pNewOne);
		// тут проще. от добавления в третий грид предыдущие два не меняются
		ID_NAME pOne(pNewOne.id, _T(""));
		otdel.push_back(pOne);
		m_gridO.SetRowCount((int)otdel.size());		
		m_gridO.InvalidateGrid();
	}
}

void DivizionDlg::OnBnClickedAdd()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	CGridCell cell = m_gridR.GetCellFocused();
	Divizion pNewOne(pLogic->ids.divizion_id++, _T(""), -1, -1);// создаем роту
	
	if (m_gridR.GetRowCount() == 0) {
		pLogic->pDivizions.divisions.push_back(pNewOne);
	}
	else {
		std::vector<ID_NAME>::iterator it = rota.begin();
		std::advance(it, cell.m_iRow);
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (it->id == pLogic->pDivizions.divisions[i].id) {
				std::vector<Divizion>::iterator itInsert = pLogic->pDivizions.divisions.begin();
				std::advance(itInsert, i);
				pLogic->pDivizions.divisions.insert(itInsert, pNewOne);
				break;
			}
		}
	}

	parseDivision(pNewOne.id, -1);
	m_gridR.SetRowCount((int)rota.size());
	m_gridO.SetRowCount((int)otdel.size());
	m_gridV.SetRowCount((int)vzvod.size());
	m_gridR.InvalidateGrid();
	m_gridV.InvalidateGrid();
	m_gridO.InvalidateGrid();
}

void DivizionDlg::OnBnClickedDel()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	if (rota.size() < 1) {
		return;
	}
	std::vector<ID_NAME>::iterator it;	it = rota.begin();
	std::advance(it, m_gridR.GetCellFocused().m_iRow);
	// проверка на использование. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (it->id == pLogic->pDivizions.divisions[i].rotaId) {
			canDelete = false;
			break;
		}
	}
	for (int i = 0; i < (int)pLogic->pPersons.persons.size(); i++) {
		if (it->id == pLogic->pPersons.persons[i].divizionId) {
			canDelete = false;
			break;
		}
	}
	if (canDelete) {
		int id2del = it->id;
		rota.erase(it);
		m_gridR.SetRowCount(m_gridR.GetRowCount() - 1);
		m_gridR.InvalidateGrid();
		// удалить запись из настоящего массива
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (id2del == pLogic->pDivizions.divisions[i].id) {
				std::vector<Divizion>::iterator itDelete = pLogic->pDivizions.divisions.begin();
				std::advance(itDelete, i);
				pLogic->pDivizions.divisions.erase(itDelete);
				break;
			}
		}
		// выяснить, куда перешел курсор выделения в гриде, если там хоть что-то осталось
		if (!rota.empty()) {
			std::vector<ID_NAME>::const_iterator itNew;	itNew = rota.begin();
			std::advance(itNew, m_gridR.GetCellFocused().m_iRow);
			parseDivision(itNew->id, -1);
		}
		else {
			parseDivision(-1, -1);
		}
		m_gridR.SetRowCount((int)rota.size());
		m_gridO.SetRowCount((int)otdel.size());
		m_gridV.SetRowCount((int)vzvod.size());
		m_gridR.InvalidateGrid();
		m_gridV.InvalidateGrid();
		m_gridO.InvalidateGrid();
	}
	else {
		AfxMessageBox(_T("Дана рота використовується i не може бути видалена."));
	}

}

void DivizionDlg::OnBnClickedAddv()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	if (rota.empty()) {
		AfxMessageBox(_T("Спочатку треба додати роту."));
		return;
	}

	std::vector<ID_NAME>::const_iterator itR;	itR = rota.begin();
	std::advance(itR, m_gridR.GetCellFocused().m_iRow);
	CGridCell cell = m_gridV.GetCellFocused();
	Divizion pNewOne(pLogic->ids.divizion_id++, _T(""), itR->id, -1);// создаем взвод
	
	if (m_gridV.GetRowCount() == 0) {
		pLogic->pDivizions.divisions.push_back(pNewOne);
	}
	else {
		std::vector<ID_NAME>::iterator it = vzvod.begin();
		std::advance(it, cell.m_iRow);
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (it->id == pLogic->pDivizions.divisions[i].id) {
				std::vector<Divizion>::iterator itInsert = pLogic->pDivizions.divisions.begin();
				std::advance(itInsert, i);
				pLogic->pDivizions.divisions.insert(itInsert, pNewOne);
				break;
			}
		}
	}

	parseDivision(pNewOne.rotaId, pNewOne.id);
	m_gridR.SetRowCount((int)rota.size());
	m_gridO.SetRowCount((int)otdel.size());
	m_gridV.SetRowCount((int)vzvod.size());
	m_gridR.InvalidateGrid();
	m_gridV.InvalidateGrid();
	m_gridO.InvalidateGrid();
}

void DivizionDlg::OnBnClickedDelv()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	if (vzvod.size() < 1) {
		return;
	}
	std::vector<ID_NAME>::iterator it;	it = vzvod.begin();
	std::advance(it, m_gridV.GetCellFocused().m_iRow);
	// проверка на использование. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (it->id == pLogic->pDivizions.divisions[i].vzvodId) {
			canDelete = false;
			break;
		}
	}
	for (int i = 0; i < (int)pLogic->pPersons.persons.size(); i++) {
		if (it->id == pLogic->pPersons.persons[i].divizionId) {
			canDelete = false;
			break;
		}
	}
	if (canDelete) {
		int id2del = it->id;
		vzvod.erase(it);
		m_gridV.SetRowCount(m_gridV.GetRowCount() - 1);
		m_gridV.InvalidateGrid();
		// удалить запись из настоящего массива
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (id2del == pLogic->pDivizions.divisions[i].id) {
				std::vector<Divizion>::iterator itDelete = pLogic->pDivizions.divisions.begin();
				std::advance(itDelete, i);
				pLogic->pDivizions.divisions.erase(itDelete);
				break;
			}
		}
		// выяснить, куда перешел курсор выделения в гриде, если там хоть что-то осталось
		int rotId = -1, vzvdId = -1;
		if (!vzvod.empty()) {
			std::vector<ID_NAME>::const_iterator itNew;	itNew = vzvod.begin();
			std::advance(itNew, m_gridV.GetCellFocused().m_iRow);
			vzvdId = itNew->id;
		}
		if (!rota.empty()) {
			std::vector<ID_NAME>::const_iterator itR;	itR = rota.begin();
			std::advance(itR, m_gridR.GetCellFocused().m_iRow);
			rotId = itR->id;
		}
		parseDivision(rotId, vzvdId);
		
		m_gridR.SetRowCount((int)rota.size());
		m_gridO.SetRowCount((int)otdel.size());
		m_gridV.SetRowCount((int)vzvod.size());
		m_gridR.InvalidateGrid();
		m_gridV.InvalidateGrid();
		m_gridO.InvalidateGrid();
	}
	else {
		AfxMessageBox(_T("Даний взвод використовується i не може бути видалений."));
	}
}

void DivizionDlg::OnBnClickedAddo()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	if (vzvod.empty()) {
		AfxMessageBox(_T("Спочатку треба додати взвод."));
		return;
	}

	std::vector<ID_NAME>::const_iterator itR;	itR = rota.begin();
	std::advance(itR, m_gridR.GetCellFocused().m_iRow);
	std::vector<ID_NAME>::const_iterator itV;	itV = vzvod.begin();
	std::advance(itV, m_gridV.GetCellFocused().m_iRow);

	CGridCell cell = m_gridO.GetCellFocused();
	Divizion pNewOne(pLogic->ids.divizion_id++, _T(""), itR->id, itV->id);// создаем отделение
	ID_NAME pOne(pNewOne.id, _T(""));
	if (m_gridO.GetRowCount() == 0) {
		pLogic->pDivizions.divisions.push_back(pNewOne);
		otdel.push_back(pOne);
	}
	else {
		std::vector<ID_NAME>::iterator it = otdel.begin();
		std::advance(it, cell.m_iRow);
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (it->id == pLogic->pDivizions.divisions[i].id) {
				std::vector<Divizion>::iterator itInsert = pLogic->pDivizions.divisions.begin();
				std::advance(itInsert, i);
				pLogic->pDivizions.divisions.insert(itInsert, pNewOne);
				break;
			}
		}
		otdel.insert(it, pOne);
	}
	
	m_gridO.SetRowCount((int)otdel.size());
	m_gridO.InvalidateGrid();
}

void DivizionDlg::OnBnClickedDelo()
{
	m_gridR.PressReturn();
	m_gridV.PressReturn();
	m_gridO.PressReturn();
	if (otdel.size() < 1) {
		return;
	}
	std::vector<ID_NAME>::iterator it;	it = otdel.begin();
	std::advance(it, m_gridO.GetCellFocused().m_iRow);
	// проверка на использование. и если свободен, удалить
	bool canDelete = true;
	for (int i = 0; i < (int)pLogic->pPersons.persons.size(); i++) {
		if (it->id == pLogic->pPersons.persons[i].divizionId) {
			canDelete = false;
			break;
		}
	}
	if (canDelete) {
		int id2del = it->id;
		otdel.erase(it);
		m_gridO.SetRowCount(m_gridO.GetRowCount() - 1);
		m_gridO.InvalidateGrid();
		// удалить запись из настоящего массива
		for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
			if (id2del == pLogic->pDivizions.divisions[i].id) {
				std::vector<Divizion>::iterator itDelete = pLogic->pDivizions.divisions.begin();
				std::advance(itDelete, i);
				pLogic->pDivizions.divisions.erase(itDelete);
				break;
			}
		}
	}
	else {
		AfxMessageBox(_T("Данe відділення використовується i не може бути видалене."));
	}
}
