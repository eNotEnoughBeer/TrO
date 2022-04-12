#pragma once
#include <atlstr.h>
#include "HelperFunctions.h"
#import "EXCEL8.OLB" auto_search auto_rename

class Password
{ //!!! ���� ����� ������: 123456789
private:
	bool isConnected;
	Excel::_WorkbookPtr book;
	Excel::_WorksheetPtr sheet;
public:
	CString password;
	CString hash;
	int badlogin;

	Password() {
		isConnected = false;
		CString dbPath; dbPath.Format(_T("%s\\db.xls"), getCurPath().GetString());
		::CoInitialize(NULL);
		Excel::_ApplicationPtr excel(_T("Excel.Application"));
		book = excel->Workbooks->Open(dbPath.GetString());
		if (book == NULL)
		{
			MessageBox(NULL, _T("db.xls"), _T("�� �������� ��"), 0);
		}
		else
		{
			sheet = book->Worksheets->GetItem(_T("PASSWD"));
			isConnected = true;
		}
	}

	void freeXLS() {
		if (isConnected) {
			book->Close();
			::CoUninitialize();
			isConnected = false;
		}
	}

	bool Read() {
		bool res = false;
		if(isConnected)
		{
			Excel::RangePtr pRange;
			_bstr_t bstr;
			

			pRange = sheet->Cells->Item[1][1]; // "�1"
			bstr = pRange->Value;			
			hash = ((std::wstring)bstr).c_str();

			pRange = sheet->Cells->Item[1][2]; // "�2"
			bstr = pRange->Value;
			CString tmp = ((std::wstring)bstr).c_str();
			if (tmp.Compare(_T("-15")) != 0) {
				decodeStr(_T("LCTTRO"), tmp); // ��� ��������� �� �� ������ � �� �������, �.�. ������ ����� ������ �� ������
			}
			badlogin = Str2Int(tmp);

			res = true;
		}

		return res;
	}

	bool Write() {
		bool res = false;
		if(isConnected)
		{
			Excel::RangePtr pRange;
			_bstr_t bstr;

			if (badlogin == -15) {
				// ��� �������� ������
				badlogin = 0;
				pRange = sheet->Cells->Item[1][1]; // "�2"
				pRange->Value = hash.GetString();
			}

			CString tmp = Int2Str(badlogin).c_str();
			encodeStr(_T("LCTTRO"), tmp); // ��� ��������� �� �� ������ � �� �������, �.�. ������ ����� ������ �� ������
			pRange = sheet->Cells->Item[1][2]; // "�2"
			pRange->Value = tmp.GetString();
			book->Save();
			res = true;
		}

		return res;
	}

};