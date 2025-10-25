#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <cstdint>

#pragma pack(push, 1)
struct metadata_string {
    uint16_t length;
    uint64_t pos;
};

struct metadata_list {
    uint16_t count;
    struct metadata_string *items;
};

struct metadata_header {
    uint32_t magic;
    uint16_t version;
    uint32_t header_len;
    uint32_t file_counts;

    struct metadata_string nama_module;
    struct metadata_string description;
    struct metadata_string license;
    struct metadata_string version_str;
    struct metadata_string author;
    struct metadata_string main_file;

    struct metadata_list capability;
};

struct  metadata_file {
    uint64_t offset;
    uint32_t metadata_length;
    uint32_t size;
    struct metadata_string nama_file;
};

#pragma pack(pop)
#endif // __METADATA__HH__