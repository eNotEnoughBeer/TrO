#pragma once
#include <atlstr.h>
#include "sqlite3.h"
#include <vector>

class WeaponNumber
{
public:
	int id;
	CString name;
	WeaponNumber() {
		id = -1;
		name = _T("");
	}
	WeaponNumber(int index, CString curName) {
		id = index;
		name = curName;
	}

};

class WeaponType
{
public:
	int id;
	CString name;
	std::vector<WeaponNumber> pWeaponNumbers;
	WeaponType() {
		id = -1;
		name = _T("");
		pWeaponNumbers.clear();
	}
	WeaponType(int index, CString curName) {
		id = index;
		name = curName;
		pWeaponNumbers.clear();
	}

};

class Weapons {
public:
	sqlite3* db;
	std::vector<WeaponType> pWeapons;
	Weapons() {
		pWeapons.clear();
	}

	int checkTable()
	{
		char* errMsg;
		std::string sql = "CREATE TABLE IF NOT EXISTS WEPTYP("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"NAME TEXT, "
			"ROWPOS INTEGER NOT NULL); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		sql = "CREATE TABLE IF NOT EXISTS WEPNUM("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"NAME TEXT, "
			"WEPTYP_ID INTEGER, "
			"ROWPOS INTEGER NOT NULL); ";

		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}

		return 0;
	}

	void ReadSQL(CString key) {
		USES_CONVERSION;
		pWeapons.clear();
		CString tmp;
		WeaponType pOneWeptyp;
		sqlite3_stmt* stmt;
		// читаем главную таблицу
		std::string sql = "SELECT * FROM WEPTYP ORDER BY ROWPOS";
		if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				tmp = CA2W((char*)sqlite3_column_text(stmt, 0));
				pOneWeptyp.id = Str2Int(tmp.GetString());

				tmp = CA2W((char*)sqlite3_column_text(stmt, 1));
				decodeStr(key, tmp);
				pOneWeptyp.name = tmp;

				pWeapons.push_back(pOneWeptyp);
			}

			sqlite3_finalize(stmt);
		}
		// читаем вложенную с табельными номерами
		WeaponNumber pOneNumber;
		for (int i = 0; i < (int)pWeapons.size(); i++) {
			sql = "SELECT * FROM WEPNUM WHERE WEPTYP_ID=";
			sql += CW2A(Int2Str(pWeapons.at(i).id).c_str());
			sql +=" ORDER BY ROWPOS";
			if (sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
				int colQ = sqlite3_column_count(stmt);
				int execCode = 0;

				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					tmp = CA2W((char*)sqlite3_column_text(stmt, 0));
					pOneNumber.id = Str2Int(tmp.GetString());

					tmp = CA2W((char*)sqlite3_column_text(stmt, 1));
					decodeStr(key, tmp);
					pOneNumber.name = tmp;

					pWeapons[i].pWeaponNumbers.push_back(pOneNumber);
				}

				sqlite3_finalize(stmt);
			}
		}

	}

	int WriteSQL(CString key) {
		USES_CONVERSION;
		sqlite3_stmt* stmt;
		std::string sql;
		// пишем главную таблицу
		for (int i = 0; i < (int)pWeapons.size(); i++) {
			if (pWeapons.at(i).id == -1) {
				//insert
				sql = "INSERT INTO WEPTYP(NAME, ROWPOS) VALUES (?, ?);";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = pWeapons.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);

				sqlite3_bind_int(stmt, 2, i);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}
				pWeapons[i].id = (int)sqlite3_last_insert_rowid(db);
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
			else {
				//update
				sql = "UPDATE WEPTYP SET NAME=?, ROWPOS=? WHERE ID= ?";
				if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
					return 1;
				}

				CString tmp = pWeapons.at(i).name;
				encodeStr(key, tmp);
				std::string name_str = CW2A(tmp.GetString());
				sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);

				sqlite3_bind_int(stmt, 2, i);
				sqlite3_bind_int(stmt, 3, pWeapons.at(i).id);

				if (sqlite3_step(stmt) != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 1;
				}
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
		}

		// пишем вложенную таблицу
		for (int i = 0; i < (int)pWeapons.size(); i++) {
			for (int a = 0; a < (int)pWeapons.at(i).pWeaponNumbers.size(); a++) {
				if (pWeapons.at(i).pWeaponNumbers.at(a).id == -1) {
					//insert
					sql = "INSERT INTO WEPNUM(NAME, WEPTYP_ID, ROWPOS) VALUES (?, ?, ?);";
					if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
						return 1;
					}

					CString tmp = pWeapons.at(i).pWeaponNumbers.at(a).name;
					encodeStr(key, tmp);
					std::string name_str = CW2A(tmp.GetString());
					sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);
					sqlite3_bind_int(stmt, 2, pWeapons.at(i).id);
					sqlite3_bind_int(stmt, 3, a);

					if (sqlite3_step(stmt) != SQLITE_DONE) {
						sqlite3_finalize(stmt);
						return 1;
					}
					pWeapons[i].pWeaponNumbers[a].id = (int)sqlite3_last_insert_rowid(db);
					sqlite3_reset(stmt);
					sqlite3_clear_bindings(stmt);
				}
				else {
					//update
					sql = "UPDATE WEPNUM SET NAME=?, WEPTYP_ID=?, ROWPOS=? WHERE ID= ?";
					if (sqlite3_prepare_v2(db, sql.c_str(), strlen(sql.c_str()), &stmt, nullptr) != SQLITE_OK) {
						return 1;
					}

					CString tmp = pWeapons.at(i).pWeaponNumbers.at(a).name;
					encodeStr(key, tmp);
					std::string name_str = CW2A(tmp.GetString());
					sqlite3_bind_text(stmt, 1, name_str.c_str(), strlen(name_str.c_str()), NULL);
					sqlite3_bind_int(stmt, 2, pWeapons.at(i).id);
					sqlite3_bind_int(stmt, 3, a);
					sqlite3_bind_int(stmt, 4, pWeapons.at(i).pWeaponNumbers.at(a).id);

					if (sqlite3_step(stmt) != SQLITE_DONE) {
						sqlite3_finalize(stmt);
						return 1;
					}
					sqlite3_reset(stmt);
					sqlite3_clear_bindings(stmt);
				}
			}
		}
		return 0;
	}

	int DeleteSQLType(std::vector<int> ids2del)
	{
		for (int i = 0; i < (int)ids2del.size(); i++) {
			if (ids2del.at(i) == -1)
				continue;

			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM WEPTYP WHERE ID=?";
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

	int DeleteSQLNumber(std::vector<int> ids2del)
	{
		for (int i = 0; i < (int)ids2del.size(); i++) {
			if (ids2del.at(i) == -1)
				continue;

			sqlite3_stmt* stmt;
			std::string sql = "DELETE FROM WEPNUM WHERE ID=?";
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