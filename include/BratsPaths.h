#pragma once
#include <map>
#include <string>

struct BratsPaths {
    std::string flair;
    std::string t1;
    std::string t1c;
    std::string t2;
    std::string seg;
};

extern const std::map<std::string, BratsPaths> allBratsMap;
