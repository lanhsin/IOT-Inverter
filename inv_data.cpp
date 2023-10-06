#include "inv_data.h"
#include <nlohmann/json.hpp>
#include <iostream>         // cout

void inv_data::clear(void) 
{
    memset(Serial, 0, 24);
    R_Voltage = R_Current = R_Power = R_Frequency = 0;
    S_Voltage = S_Current = S_Power = S_Frequency = 0;
    T_Voltage = T_Current = T_Power = T_Frequency = 0;
    for (unsigned int i = 0; i < DC_NUM_MAX; i++)
        Voltage[i] = Current[i] = Power[i] = 0;

    Temperature = 0;
    KWRating = 0;
    Error.clear();
    ETotal = HTotal = EToday = Pac = 0;
}

void inv_data::print(inv_brand brand, unsigned char dc_number)
{
    nlohmann::ordered_json j_object;
    j_object["Timestamp"] = Timestamp;
    j_object["Serial"] = Serial;
    j_object["Brand"] = inv_name(brand);
    j_object["Status"] = Status;
    j_object["ERR"] = Error.c_str();
    j_object["Temp"] = Temperature;
    j_object["KWR"] = KWRating;
    j_object["PAC"] = Pac;
    j_object["Eday"] = EToday;
    j_object["Etotal"] = ETotal;
    j_object["Htotal"] = HTotal;

    nlohmann::json j_dc = nlohmann::json::array();
    for (unsigned int i = 0; i < dc_number; i++)
        j_dc.push_back({Power[i], Voltage[i], Current[i]});
    j_object["Power Voltage Current"] = j_dc;

    size_t i = 0, k = 0;
    std::cout << "{\n";
    for (auto& x : j_object.items())
    {
        if (x.key().compare("Power Voltage Current") == 0)
        {
            std::cout << "    \"" << x.key() << "\": [\n";
            for (auto& y : j_dc.items()) {
                std::cout << "        " << y.value();
                if (++k == j_dc.size())
                    std::cout << '\n';
                else
                    std::cout << ",\n";
            }
            std::cout << "    ]\n";
            ++i;
        }
        else {
            std::cout << "    \"" << x.key() << "\": " << x.value();
            if (++i == j_object.size())
                std::cout << "\n";
            else
                std::cout << ",\n";
        }
    }

    j_object["RVac"] = R_Voltage;
    j_object["RIac"] = R_Current;
    j_object["SVac"] = S_Voltage;
    j_object["SIac"] = S_Current;
    j_object["TVac"] = T_Voltage;
    j_object["TIac"] = T_Current;
    j_object["R_Frequency"] = R_Frequency;
    j_object["S_Frequency"] = S_Frequency;
    j_object["T_Frequency"] = T_Frequency;

    std::cout << "}\n";
}