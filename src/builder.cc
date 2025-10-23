#include "builder.hh"
#include "manifest.hh"
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

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

void Builder::build(std::string output) {

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

  std::cout << "author : " << std::get<std::string>(m["author"]) << std::endl; 
}