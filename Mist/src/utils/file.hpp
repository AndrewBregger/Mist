#pragma once

#include "common.hpp"

// it is assumed this class is given the absolute path of the file.

namespace io {
	class File {
    public:
        File(const std::string& path);

        bool load(bool force = false);

        std::string extention();
        std::string fullpath();
    
        u64 id();
        const std::string& dir();
        const std::string& name();
        const std::string& value();
        bool is_loaded();


        // optional api
        
        // returns the path of the file relative to the given dir
        std::string relative_path(const std::string& dir);
    
    
        // creates a hash of the filename for easily identifying it.
        static u64 hash_filename(const std::string& filename);
    private:
        // rune* content{nullptr}; // the buffer when converted to unicode.
        std::string content;
        u64 uid{0};

        std::string path;
        std::string filename;
        bool loaded{false};
	};
}
