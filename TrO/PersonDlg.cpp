#include "pch.h"
#include "TrO.h"
#include "PersonDlg.h"
#include "afxdialogex.h"

// PersonDlg dialog

IMPLEMENT_DYNAMIC(PersonDlg, CDialog)

PersonDlg::PersonDlg(CWnd* pParent /*=nullptr*/): CDialog(IDD_PERSON_DLG, pParent){}
PersonDlg::~PersonDlg(){}
void PersonDlg::OnBnClickedCancel() { CDialog::OnCancel(); }

void PersonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID_WEP, m_gridWep);
	DDX_Control(pDX, IDC_GRID_REL, m_gridRel);
	DDX_Control(pDX, IDC_CMBPOSIT, m_comboPosit);
	DDX_Control(pDX, IDC_CMBRANK, m_comboRank);
	DDX_Control(pDX, IDC_CMBDIVIZ, m_comboDiviz);
}

BEGIN_MESSAGE_MAP(PersonDlg, CDialog)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID_WEP, OnGetDispinfoGridWep)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRID_WEP, OnSetDispinfoGridWep)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRID_WEP, OnKeyDownGridResectWep)
	ON_NOTIFY(VGN_GETEDITSTYLE, IDC_GRID_WEP, OnGetEditStyleGridWep)
	ON_NOTIFY(VGN_GETEDITLIST, IDC_GRID_WEP, OnGetEditListGridWep)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID_REL, OnGetDispinfoGridRel)
	ON_NOTIFY(VGN_SETDISPINFO, IDC_GRID_REL, OnSetDispinfoGridRel)
	ON_NOTIFY(NM_KEYDOWN, IDC_GRID_REL, OnKeyDownGridResectRel)
	ON_BN_CLICKED(IDCANCEL, &PersonDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ADDREL, &PersonDlg::OnBnClickedAddrel)
	ON_BN_CLICKED(IDC_DELREL, &PersonDlg::OnBnClickedDelrel)
	ON_BN_CLICKED(IDC_ADDWEP, &PersonDlg::OnBnClickedAddwep)
	ON_BN_CLICKED(IDC_DELWEP, &PersonDlg::OnBnClickedDelwep)
	ON_BN_CLICKED(IDCREADY, &PersonDlg::OnBnClickedCready)
END_MESSAGE_MAP()

BOOL PersonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();

	weaponArr.clear();
	for (int i = 0; i < (int)pLogic->pWeaponTypes.weaponTypes.size(); i++) {
		for (int j = 0; j < (int)pLogic->pWeaponNums.weaponNums.size(); j++) {
			if (pLogic->pWeaponTypes.weaponTypes[i].id == pLogic->pWeaponNums.weaponNums[j].typeId) {
				_ID_NAME tmp(pLogic->pWeaponNums.weaponNums[j].id);
				tmp.name.Format(_T("%s %s"), pLogic->pWeaponTypes.weaponTypes[i].name.GetString(), pLogic->pWeaponNums.weaponNums[j].name.GetString());
				weaponArr.push_back(tmp);
			}
		}
	}
	// сортирнем список по возрастанию
	std::sort(weaponArr.begin(), weaponArr.end(), [](_ID_NAME const& a, _ID_NAME const& b) {return wcscmp(a.name.GetString(), b.name.GetString()) < 0; });
	
	m_gridRel.AddColumn(_T(""), 370);
	m_gridRel.AddColumn(_T(""), 150);
	CGridHeaderSection* pSection = m_gridRel.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("П.I.Б."));
	pSection = m_gridRel.GetHeaderSection(1, 0);
	pSection->SetCaption(_T("Телефон"));
	pSection->SetAlignment(LVCFMT_CENTER);
	m_gridRel.GetHeader()->SetSectionHeight(24);
	m_gridRel.SetAllowEdit();
	m_gridRel.SetRowCount((int)rels->size());
	m_gridRel.SetAlwaysSelected();

	m_gridWep.AddColumn(_T(""), 165);
	pSection = m_gridWep.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("Тип Номер"));
	m_gridWep.GetHeader()->SetSectionHeight(24);
	m_gridWep.SetAllowEdit();
	m_gridWep.SetRowCount(pPerson->weapons.size());
	m_gridWep.SetAlwaysSelected();

	// воткнуть во все окна данные из объекта
	FillComboPosit();
	FillComboRank();
	FillComboDiviz();
	GetDlgItem(IDC_EDTCODE)->SetWindowText(pPerson->code.GetString());
	GetDlgItem(IDC_EDTPIB)->SetWindowText(pPerson->fio.GetString());
	GetDlgItem(IDC_EDTCLICH)->SetWindowText(pPerson->callsign.GetString());
	GetDlgItem(IDC_EDTPHONE)->SetWindowText(pPerson->phone.GetString());
	GetDlgItem(IDC_EDTBIRTH)->SetWindowText(pPerson->birthday.GetString());
	GetDlgItem(IDC_EDTHOUSE)->SetWindowText(pPerson->address.GetString());
	GetDlgItem(IDC_EDTCOMM)->SetWindowText(pPerson->comment.GetString());
	GetDlgItem(IDC_EDTSHOES)->SetWindowText(Dbl2Str(pPerson->shoesSize,1).c_str());
	GetDlgItem(IDC_EDTCLTH)->SetWindowText(Dbl2Str(pPerson->clothersSize, 1).c_str());
	GetDlgItem(IDC_EDTUBD)->SetWindowText(pPerson->ubd.GetString());
	GetDlgItem(IDC_EDTSTDATE)->SetWindowText(pPerson->startDate.GetString());

	return TRUE;
}

