#pragma once
#include <atlstr.h>
#include "sqlite3.h"
#include <vector>

class Relative
{
public:
	int id;
	CString fio;
	CString phone;
	int personId;
	Relative() {
		id = -1;
		fio = _T("");
		phone = _T("");
		personId = -1;
	}
	Relative(int index, int person_Id, CString relFio, CString relPhone) {
		id = index;
		fio = relFio;
		phone = relPhone;
		personId = person_Id;
	}

	int WriteSQL(sqlite3* db, CString key, int rowpos = 0) {
		std::string sql;
		if (this->id == -1) {
			//insert
			CString _fio = this->fio;
			encodeStr(key, _fio);
			CString _phone = this->phone;
			encodeStr(key, _phone);
			CString sql_wchar; sql_wchar.Format(_T("INSERT INTO RELATIVE(NAME, PHONE, PERSON_ID, ROWPOS) VALUES (\'%s\', \'%s\', %d, %d);"),
				_fio.GetString(), _phone.GetString(), this->personId, rowpos);
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
			CString _fio = this->fio;
			encodeStr(key, _fio);
			CString _phone = this->phone;
			encodeStr(key, _phone);
			CString sql_wchar; sql_wchar.Format(_T("UPDATE RELATIVE SET NAME=\'%s\', PHONE=\'%s\', PERSON_ID=%d, ROWPOS=%d WHERE ID= %d;"),
				_fio.GetString(), _phone.GetString(), this->personId, rowpos, this->id);
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

class Relatives {
public:
	sqlite3* db;
	std::vector<Relative> relatives;
	Relatives() {
		relatives.clear();
	}

	int checkTable()
	{
		char* errMsg;
		std::string sql = "CREATE TABLE IF NOT EXISTS RELATIVE("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"NAME TEXT, "
			"PHONE TEXT, "
			"PERSON_ID INTEGER, "
			"ROWPOS INTEGER NOT NULL); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		return 0;
	}

	void ReadSQL(CString key) {
		//USES_CONVERSION;
		relatives.clear();
		CString tmp;
		Relative pOneRelative;
		sqlite3_stmt* stmt;
		std::string sql = "SELECT * FROM RELATIVE ORDER BY ROWPOS;";
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				tmp = get_utf16((char*)sqlite3_column_text(stmt, 0)).c_str();
				pOneRelative.id = Str2Int(tmp.GetString());

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 1)).c_str();
				decodeStr(key, tmp);
				pOneRelative.fio = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 2)).c_str();
				decodeStr(key, tmp);
				pOneRelative.phone = tmp;

				tmp = get_utf16((char*)sqlite3_column_text(stmt, 3)).c_str();
				pOneRelative.personId = Str2Int(tmp.GetString());

				relatives.push_back(pOneRelative);
			}

			sqlite3_finalize(stmt);
		}
	}

	int WriteSQL(CString key) {
		//USES_CONVERSION;
		//sqlite3_stmt* stmt;
		std::string sql;
		for (int i = 0; i < (int)relatives.size(); i++) {
			if (relatives.at(i).id == -1) {
				//insert
				CString _fio = relatives.at(i).fio;
				encodeStr(key, _fio);
				CString _phone = relatives.at(i).phone;
				encodeStr(key, _phone);
				CString sql_wchar; sql_wchar.Format(_T("INSERT INTO RELATIVE(NAME, PHONE, PERSON_ID, ROWPOS) VALUES (\'%s\', \'%s\', %d, %d);"),
					_fio.GetString(), _phone.GetString(), relatives.at(i).personId, i);
				sql = get_utf8(sql_wchar.GetString());
				char* errMsg;
				if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
					sqlite3_free(errMsg);
					return 1;
				}
				relatives[i].id = (int)sqlite3_last_insert_rowid(db);
			}
			else {
				//update
				CString _fio = relatives.at(i).fio;
				encodeStr(key, _fio);
				CString _phone = relatives.at(i).phone;
				encodeStr(key, _phone);
				CString sql_wchar; sql_wchar.Format(_T("UPDATE RELATIVE SET NAME=\'%s\', PHONE=\'%s\', PERSON_ID=%d, ROWPOS=%d WHERE ID= %d;"),
					_fio.GetString(), _phone.GetString(), relatives.at(i).personId, i, relatives.at(i).id);
				sql = get_utf8(sql_wchar.GetString());
				char* errMsg;
				if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
					sqlite3_free(errMsg);
					return 1;
				}
			}
		}
		return 0;
	}

	int DeleteSQL(std::vector<int> ids2del)
	{
		for (int i = 0; i < (int)ids2del.size(); i++) {
			if (ids2del.at(i) == -1)
				continue;

			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM RELATIVE WHERE ID=?;";
			if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
				return 1;
			}

			sqlite3_bind_int(stmt, 1, ids2del.at(i));

			if (sqlite3_step(stmt) != SQLITE_DONE) {
				sqlite3_finalize(stmt);
				return 1;
			}

			sqlite3_finalize(stmt);
		}
		return 0;
	}

	int DeleteSQL_4Person(int personId)
	{
		if (personId > 0) {
			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM RELATIVE WHERE PERSON_ID=?;";
			if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
				return 1;
			}

			sqlite3_bind_int(stmt, 1, personId);

			if (sqlite3_step(stmt) != SQLITE_DONE) {
				sqlite3_finalize(stmt);
				return 1;
			}

			sqlite3_finalize(stmt);
		}
		return 0;
	}
};