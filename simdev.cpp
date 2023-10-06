#include "inv_data.h"
#include "common/common.h"

/*
* DELTA Modbus Definition_EU  (V01.08 20180605).pdf
* RPI H series (1 phase) - Delta Modbus Definition.xls
*/
modbus_status inv_simdev_impl(inv_data *data)
{
    data->Pac = 20;
    /* use scale factor, 0.01A for Iac 0.1V for V , 1W for Power */
    data->R_Voltage = 2560;
    data->R_Current = 40;
    data->R_Power = data->R_Voltage * data->R_Current;
    data->R_Frequency = 6000;

    /* use scale factor, 0.01A for Iac 0.1V for V , 1W for Power */
    data->Voltage[0] = 3000;
    data->Current[0] = 10;
    data->Power[0] = data->Voltage[0] * data->Current[0];
    data->Status = Work;

    /* 0x430  1072 Today Energy */
    data->EToday = 50000;
    /*  0x434 Life Energy  10Wh */
    data->ETotal = 60040;
    /*  0x436 Life  Runtime 1s */
    data->HTotal = 5;

    /* 0x43A 1082 Inverter Temperature 1°C */
    data->Temperature = 540;

    data->KWRating = 5000;

    return Normal;
}