void PersonDlg::FillComboPosit()
{
	m_comboPosit.ResetContent();
	if (!pLogic->pPositions.positions.empty())
		for (int a = 0; a < (int)pLogic->pPositions.positions.size(); a++)
		{
			int i = m_comboPosit.AddString(pLogic->pPositions.positions[a].name.GetString());
			m_comboPosit.SetItemData(i, (DWORD)pLogic->pPositions.positions[a].id);
			if (pPerson->positionId == pLogic->pPositions.positions[a].id) {
				m_comboPosit.SetCurSel(a);
			}
		}
}

void PersonDlg::FillComboRank()
{
	m_comboRank.ResetContent();
	if (!pLogic->pRanks.ranks.empty())
		for (int a = 0; a < (int)pLogic->pRanks.ranks.size(); a++)
		{
			int i = m_comboRank.AddString(pLogic->pRanks.ranks[a].name.GetString());
			m_comboRank.SetItemData(i, (DWORD)pLogic->pRanks.ranks[a].id);
			if (pPerson->rankId == pLogic->pRanks.ranks[a].id) {
				m_comboRank.SetCurSel(a);
			}
		}
}

void PersonDlg::FillComboDiviz()
{
	std::vector<_ID_NAME> res = parseDivision();
	m_comboDiviz.ResetContent();
	if (!pLogic->pDivizions.divisions.empty()) {
		for (int a = 0; a < (int)res.size(); a++)
		{
			int i = m_comboDiviz.AddString(res[a].name.GetString());
			m_comboDiviz.SetItemData(i, (DWORD)res[a].id);
			if (pPerson->divizionId == res[a].id) {
				m_comboDiviz.SetCurSel(a);
			}
		}
	}
}

std::vector<_ID_NAME> PersonDlg::parseDivision()
{
	std::vector<_ID_NAME> result; result.clear();
	std::vector<_ID_NAME> rota; rota.clear();
	
	// выбираем все роты
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (pLogic->pDivizions.divisions[i].rotaId == -1 && pLogic->pDivizions.divisions[i].vzvodId == -1) {
			_ID_NAME pOne(pLogic->pDivizions.divisions[i].id); 
			pOne.name = pLogic->pDivizions.divisions[i].name;
			rota.push_back(pOne);
		}
	}

	for (int i = 0; i < (int)rota.size(); i++) {
		result.push_back(rota[i]);// первой ставим роту

		// выбираем взводы для роты
		for (int j = 0; j < (int)pLogic->pDivizions.divisions.size(); j++) {
			if (pLogic->pDivizions.divisions[j].rotaId == rota[i].id && pLogic->pDivizions.divisions[j].vzvodId == -1) {
				_ID_NAME pOne(pLogic->pDivizions.divisions[j].id);
				pOne.name.Format(_T("-%s"), pLogic->pDivizions.divisions[j].name.GetString());
				result.push_back(pOne);// теперь ставим i-й взвод роты
				for (int a = 0; a < (int)pLogic->pDivizions.divisions.size(); a++) {
					if (pLogic->pDivizions.divisions[j].rotaId == rota[i].id && pLogic->pDivizions.divisions[a].vzvodId == pOne.id) {
						_ID_NAME pTwo(pLogic->pDivizions.divisions[a].id);
						pTwo.name.Format(_T("--%s"), pLogic->pDivizions.divisions[a].name.GetString());
						result.push_back(pTwo);// теперь ставим i-й отделение взвода роты
					}
				}
			}
		}
	}
	
	return result;
}

