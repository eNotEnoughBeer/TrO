#pragma once
#include <atlstr.h>
#include "sqlite3.h"
#include "HelperFunctions.h"

class Password
{
public:
	sqlite3* db;
	CString password;
	CString hash;
	int badlogin;
	bool isNewOne;

	int checkTable()
	{
		char* errMsg;
		std::string sql = "CREATE TABLE IF NOT EXISTS PASSWD("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"HASH TEXT NOT NULL, "
			"WCNT TEXT); ";
			
		if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			return 1;
		}
		return 0;
	}

	bool ReadSQL() {
		USES_CONVERSION;
		sqlite3_stmt* stmt;
		const char* query = "SELECT * FROM PASSWD";
		if (sqlite3_prepare(db, query, -1, &stmt, 0) == SQLITE_OK) {
			int colQ = sqlite3_column_count(stmt);
			int execCode = 0;
			execCode = sqlite3_step(stmt);

			if (execCode == SQLITE_ROW) {
				hash = CA2W((char*)sqlite3_column_text(stmt,1));
				CString tmp = CA2W((char*)sqlite3_column_text(stmt, 2));
				decodeStr(_T("LCTTRO"), tmp); // это шифруется не по паролю а по статике, т.к. пароль можно ввести не верный
				badlogin = Str2Int(tmp);	
				isNewOne = false;
			}
			else if (execCode == SQLITE_DONE) {
				badlogin = -15;
				isNewOne = true;
			}
			sqlite3_finalize(stmt);
		}
		return true;
	}

	int WriteSQL() {
		USES_CONVERSION;
		if (isNewOne) {// создаем пароль
			const char* istmt = "INSERT INTO PASSWD(HASH, WCNT) VALUES (?,?)";
			sqlite3_stmt* stmt;
			int val = sqlite3_prepare_v2(db, istmt, strlen(istmt), &stmt, nullptr);
			if (val != SQLITE_OK) {
				return 1;
			}
			badlogin = 0;
			CString tmp = Int2Str(badlogin).c_str();
			encodeStr(_T("LCTTRO"), tmp);

			std::string hash_str = CW2A(hash.GetString());
			std::string badlogin_str = CW2A(tmp.GetString());

			val = sqlite3_bind_text(stmt, 1, hash_str.c_str(), strlen(hash_str.c_str()), NULL);
			val = sqlite3_bind_text(stmt, 2, badlogin_str.c_str(), strlen(badlogin_str.c_str()), NULL);

			if (sqlite3_step(stmt) != SQLITE_DONE) {
				sqlite3_finalize(stmt);
				return 1;
			}

			sqlite3_reset(stmt);
			sqlite3_clear_bindings(stmt);
		}
		else { //UPDATE с возможностью смены пароля (вернее его хэша)
			const char* istmt = "UPDATE PASSWD SET HASH=?, WCNT=? WHERE ID=1";
			sqlite3_stmt* stmt;
			int val = sqlite3_prepare_v2(db, istmt, strlen(istmt), &stmt, nullptr);
			if (val != SQLITE_OK) {
				return 1;
			}

			CString tmp = Int2Str(badlogin).c_str();
			encodeStr(_T("LCTTRO"), tmp);

			std::string hash_str = CW2A(hash.GetString());
			std::string badlogin_str = CW2A(tmp.GetString());

			val = sqlite3_bind_text(stmt, 1, hash_str.c_str(), strlen(hash_str.c_str()), NULL);
			val = sqlite3_bind_text(stmt, 2, badlogin_str.c_str(), strlen(badlogin_str.c_str()), NULL);

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