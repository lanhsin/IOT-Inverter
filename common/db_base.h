#ifndef DB_BASE_H
#define DB_BASE_H

#include <sqlite3.h>

class db_base
{
    sqlite3 *m_dbHandle;

public:
    bool open(const char* database);
    void close(void);
    bool exec(const char* sql);
    unsigned int get_one_value(const char* sql);
};

#endif  // DB_BASE_H