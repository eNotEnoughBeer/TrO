#include "pch.h"
#include "framework.h"
#include "TrO.h"
#include "TrODlg.h"
#include "afxdialogex.h"
#import "EXCEL8.OLB" auto_search auto_rename

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTrODlg::CTrODlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTrODlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID, m_grid);
}

BEGIN_MESSAGE_MAP(CTrODlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CONTEXTMENU()
	ON_COMMAND_RANGE(32771, 32775, MenuHandler)
	ON_COMMAND_RANGE(32776, 32777, MenuHandlerPerson)
	ON_COMMAND_RANGE(32779, 32780, MenuHandlerExport)
	ON_NOTIFY(VGN_GETDISPINFO, IDC_GRID, OnGetDispinfoGrid)
END_MESSAGE_MAP()

// CTrODlg message handlers

BOOL CTrODlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	menu.LoadMenu(MAKEINTRESOURCE(IDR_MENU1));
	popupmenu.LoadMenu(MAKEINTRESOURCE(IDR_POPUPMENU));

	CGridHeaderSection* pSection;
	m_grid.AddColumn(_T(""), 100, LVCFMT_LEFT); // ������
	m_grid.AddColumn(_T(""), 90, LVCFMT_LEFT); // ���
	m_grid.AddColumn(_T(""), 90, LVCFMT_LEFT); // ������
	m_grid.AddColumn(_T(""), 250, LVCFMT_LEFT); // ϲ�
	m_grid.AddColumn(_T(""), 100, LVCFMT_LEFT); // ��������
	m_grid.AddColumn(_T(""), 100, LVCFMT_LEFT); // ���� ����������
	m_grid.AddColumn(_T(""), 300, LVCFMT_LEFT); // ��������
	m_grid.AddColumn(_T(""), 140, LVCFMT_LEFT); // �������
	m_grid.AddColumn(_T(""), 120, LVCFMT_LEFT); // ������/�����
	m_grid.AddColumn(_T(""), 250, LVCFMT_LEFT); // �����
	m_grid.AddColumn(_T(""), 200, LVCFMT_LEFT); // ������� ������

	pSection = m_grid.GetHeaderSection(0, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("������"));

	pSection = m_grid.GetHeaderSection(1, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("���"));

	pSection = m_grid.GetHeaderSection(2, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("������"));

	pSection = m_grid.GetHeaderSection(3, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("�.�.�."));

	pSection = m_grid.GetHeaderSection(4, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("��������"));

	pSection = m_grid.GetHeaderSection(5, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("�.�."));

	pSection = m_grid.GetHeaderSection(6, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("��������"));

	pSection = m_grid.GetHeaderSection(7, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("�������"));

	pSection = m_grid.GetHeaderSection(8, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("������/�����"));

	pSection = m_grid.GetHeaderSection(9, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("�����"));

	pSection = m_grid.GetHeaderSection(10, 0);
	pSection->SetAlignment(LVCFMT_CENTER);
	pSection->SetCaption(_T("������� ������"));

	m_grid.GetHeader()->SetSectionHeight(30);
	m_grid.SetAllowEdit(FALSE);
	m_grid.SetAlwaysSelected();
	m_grid.SetRowCount((int)pLogic->pPersons.persons.size());

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTrODlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CTrODlg::OnQueryDragIcon(){return static_cast<HCURSOR>(m_hIcon);}

void CTrODlg::MenuHandler(UINT id)
{
	switch (id)
	{
	case 32771:// ���������
		addPosition();
		break;
	case 32772:// ������
		addRank();
		break;
	case 32773:// ������
		addWeapon();
		break;
	case 32774:// �������������
		addDivizion();
		break;
	case 32775:// ����
		addPerson();
		break;
	}
}

void CTrODlg::addPosition()
{
	PositDlg dlg;
	dlg.pLogic = pLogic;
	dlg.DoModal();
}

void CTrODlg::addRank() 
{
	RankDlg dlg;
	dlg.pLogic = pLogic;
	dlg.DoModal();
}

void CTrODlg::addWeapon()
{
	WeaponDlg dlg;
	dlg.pLogic = pLogic;
	dlg.DoModal();
}

void CTrODlg::addDivizion()
{
	DivizionDlg dlg;
	dlg.pLogic = pLogic;
	dlg.DoModal();
}

void CTrODlg::addPerson()
{
	Person pOne;
	std::vector<Relative> rels; rels.clear();
	PersonDlg dlg;
	dlg.pLogic = pLogic;
	dlg.pPerson = &pOne;
	dlg.rels = &rels;
	if (IDOK == dlg.DoModal()) {
		// ��������� ���������� ������
		pOne.id = pLogic->ids.person_id++;
		// �������� ��������� � �������, �������� ����
		pLogic->pPersons.persons.push_back(pOne);
		// �� �� ���� � ��������������
		for (int i = 0; i < (int)rels.size(); i++) {
			rels[i].id = pLogic->ids.relative_id++;
			rels[i].personId = pOne.id;
			pLogic->pRelatives.relatives.push_back(rels[i]);
		}
		m_grid.SetRowCount((int)pLogic->pPersons.persons.size());
	}
}

void CTrODlg::OnGetDispinfoGrid(LPNMHDR lpNMHDR, LRESULT* pResult)
{
	VG_DISPINFO* pDispInfo = reinterpret_cast<VG_DISPINFO*>(lpNMHDR);
	if (pLogic->pPersons.persons.size() == 0)
		return;
	std::vector<Person>::const_iterator it = pLogic->pPersons.persons.begin();
	std::advance(it, pDispInfo->item.iRow);
	wchar_t szBuf[256] = _T("\0");
	switch (pDispInfo->item.iColumn)
	{
	case 0:	// ������
		wcscpy(szBuf, positionId2positionName(it->positionId).GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 1: // ���
		wcscpy(szBuf, it->code.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 2:	// ������
		wcscpy(szBuf, rankId2rankName(it->rankId).GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 3: // ϲ�
		wcscpy(szBuf, it->fio.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 4: // ��������
		wcscpy(szBuf, it->callsign.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 5: // ���� ����������
		wcscpy(szBuf, it->birthday.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 6: // ��������
		wcscpy(szBuf, it->address.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 7: // �������
		wcscpy(szBuf, it->phone.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	case 8: { // ������/�����
		CString tmp; tmp.Format(_T("%.1f/%.1f"), it->shoesSize, it->clothersSize);
		wcscpy(szBuf, tmp.GetString());
		pDispInfo->item.pszText = szBuf;
		break; }
	case 9: { // �����
		CString tmp; tmp.Empty();
		for (int i = 0; i < (int)it->weapons.size(); i++) {
			tmp.Append(weaponId2fullName(it->weapons[i]));
			if (i != it->weapons.size() - 1) {
				tmp.Append(_T(", "));
			}
		}
		wcscpy(szBuf, tmp.GetString());
		pDispInfo->item.pszText = szBuf;
		break; }
	case 10: // ������� ������
		wcscpy(szBuf, it->comment.GetString());
		pDispInfo->item.pszText = szBuf;
		break;
	}
}

CString CTrODlg::positionId2positionName(int id) {
	CString res = _T("");
	for (int i = 0; i < (int)pLogic->pPositions.positions.size(); i++) {
		if (id == pLogic->pPositions.positions[i].id) {
			res = pLogic->pPositions.positions[i].name;
			break;
		}
	}
	return res;
}

CString CTrODlg::positionId2rankMaxName(int id) {
	CString res = _T("");
	int rankId = -1;
	for (int i = 0; i < (int)pLogic->pPositions.positions.size(); i++) {
		if (id == pLogic->pPositions.positions[i].id) {
			rankId = pLogic->pPositions.positions[i].rankId;
			break;
		}
	}
	return rankId2rankName(rankId);
}

CString CTrODlg::rankId2rankName(int id) {
	CString res = _T("");
	for (int i = 0; i < (int)pLogic->pRanks.ranks.size(); i++) {
		if (id == pLogic->pRanks.ranks[i].id) {
			res = pLogic->pRanks.ranks[i].name;
			break;
		}
	}
	return res;
}

CString CTrODlg::weaponId2fullName(int id) {
	CString res = _T("");
	for (int i = 0; i < (int)pLogic->pWeaponNums.weaponNums.size(); i++) {
		if (id == pLogic->pWeaponNums.weaponNums[i].id) {
			for (int j = 0; j < (int)pLogic->pWeaponTypes.weaponTypes.size(); j++) {
				if (pLogic->pWeaponNums.weaponNums[i].typeId == pLogic->pWeaponTypes.weaponTypes[j].id) {
					res.Format(_T("%s %s"), pLogic->pWeaponTypes.weaponTypes[j].name.GetString(), pLogic->pWeaponNums.weaponNums[i].name.GetString());
					break;
				}
			}
			break;
		}
	}
	return res;
}

CString CTrODlg::relatives2str(int personId)
{
	CString res;
	std::vector<Relative> rels; rels.clear();
	for (int i = 0; i < (int)pLogic->pRelatives.relatives.size(); i++) {
		if (personId == pLogic->pRelatives.relatives[i].personId) {
			rels.push_back(pLogic->pRelatives.relatives[i]);
		}
	}

	for (int a = 0; a < (int)rels.size(); a++) {
		CString tmp; tmp.Format(_T("%s %s"), rels[a].fio.GetString(), rels[a].phone.GetString());
		res.Append(tmp.GetString());
		if (a != rels.size() - 1) {
			res.Append(_T(", "));
		}
	}

	return res;
}

void CTrODlg::MenuHandlerPerson(UINT id)
{
	switch (id)
	{
	case 32776:// ��������������
		editPerson();
		break;

	case 32777: // �������
		deletePerson();
		break;
	}
}

void CTrODlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_grid.GetRowCount() > 0) {
		popupmenu.GetSubMenu(0)->TrackPopupMenu(TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
	}
}

void CTrODlg::editPerson()
{
	// ������ ������ ����� ������
	Person pOne;
	pOne = pLogic->pPersons.persons[m_grid.GetCellFocused().m_iRow];

	std::vector<Relative> rels; rels.clear();
	for (int i = 0; i < (int)pLogic->pRelatives.relatives.size(); i++) {
		if (pOne.id == pLogic->pRelatives.relatives[i].personId) {
			rels.push_back(pLogic->pRelatives.relatives[i]);
		}
	}
	PersonDlg dlg;
	dlg.pLogic = pLogic;
	dlg.pPerson = &pOne;
	dlg.rels = &rels;
	if (IDOK == dlg.DoModal()) {
		// ������� � ���
		pLogic->pPersons.persons[m_grid.GetCellFocused().m_iRow] = pOne;
		// � �������������� ����� �����. ������� �������� ���� ����������� � �������� �������
		for (int i = 0; i < (int)pLogic->pRelatives.relatives.size(); i++) {
			if (pOne.id == pLogic->pRelatives.relatives[i].personId) {
				std::vector<Relative>::iterator it;	it = pLogic->pRelatives.relatives.begin();
				std::advance(it, i--);
				pLogic->pRelatives.relatives.erase(it);
			}
		}
		// � ����� ���������� � �������� ������ ��, ��� ���� � rels
		for (int i = 0; i < (int)rels.size(); i++) {
			if (rels[i].id == -1) { // ����� ��������� ����� �����������
				rels[i].id = pLogic->ids.relative_id++;
			}
			pLogic->pRelatives.relatives.push_back(rels[i]);
		}
		m_grid.InvalidateGrid();
	}
}

void CTrODlg::deletePerson() 
{
	CString message; 
	message.Format(_T("�� ����� ������ �������� \"%s\" �� ����?"), pLogic->pPersons.persons[m_grid.GetCellFocused().m_iRow].fio.GetString());
	if (IDYES != MessageBox(message.GetString(), _T("���������"), MB_YESNO))	return;

	std::vector<Person>::iterator itPerson = pLogic->pPersons.persons.begin();
	std::advance(itPerson, m_grid.GetCellFocused().m_iRow);
	// ��� �������� �����, ������� ���� ��� �������������
	for (int i = 0; i < (int)pLogic->pRelatives.relatives.size(); i++) {
		if (itPerson->id == pLogic->pRelatives.relatives[i].personId) {
			std::vector<Relative>::iterator it;	it = pLogic->pRelatives.relatives.begin();
			std::advance(it, i--);
			pLogic->pRelatives.relatives.erase(it);
		}
	}

	pLogic->pPersons.persons.erase(itPerson);
	m_grid.SetRowCount((int)pLogic->pPersons.persons.size());
	m_grid.InvalidateGrid();
}

void CTrODlg::MenuHandlerExport(UINT id)
{
	switch (id)
	{
	case 32779:// ����� � ������ ������� ������� 
		totalExcelExpert();
		break;

	case 32780: // 
		//...
		break;
	}
}

std::vector<ID_NAME> CTrODlg::divizionSort()
{
	std::vector<ID_NAME> result; result.clear();
	std::vector<ID_NAME> rota; rota.clear();

	// �������� ��� ����
	for (int i = 0; i < (int)pLogic->pDivizions.divisions.size(); i++) {
		if (pLogic->pDivizions.divisions[i].rotaId == -1 && pLogic->pDivizions.divisions[i].vzvodId == -1) {
			ID_NAME pOne(pLogic->pDivizions.divisions[i].id, pLogic->pDivizions.divisions[i].name);
			rota.push_back(pOne);
		}
	}

	for (int i = 0; i < (int)rota.size(); i++) {
		result.push_back(rota[i]);// ������ ������ ����

		// �������� ������ ��� ����
		for (int j = 0; j < (int)pLogic->pDivizions.divisions.size(); j++) {
			if (pLogic->pDivizions.divisions[j].rotaId == rota[i].id && pLogic->pDivizions.divisions[j].vzvodId == -1) {
				CString aaa;  aaa.Format(_T("%s %s"), rota[i].name.GetString(), pLogic->pDivizions.divisions[j].name.GetString());
				ID_NAME pOne(pLogic->pDivizions.divisions[j].id,aaa);
				result.push_back(pOne);// ������ ������ i-� ����� ����
				for (int a = 0; a < (int)pLogic->pDivizions.divisions.size(); a++) {
					if (pLogic->pDivizions.divisions[j].rotaId == rota[i].id && pLogic->pDivizions.divisions[a].vzvodId == pOne.id) {
						CString bbb; bbb.Format(_T("%s %s %s"), rota[i].name.GetString(), pLogic->pDivizions.divisions[j].name.GetString(), pLogic->pDivizions.divisions[a].name.GetString());
						ID_NAME pTwo(pLogic->pDivizions.divisions[a].id,bbb);		
						result.push_back(pTwo);// ������ ������ i-� ��������� ������ ����
					}
				}
			}
		}
	}

	return result;
}

void CTrODlg::totalExcelExpert()
{
	std::vector<ID_NAME> divizionsToExcel = divizionSort();
	std::vector<Person> totalOutput; totalOutput.clear();
	std::vector<Person> tmpArr; 
	
	//std::sort(tmpArr.begin(), tmpArr.end(), [](Person const& a, Person const& b) {return a.rankId < b.rankId;});
	// ���������� ������ � ������� �� ����-�����-���������
	// ����� ������ ���������� ������ ���������� �� ������� ������
	// �� � ���������� �������� ������� � ������, �������� ������ ����-�����-��������� 
	for (int i = 0; i < (int)divizionsToExcel.size(); i++) {
		tmpArr.clear();

		for (int a = 0; a < (int)pLogic->pPersons.persons.size(); a++) {
			// ��� ����� ���� � �������� divizionId 
			if (divizionsToExcel[i].id == pLogic->pPersons.persons[a].divizionId) {
				tmpArr.push_back(pLogic->pPersons.persons[a]);
			}
		}
		// ���������� �� �������. ��� ������ ID, ��� ���� ������
		std::sort(tmpArr.begin(), tmpArr.end(), [](Person const& a, Person const& b) {return a.rankId < b.rankId;});
		Person pPseudoPerson;
		pPseudoPerson.id = -15;
		pPseudoPerson.fio = divizionsToExcel[i].name;
		totalOutput.push_back(pPseudoPerson);
		for (int a = 0; a < (int)tmpArr.size(); a++) {
			totalOutput.push_back(tmpArr[a]);
		}
	}

	// ��� � ��� ���� totalOutput, � ������� ��� ���������� ����������. 
	// ����� �������� � MSExcel
	::CoInitialize(NULL);
	using namespace Excel;
	_ApplicationPtr excel(_T("Excel.Application"));
	// ������ ����� �����
	_WorkbookPtr  book = excel->Workbooks->Add();
	_WorksheetPtr sheet = book->Worksheets->Item[1L];
	sheet->Range[_T("A1")]->Activate();
	// �������
	Excel::RangePtr pRange;
	pRange = sheet->Cells->Item[1][1];
	pRange->ColumnWidth = 4.0f;
	pRange = sheet->Cells->Item[1][2];
	pRange->ColumnWidth = 20.0f;
	pRange = sheet->Cells->Item[1][3];
	pRange->ColumnWidth = 6.0f;
	pRange = sheet->Cells->Item[1][4];
	pRange->ColumnWidth = 14.0f;
	pRange = sheet->Cells->Item[1][5];
	pRange->ColumnWidth = 14.0f;
	pRange = sheet->Cells->Item[1][6];
	pRange->ColumnWidth = 32.0f;
	pRange = sheet->Cells->Item[1][7];
	pRange->ColumnWidth = 11.0f;

	pRange = sheet->Cells->Item[1][8];
	pRange->ColumnWidth = 8.0f;
	pRange = sheet->Cells->Item[1][9];
	pRange->ColumnWidth = 11.55f;
	pRange = sheet->Cells->Item[1][10];
	pRange->ColumnWidth = 30.0f;
	pRange = sheet->Cells->Item[1][11];
	pRange->ColumnWidth = 13.55f;
	pRange = sheet->Cells->Item[1][12];
	pRange->ColumnWidth = 45.0f;
	pRange = sheet->Cells->Item[1][13];
	pRange->ColumnWidth = 35.0f;
	// �����
	sheet->Range[_T("A1")]->FormulaR1C1 = _T("� �.�.");
	sheet->Range[_T("B1")]->FormulaR1C1 = _T("������");
	sheet->Range[_T("C1")]->FormulaR1C1 = _T("���");
	sheet->Range[_T("D1")]->FormulaR1C1 = _T("������ ������");
	sheet->Range[_T("E1")]->FormulaR1C1 = _T("³������� ������");
	sheet->Range[_T("F1")]->FormulaR1C1 = _T("�.�.�.");
	sheet->Range[_T("G1")]->FormulaR1C1 = _T("���� ���������� �� ������");
	sheet->Range[_T("H1")]->FormulaR1C1 = _T("���");
	sheet->Range[_T("I1")]->FormulaR1C1 = _T("���� ����������");
	sheet->Range[_T("J1")]->FormulaR1C1 = _T("̳��� ���������");
	sheet->Range[_T("K1")]->FormulaR1C1 = _T("�������");
	sheet->Range[_T("L1")]->FormulaR1C1 = _T("������");
	sheet->Range[_T("M1")]->FormulaR1C1 = _T("�����");

	sheet->Range[_T("A1:M1")]->HorizontalAlignment = -4108;
	sheet->Range[_T("A1:M1")]->VerticalAlignment = -4108;
	sheet->Range[_T("A1:M1")]->WrapText = 1;
	sheet->Range[_T("A1:M1")]->Font->Bold = true;

	CString tmp;
	int curIndex = 1;

	for (int i = 0; i < (int)totalOutput.size(); i++) {
		if (totalOutput[i].id == -15) {
			// ���������� ��� ������ �� � �� �
			// �������� � ��� totalOutput[i].fio 
			CString mergeCells; mergeCells.Format(_T("A%d:M%d"), i + 2, i + 2);
			tmp = totalOutput[i].fio;
			pRange = sheet->Cells->Item[i + 2][1];
			sheet->Range[mergeCells.GetString()]->Interior->Color = RGB(185,185,185);
			sheet->Range[mergeCells.GetString()]->HorizontalAlignment = -4108;
			sheet->Range[mergeCells.GetString()]->VerticalAlignment = -4108;
			sheet->Range[mergeCells.GetString()]->MergeCells = true;
			sheet->Range[mergeCells.GetString()]->Font->Bold = true;
			sheet->Range[mergeCells.GetString()]->NumberFormat = _T("@");
			sheet->Range[mergeCells.GetString()]->Value = tmp.GetString();
		}
		else {
			tmp = Int2Str(curIndex).c_str();
			pRange = sheet->Cells->Item[i + 2][1];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = positionId2positionName(totalOutput[i].positionId);
			pRange = sheet->Cells->Item[i + 2][2];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();
			
			tmp = totalOutput[i].code;
			pRange = sheet->Cells->Item[i + 2][3];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = positionId2rankMaxName(totalOutput[i].positionId);
			pRange = sheet->Cells->Item[i + 2][4];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = rankId2rankName(totalOutput[i].rankId);
			pRange = sheet->Cells->Item[i + 2][5];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = totalOutput[i].fio;
			pRange = sheet->Cells->Item[i + 2][6];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = totalOutput[i].startDate;
			pRange = sheet->Cells->Item[i + 2][7];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = totalOutput[i].ubd;
			pRange = sheet->Cells->Item[i + 2][8];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = totalOutput[i].birthday;
			pRange = sheet->Cells->Item[i + 2][9];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = totalOutput[i].address;
			pRange = sheet->Cells->Item[i + 2][10];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			tmp = totalOutput[i].phone;
			pRange = sheet->Cells->Item[i + 2][11];
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			//12 ������
			tmp = relatives2str(totalOutput[i].id);
			pRange = sheet->Cells->Item[i + 2][12];
			pRange->NumberFormat = _T("@");
			pRange->WrapText = 1;
			pRange->Value = tmp.GetString();

			tmp.Empty();
			for (int a = 0; a < (int)totalOutput[i].weapons.size(); a++) {
				tmp.Append(weaponId2fullName(totalOutput[i].weapons[a]));
				if (a != totalOutput[i].weapons.size() - 1) {
					tmp.Append(_T(", "));
				}
			}
			pRange = sheet->Cells->Item[i + 2][13];
			pRange->WrapText = 1;
			pRange->NumberFormat = _T("@");
			pRange->Value = tmp.GetString();

			curIndex++;
		}
	}
	// �����
	tmp.Format(_T("A1:M%d"), (int)totalOutput.size() + 1);	
	sheet->Range[tmp.GetString()]->Borders->PutValue(1);
	excel->Visible = true;
	::CoUninitialize();
}