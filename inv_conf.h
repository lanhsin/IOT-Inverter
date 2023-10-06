#ifndef INV_CONF_H
#define INV_CONF_H

const unsigned int MAX_INV_NUM = 160;

enum inv_brand {
    SimDev       = 0
};

struct inv_conf {
    inv_brand brand;
    unsigned char mppt;
    unsigned char phase;
    unsigned char port;
    unsigned int slaveId;
    unsigned int baud;
    float kwr;
    char serial[24];
};

class Configs {
    
    bool bTrace = false;
    unsigned int count = 0;     // run number of times
    unsigned int duration = 10;
    unsigned int numOfInverter = 0;
    
    inv_conf invConfigs[MAX_INV_NUM];

public:

    bool get_trace(void) { return bTrace; }
    void set_trace(bool val) { bTrace = val; }

    unsigned int get_duration(void) { return duration; }
    void set_duration(unsigned int interval) { duration = interval; }

    void set_count(unsigned int times) { count = times; }
    unsigned int get_count(void) { return count; }

    // Inverter functions
    unsigned int get_numOfInverter(void) {
        return numOfInverter;
    }

    inv_brand get_inv_brand(int inv_num) {
        return invConfigs[inv_num].brand;
    }

    inv_conf get_Inv_config(int inv_num) {
        return invConfigs[inv_num];
    }

    const char* get_Inv_serial(int inv_num) {
        return invConfigs[inv_num].serial;
    }

    unsigned char get_Inv_mppt(int inv_num) {
        return invConfigs[inv_num].mppt;
    }

    bool init(void);
    void dump(void);
};


const char* inv_name(inv_brand invBrand);

#endif //INV_CONF_H