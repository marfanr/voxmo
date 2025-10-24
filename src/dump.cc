#include "metadata.hh"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Helper untuk baca string dinamis dari file biner
std::string read_string(std::ifstream &in, metadata_string &s) {
  in.read(reinterpret_cast<char *>(&s.length), sizeof(s.length));
  std::vector<char> buf(s.length);
  in.read(buf.data(), s.length);
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

  if (!in) {
    std::cerr << "Failed to read basic header fields\n";
    return 1;
  }

  std::cout << "=== Voxmo Header Info ===\n";
  std::cout << "Magic: 0x" << std::hex << header.magic << std::dec << "\n";
  std::cout << "Version: " << header.version << "\n";
  std::cout << "Header Length: " << header.header_len << "\n";

  // Baca string-string metadata
  std::cout << "Nama Module: " << read_string(in, header.nama_module) << "\n";
  std::cout << "Description: " << read_string(in, header.description) << "\n";
  std::cout << "License: " << read_string(in, header.license) << "\n";
  std::cout << "Version: " << read_string(in, header.version_str) << "\n";
  std::cout << "Author: " << read_string(in, header.author) << "\n";
  std::cout << "Main File: " << read_string(in, header.main_file) << "\n";

  //   Baca capability list
  in.read(reinterpret_cast<char *>(&header.capability.count),
          sizeof(header.capability.count));
  std::cout << "\nCapabilities (" << header.capability.count << "):\n";

  header.capability.items = new metadata_string[header.capability.count];
  for (int i = 0; i < header.capability.count; ++i) {
    std::cout << "  - " << read_string(in, header.capability.items[i]) << "\n";
  }

  // Sekarang baca tabel file
  std::cout << "\n=== Files ===\n";
  while (in.peek() != EOF) {
      metadata_file mf;
      in.read(reinterpret_cast<char*>(&mf.next_offset), sizeof(mf.next_offset));
      in.read(reinterpret_cast<char*>(&mf.metadata_length), sizeof(mf.metadata_length));
      in.read(reinterpret_cast<char*>(&mf.size), sizeof(mf.size));
      std::string filename = read_string(in, mf.nama_file);
      std::cout << "File: " << filename
                << " | Size: " << mf.size
                << " | Next offset: " << mf.next_offset << "\n";

      in.seekg(mf.size, std::ios::cur);
  }

  std::cout << "\n=== End of Dump ===\n";
  return 0;
}
