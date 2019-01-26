#pragma once

#include "common.hpp"
#include "fileio.hpp"

// it is assumed this class is given the absolute path of the file.

namespace io {
	class File : public FileIO {
    public:
        File(const fs::path& p);

        virtual ~File();

        std::string extention();

        virtual bool load();

        const std::string& value();
    private:
        std::string content;
	};
}
