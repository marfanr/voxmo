#include "builder.hh"
#include "file_loader.hh"
#include <memory>
#include <string>
#include <vector>
#include "metadata.hh"

int main(int argc, char** argv) {

    std::string output;
    std::vector<std::string> files;

    // parsing parameter
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            output = argv[++i];
            continue;
        } else {
            files.push_back(arg);
            continue;   
        }
    }

    if (output.empty()) {
        output = "out.voxmo";
    }

    std::shared_ptr<FileLoader> fl = std::make_shared<FileLoader>();
    std::shared_ptr<Builder> builder = std::make_shared<Builder>();
    builder->add_loader(fl);

    fl->add(files);
    
    builder->build(output);
    return 0;
}