#pragma once
#include <atlstr.h>
#include "sqlite3.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

	int WriteSQL(sqlite3* db, CString key) {
		USES_CONVERSION;
		std::string sql;
		std::string _str;
		CString tmp;
		if (id == -1) {
			//insert
			CString _code = this->code;
			encodeStr(key, _code);
			CString _fio = this->fio;
			encodeStr(key, _fio);
			CString _callsign = this->callsign;
			encodeStr(key, _callsign);
			CString _birthday = this->birthday;
			encodeStr(key, _birthday);
			CString _address = this->address;
			encodeStr(key, _address);
			CString _phone = this->phone;
			encodeStr(key, _phone);
			CString _shoes = Dbl2Str(this->shoesSize, 1).c_str();
			encodeStr(key, _shoes);
			CString _clothers = Dbl2Str(this->clothersSize, 1).c_str();
			encodeStr(key, _clothers);
			CString _weapon = mas2string(this->weapons);
			encodeStr(key, _weapon);
			CString _comment = this->comment;
			encodeStr(key, _comment);
			CString _ubd = this->ubd;
			encodeStr(key, _ubd);
			CString _stDate = this->startDate;
			encodeStr(key, _stDate);

			CString sql_wchar; sql_wchar.Format(_T("INSERT INTO PERSON(POS_ID, CDE, R_ID, FIO, CSGN, BRTH, ADDR, PHN, SHS, CLTH, WPN, CMNT, DIV_ID, UDB, STDT) VALUES(%d, \'%s\', %d, \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', %d, \'%s\', \'%s\'); "),
				this->positionId,
				_code.GetString(),
				this->rankId,
				_fio.GetString(),
				_callsign.GetString(),
				_birthday.GetString(),
				_address.GetString(),
				_phone.GetString(),
				_shoes.GetString(),
				_clothers.GetString(),
				_weapon.GetString(),
				_comment.GetString(),
				this->divizionId,
				_ubd.GetString(),
				_stDate.GetString());
			
			sql = get_utf8(sql_wchar.GetString());
			char* errMsg;
			if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
				sqlite3_free(errMsg);
				return 1;
			}
			this->id = (int)sqlite3_last_insert_rowid(db);
		}
		else {
			//update
			CString _code = this->code;
			encodeStr(key, _code);
			CString _fio = this->fio;
			encodeStr(key, _fio);
			CString _callsign = this->callsign;
			encodeStr(key, _callsign);
			CString _birthday = this->birthday;
			encodeStr(key, _birthday);
			CString _address = this->address;
			encodeStr(key, _address);
			CString _phone = this->phone;
			encodeStr(key, _phone);
			CString _shoes = Dbl2Str(this->shoesSize, 1).c_str();
			encodeStr(key, _shoes);
			CString _clothers = Dbl2Str(this->clothersSize, 1).c_str();
			encodeStr(key, _clothers);
			CString _weapon = mas2string(this->weapons);
			encodeStr(key, _weapon);
			CString _comment = this->comment;
			encodeStr(key, _comment);
			CString _ubd = this->ubd;
			encodeStr(key, _ubd);
			CString _stDate = this->startDate;
			encodeStr(key, _stDate);
																																									
			CString sql_wchar; sql_wchar.Format(
				_T("UPDATE PERSON SET POS_ID=%d, CDE=\'%s\', R_ID=%d, FIO=\'%s\', CSGN=\'%s\', BRTH=\'%s\', ADDR=\'%s\', PHN=\'%s\', SHS=\'%s\', CLTH=\'%s\', WPN=\'%s\', CMNT=\'%s\', DIV_ID=%d, UDB=\'%s\', STDT=\'%s\' WHERE ID=%d; "),
				this->positionId,
				_code.GetString(),
				this->rankId,
				_fio.GetString(),
				_callsign.GetString(),
				_birthday.GetString(),
				_address.GetString(),
				_phone.GetString(),
				_shoes.GetString(),
				_clothers.GetString(),
				_weapon.GetString(),
				_comment.GetString(),
				this->divizionId,
				_ubd.GetString(),
				_stDate.GetString(),
				this->id);

			sql = get_utf8(sql_wchar.GetString());
			char* errMsg;
			if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
				sqlite3_free(errMsg);
				return 1;
			}
		}
		return 0;
	}
	
};

class Persons {
public:
	sqlite3* db;
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
	
	int checkTable()
	{
		char* errMsg;
		std::string sql = "CREATE TABLE IF NOT EXISTS PERSON("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"POS_ID INTEGER, "
			"CDE TEXT, "
			"R_ID INTEGER, "
			"FIO TEXT, "
			"CSGN TEXT, "
			"BRTH TEXT, "
			"ADDR TEXT, "
			"PHN TEXT, "
			"SHS TEXT, "
			"CLTH TEXT, "
			"WPN TEXT, "
			"CMNT TEXT, "
			"DIV_ID INTEGER, "
			"UDB TEXT, "
			"STDT TEXT); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		return 0;
	}

	void ReadSQL(CString key) {
		USES_CONVERSION;
		persons.clear();
		CString tmp;
		Person pOnePerson;
		sqlite3_stmt* stmt;
		std::string sql = "SELECT * FROM PERSON ORDER BY ID";
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				tmp = get_utf16((char*)sqlite3_column_text(stmt, 0)).c_str();
				pOnePerson.id = Str2Int(tmp.GetString());

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 1)).c_str();
				pOnePerson.positionId = Str2Int(tmp.GetString());

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 2)).c_str();
				decodeStr(key, tmp);
				pOnePerson.code = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 3)).c_str();
				pOnePerson.rankId = Str2Int(tmp.GetString());

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 4)).c_str();
				decodeStr(key, tmp);
				pOnePerson.fio= tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 5)).c_str();
				decodeStr(key, tmp);
				pOnePerson.callsign = tmp;
				
				tmp = get_utf16((char*)sqlite3_column_text(stmt, 6)).c_str();
				decodeStr(key, tmp);
				pOnePerson.birthday = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 7)).c_str();
				decodeStr(key, tmp);
				pOnePerson.address = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 8)).c_str();
				decodeStr(key, tmp);
				pOnePerson.phone = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 9)).c_str();
				decodeStr(key, tmp);
				pOnePerson.shoesSize = Str2Dbl(tmp);

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 10)).c_str();
				decodeStr(key, tmp);
				pOnePerson.clothersSize = Str2Dbl(tmp);

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 11)).c_str();
				decodeStr(key, tmp);
				pOnePerson.weapons = string2mas(tmp);

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 12)).c_str();
				decodeStr(key, tmp);
				pOnePerson.comment = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 13)).c_str();
				pOnePerson.divizionId = Str2Int(tmp.GetString());

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 14)).c_str();
				decodeStr(key, tmp);
				pOnePerson.ubd = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 15)).c_str();
				decodeStr(key, tmp);
				pOnePerson.startDate = tmp;

				persons.push_back(pOnePerson);
			}

			sqlite3_finalize(stmt);
		}
	}
	
	int WriteSQL(CString key) {
		int res = 0;
		for (int i = 0; i < (int)persons.size(); i++) {
			res += persons[i].WriteSQL(db, key);
		}
		return res;
	}

	int DeleteSQL(int id2del)
	{
		if (id2del < 0)
			return 0;

		// собственно удаление
		sqlite3_stmt* stmt;
		std::string sql = "DELETE FROM PERSON WHERE ID=?";
		if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
			return 1;
		}

		sqlite3_bind_int(stmt, 1, id2del);

		if (sqlite3_step(stmt) != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			return 1;
		}

		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);
		
		return 0;
	}
};