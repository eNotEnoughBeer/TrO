#pragma once
#include <atlstr.h>
#include "HelperFunctions.h"
#include "Division.h"
#include "Person.h"
#include "Position.h"
#include "Rank.h"
#include "Relative.h"
#include "WeaponNum.h"
#include "WeaponType.h"
#include "UnicIds.h"

#import "EXCEL8.OLB" auto_search auto_rename

class MainLogic {
public:
	CString encriptionKey;
	bool isConnected;
	Excel::_WorkbookPtr book;
	Excel::_WorksheetPtr sheetDivizions;
	Excel::_WorksheetPtr sheetPersons;
	Excel::_WorksheetPtr sheetPositions;
	Excel::_WorksheetPtr sheetRanks;
	Excel::_WorksheetPtr sheetRelatives;
	Excel::_WorksheetPtr sheetWeaponNums;
	Excel::_WorksheetPtr sheetWeaponTypes;
	Excel::_WorksheetPtr sheetUnicIds;

public:	
	Divizions pDivizions;
	Persons pPersons;
	Positions pPositions;
	Ranks pRanks;
	Relatives pRelatives;
	WeaponNums pWeaponNums;
	WeaponTypes pWeaponTypes;
	UnicIds ids;

	void connectXLS() {
		isConnected = false;
		CString dbPath; 
		dbPath.Format(_T("%s\\db.xls"), getCurPath().GetString());
		::CoInitialize(NULL);
		Excel::_ApplicationPtr excel(_T("Excel.Application"));
		book = excel->Workbooks->Open(dbPath.GetString());
		if (book == NULL)
		{
			MessageBox(NULL, _T("db.xls"), _T("Не знайдена БД"), 0);
		}
		else
		{
			sheetDivizions = book->Worksheets->GetItem(_T("DIVIZION"));
			sheetPersons = book->Worksheets->GetItem(_T("PERSON"));
			sheetPositions = book->Worksheets->GetItem(_T("POSITION"));
			sheetRanks = book->Worksheets->GetItem(_T("RANK"));
			sheetRelatives = book->Worksheets->GetItem(_T("RELATIVE"));
			sheetWeaponNums = book->Worksheets->GetItem(_T("WEPNUM"));
			sheetWeaponTypes = book->Worksheets->GetItem(_T("WEPTYP"));
			sheetUnicIds = book->Worksheets->GetItem(_T("UNICIDS"));
			isConnected = true;
		}
	}

	void setEncriptionKey(CString key) {
		encriptionKey = key;
	}

	void freeXLS() {
		if (isConnected) {
			book->Close();
			::CoUninitialize();
			isConnected = false;
		}
	}

	void Read() {
		if (isConnected){
			pDivizions.Read(sheetDivizions, encriptionKey);
			pPersons.Read(sheetPersons, encriptionKey);
			pPositions.Read(sheetPositions, encriptionKey);
			pRanks.Read(sheetRanks, encriptionKey);
			pRelatives.Read(sheetRelatives, encriptionKey);
			pWeaponNums.Read(sheetWeaponNums, encriptionKey);
			pWeaponTypes.Read(sheetWeaponTypes, encriptionKey);
			ids.Read(sheetUnicIds, encriptionKey);
		}
	}
	void Write() {
		if (isConnected) {
			pDivizions.Write(book, sheetDivizions, encriptionKey);
			pPersons.Write(book, sheetPersons, encriptionKey);
			pPositions.Write(book, sheetPositions, encriptionKey);
			pRanks.Write(book, sheetRanks, encriptionKey);
			pRelatives.Write(book, sheetRelatives, encriptionKey);
			pWeaponNums.Write(book, sheetWeaponNums, encriptionKey);
			pWeaponTypes.Write(book, sheetWeaponTypes, encriptionKey);
			ids.Write(book, sheetUnicIds, encriptionKey);
		}
	}
};