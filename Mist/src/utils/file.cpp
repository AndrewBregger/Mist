#include "file.hpp"
#include <cstdio>
#include <iostream>

#include <sstream>
#include <fstream>


#include <functional>

namespace io {
    File::File(const fs::path &p) : FileIO(io::IOFile, p) {
    }

    File::~File() {
    }

    std::string File::extention() {
        return path().extension().string();
    }

    bool File::load() {
        // this shouldn't happen
        if(kind() != io::IOFile)
            return false;

        std::ifstream iis(absolute_path().string());
        if(iis) {
            std::stringstream ss;
            ss << iis.rdbuf();
            content = ss.str();
            return true;
        }
        return false;
    }

    const std::string &File::value() {
        return content;
    }

}
