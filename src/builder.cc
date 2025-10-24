#include "builder.hh"
#include "manifest.hh"
#include "metadata.hh"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <variant>
#include <vector>

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

static void set_metadata_string(metadata_string &dest, const std::string &src) {
  dest.length = static_cast<uint16_t>(src.length());
  dest.data = new char[dest.length];
  std::copy(src.begin(), src.end(), dest.data);
}

void set_metadata_list(struct metadata_list &dest,
                       const std::vector<std::string> &src) {
  dest.count = static_cast<uint16_t>(src.size());
  dest.items = new metadata_string[dest.count];

  for (uint16_t i = 0; i < dest.count; ++i) {
    const auto &s = src[i];
    dest.items[i].length = static_cast<uint16_t>(s.length());
    dest.items[i].data = new char[dest.items[i].length];
    std::copy(s.begin(), s.end(), dest.items[i].data);
  }
}

void free_metadata_list(metadata_list &list) {
  for (uint16_t i = 0; i < list.count; ++i) {
    delete[] list.items[i].data;
  }
  delete[] list.items;
  list.items = nullptr;
  list.count = 0;
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

  std::ofstream out(filename, std::ios::binary | std::ios::out);

  // creating metadata
  metadata_header header;
  header.magic = VOXMO_MAGIC;
  header.version = 1;
  header.header_len = 0;

  std::string name = std::get<std::string>(m["name"]);
  set_metadata_string(header.nama_module, name);
  header.header_len += header.nama_module.length;
  set_metadata_string(header.description,
                      std::get<std::string>(m["description"]));
  header.header_len += header.description.length;
  set_metadata_string(header.license, std::get<std::string>(m["license"]));
  header.header_len += header.license.length;
  set_metadata_string(header.version_str, std::get<std::string>(m["version"]));
  header.header_len += header.version_str.length;
  set_metadata_string(header.author, std::get<std::string>(m["author"]));
  header.header_len += header.author.length;
  set_metadata_string(header.main_file, std::get<std::string>(m["main"]));
  header.header_len += header.main_file.length;

  if (std::holds_alternative<std::vector<std::string>>(m["capability"])) {
    set_metadata_list(header.capability,
                      std::get<std::vector<std::string>>(m["capability"]));
    header.header_len += header.capability.count * sizeof(metadata_string);
  } else {
    set_metadata_list(header.capability, std::vector<std::string>());
    header.header_len += sizeof(metadata_list);
  }

  // write header
  out.write((char *)&header, sizeof(header));

  // add file except metadata
  for (const auto &f : loader->get_files()) {
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") {
      continue;
    }


    metadata_file mf;
    mf.size = f->data.size();
    set_metadata_string(mf.nama_file, p.filename().string());
    mf.metadata_length = sizeof(mf) + mf.nama_file.length;
    mf.next_offset = header.header_len + mf.metadata_length + mf.size;

    out.write((char *)&mf, sizeof(mf));
    out.write(f->data.data(), f->data.size());
  }
}