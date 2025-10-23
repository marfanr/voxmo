#include "file_loader.hh"
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

    FileLoader fl;
    fl.add(files);
    return 0;
}