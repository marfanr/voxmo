#include "manifest.hh"
#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

Manifest::Manifest(std::vector<std::string> data) {
  for (std::string &line : data) {
    line.erase(line.begin(),
               std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                 return !std::isspace(ch);
               }));
    line.erase(std::find_if(line.rbegin(), line.rend(),
                            [](unsigned char ch) { return !std::isspace(ch); })
                   .base(),
               line.end());

    // skip empty lines
    if (line.empty())
      continue;

    if (line[0] == '-') {
      std::string item = line.substr(1); // buang '-'
      // trim
      item.erase(item.begin(),
                 std::find_if(item.begin(), item.end(), [](unsigned char ch) {
                   return !std::isspace(ch);
                 }));

      if (std::holds_alternative<std::vector<std::string>>(
              manifest_data.back().second)) {
        std::get<std::vector<std::string>>(manifest_data.back().second)
            .push_back(item);
      } else {
        std::vector<std::string> vec;
        vec.push_back(item);
        manifest_data.back().second = vec;
      }

    } else {
      std::string key;
      value val;

      auto pos = line.find(':');
      if (pos != std::string::npos) {
        key = line.substr(0, pos);
        val = line.substr(pos + 1);
      } else {
        key = line;
        val = "";
      }

      // trim
      if (std::holds_alternative<std::string>(val)) {
        std::string &str = std::get<std::string>(val);

        // trim
        str.erase(str.begin(),
                  std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                  }));
      }

      auto pair = std::make_pair(key, val);
      manifest_data.push_back(pair);
    }
  }
}

std::optional<Manifest::value> Manifest::operator[](std::string key) {
  for (auto &d : manifest_data) {
    if (d.first == key) {
      return d.second;
    }
  }
  return std::nullopt;
}

void Manifest::set(std::string key, value val) {
  for (auto &d : manifest_data) {
    if (d.first == key) {
      d.second = val;
      return;
    }
  }
  manifest_data.push_back(std::make_pair(key, val));
}