#include "metadata.hh"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Helper untuk baca string dinamis dari file biner
std::string read_string(std::ifstream &in, struct metadata_string &str) {
  std::vector<char> buf(str.length);
  uint64_t curr_pos = in.tellg();
  in.seekg(str.pos, std::ios::beg);
  in.read(buf.data(), str.length);
  in.seekg(curr_pos, std::ios::beg);
  // pos += len;
  return std::string(buf.begin(), buf.end());
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: voxmo-dump <filename.voxmo>\n";
    return 1;
  }

  std::ifstream in(argv[1], std::ios::binary);
  if (!in) {
    std::cerr << "Failed to open file: " << argv[1] << "\n";
    return 1;
  }

  metadata_header header;
  in.seekg(0, std::ios::beg);

  // in.read(reinterpret_cast<char*>(&header), sizeof(header));
  in.read(reinterpret_cast<char *>(&header.magic), sizeof(header.magic));
  in.read(reinterpret_cast<char *>(&header.version), sizeof(header.version));
  in.read(reinterpret_cast<char *>(&header.header_len),
          sizeof(header.header_len));
  in.read(reinterpret_cast<char *>(&header.file_counts),
          sizeof(header.file_counts));
  in.read(reinterpret_cast<char *>(&header.nama_module.length),
          sizeof(header.nama_module.length));
  in.read(reinterpret_cast<char *>(&header.nama_module.pos),
          sizeof(header.nama_module.pos));
  in.read(reinterpret_cast<char *>(&header.description.length),
          sizeof(header.description.length));
  in.read(reinterpret_cast<char *>(&header.description.pos),
          sizeof(header.description.pos));
  in.read(reinterpret_cast<char *>(&header.license.length),
          sizeof(header.license.length));
  in.read(reinterpret_cast<char *>(&header.license.pos),
          sizeof(header.license.pos));
  in.read(reinterpret_cast<char *>(&header.version_str.length),
          sizeof(header.version_str.length));
  in.read(reinterpret_cast<char *>(&header.version_str.pos),
          sizeof(header.version_str.pos));
  in.read(reinterpret_cast<char *>(&header.author.length),
          sizeof(header.author.length));
  in.read(reinterpret_cast<char *>(&header.author.pos),
          sizeof(header.author.pos));
  in.read(reinterpret_cast<char *>(&header.main_file.length),
          sizeof(header.main_file.length));
  in.read(reinterpret_cast<char *>(&header.main_file.pos),
          sizeof(header.main_file.pos));

  std::cout << "=== Voxmo Header Info ===\n";
  std::cout << "Magic: 0x" << std::hex << header.magic << std::dec << "\n";
  std::cout << "Version: " << header.version << "\n";
  std::cout << "Header Length: " << header.header_len << "\n";
  std::cout << "File Count: " << header.file_counts << "\n";
  std::cout << "Nama Module len " << header.nama_module.length << " pos "
            << header.nama_module.pos << "\n";
  std::cout << "Description len " << header.description.length << " pos "
            << header.description.pos << "\n";
  std::cout << "License len " << header.license.length << " pos "
            << header.license.pos << "\n";
  std::cout << "Version len " << header.version_str.length << " pos "
            << header.version_str.pos << "\n";
  std::cout << "Author len " << header.author.length << " pos "
            << header.author.pos << "\n";
  std::cout << "Main File len " << header.main_file.length << " pos "
            << header.main_file.pos << "\n\n";

  uint64_t string_pos = sizeof(metadata_header) +
                        header.file_counts * sizeof(metadata_file) +
                        header.capability.count * sizeof(metadata_string);

  // Baca string-string metadata
  std::cout << "Nama Module: " << read_string(in, header.nama_module)
            << std::endl;
  std::cout << "Description: " << read_string(in, header.description) << "\n";
  std::cout << "License: " << read_string(in, header.license) << "\n";
  std::cout << "Version: " << read_string(in, header.version_str) << "\n";
  std::cout << "Author: " << read_string(in, header.author) << "\n";
  std::cout << "Main File: " << read_string(in, header.main_file) << "\n";

  //   Baca capability list
  in.read(reinterpret_cast<char *>(&header.capability.count),
          sizeof(header.capability.count));
  std::cout << "\nCapabilities (" << header.capability.count << "):\n";

  in.read(reinterpret_cast<char *>(&header.capability.metadata_pos),
          sizeof(header.capability.metadata_pos));
  std::cout << "Metadata Pos: " << header.capability.metadata_pos << "\n";

  in.read(reinterpret_cast<char *>(&header.dependency.count),
          sizeof(header.dependency.count));
  std::cout << "\nDependencies (" << header.dependency.count << "):\n";
  in.read(reinterpret_cast<char *>(&header.dependency.metadata_pos),
          sizeof(header.dependency.metadata_pos));
  std::cout << "Metadata Pos: " << header.dependency.metadata_pos << "\n";

  std::cout << "\n=== Capabilities ===\n";

  in.seekg(header.capability.metadata_pos, std::ios::beg);
  for (int i = 0; i < header.capability.count; ++i) {
    metadata_string *metadata = new metadata_string;
    in.read(reinterpret_cast<char *>(&metadata->length),
            sizeof(metadata->length));
    in.read(reinterpret_cast<char *>(&metadata->pos), sizeof(metadata->pos));
    std::cout << "  - " << read_string(in, *metadata) << "\n";
  }

  std::cout << "\n=== Dependencies ===\n";
  in.seekg(header.dependency.metadata_pos, std::ios::beg);
  for (int i = 0; i < header.dependency.count; ++i) {
    metadata_string *metadata = new metadata_string;
    in.read(reinterpret_cast<char *>(&metadata->length),
            sizeof(metadata->length));
    in.read(reinterpret_cast<char *>(&metadata->pos), sizeof(metadata->pos));
    std::cout << "  - " << read_string(in, *metadata) << "\n";
  }

  // Sekarang baca tabel file
  std::cout << "\n=== Files ===\n";
  for (int i = 0; i < header.file_counts; ++i) {
    metadata_file mf;
    in.read(reinterpret_cast<char *>(&mf.offset), sizeof(mf.offset));
    in.read(reinterpret_cast<char *>(&mf.metadata_length),
            sizeof(mf.metadata_length));
    in.read(reinterpret_cast<char *>(&mf.size), sizeof(mf.size));
    in.read(reinterpret_cast<char *>(&mf.nama_file.length),
            sizeof(mf.nama_file.length));
    in.read(reinterpret_cast<char *>(&mf.nama_file.pos),
            sizeof(mf.nama_file.pos));

    std::cout << "  - " << read_string(in, mf.nama_file) << "\n";
    std::cout << "    Nama File Length: " << mf.nama_file.length << "\n";
    std::cout << "    Nama File Pos: " << mf.nama_file.pos << "\n";
    std::cout << "    Size: " << mf.size << "\n";
    std::cout << "    File Offset: " << mf.offset << "\n";
    std::cout << "    Metadata Length: " << mf.metadata_length << "\n";
  }

  std::cout << "\n=== End of Dump ===\n";
  return 0;
}
