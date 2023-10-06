#include "inv_conf.h"
#include <nlohmann/json.hpp>
#include <fstream>      // ofstream
#include <iostream>

void to_json(nlohmann::json& j, const inv_conf& s) {
    j = nlohmann::json{
            {"port", s.port},
            {"baud", s.baud},
            {"slaveId", s.slaveId},
            {"brand", s.brand},
            {"mppt", s.mppt},
            {"phase", s.phase},
            {"kwr", s.kwr},
            {"serial", s.serial}};
}

void from_json(const nlohmann::json& j, inv_conf& s) {
    j.at("port").get_to(s.port);
    j.at("baud").get_to(s.baud);
    j.at("slaveId").get_to(s.slaveId);
    j.at("brand").get_to(s.brand);
    j.at("mppt").get_to(s.mppt);
    j.at("phase").get_to(s.phase);
    j.at("kwr").get_to(s.kwr);
    strcpy(s.serial, j.at("serial").get<std::string>().c_str());
}

bool Configs::init(void)
{
    std::ifstream i("solar.conf");
    if(false == i.is_open())
        return false;
    nlohmann::json json_body = nlohmann::json::parse(i);
    if (json_body.is_discarded())
        return false;
    duration = json_body["duration"];
    nlohmann::json j_plants = json_body["plants"];
    if (j_plants.is_array() == false)
        return false;

    for (size_t i = 0;  i < j_plants.size(); i++) {
        nlohmann::json j_plant = j_plants.at(i);
        nlohmann::json j_devices = j_plant["devices"];
        //float price = j_plant.at("price").get<float>();
        if (j_devices.is_array() == false)
            return false;
        for (size_t k = 0;  k < j_devices.size(); k++) {
             
            nlohmann::json j_device = j_devices.at(k);
            invConfigs[numOfInverter++] = j_device.get<inv_conf>(); // Copy assignment
        }
    }

    return true;
}

void Configs::dump(void)
{
    std::cout << "duration: " << duration <<'\n';
    for (unsigned int i = 0; i < numOfInverter; i++){
        nlohmann::json j = invConfigs[i];
        std::cout << j.dump(4) << '\n';
    }
}

const char* inv_name(inv_brand invBrand)
{
    switch (invBrand) {
        case SolarKing:
            return "POWERCOM";
        case GoodWe:
            return "GOODWE";
        case PrimeVolt:
            return "PrimeVOLT";
        case KACO:
            return "KACO";
        case EATON:
            return "EATON";
        case DELTA:
            return "DELTA";
        case SMA:
            return "SMA";
        case GroWatt:
            return "Growatt";
        case AEC:
            return "AEC";
        case Motech:
            return "MOTECH";
        case Ablerex:
            return "Ablerex";
        case HUAWEI:
            return "HUAWEI";
        case ABB:
            return "ABB";
        case DELTA_WIFI:
            return "DELTA_WIFI";
        case TECO:
            return "TECO";
        case SolarEdge:
            return "SolarEdge";
        case ALTENERGY:
            return "ALTENERGY";
        case SUNWAY:
            return "SUNWAY";
        case CyberPower:
            return "CyberPower";
        case AISWEI:
            return "AISWEI";
        case SUNGROW:
            return "SUNGROW";
        case SimDev:
            return  "SIMDEV";
        default:
            return "UNKNOWN";
    }
}