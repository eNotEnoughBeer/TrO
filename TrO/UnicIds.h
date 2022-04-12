#pragma once
#include <atlstr.h>
#include "HelperFunctions.h"
#import "EXCEL8.OLB" auto_search auto_rename

class UnicIds
{
public:
	int divizion_id;
	int person_id;
	int position_id;
	int rank_id;
	int relative_id;
	int wepnum_id;
	int weptype_id;

	void Read(Excel::_WorksheetPtr sheet, CString key) {
		Excel::RangePtr pRange;
		_bstr_t bstr;
		CString tmp;

		pRange = sheet->Cells->Item[1][1]; // "a1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		divizion_id = Str2Int(tmp);

		pRange = sheet->Cells->Item[2][1]; // "b1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		person_id = Str2Int(tmp);

		pRange = sheet->Cells->Item[3][1]; // "c1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		position_id = Str2Int(tmp);

		pRange = sheet->Cells->Item[4][1]; // "d1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		rank_id = Str2Int(tmp);

		pRange = sheet->Cells->Item[5][1]; // "e1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		relative_id = Str2Int(tmp);

		pRange = sheet->Cells->Item[6][1]; // "f1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		wepnum_id = Str2Int(tmp);

		pRange = sheet->Cells->Item[7][1]; // "g1"
		bstr = pRange->Value;
		tmp = ((std::wstring)bstr).c_str();
		if (!tmp.IsEmpty()) {
			decodeStr(key, tmp);
		}
		else tmp = _T("1");
		weptype_id = Str2Int(tmp);

	}

	void Write(Excel::_WorkbookPtr book, Excel::_WorksheetPtr sheet, CString key) {
		Excel::RangePtr pRange;
		_bstr_t bstr;
		CString tmp;

		tmp = Int2Str(divizion_id).c_str();
		encodeStr(key, tmp); 
		pRange = sheet->Cells->Item[1][1]; // "à1"
		pRange->Value = tmp.GetString();

		tmp = Int2Str(person_id).c_str();
		encodeStr(key, tmp);
		pRange = sheet->Cells->Item[2][1]; // "b1"
		pRange->Value = tmp.GetString();

		tmp = Int2Str(position_id).c_str();
		encodeStr(key, tmp);
		pRange = sheet->Cells->Item[3][1]; // "c1"
		pRange->Value = tmp.GetString();

		tmp = Int2Str(rank_id).c_str();
		encodeStr(key, tmp);
		pRange = sheet->Cells->Item[4][1]; // "d1"
		pRange->Value = tmp.GetString();

		tmp = Int2Str(relative_id).c_str();
		encodeStr(key, tmp);
		pRange = sheet->Cells->Item[5][1]; // "e1"
		pRange->Value = tmp.GetString();

		tmp = Int2Str(wepnum_id).c_str();
		encodeStr(key, tmp);
		pRange = sheet->Cells->Item[6][1]; // "f1"
		pRange->Value = tmp.GetString();

		tmp = Int2Str(weptype_id).c_str();
		encodeStr(key, tmp);
		pRange = sheet->Cells->Item[7][1]; // "g1"
		pRange->Value = tmp.GetString();

		book->Save();
	}
};