#include "inverter.h"
#include <iostream>         // cout
#include <cstring>          // strcpy

modbus_status inverter::fill_data(bool bTrace, long now, const inv_conf& c)
{
    modbus_status status;

    data.Timestamp = now;
    strcpy(data.Serial, c.serial);

    switch(c.brand) {
        case SimDev:
            status = inv_simdev_impl(&data);
            break;
        default:
            status = Disconnect;
            break;
    }
    return status;
}

void inverter::get_KWRating(float config_kwr)
{
    if (data.KWRating == 0)
        data.KWRating = config_kwr;
}

bool inverter::get_EToday(unsigned int table_index, inv_brand brand, float config_kwr)
{
    bool ret = true;
    switch(brand) {
        default:
            break;
    };

    return ret;
}