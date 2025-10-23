#include "builder.hh"
#include "manifest.hh"
#include "metadata.hh"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
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

  // creating output file
  std::ofstream out(filename, std::ios::out | std::ios::binary);
  if (!out) {
    std::cerr << "Failed to open file " << filename << "\n";
    return;
  }

  // creating metadata
  metadata_header header;
  header.magic = VOXMO_MAGIC;
  header.version = 1;
  header.header_len = 0;
  header.nama_module_len = manifest->path.length();
  header.header_len = sizeof(header) + header.nama_module_len;
  out.write((char *)&header, sizeof(header));

  char *nama_module = new char[header.nama_module_len];
  std::copy(manifest->path.begin(), manifest->path.end(), nama_module);
  out.write(nama_module, header.nama_module_len);
  delete[] nama_module;

  for (const auto &f : loader->get_files()) {
    std::filesystem::path p(f->path);
    if (p.filename() == "manifest.yaml") {
      continue;
    }

    metadata_file mf;
    mf.size = f->data.size();
    mf.nama_file_len = mf.nama_file_len =
        static_cast<uint16_t>(std::string(p.filename()).length());
    mf.next_offset = sizeof(header) + header.nama_module_len + sizeof(mf) +
                     mf.nama_file_len + f->data.size();
    mf.metadata_length = sizeof(mf) + mf.nama_file_len;
    out.write((char *)&mf, sizeof(mf));

    std::string name_str = p.filename().string();
    out.write(name_str.data(), name_str.size());
    mf.nama_file_len = static_cast<uint16_t>(name_str.size());

    out.write(f->data.data(), f->data.size());
  }
}