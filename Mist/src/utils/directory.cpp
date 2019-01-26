//
// Created by Andrew Bregger on 2019-01-07.
//

#include "directory.hpp"

#include <iostream>

namespace io {
    Directory::Directory(const fs::path &path, bool only_source) : FileIO(io::IODirectory, path), only_source(only_source) {
    }

    bool Directory::contain(const fs::path &p) {
        return contain(FileIO::hash_name(p));
    }

    bool Directory::contain(u64 hash) {
        return content.find(hash) != content.end();
    }

    bool Directory::load() {
        if(!fs::is_directory(path()))
            return false;

        for(auto& _p : fs::directory_iterator(path())) {
            if(fs::is_directory(_p)) {
                auto dir = new io::Directory(_p);
                dir->load();
                content.insert(std::make_pair(dir->id(), dir));
            }
            else if(fs::is_regular_file(_p)) {
                if(only_source) {
                    auto ext = _p.path().extension();
                    if(ext != "mst")
                        continue;
                }
                auto file = new io::File(_p);
                content.insert(std::make_pair(file->id(), file));
            }
            else {
                std::cout << "Not a valid file: " << _p << std::endl;
            }
        }

        return true;
    }

    std::tuple<FileIO *, bool> Directory::find(const fs::path &p) {
        return find(FileIO::hash_name(p));
    }

    std::tuple<FileIO *, bool> Directory::find(u64 id) {
        if(contain(id))
            return std::make_tuple(content[id], true);

        for(const auto& [key, value] : content) {
            if(!value->is_file()) {
                auto dir = dynamic_cast<io::Directory*>(value);
                auto [file, valid] = dir->find(id);
                if(valid)
                    return std::make_tuple(file, valid);
            }
        }

        return std::make_tuple(nullptr, false);
    }
}