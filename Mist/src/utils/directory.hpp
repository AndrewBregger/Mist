//
// Created by Andrew Bregger on 2019-01-07.
//

#pragma once

#include <unordered_map>
#include "file.hpp"
#include "fileio.hpp"
#include <tuple>

/// this is an index of the source directory
namespace io {
    class Directory : public FileIO {
    public:
        Directory(const fs::path& path, bool only_source = false);

        virtual bool load();

        bool contain(const fs::path& p);

        bool contain(u64 hash);

        std::tuple<FileIO*, bool> find(const fs::path& p);

        std::tuple<FileIO*, bool> find(u64 id);

    private:
        std::unordered_map<u64, FileIO*> content;

        bool only_source;
    };
}
