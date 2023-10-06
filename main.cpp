#include "inverter.h"
#include "common/common.h"
#include "common/network.h"
#include "inv_db.h"
#include <signal.h>                 // signal
#include <unistd.h>                 // sleep
#include <cstring>                  // strcpy
#include <fstream>                  // ofstream
#include <iostream>
#include <map>                      // map
//#include <iomanip>                // put_time

#define START_HOUR_TIME                 4
#define END_HOUR_TIME                   20

Configs configs;
inv_db db;

void* inv_main(void *arg)
{
    inverter inv(&db);
    modbus_status status;

    std::map<int, std::string> inv_status;
    std::time_t next_interval = 0;
    const unsigned int config_count = configs.get_count();
    unsigned int count = 0;
    do 
    {
        const std::time_t now = std::time(nullptr);
        //std::cout << "\nlocal: " << std::put_time(std::localtime(&now), "%c %Z") << '\n';
        if (now < next_interval) {
            sleep(1);
            continue;
        }
        else
            next_interval = now + configs.get_duration() * 60;

        //Check working time
        bool atWork =   std::localtime(&now)->tm_hour >= START_HOUR_TIME && 
                        std::localtime(&now)->tm_hour < END_HOUR_TIME;

        //Get Inverter Data
        unsigned int okCount = 0;
        inv_status.clear();
        for (unsigned int index = 0; index < configs.get_numOfInverter(); ++index) 
        {
            inv_conf config = configs.get_Inv_config(index);
            inv.clear_data();

            if (atWork)
                status = inv.fill_data(configs.get_trace(), now, config);
            else
                status = Disconnect;

            if (Normal == status)
            {
                inv.get_KWRating(config.kwr);
                inv.get_EToday(index, config.brand, config.kwr);
                db.insert(index, inv.get_data());
                if (configs.get_trace())
                    inv.print_data(config.brand, config.mppt);

                okCount++;
            }
            else            
                inv_status[status].append(std::to_string(config.slaveId) + ' ');
        }
        if (atWork)
            std::cout << "\n# Ok/Total = " << okCount << '/' << configs.get_numOfInverter() << '\n';

        // non-zero config count value .
        if (config_count > 0) { 
            if (++count >= config_count) // only run config_count times
                break;
        }

    }while(true);

    return nullptr;
}

bool parse_args(int argc, char **argv)
{
    bool ret = true;
    for(int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-v") == 0)
        {
            std::cout << INVIOT_VERSION;
            ret = true;
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            configs.set_count(atoi(argv[++i]));
            ret = true;
        }
        else if (strcmp(argv[i], "-T") == 0) 
        {
            configs.set_trace(true);
            ret = true;
        }
        else
        {
            std::cout << "Usage:  invIOT [options...]  \n";
            std::cout << "-h,                Show help message\n";
            std::cout << "-v,                Show version\n";
            std::cout << "-c count,          Run number of times\n";
            std::cout << "-T,                Trace modbus protocol\n";
            ret = false;
        }
    }

    return ret;
}

/* Signal handler for SIGINT and SIGTERM - just stop gracefully. */
void handle_sigint(int signal)
{
    db.close();
    exit(-1);
}

int main(int argc, char **argv)
{
    std::cout << "INV-IOT is a tool to collect inverter information\n";
    std::cout << "The versoin is " << INVIOT_VERSION << "\n\n";

    if (false == local_time_init())
        return 0;
    else if (false == configs.init())
        return 0;
    else if (false == parse_args(argc, argv))
        return 0;

    if (false == db.open("invdata.db"))
        return 0;
    for (unsigned int i = 0; i < configs.get_numOfInverter(); i++)
    {
        if (false == db.create(i, configs.get_Inv_mppt(i)))
            return 0;
    }
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    pthread_t tid;
    int ret = pthread_create(&tid, nullptr, inv_main, nullptr);
    if (ret != 0)
        std::cerr << "Error: pthread_create\n";
    else if (pthread_join(tid, nullptr) != 0)
        std::cerr << "Error: pthread_join\n";

    db.close();

    return 0;
}
