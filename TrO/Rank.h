#pragma once
#include <atlstr.h>
#include <vector>
#include "sqlite3.h"
#import "EXCEL8.OLB" auto_search auto_rename

class Rank
{
public:
	int id;
	CString name;
	Rank() {
		id = -1;
		name = _T("");
	}
	Rank(int index, CString curName) {
		id = index;
		name = curName;
	}

};

class Ranks {
public:
	sqlite3* db;
	std::vector<Rank> ranks;
	Ranks() {
		ranks.clear();
	}

	int checkTable()
	{
		bool needInserts = false;
		char* errMsg;
		std::string sql = "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 'RANK'; ";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;
			execCode = sqlite3_step(stmt);
			if (execCode == SQLITE_ROW) {
				if (0 == Str2Int(CA2W((char*)sqlite3_column_text(stmt, 0)))) {
					needInserts = true;
				}
			}
			sqlite3_finalize(stmt);
		}

		sql = "CREATE TABLE IF NOT EXISTS RANK("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"NAME TEXT, "
			"ROWPOS INTEGER NOT NULL); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		//-> а теперь заполнить каталог начальными данными
		if (needInserts) {
			USES_CONVERSION;
			std::vector<std::wstring> values;
			values.push_back(_T("Полковник"));
			values.push_back(_T("Підполковник"));
			values.push_back(_T("Майор"));
			values.push_back(_T("Капітан"));
			values.push_back(_T("Старший лейтенант"));
			values.push_back(_T("Лейтенант"));
			values.push_back(_T("Молодший лейтенант"));
			values.push_back(_T("Головний майстер-сержант"));
			values.push_back(_T("Старший майстер-сержант"));
			values.push_back(_T("Майстер-сержант"));
			values.push_back(_T("Штаб-сержант"));
			values.push_back(_T("Головний сержант"));
			values.push_back(_T("Сержант"));
			values.push_back(_T("Молодший сержант"));
			values.push_back(_T("Старший солдат"));
			values.push_back(_T("Солдат"));
			for (int i = 0; i < (int)values.size(); i++) {
				sql = "INSERT INTO RANK(NAME, ROWPOS) VALUES(?,?); ";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}
				std::string _str = CW2A(values.at(i).c_str());

				sqlite3_bind_text(stmt, 1, _str.c_str(), strlen(_str.c_str()), NULL);
				sqlite3_bind_int(stmt, 2, i);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}

				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
		}
		//<-

		return 0;
	}

	void ReadSQL(CString key) {
		USES_CONVERSION;
		ranks.clear();
		CString tmp;
		Rank pOneRank;
		sqlite3_stmt* stmt;
		std::string sql = "SELECT * FROM RANK ORDER BY ROWPOS";
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{				
				tmp = CA2W((char*)sqlite3_column_text(stmt, 0));
				pOneRank.id = Str2Int(tmp.GetString());

				tmp = CA2W((char*)sqlite3_column_text(stmt, 1));
				decodeStr(key, tmp);
				pOneRank.name = tmp;

				ranks.push_back(pOneRank);
			}

			sqlite3_finalize(stmt);
		}
	}

	int WriteSQL(CString key) {
		USES_CONVERSION;
		sqlite3_stmt* stmt;
		std::string sql;
		for (int i = 0; i < (int)ranks.size(); i++) {
			if (ranks.at(i).id == -1) {
				//insert
				sql = "INSERT INTO RANK(NAME, ROWPOS) VALUES (?, ?);";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = ranks.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);
				sqlite3_bind_int(stmt, 2, i);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}
				ranks[i].id = (int)sqlite3_last_insert_rowid(db);
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
			else {
				//update
				sql = "UPDATE RANK SET NAME=?, ROWPOS=? WHERE ID= ?";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = ranks.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);
				sqlite3_bind_int(stmt, 2, i);
				sqlite3_bind_int(stmt, 3, ranks.at(i).id);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
		}
		return 0;
	}

	int DeleteSQL(std::vector<int> ids2del)
	{
		for (int i = 0; i < (int)ids2del.size(); i++){
			if (ids2del.at(i) == -1)
				continue;

			// собственно удаление
			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM RANK WHERE ID=?";
			if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
				return 1;
			}

			sqlite3_bind_int(stmt, 1, ids2del.at(i));

			if (sqlite3_step(stmt) != SQLITE_DONE) {
				sqlite3_finalize(stmt);
				return 1;
			}
			
			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
		}
		return 0;
	}
};