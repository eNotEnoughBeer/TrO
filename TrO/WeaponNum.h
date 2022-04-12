#pragma once
#include <atlstr.h>
#include <vector>
#import "EXCEL8.OLB" auto_search auto_rename

class WeaponNum
{
public:
	int id;
	int typeId;
	CString name;
	WeaponNum() {
		id = -1;
		typeId = -1;
		name = _T("");
	}
	WeaponNum(int index, int type_Id, CString curName) {
		id = index;
		typeId = type_Id;
		name = curName;
	}

};

class WeaponNums {
public:
	std::vector<WeaponNum> weaponNums;
	WeaponNums() {
		weaponNums.clear();
	}

	void Read(Excel::_WorksheetPtr sheet, CString key) {
		int nRows = 1, nCols = 1;
		long i = 0, j = 0;
		std::vector<int> emptyRows; emptyRows.clear();
		try {
			Excel::RangePtr pRange = sheet->UsedRange;
			int startRow = 1, endRow = (pRange->Rows->Count > nRows) ? pRange->Rows->Count : nRows; nRows = endRow;
			int startCol = 1, endCol = (pRange->Columns->Count > nCols) ? pRange->Columns->Count : nCols; nCols = endCol;
			if (startRow == endRow && startCol == endCol) {
				// �������� ������ 
				return;
			}

			weaponNums.resize((size_t)(nRows));
			_variant_t varr = pRange->GetValue2(), val;
			long iLBound[2] = { 0,0 }, iUBound[2] = { 0,0 };
			HRESULT hr;
			_bstr_t str;
			CString tmp;
			if (FAILED(hr = SafeArrayGetLBound(varr.parray, 1, &iLBound[0]))) throw _com_error(hr);
			if (FAILED(hr = SafeArrayGetUBound(varr.parray, 1, &iUBound[0]))) throw _com_error(hr);
			if (FAILED(hr = SafeArrayGetLBound(varr.parray, 2, &iLBound[1]))) throw _com_error(hr);
			if (FAILED(hr = SafeArrayGetUBound(varr.parray, 2, &iUBound[1]))) throw _com_error(hr);

			for (i = 0; i <= iUBound[0] - iLBound[0]; i++)
			{
				for (j = 0; j <= iUBound[1] - iLBound[1]; j++)
				{
					long ind[2] = { iLBound[0] + i, iLBound[1] + j };
					if (FAILED(hr = SafeArrayGetElement(varr.parray, ind, &val))) throw _com_error(hr);
					try
					{
						switch (j) {
						case 0:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (tmp.IsEmpty()) {
								emptyRows.push_back(i);
							}
							decodeStr(key, tmp);
							weaponNums[i].id = Str2Int(tmp.GetString());
							break;
						case 1:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							decodeStr(key, tmp);
							weaponNums[i].typeId = Str2Int(tmp.GetString());
							break;
						case 2:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							decodeStr(key, tmp);
							weaponNums[i].name = tmp;
							break;
						}

					}
					catch (_com_error& er)
					{ // ���� ���� ����� ������������ ������
						CString s; s.Format(L"������� ������� WEPNUM. ���: 0x%8X %s [%d, %d]", er.Error(), er.ErrorMessage(), i, j);
						AfxMessageBox(s, MB_ICONERROR);
						weaponNums.clear();
						return;
					}
				}
			}
		}
		catch (_com_error& err)
		{
			CString s; s.Format(L"������� ������� WEPNUM. ���: 0x%8X %s [%d, %d]", err.Error(), err.ErrorMessage(), i, j);
			AfxMessageBox(s, MB_ICONERROR);
			weaponNums.clear();
		}
		for (int a = 0; a < (int)emptyRows.size(); a++) {
			std::vector<WeaponNum>::iterator it = weaponNums.begin();
			std::advance(it, emptyRows[a]);
			weaponNums.erase(it);
		}
	}

	void Write(Excel::_WorkbookPtr book, Excel::_WorksheetPtr sheet, CString key) {
		int nRows = 1, nCols = 1;
		long i = 0, j = 0;
		CString tmp;

		Excel::RangePtr pRange = sheet->UsedRange;
		int startRow = 1, endRow = (pRange->Rows->Count > nRows) ? pRange->Rows->Count : nRows; nRows = endRow;
		int startCol = 1, endCol = (pRange->Columns->Count > nCols) ? pRange->Columns->Count : nCols; nCols = endCol;
		if (startRow != endRow || startCol != endCol) {
			// ����� ��������� �� 
			for (i = startRow; i <= endRow; i++) {
				for (j = startCol; j <= endCol; j++) {
					pRange = sheet->Cells->Item[i][j];
					pRange->Value = _T("");
				}
			}
		}

		for (i = 0; i < (long)weaponNums.size(); i++) {
			tmp = Int2Str(weaponNums[i].id).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][1];
			pRange->Value = tmp.GetString();

			tmp = Int2Str(weaponNums[i].typeId).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][2];
			pRange->Value = tmp.GetString();

			tmp = weaponNums[i].name;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][3];
			pRange->Value = tmp.GetString();
		}

		book->Save();
	}
};