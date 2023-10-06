#ifndef INV_DB_H
#define INV_DB_H

#include "inv_data.h"
#include "common/db_base.h"
#include <map>

class inv_db : public db_base
{
    std::map<unsigned int, unsigned char> dc_map;  // (table index, dc number)

public:
    bool create(unsigned int table_index, unsigned char dc_number);
    bool insert(unsigned int table_index, const inv_data& data);
    unsigned int get_ETotalofToDayMorning(unsigned int table_index, const inv_data& data);
};

#endif  // INV_DB_H