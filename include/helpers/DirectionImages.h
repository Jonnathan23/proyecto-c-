#pragma once
#include <string>
#include <map>


class DirectionImages0 {
public:
    // Rutas absolutas a cada volumen .nii.gz (paciente 0)
    static const std::string brats0Flair;
    static const std::string brats0T1;
    static const std::string brats0T1c;
    static const std::string brats0T2;
    static const std::string brats0Seg;
};

class DirectionImages2 {
public:
    // Rutas absolutas a cada volumen .nii.gz (paciente 0)
    static const std::string brats2Flair;
    static const std::string brats2T1;
    static const std::string brats2T1c;
    static const std::string brats2T2;
    static const std::string brats2Seg;
};

class DirectionImages3 {
public:
    // Rutas absolutas a cada volumen .nii.gz (paciente 0)
    static const std::string brats3Flair;
    static const std::string brats3T1;
    static const std::string brats3T1c;
    static const std::string brats3T2;
    static const std::string brats3Seg;
};

//TODO: struct BratsPaths

struct BratsPaths {
    std::string flair;
    std::string t1;
    std::string t1c;
    std::string t2;
    std::string seg;
};

extern const std::map<std::string, BratsPaths> allBratsMap;