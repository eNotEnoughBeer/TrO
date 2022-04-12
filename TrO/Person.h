#pragma once
#include <atlstr.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#import "EXCEL8.OLB" auto_search auto_rename

class Person
{
public:
	int id;
	int positionId;
	CString code;
	int rankId;
	CString fio;
	CString callsign;
	CString birthday;
	CString address;
	CString phone;
	double shoesSize;
	double clothersSize;
	std::vector<int> weapons;
	CString comment;
	int divizionId;
	CString ubd;
	CString startDate;
	Person() {
		id = -1;
		positionId = -1;
		code = _T("");
		rankId = -1;
		fio = _T("");
		callsign = _T("");
		birthday = _T("");
		address = _T("");
		phone = _T("");
		shoesSize = 0.0;
		clothersSize = 0.0;
		weapons.clear();
		comment = _T("");
		divizionId = -1;
		ubd = _T("");
		startDate = _T("");
	}
	Person(int newId) {
		id = newId;
		positionId = -1;
		code = _T("");
		rankId = -1;
		fio = _T("");
		callsign = _T("");
		birthday = _T("");
		address = _T("");
		phone = _T("");
		shoesSize = 0.0;
		clothersSize = 0.0;
		weapons.clear();
		comment = _T("");
		divizionId = -1;
		ubd = _T("");
		startDate = _T("");
	}
};

class Persons {
public:
	std::vector<Person> persons;
	Persons() {
		persons.clear();
	}

	std::vector<int> string2mas(CString str) {
		std::vector<int> mas; mas.clear();
		std::wistringstream iss(str.GetString());
		std::vector<std::wstring> tokens;
		std::copy(std::istream_iterator<std::wstring, wchar_t>(iss),
			std::istream_iterator<std::wstring, wchar_t>(),
			std::back_inserter<std::vector<std::wstring> >(tokens));
		for (int i = 0; i < (int)tokens.size(); i++) {
			mas.push_back(Str2Int(tokens[i].c_str()));
		}

		return mas;
	}

	CString mas2string(std::vector<int> mas) {
		CString str; str.Empty();
		for (int i = 0; i < (int)mas.size(); i++) {
			str.Append(Int2Str(mas[i]).c_str());
			if (i != (int)mas.size() - 1) {
				str.AppendChar(_T(' '));// через символ пробела идут иды
			}
		}
		return str;
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
				// закладка пустая 
				return;
			}

			persons.resize((size_t)(nRows));
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
							else {
								decodeStr(key, tmp);
								persons[i].id = Str2Int(tmp.GetString());
							}
							break;
						case 1:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].positionId = Str2Int(tmp.GetString());
							}
							break;
						case 2:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].code = tmp;
							}
							break;
						case 3:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].rankId = Str2Int(tmp.GetString());
							}
							break;
						case 4:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].fio = tmp;
							}
							break;
						case 5:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].callsign = tmp;
							}
							break;
						case 6:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].birthday = tmp;
							}
							break;
						case 7:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].address = tmp;
							}
							break;
						case 8:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].phone = tmp;
							}
							break;
						case 9:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].shoesSize = Str2Dbl(tmp.GetString());
							}
							break;
						case 10:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].clothersSize = Str2Dbl(tmp.GetString());
							}
							break;
						case 11:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].weapons = string2mas(tmp);
							}
							break;
						case 12:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].comment = tmp;
							}
							break;
						case 13:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].divizionId = Str2Int(tmp.GetString());
							}
							break;
						case 14:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].ubd = tmp;
							}
							break;
						case 15:
							tmp = ((std::wstring)_bstr_t(val)).c_str();
							if (!tmp.IsEmpty()) {
								decodeStr(key, tmp);
								persons[i].startDate = tmp;
							}
							break;
						}
					}
					catch (_com_error& er)
					{ // Если поле имеет недопустимый формат
						CString s; s.Format(L"Помилка читання PERSON. Код: 0x%8X %s [%d, %d]", er.Error(), er.ErrorMessage(), i, j);
						AfxMessageBox(s, MB_ICONERROR);
						persons.clear();
						return;
					}
				}
			}
		}
		catch (_com_error& err)
		{
			CString s; s.Format(L"Помилка читання PERSON. Код: 0x%8X %s [%d, %d]", err.Error(), err.ErrorMessage(), i, j);
			AfxMessageBox(s, MB_ICONERROR);
			persons.clear();
		}

		for (int a = (int)emptyRows.size() - 1; a >= 0; a--) {
			std::vector<Person>::iterator it = persons.begin();
			std::advance(it, emptyRows[a]);
			persons.erase(it);
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
			// нужно почистить всё 
			for (i = startRow; i <= endRow; i++) {
				for (j = startCol; j <= endCol; j++) {
					pRange = sheet->Cells->Item[i][j];
					pRange->Value = _T("");
				}
			}
		}

		for (i = 0; i < (long)persons.size(); i++) {
			tmp = Int2Str(persons[i].id).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][1];
			pRange->Value = tmp.GetString();

			tmp = Int2Str(persons[i].positionId).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][2];
			pRange->Value = tmp.GetString();

			tmp = persons[i].code;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][3];
			pRange->Value = tmp.GetString();

			tmp = Int2Str(persons[i].rankId).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][4];
			pRange->Value = tmp.GetString();

			tmp = persons[i].fio;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][5];
			pRange->Value = tmp.GetString();

			tmp = persons[i].callsign;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][6];
			pRange->Value = tmp.GetString();

			tmp = persons[i].birthday;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][7];
			pRange->Value = tmp.GetString();

			tmp = persons[i].address;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][8];
			pRange->Value = tmp.GetString();

			tmp = persons[i].phone;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][9];
			pRange->Value = tmp.GetString();

			tmp = Dbl2Str(persons[i].shoesSize,1).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][10];
			pRange->Value = tmp.GetString();

			tmp = Dbl2Str(persons[i].clothersSize, 1).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][11];
			pRange->Value = tmp.GetString();

			tmp = mas2string(persons[i].weapons);
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][12];
			pRange->Value = tmp.GetString();

			tmp = persons[i].comment;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][13];
			pRange->Value = tmp.GetString();

			tmp = Int2Str(persons[i].divizionId).c_str();
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][14];
			pRange->Value = tmp.GetString();

			tmp = persons[i].ubd;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][15];
			pRange->Value = tmp.GetString();

			tmp = persons[i].startDate;
			encodeStr(key, tmp);
			pRange = sheet->Cells->Item[i + 1][16];
			pRange->Value = tmp.GetString();
		}

		book->Save();
	}
};