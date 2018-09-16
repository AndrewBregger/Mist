#include "file.hpp"
#include <cstdio>
#include <iostream>

#include <sstream>
#include <fstream>


#include <functional>

namespace io {
    
    std::string find_end_relative(const std::string& path, char delim) {
        u64 index = path.find_last_of(delim);
        if(index == std::string::npos)
            return path;
        return path.substr(index + 1);
    }
    
    File::File(const std::string& path) : uid(hash_filename(path)) {
        filename = find_end_relative(path, '/');
        
        u64 index = path.find_last_of('/');

        this->path = path.substr(0, index + 1);
    }

    bool File::load() {
        std::fstream ff(fullpath());
        if(!ff) return false;

        std::stringstream ss;
        ss << ff.rdbuf();
        content = ss.str();
        
        loaded = true;
        return true;
    }

    bool File::is_loaded() {
        return loaded;
    }

    std::string File::extention() {
		return find_end_relative(filename, '.');
    }


    u64 File::id() {
        return uid;
    }

    const std::string& File::dir() {
        return path;
    }

    const std::string& File::name() {
        return filename;
    }

    const std::string& File::value() {
        return content;
    }

    std::string File::fullpath() {
        return path + filename;
    }
    
    /// @NOTE: look for a better way for quick file lookup.
    u64 File::hash_filename(const std::string& filename) {
        std::hash<std::string> hasher;
        return hasher(filename);
    }
}
