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

    void setSliceAsMat();
    void setSliceMaskAsMat();
    void setEffectName(std::string effectName);
    void setSliceIndex(int index);

    cv::Mat getSliceAsMat();
    cv::Mat getSliceMaskAsMat();
    size_t getDepth() const;
    std::string getEffectName() const;
    int getSliceIndex() const;


    
    // técnicas de visión artificial
    cv::Mat processSlice(cv::Mat sliceToProceess = cv::Mat());
    cv::Mat aplyThreshold(cv::Mat sliceProcessed = cv::Mat(), double umbral = 55.0);
    cv::Mat aplyContratstStreching(cv::Mat sliceProcessed = cv::Mat());
    cv::Mat aplyUmbralBinary();
    cv::Mat aplyBitWiseOperation(cv::Mat sliceProcessed1 = cv::Mat(), std::string type = "AND");
    cv::Mat aplyCanny(cv::Mat sliceProcessed = cv::Mat());
    cv::Mat adjustBrightness(cv::Mat sliceProcessed = cv::Mat());

    // Suavizado
    cv::Mat aplyMeanFilter(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5);
    cv::Mat aplyGaussianFilter(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5, double sigmaX = 1.0);
    cv::Mat aplyMedianFilter(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5);
    cv::Mat aplyBilateralFilter(cv::Mat sliceProcessed = cv::Mat(), int diameter = 9, double sigmaColor = 75.0, double sigmaSpace = 75.0);

    // Morfologicas
    cv::Mat aplyErosion(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5);
    cv::Mat aplyDilation(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5);    
    cv::Mat aplyOpening(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5);
    cv::Mat aplyClosing(cv::Mat sliceProcessed = cv::Mat(), int kernelSize = 5);

    //histograma
    cv::Mat aplyHistogramEqualization(cv::Mat sliceProcessed = cv::Mat());
    // investigado
    cv::Mat aplyEmbossFilter(cv::Mat sliceProcessed = cv::Mat());
  private:
    VolumetricImagePointer volumetricImage;
    VolumetricImagePointer volumetricImageMask;
    
    cv::Mat slice;    
    cv::Mat sliceMask;

    std::string effectName="";    
    int sliceIndex = 0;
};