CString PersonDlg::index2name(int index) 
{
	CString res = _T("");
	for (int i = 0; i < (int)weaponArr.size(); i++) {
		if (index == weaponArr[i].id) {
			res = weaponArr[i].name;
			break;
		}
	}
	return res;
}

int PersonDlg::name2index(CString name) 
{
	int res = -1;
	for (int i = 0; i < (int)weaponArr.size(); i++) {
		if (name.Compare(weaponArr[i].name.GetString()) == 0 ) {
			res = weaponArr[i].id;
			break;
		}
	}
	return res;
}

void PersonDlg::OnGetDispinfoGridWep(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pPerson->weapons.size() == 0)
		return;
	std::vector<int>::const_iterator it = pPerson->weapons.begin();
	std::advance(it, pDispInfo->item.iRow);

	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:	// запрос
		wcscpy(szBuf, index2name(*it).GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

void PersonDlg::OnSetDispinfoGridWep(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pPerson->weapons.size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<int>::iterator it; it = pPerson->weapons.begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		*it = name2index(szBuf);
		break;
	}
	m_gridWep.InvalidateGrid();
}

void PersonDlg::OnGetEditStyleGridWep(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pDispInfo->item.iColumn == 0)
		pDispInfo->item.style = gePickList;
}

void PersonDlg::OnGetEditListGridWep(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	pDispInfo->item.pickList.RemoveAll();
	// тут сформировать массив строк "ТИП Серийный_номер"
	for (int i = 0; i < (int)weaponArr.size(); i++) {
		pDispInfo->item.pickList.Add(weaponArr[i].name.GetString());
	}

	*pResult = 0;
}

void PersonDlg::OnKeyDownGridResectWep(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_gridWep.GetRowCount())return;
	CGridCell cell = m_gridWep.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_gridWep.GetRowCount() - 1)
	{
		pPerson->weapons.push_back(-1);
		m_gridWep.SetRowCount((int)pPerson->weapons.size());
		m_gridWep.InvalidateGrid();
	}
}

void PersonDlg::OnBnClickedAddwep()
{
	m_gridWep.PressReturn();
	m_gridRel.PressReturn();
	CGridCell cell = m_gridWep.GetCellFocused();
	if (m_gridWep.GetRowCount() == 0) {
		pPerson->weapons.push_back(-1);
	}
	else {
		std::vector<int>::iterator it = pPerson->weapons.begin();
		std::advance(it, cell.m_iRow);
		pPerson->weapons.insert(it, -1);
	}
	m_gridWep.SetRowCount(m_gridWep.GetRowCount() + 1);
}

