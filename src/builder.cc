#include "builder.hh"
#include "manifest.hh"
#include "metadata.hh"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <variant>
#include <vector>

template <typename T> void write_le(std::ofstream &out, T value) {
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>,
                "Must be integer-like");
  for (size_t i = 0; i < sizeof(T); ++i) {
    out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
  }
}

#define VOXMO_MAGIC 0x564f584d

Builder::Builder() {}

void Builder::add_loader(std::shared_ptr<FileLoader> fl) { loader = fl; }

std::vector<std::string> get_lines(std::vector<char> data) {
  std::vector<std::string> lines;
  std::string line;
  for (char c : data) {
    if (c == '\n' || c == '\0') {
      lines.push_back(line);
      line.clear();
    } else {
      line.push_back(c);
    }
  }
  if (!line.empty()) {
    lines.push_back(line);
  }
  return lines;
}

void write_string_segment(std::ofstream &out, std::string str,
                          uint64_t &string_pos) {
  auto curr_pos = out.tellp();
  out.seekp(string_pos, std::ios::beg);
  out.write(str.c_str(), str.size());
  // null terminated
  out.put('\0');
  out.seekp(curr_pos, std::ios::beg);
  string_pos += str.size() + 1;
}

void Builder::build(std::string filename) {

  // find manifest
  std::shared_ptr<File> manifest = 0;
  for (const auto &f : loader->get_files()) {
    std::filesystem::path p(f->path);
    // std::cout << p.filename() << "\n"; // output: manifest.yaml

    if (p.filename() == "manifest.yaml") {
      manifest = f;
      break;
    }
  }

  if (manifest == 0) {
    std::cerr << "error manifest not found" << std::endl;
    return;
  }

  std::vector<std::string> lines = get_lines(manifest->data);
  Manifest m(std::move(lines));
  const std::vector<std::string> required_fields = {
      "name", "version", "author", "description", "license", "main"};

  for (const auto &field : required_fields) {
    if (std::holds_alternative<std::vector<std::string>>(m[field])) {
      std::cerr << "error: module must have " << field << std::endl;
      return;
    }
  }

  auto capability = std::get<std::vector<std::string>>(m["capability"]);

  std::ofstream out(filename, std::ios::binary | std::ios::out);

  // creating metadata
  metadata_header header;
  header.magic = VOXMO_MAGIC;
  header.version = 1;
  header.header_len = sizeof(header.magic) + sizeof(header.version) +
                      sizeof(header.header_len) + sizeof(header.file_counts) +
                      sizeof(header.nama_module) + sizeof(header.description) +
                      sizeof(header.license) + sizeof(header.version_str) +
                      sizeof(header.author) + sizeof(header.main_file) +
                      sizeof(header.capability.count);

  header.file_counts = loader->get_files().size() - 1;

  uint64_t string_pos = sizeof(metadata_header) +
                        header.file_counts * sizeof(metadata_file) +
                        capability.size() * sizeof(metadata_string);

  printf("string_pos: %lu\n", string_pos);

  write_le(out, header.magic);
  write_le(out, header.version);

  auto header_len_pos = out.tellp();
  write_le(out, header.header_len);
  write_le(out, header.file_counts);

  std::string name = std::get<std::string>(m["name"]);
  header.nama_module.length = static_cast<uint16_t>(name.size() + 1);
  header.nama_module.pos = string_pos;
  write_le(out, header.nama_module.length);
  write_le(out, header.nama_module.pos);
  write_string_segment(out, name, string_pos);

  std::string description = std::get<std::string>(m["description"]);
  header.description.length = static_cast<uint16_t>(description.size() + 1);
  header.description.pos = string_pos;
  write_le(out, header.description.length);
  write_le(out, header.description.pos);
  write_string_segment(out, description, string_pos);

  std::string license = std::get<std::string>(m["license"]);
  header.license.length = static_cast<uint16_t>(license.size() + 1);
  header.license.pos = string_pos;
  write_le(out, header.license.length);
  write_le(out, header.license.pos);
  write_string_segment(out, license, string_pos);

  std::string version = std::get<std::string>(m["version"]);
  header.version_str.length = static_cast<uint16_t>(version.size() + 1);
  header.version_str.pos = string_pos;
  write_le(out, header.version_str.length);
  write_le(out, header.version_str.pos);
  write_string_segment(out, version, string_pos);

  std::string author = std::get<std::string>(m["author"]);
  header.author.length = static_cast<uint16_t>(author.size() + 1);
  header.author.pos = string_pos;
  write_le(out, header.author.length);
  write_le(out, header.author.pos);
  write_string_segment(out, author, string_pos);

  std::string main = std::get<std::string>(m["main"]);
  main.erase(std::remove_if(main.begin(), main.end(),
                            [](unsigned char c) { return std::isspace(c); }),
             main.end());
             
  header.main_file.length = static_cast<uint16_t>(main.size() + 1);
  header.main_file.pos = string_pos;
  write_le(out, header.main_file.length);
  write_le(out, header.main_file.pos);
  write_string_segment(out, main, string_pos);

  header.capability.count = capability.size();
  write_le(out, header.capability.count);

  header.capability.items = new metadata_string[capability.size()];
  for (int i = 0; i < capability.size(); ++i) {
    header.capability.items[i].length =
        static_cast<uint16_t>(capability[i].size() + 1);
    write_le(out, header.capability.items[i].length);
    header.capability.items[i].pos = string_pos;
    write_le(out, header.capability.items[i].pos);
    write_string_segment(out, capability[i], string_pos);
  }

  header.header_len += capability.size() * sizeof(metadata_string);

  // update header len
  {
    auto curr_pos = out.tellp();
    out.seekp(header_len_pos, std::ios::beg);
    write_le(out, header.header_len);
    out.seekp(curr_pos, std::ios::beg);
  }

  std::vector<uint64_t> file_offset_pos;
  for (const auto &f : loader->get_files()) {
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") {
      continue;
    }

    metadata_file mf;
    mf.size = f->data.size();
    mf.nama_file.length =
        static_cast<uint16_t>(p.filename().string().size() + 1);
    mf.nama_file.pos = string_pos;
    mf.metadata_length = sizeof(mf);
    mf.offset = 0;

    auto curr_pos = out.tellp();
    file_offset_pos.push_back(curr_pos);
    write_le(out, mf.offset);
    write_le(out, mf.metadata_length);
    write_le(out, mf.size);
    write_le(out, mf.nama_file.length);
    write_le(out, mf.nama_file.pos);
    write_string_segment(out, p.filename().string(), string_pos);
  }

  uint64_t file_pos = string_pos;
  auto files = loader->get_files();
  int file_idx = 0;
  for (int i = 0; i < files.size(); ++i) {
    const auto &f = files[i];
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") {
      continue;
    }

    out.seekp(file_offset_pos[file_idx++], std::ios::beg);
    write_le(out, file_pos);
    out.seekp(file_pos, std::ios::beg);
    out.write(f->data.data(), f->data.size());
    file_pos += f->data.size();
  }

  printf("last file_pos: %lu\n", file_pos);
  printf("build done\n");
}
