#ifndef NETWORK_H
#define NETOWRK_H

#include <stddef.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

void get_macAddr (char *id);
void set_hostIP(const char* if_name, const char* if_addr, const char* if_mask);

bool get_curl_start(MemoryStruct *chunk, const char* url);
void get_curl_end(MemoryStruct *chunk);

#endif //NETWORK_H