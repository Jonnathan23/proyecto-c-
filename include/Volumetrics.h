#pragma once

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <opencv2/core.hpp>
#include <string>


using VolumetricImageType = itk::Image<float, 3>;
using VolumetricImagePointer = VolumetricImageType::Pointer;

class Volumetrics {
public:
    Volumetrics();
    ~Volumetrics();

    bool loadVolumetric(std::string path);

    cv::Mat setSliceAsMat(int slice);
    cv::Mat getSliceAsMat(int slice);

    cv::Mat setSliceMaskAsMat(int slice);
    cv::Mat getSliceMaskAsMat(int slice);

    cv::Mat processSlice();

private:
    VolumetricImagePointer volumetricImage;
    cv::Mat slice;
    cv::Mat sliceMask;
};