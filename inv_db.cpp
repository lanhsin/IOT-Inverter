#include "inv_db.h"
#include "common/common.h"
#include <sstream>
#include <iomanip>

bool inv_db::create(unsigned int table_index, unsigned char dc_number)
{
    std::string sql { "CREATE TABLE IF NOT EXISTS " };
    sql.append( "data_" + std::to_string(table_index));
    sql.append( " (Serial varchar(24) NOT NULL, ");
    sql.append( "Timestamp datetime NOT NULL, ");
    sql.append( "Status int, KWRating int, Temperature int,");
    sql.append( "Pac int, EToday int, ETotal int, HTotal int, ");
    for (auto i = 0; i < dc_number; i++)
    {
        sql.append('D' + std::to_string(i) + "_Voltage int, " );
        sql.append('D' + std::to_string(i) + "_Current int, " );
        sql.append('D' + std::to_string(i) + "_Power int, " );
    }
    sql.append("R_Voltage int, R_Current int, R_Power int,");
    sql.append("S_Voltage int, S_Current int, S_Power int, ");
    sql.append("T_Voltage int, T_Current int, T_Power int, R_Frequency int, ");
    sql.append("Error text, PRIMARY KEY (Timestamp, Serial))");

    bool ret = exec(sql.c_str());
    if (ret == false)
        close();
    else
        dc_map.insert({table_index, dc_number});

    return ret;
}

bool inv_db::insert(unsigned int table_index, const inv_data& data)
{
    unsigned char dc_number = dc_map.at(table_index);

    std::ostringstream sql("", std::ios_base::ate);
    sql << "INSERT INTO \"data_" << table_index << "\" VALUES ( '" <<
    data.Serial << "','" <<
    std::put_time(localtime(&data.Timestamp), "%Y-%m-%d %H:%M:%S") << "'," <<
    data.Status << ',' << data.KWRating << ',' << data.Temperature << ',' <<
    data.Pac << ',' << data.EToday << ',' << data.ETotal << ',' << data.HTotal << ',';
    for (unsigned char i = 0; i < dc_number; i++)
        sql << data.Voltage[i] << ',' << data.Current[i] << ',' << data.Power[i] << ',';
    sql << data.R_Voltage << ',' << data.R_Current << ',' << data.R_Power << ',' <<
    data.S_Voltage << ',' << data.S_Current << ',' << data.S_Power << ',' <<
    data.T_Voltage << ',' << data.T_Current << ',' << data.T_Power << ',' <<
    data.R_Frequency << ",'" << data.Error.c_str() << "')";

    return exec(sql.str().c_str());
}

unsigned int inv_db::get_ETotalofToDayMorning(unsigned int table_index, const inv_data& data)
{
    char mbstr[32] = {0};
    // get time using time_t
    std::time_t t = data.Timestamp;
    // get time using string
    std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d 00:00:00", std::localtime(&t));

    std::string sql { "select ETotal from \"data_" };
    sql.append(std::to_string(table_index));
    sql.append("\" where Timestamp > '");
    sql.append(mbstr);
    sql.append("' And Serial = '");
    sql.append(data.Serial);
    sql.append("' order by Timestamp asc limit 1;");

    return get_one_value(sql.c_str());
}