#include "file_loader.hh"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#if __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#endif

FileLoader::FileLoader() {}

void FileLoader::add(std::vector<std::string> paths) {

  for (std::string path : paths) {
    auto f = openFile(path);
    files.push_back(f);
  }
}

std::shared_ptr<File> FileLoader::openFile(std::string path) {
  int fd = open(path.c_str(), O_RDONLY | std::ios::binary);

  if (fd == -1) {
    std::cerr << "failed to open file " << path << std::endl;
    std::exit(1);
    return 0;
  }
#if __linux__
auto f= openFileLinux(path, fd);
  // close(fd);
  #else
  std::cout << "not linux" << std::endl;
  #endif
  
  close(fd);
  return f;
}

std::shared_ptr<File> FileLoader::openFileLinux(std::string path, int fd) {
  struct stat st;
  fstat(fd, &st);

  char *data = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == MAP_FAILED) {
    perror("mmap gagal");
    return 0;
  }

  write(fd, data, st.st_size);
   
  File file(path, std::vector<char>(data, data + st.st_size));
  munmap(data, st.st_size);
  return std::make_shared<File>(file);
}


std::vector<std::shared_ptr<File>> FileLoader::get_files() {
  return files;
}


File::File(std::string path, std::vector<char> data) : path(path), data(data) {
}