void PersonDlg::OnBnClickedDelwep()
{
	m_gridWep.PressReturn();
	m_gridRel.PressReturn();
	if (pPerson->weapons.size() < 1) {
		return;
	}

	std::vector<int>::iterator it;	it = pPerson->weapons.begin();
	std::advance(it, m_gridWep.GetCellFocused().m_iRow);
	pPerson->weapons.erase(it);
	m_gridWep.SetRowCount(m_gridWep.GetRowCount() - 1);
	m_gridWep.Invalidate();
}
//-------------------------------------------------
void PersonDlg::OnGetDispinfoGridRel(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (rels->size() == 0)
		return;
	std::vector<Relative>::const_iterator it = rels->begin();
	std::advance(it, pDispInfo->item.iRow);

	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, it->fio.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 1:	
		wcscpy(szBuf, it->phone.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

void PersonDlg::OnSetDispinfoGridRel(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (rels->size() == 0)
		return;
	wchar_t szBuf[256];

	std::vector<Relative>::iterator it; it = rels->begin();
	std::advance(it, pDispInfo->item.iRow);
	switch (pDispInfo->item.iColumn)
	{
	case 0:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->fio = szBuf;
		break;
	case 1:
		wcscpy(szBuf, pDispInfo->item.pszText);
		it->phone = szBuf;
		break;
	}
	m_gridRel.InvalidateGrid();
}

void PersonDlg::OnKeyDownGridResectRel(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	LPNMKEY lpNMKey = reinterpret_cast<LPNMKEY>(lpNMHDR);
	if (!m_gridRel.GetRowCount())return;
	CGridCell cell = m_gridRel.GetCellFocused();
	if (lpNMKey->nVKey == VK_DOWN && cell.m_iRow == m_gridRel.GetRowCount() - 1)
	{
		Relative pNewOne;
		pNewOne.personId = pPerson->id;
		rels->push_back(pNewOne);
		m_gridRel.SetRowCount((int)rels->size());
		m_gridRel.InvalidateGrid();
	}
}

void PersonDlg::OnBnClickedAddrel()
{
	m_gridWep.PressReturn();
	m_gridRel.PressReturn();
	CGridCell cell = m_gridRel.GetCellFocused();
	Relative pNewOne;
	pNewOne.personId = pPerson->id;
	if (m_gridRel.GetRowCount() == 0) {
		rels->push_back(pNewOne);
	}
	else {
		std::vector<Relative>::iterator it = rels->begin();
		std::advance(it, cell.m_iRow);
		rels->insert(it, pNewOne);
	}
	m_gridRel.SetRowCount(m_gridRel.GetRowCount() + 1);
}

void PersonDlg::OnBnClickedDelrel()
{
	m_gridWep.PressReturn();
	m_gridRel.PressReturn();
	if (rels->size() < 1) {
		return;
	}

	std::vector<Relative>::iterator it;	it = rels->begin();
	std::advance(it, m_gridRel.GetCellFocused().m_iRow);
	rels->erase(it);
	m_gridRel.SetRowCount(m_gridRel.GetRowCount() - 1);
	m_gridRel.Invalidate();
}

void PersonDlg::OnBnClickedCready()
{
	m_gridWep.PressReturn();
	m_gridRel.PressReturn();
	GetDlgItem(IDC_EDTCODE)->GetWindowText(pPerson->code);
	GetDlgItem(IDC_EDTPIB)->GetWindowText(pPerson->fio);
	GetDlgItem(IDC_EDTCLICH)->GetWindowText(pPerson->callsign);
	GetDlgItem(IDC_EDTPHONE)->GetWindowText(pPerson->phone);
	GetDlgItem(IDC_EDTBIRTH)->GetWindowText(pPerson->birthday);
	GetDlgItem(IDC_EDTHOUSE)->GetWindowText(pPerson->address);
	GetDlgItem(IDC_EDTCOMM)->GetWindowText(pPerson->comment);
	GetDlgItem(IDC_EDTUBD)->GetWindowText(pPerson->ubd);
	GetDlgItem(IDC_EDTSTDATE)->GetWindowText(pPerson->startDate);
	CString tmp;
	GetDlgItem(IDC_EDTSHOES)->GetWindowText(tmp); pPerson->shoesSize = Str2Dbl(tmp.GetString());
	GetDlgItem(IDC_EDTCLTH)->GetWindowText(tmp);  pPerson->clothersSize = Str2Dbl(tmp.GetString());
	// теперь три комбобокса и всё
	pPerson->positionId = m_comboPosit.GetItemData(m_comboPosit.GetCurSel());
	pPerson->rankId = m_comboRank.GetItemData(m_comboRank.GetCurSel());
	pPerson->divizionId = m_comboDiviz.GetItemData(m_comboDiviz.GetCurSel());
	EndDialog(IDOK);
}
