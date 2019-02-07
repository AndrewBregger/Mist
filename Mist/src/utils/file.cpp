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

    bool File::has_line(u64 line) {
        return false;
    }

    bool File::cache_has_line(u64 line) {
        return line <= line_cache.size();
    }

    void File::build_cache_to(u64 line) {
        if(line_cache.empty()) {
            u64 index = 0;
            u64 start = index;

            while(content[index] != '\n')
                index++;

            line_cache.push_back(LineInfo {content.substr(start, index - start), start, index});
        }

        const LineInfo& lineInfo = line_cache.back();
        u64 startIndex = lineInfo.end + 1;

        u64 start = startIndex;

        while(line_cache.size() < line) {
            while(content[startIndex] != '\n')
                startIndex++;
            line_cache.push_back(LineInfo {content.substr(start, startIndex - start), start, startIndex});
            start = ++startIndex;
        }
    }

    const std::string &File::get_line(u64 line) {
        if(cache_has_line(line))
            return line_cache[line - 1].line;

        build_cache_to(line);

        return line_cache[line - 1].line;
    }
}
