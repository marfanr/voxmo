#ifndef __BUILDER_HH__
#define __BUILDER_HH__

#include "file_loader.hh"
#include <memory>
#include <string>

class Builder {
public:
  Builder();
  ~Builder() = default;
  void add_loader(std::shared_ptr<FileLoader> fl);
  void build(std::string filename);

private:
std::shared_ptr<FileLoader> loader;
};

#endif // __BUILDER_HH__