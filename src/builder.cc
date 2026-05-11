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

// Tulis struct/tipe apapun langsung ke ofstream (little-endian di x86/x64)
template <typename T>
void write_struct(std::ofstream &out, const T &val) {
  out.write(reinterpret_cast<const char *>(&val), sizeof(T));
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
  if (!line.empty()) lines.push_back(line);
  return lines;
}

// Seek ke string_pos, tulis string + null terminator, kembali ke posisi semula
void write_string_segment(std::ofstream &out, const std::string &str,
                          uint64_t &string_pos) {
  auto curr_pos = out.tellp();
  out.seekp(string_pos, std::ios::beg);
  out.write(str.c_str(), str.size());
  out.put('\0');
  out.seekp(curr_pos, std::ios::beg);
  string_pos += str.size() + 1;
}

void Builder::build(std::string filename) {

  // Cari manifest.yaml
  std::shared_ptr<File> manifest = nullptr;
  for (const auto &f : loader->get_files()) {
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") {
      manifest = f;
      break;
    }
  }

  if (!manifest) {
    std::cerr << "error: manifest not found" << std::endl;
    return;
  }

  std::vector<std::string> lines = get_lines(manifest->data);
  Manifest m(std::move(lines));

  const std::vector<std::string> required_fields = {
      "name", "version", "author", "description", "license", "main"};

  for (const auto &field : required_fields) {
    auto val = m[field];
    if (!val.has_value() ||
        std::holds_alternative<std::vector<std::string>>(val.value())) {
      std::cerr << "error: module must have " << field << std::endl;
      return;
    }
  }

  if (!m["capability"].has_value()) m.set("capability", std::vector<std::string>{});
  if (!m["dependency"].has_value()) m.set("dependency", std::vector<std::string>{});

  auto capability = std::get<std::vector<std::string>>(m["capability"].value());
  auto dependency = std::get<std::vector<std::string>>(m["dependency"].value());

  // Hitung ukuran awal header (fixed fields saja, sebelum capability/dependency entries)
  metadata_header header{};
  header.magic   = VOXMO_MAGIC;
  header.version = 1;
  header.header_len =
      sizeof(header.magic) +
      sizeof(header.version) +
      sizeof(header.header_len) +
      sizeof(header.file_counts) +
      sizeof(header.nama_module) +
      sizeof(header.description) +
      sizeof(header.license) +
      sizeof(header.version_str) +
      sizeof(header.author) +
      sizeof(header.main_file) +
      sizeof(header.capability) +
      sizeof(header.dependency);

  // capability entries dimulai tepat setelah fixed header
  uint32_t capability_offset = header.header_len;

  // jumlah file tanpa manifest.yaml
  header.file_counts = static_cast<uint32_t>(loader->get_files().size() - 1);

  uint64_t string_pos =
      sizeof(metadata_header) +
      header.file_counts  * sizeof(metadata_file) +
      capability.size()   * sizeof(metadata_string) +
      dependency.size()   * sizeof(metadata_string);

  printf("string_pos: %lu\n", string_pos);

  std::ofstream out(filename, std::ios::binary | std::ios::out);

  auto write_str_field = [&](metadata_string &field, const std::string &str) {
    field.length = static_cast<uint16_t>(str.size() + 1);
    field.pos    = string_pos;
    write_struct(out, field);
    write_string_segment(out, str, string_pos);
  };

  write_struct(out, header.magic);
  write_struct(out, header.version);

  // Catat posisi header_len untuk di-patch nanti setelah ukuran finalnya diketahui
  auto header_len_pos = out.tellp();
  write_struct(out, header.header_len);
  write_struct(out, header.file_counts);

  std::string name        = std::get<std::string>(m["name"].value());
  std::string description = std::get<std::string>(m["description"].value());
  std::string license     = std::get<std::string>(m["license"].value());
  std::string version     = std::get<std::string>(m["version"].value());
  std::string author      = std::get<std::string>(m["author"].value());
  std::string main        = std::get<std::string>(m["main"].value());

  // Strip whitespace dari field main
  main.erase(std::remove_if(main.begin(), main.end(),
                            [](unsigned char c) { return std::isspace(c); }),
             main.end());

  write_str_field(header.nama_module, name);
  write_str_field(header.description, description);
  write_str_field(header.license,     license);
  write_str_field(header.version_str, version);
  write_str_field(header.author,      author);
  write_str_field(header.main_file,   main);

  // metadata_list capability
  header.capability.count        = static_cast<uint16_t>(capability.size());
  header.capability.metadata_pos = capability_offset;
  write_struct(out, header.capability);
  std::cout << "capability count: "        << header.capability.count        << std::endl;
  std::cout << "capability metadata_pos: " << header.capability.metadata_pos << std::endl;

  // metadata_list dependency
  header.dependency.count        = static_cast<uint16_t>(dependency.size());
  header.dependency.metadata_pos = capability_offset +
                                   capability.size() * sizeof(metadata_string);
  write_struct(out, header.dependency);
  std::cout << "dependency count: "        << header.dependency.count        << std::endl;
  std::cout << "dependency metadata_pos: " << header.dependency.metadata_pos << std::endl;

  // === Capability entries ===
  std::vector<metadata_string> metadata_capability(capability.size());
  for (size_t i = 0; i < capability.size(); ++i) {
    write_str_field(metadata_capability[i], capability[i]);
  }
  header.header_len += static_cast<uint32_t>(capability.size() * sizeof(metadata_string));

  // === Dependency entries ===
  std::vector<metadata_string> metadata_dependency(dependency.size());
  for (size_t i = 0; i < dependency.size(); ++i) {
    std::cout << "dependency: " << dependency[i] << std::endl;
    write_str_field(metadata_dependency[i], dependency[i]);
  }
  header.header_len += static_cast<uint32_t>(dependency.size() * sizeof(metadata_string));

  // Patch header_len dengan nilai final
  {
    auto curr_pos = out.tellp();
    out.seekp(header_len_pos, std::ios::beg);
    write_struct(out, header.header_len);
    out.seekp(curr_pos, std::ios::beg);
  }

// file metadata entries
  std::vector<std::streampos> file_offset_pos;

  for (const auto &f : loader->get_files()) {
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") continue;

    if (f->data.size() > UINT32_MAX) {
      std::cerr << "error: file too large: " << p.filename() << std::endl;
      return;
    }

    metadata_file mf{};
    mf.offset          = 0; // akan di-patch
    mf.metadata_length = static_cast<uint32_t>(sizeof(mf));
    mf.size            = static_cast<uint32_t>(f->data.size());
    mf.nama_file.length = static_cast<uint16_t>(p.filename().string().size() + 1);
    mf.nama_file.pos    = string_pos;

    file_offset_pos.push_back(out.tellp());

    write_struct(out, mf);
    write_string_segment(out, p.filename().string(), string_pos);
  }

  uint64_t file_pos = string_pos;
  auto files = loader->get_files();
  int file_idx = 0;

  for (size_t i = 0; i < files.size(); ++i) {
    const auto &f = files[i];
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") continue;

    // Patch field `offset` di metadata_file yang sudah ditulis
    out.seekp(file_offset_pos[file_idx++], std::ios::beg);
    write_struct(out, file_pos); // uint64_t, sesuai tipe mf.offset

    // Tulis data file
    out.seekp(file_pos, std::ios::beg);
    out.write(f->data.data(), f->data.size());
    file_pos += f->data.size();
  }

  printf("last file_pos: %lu\n", file_pos);
  printf("build done\n");
}