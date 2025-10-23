#ifndef __FILE_LOADER_HH__
#define __FILE_LOADER_HH__

#include <memory>
#include <string>
#include <vector>

class File {
    public:
    File(std::string path, std::vector<char> data);
    ~File() = default;

    std::string path;
    std::vector<char> data;

};

class FileLoader 
{
    public:
        FileLoader();
        void add(std::vector<std::string> paths);
        ~FileLoader() = default;
        std::vector<std::shared_ptr<File>> get_files();


    private:
        std::shared_ptr<File> openFile(std::string path);
        std::shared_ptr<File> openFileLinux(std::string path, int fd);
        std::vector<std::shared_ptr<File>> files;

        
};

#endif // __FILE_LOADER_HH__