//
// Created by Andrew Bregger on 2019-01-08.
//

#include "fileio.hpp"

io::FileIO::FileIO(FileKind k, const fs::path &p) : k(k), p(p),
n(p.filename()), uid(FileIO::hash_name(p.string())) {
}

io::FileIO::~FileIO() {
}

const fs::path &io::FileIO::path() {
    return p;
}

std::string io::FileIO::name() {
    return n.string();
}

// stupid way of doing this.
// but oh well.
fs::path io::FileIO::relative_path(const fs::path &dir) {

    fs::path local_dir;

    if (!fs::is_directory(dir)) {
        if (dir.has_parent_path())
            local_dir = dir.parent_path();
        else {
            local_dir = dir;
            local_dir.remove_filename();
        }
    }
    else
        local_dir = dir;

    auto dir_abs = fs::absolute(local_dir);
    auto abs = absolute_path();

    auto abs_iter = dir_abs.begin(), l_iter = abs.begin();
    for(;
        abs_iter != dir_abs.end() && l_iter != abs.end();
        ++abs_iter, ++l_iter) {
        if(*abs_iter != *l_iter)
            break;
    }

    if(abs_iter != dir_abs.end())
        return fs::path("");

    fs::path n_path;
    while(l_iter != abs.end()) {
        n_path /= *l_iter;
        ++l_iter;
    }
    return n_path;
}

fs::path io::FileIO::absolute_path() {
    return fs::absolute(p);
}

bool io::FileIO::is_file() {
    return k == IOFile;
}

bool io::FileIO::is_load() {
    return loaded;
}


u64 io::FileIO::hash_name(const fs::path &path) {
    return hash_name(fs::absolute(path).string());
}


u64 io::FileIO::hash_name(const std::string &filename) {
    std::hash<std::string> hasher;
    return hasher(filename);
}
