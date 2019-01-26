//
// Created by Andrew Bregger on 2019-01-08.
//

#pragma once

#include "common.hpp"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace io {


    enum FileKind {
        IOFile,
        IODirectory
    };


    class FileIO {

    public:
        FileIO(FileKind k, const fs::path &p);

        virtual ~FileIO();

        inline FileKind kind() { return k; }

        const fs::path& path();
        std::string name();

        fs::path relative_path(const fs::path& dir);
        fs::path absolute_path();

        bool is_file();

        bool is_load();

        virtual bool load() = 0;

        inline u64 id() { return uid; }

        static u64 hash_name(const fs::path& path);

        static u64 hash_name(const std::string& filename);
    private:
        FileKind k;         /// the type of io this is.
        fs::path n;         /// the name of the entity
        fs::path p;         /// the path of the entity
        u64 uid{0};         /// unique id of the entity
        bool loaded{false}; /// whether it is loaded
    };
}
