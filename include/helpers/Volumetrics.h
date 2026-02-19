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

    bool loadVolumetric(std::string path, std::string type = "flair");

    void setSliceAsMat(int sliceIndex);
    void setSliceMaskAsMat(int sliceIndex);

    cv::Mat getSliceAsMat();

    cv::Mat getSliceMaskAsMat();

    cv::Mat processSlice();

    

  private:
    VolumetricImagePointer volumetricImage;
    VolumetricImagePointer volumetricImageMask;
    //? modificar para guardar uno o más slices y ya no se manejara por indice??
    cv::Mat slice;
    //? modificar para guardar uno o más máscaras
    cv::Mat sliceMask;

    //? modificar para guardar uno o más slices y ya no se manejara por indice??
    //?Vector<cv::Mat> processedSlice;
};