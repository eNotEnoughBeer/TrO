#pragma once
#include <atlstr.h>
#include "sqlite3.h"
#include "HelperFunctions.h"
#include "Division.h"
#include "Person.h"
#include "Position.h"
#include "Rank.h"
#include "Relative.h"
#include "WeaponType.h"

class MainLogic {
public:
	CString encriptionKey;
	bool isConnected;
public:	
	sqlite3* db;
	Divizions pDivizions;
	Persons pPersons;
	Positions pPositions;
	Ranks pRanks;
	Relatives pRelatives;
	Weapons pWeapons;

	void Connect() {	
		if (sqlite3_open(getDBpath().c_str(), &db) != SQLITE_OK) {
			AfxMessageBox((LPCTSTR)sqlite3_errmsg(db));
			return;
		}
		isConnected = true;
		pRanks.db = db;
		pDivizions.db = db;
		pPersons.db = db;
		pPositions.db = db;
		pRelatives.db = db;
		pWeapons.db = db;
		checkTables();
	}

	~MainLogic() {
		if (isConnected)
			sqlite3_close(db);
	}

	std::string getDBpath() 
	{
		char fName[_MAX_PATH + 1];
		GetModuleFileNameA(nullptr, fName, sizeof(fName) - 1);
		int i = (int)strlen(fName);
		while ((i > 0) && (fName[i] != '\\')) i--;
		if (i)	fName[i] = '\0'; 
		std::string res = fName;
		res += "\\dbase.db";
		return res;
	}

	void checkTables() {
		int res = 0;
		res += pRanks.checkTable();
		res += pPositions.checkTable();
		res += pWeapons.checkTable();
		res += pRelatives.checkTable();
		res += pDivizions.checkTable();
		res += pPersons.checkTable();
		if (res) {
			AfxMessageBox(_T("Не вдалося ініціювати БД. Подальша робота не можлива"));
		}
	}

	void setEncriptionKey(CString key) {
		encriptionKey = key;
	}

	void ReadSQL() {
		if (isConnected) {
			pRanks.ReadSQL(encriptionKey);
			pPositions.ReadSQL(encriptionKey);
			pWeapons.ReadSQL(encriptionKey);
			pRelatives.ReadSQL(encriptionKey);
			pDivizions.ReadSQL(encriptionKey);		
			pPersons.ReadSQL(encriptionKey);
		}
	}

	void WriteSQL() {
		if (isConnected) {
			pRanks.WriteSQL(encriptionKey);
			pPositions.WriteSQL(encriptionKey);
			pWeapons.WriteSQL(encriptionKey);
			pRelatives.WriteSQL(encriptionKey);
			pDivizions.WriteSQL(encriptionKey);
			pPersons.WriteSQL(encriptionKey);
		}
	}
};