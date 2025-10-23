#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <cstdint>
struct  metadata_file {
    uint32_t offset;
    uint32_t size;
    uint16_t nama_file_len;
    // char name[nama_file_len]
};

struct metadata_header {
    uint16_t magic;
    uint16_t nama_module_len;
    char * name;
    uint16_t version;
    // char nama_module[nama_module_len]
};

#endif // __METADATA__HH__