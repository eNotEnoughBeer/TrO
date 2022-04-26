#pragma once
#include <atlstr.h>
#include <vector>
#include "sqlite3.h"

class Divizion
{
public:
	int id;
	CString name;
	int rotaId;
	int vzvodId;
	Divizion() {
		id = -1;
		name = _T("");
		rotaId = -1;
		vzvodId = -1;
	}
	Divizion(int index, CString curName, int rota_Id, int vzvod_Id) {
		id = index;
		name = curName;
		rotaId = rota_Id;
		vzvodId = vzvod_Id;
	}

};

class Divizions {
public:
	sqlite3* db;
	std::vector<Divizion> divisions;
	Divizions() {
		divisions.clear();
	}

	int checkTable()
	{
		char* errMsg;
		std::string sql = "CREATE TABLE IF NOT EXISTS DIVIZION("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"NAME TEXT, "
			"ROTA_ID INTEGER, "
			"VZVOD_ID INTEGER, "
			"ROWPOS INTEGER NOT NULL); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		return 0;
	}

	void ReadSQL(CString key) {
		USES_CONVERSION;
		divisions.clear();
		CString tmp;
		Divizion pOneDivizion;
		sqlite3_stmt* stmt;
		std::string sql = "SELECT * FROM DIVIZION ORDER BY ROWPOS";
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				tmp = CA2W((char*)sqlite3_column_text(stmt, 0));
				pOneDivizion.id = Str2Int(tmp.GetString());

				tmp = CA2W((char*)sqlite3_column_text(stmt, 1));
				decodeStr(key, tmp);
				pOneDivizion.name = tmp;

				tmp = CA2W((char*)sqlite3_column_text(stmt, 2));
				pOneDivizion.rotaId = Str2Int(tmp.GetString());

				tmp = CA2W((char*)sqlite3_column_text(stmt, 3));
				pOneDivizion.vzvodId = Str2Int(tmp.GetString());

				divisions.push_back(pOneDivizion);
			}

			sqlite3_finalize(stmt);
		}
	}

	int WriteSQL(CString key) {
		USES_CONVERSION;
		sqlite3_stmt* stmt;
		std::string sql;
		for (int i = 0; i < (int)divisions.size(); i++) {
			if (divisions.at(i).id < 0) {
				//insert
				sql = "INSERT INTO DIVIZION(NAME, ROTA_ID, VZVOD_ID, ROWPOS) VALUES (?, ?, ?, ?);";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = divisions.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);

				sqlite3_bind_int(stmt, 2, divisions.at(i).rotaId);
				sqlite3_bind_int(stmt, 3, divisions.at(i).vzvodId);
				sqlite3_bind_int(stmt, 4, i);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}
				int previousId = divisions.at(i).id;
				divisions[i].id = (int)sqlite3_last_insert_rowid(db);
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);

				for (int a = 0; a < (int)divisions.size(); a++) {
					if (previousId == divisions.at(a).rotaId) {
						divisions[a].rotaId = divisions.at(i).id;
					}
					if (previousId == divisions.at(a).vzvodId) {
						divisions[a].vzvodId = divisions.at(i).id;
					}
				}
			}
			else {
				//update
				sql = "UPDATE DIVIZION SET NAME=?, ROTA_ID=?, VZVOD_ID=?, ROWPOS=? WHERE ID= ?";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = divisions.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);

				
				sqlite3_bind_int(stmt, 2, divisions.at(i).rotaId);
				sqlite3_bind_int(stmt, 3, divisions.at(i).vzvodId);
				sqlite3_bind_int(stmt, 4, i);
				sqlite3_bind_int(stmt, 5, divisions.at(i).id);

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
		for (int i = 0; i < (int)ids2del.size(); i++) {
			if (ids2del.at(i) < 0)
				continue;

			// собственно удаление
			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM DIVIZION WHERE ID=?";
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