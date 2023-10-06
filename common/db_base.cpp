#include "db_base.h"
#include <string>
#include <iostream>

bool db_base::open(const char* database)
{
    bool ret = false;

    if (SQLITE_OK != sqlite3_open(database, &m_dbHandle))
        std::cerr << "Can't open SQLite db [" << database << "]\n";
    else
        ret = true;

    return ret;
}

void db_base::close(void)
{
    sqlite3_close(m_dbHandle);
}

bool db_base::exec(const char* sql)
{
    bool ret = false;

    if (SQLITE_OK != sqlite3_exec(m_dbHandle, sql, nullptr, nullptr, nullptr))
       std::cerr << "sqlite3_exec failed " << sqlite3_errmsg(m_dbHandle) << '\n';
    else
        ret = true;

    return ret;
}

unsigned int db_base::get_one_value(const char* sql)
{
    sqlite3_stmt *stmt = nullptr;
    if (SQLITE_OK != sqlite3_prepare_v2(m_dbHandle,  sql, -1, &stmt, nullptr))
        std::cerr << sqlite3_errmsg(m_dbHandle) << '\n';

    unsigned int result = 0;
    while (SQLITE_ROW == sqlite3_step(stmt)) {
        result = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return result;
}