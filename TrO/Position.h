#pragma once
#include <atlstr.h>
#include "sqlite3.h"
#include <vector>

class Position
{
public:
	int id;
	CString name;
	int rankId;
	Position() { 
		id = -1;
		name = _T("");
		rankId = -1;
	}
	Position(int index, CString curName, int rankID) {
		id = index;
		name = curName;
		rankId = rankID;
	}

};

class Positions {
public: 
	sqlite3* db;
	std::vector<Position> positions;
	Positions() { 
		positions.clear(); 
	}

	int checkTable()
	{	
		char* errMsg;
		std::string sql = "CREATE TABLE IF NOT EXISTS POSITION("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"NAME TEXT, "
			"RANK_ID TEXT, "
			"ROWPOS INTEGER NOT NULL); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		return 0;
	}

	void ReadSQL(CString key) {
		USES_CONVERSION;
		positions.clear();
		CString tmp;
		Position pOnePosition;
		sqlite3_stmt* stmt;
		std::string sql = "SELECT * FROM POSITION ORDER BY ROWPOS";
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				tmp = CA2W((char*)sqlite3_column_text(stmt, 0));
				pOnePosition.id = Str2Int(tmp.GetString());

				tmp = CA2W((char*)sqlite3_column_text(stmt, 1));
				decodeStr(key, tmp);
				pOnePosition.name = tmp;

				tmp = CA2W((char*)sqlite3_column_text(stmt, 2));
				decodeStr(key, tmp);
				pOnePosition.rankId = Str2Int(tmp.GetString());

				positions.push_back(pOnePosition);
			}

			sqlite3_finalize(stmt);
		}
	}

	int WriteSQL(CString key) {
		USES_CONVERSION;
		sqlite3_stmt* stmt;
		std::string sql;
		for (int i = 0; i < (int)positions.size(); i++) {
			if (positions.at(i).id == -1) {
				//insert
				sql = "INSERT INTO POSITION(NAME, RANK_ID, ROWPOS) VALUES (?, ?, ?);";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = positions.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);

				tmp = Int2Str(positions.at(i).rankId).c_str();
				encodeStr(key, tmp);
				std::string rankid_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 2, rankid_str.c_str(), strlen(rankid_str.c_str()), NULL);

				sqlite3_bind_int(stmt, 3, i);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}
				positions[i].id = (int)sqlite3_last_insert_rowid(db);
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
			else {
				//update
				sql = "UPDATE POSITION SET NAME=?, RANK_ID=?, ROWPOS=? WHERE ID= ?";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = positions.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);

				tmp = Int2Str(positions.at(i).rankId).c_str();
				encodeStr(key, tmp);
				std::string rankid_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 2, rankid_str.c_str(), strlen(rankid_str.c_str()), NULL);

				sqlite3_bind_int(stmt, 3, i);
				sqlite3_bind_int(stmt, 4, positions.at(i).id);

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
			if (ids2del.at(i) == -1)
				continue;

			// собственно удаление
			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM POSITION WHERE ID=?";
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