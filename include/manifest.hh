#ifndef __MANIFEST_HH__
#define __MANIFEST_HH__

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>
class Manifest {
    public:
    Manifest(std::vector<std::string> data);
    ~Manifest() = default;
    typedef std::variant<std::string, std::vector<std::string>> value;

    value get(std::string key);
    void set(std::string key, value val);

    std::optional<value> operator[](std::string key);
    

    private:
    std::vector<std::pair<std::string, value>> manifest_data;
};

#endif // __MANIFEST_HH__