#ifndef INVERTER_H
#define INVERTER_H

#include "inv_data.h"
#include "inv_db.h"

class inverter {

    inv_data data;
    inv_db* db;

public:
    inverter(inv_db* database) { db = database; }
    void get_KWRating(float config_kwr);
    bool get_EToday(unsigned int table_index, inv_brand inv_brand, float config_kwr);

    void clear_data(void) { data.clear(); }
    const inv_data& get_data(void) { return data; }
    modbus_status fill_data(bool bTrace, long now, const inv_conf& c);
    void print_data(inv_brand inv_brand, unsigned char dc_number) { data.print(inv_brand, dc_number); }
};

modbus_status inv_simdev_impl(inv_data *data);

#endif //INVERTER_H
