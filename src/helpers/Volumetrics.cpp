#include <iostream>
#include <itkExtractImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkNiftiImageIOFactory.h>
#include <opencv2/imgproc.hpp>

#include "Volumetrics.h"

using namespace std;
using namespace itk;
using namespace cv;

Volumetrics::Volumetrics() {}
Volumetrics::~Volumetrics() {}

bool Volumetrics::loadVolumetric(string path) {
    NiftiImageIOFactory::RegisterOneFactory();

    // 2) Definir el tipo de lector (imagen 3D float)
    using ReaderType = ImageFileReader<VolumetricImageType>;
    ReaderType::Pointer reader = ReaderType::New();

    reader->SetFileName(path);

    // 4) Intentar leer el volumen. Si falla, atrapar la excepción y retornar false.
    try {
        reader->Update();
    } catch (ExceptionObject &e) {
        cerr << "Error al leer el volumen NIfTI: " << e << endl;
        return false;
    }

    volumetricImage = reader->GetOutput();

    return true;
}

void Volumetrics::setSliceAsMat(int sliceIndex) {
    if (!volumetricImage) {
        cerr << "Volumetrics::setSliceAsMat: volumetricImage no está cargado.\n";
        slice = Mat(); // slice vacío
        return;
    }

    // Obtener las dimensiones Z del volumen
    auto region3D = volumetricImage->GetLargestPossibleRegion();
    auto size3D = region3D.GetSize();
    size_t depth = size3D[2];

    if (sliceIndex < 0 || static_cast<size_t>(sliceIndex) >= depth) {
        cerr << "Volumetrics::setSliceAsMat: índice fuera de rango (Z = "
             << sliceIndex << ", depth = " << depth << ").\n";
        slice = Mat();
        return;
    }

    // 2) Definir la región 3D para extraer un slice en Z = sliceIndex
    ImageRegion<3> sliceRegion;
    {
        // Tomamos el índice y el tamaño de la región entera
        auto start3D = region3D.GetIndex();
        auto sizeRegion3D = region3D.GetSize();

        // Vamos a extraer solamente ese corte:
        // tamaño en Z = 0 (así ITK sabe que es 2D),
        // índice Z = sliceIndex
        sizeRegion3D[2] = 0;
        start3D[2] = sliceIndex;

        sliceRegion.SetSize(sizeRegion3D);
        sliceRegion.SetIndex(start3D);
    }

    // 3) Configurar el filtro de extracción de ITK (2D) a partir de la imagen 3D
    using ExtractFilterType = ExtractImageFilter<VolumetricImageType, Image<float, 2>>;
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();

    extractFilter->SetInput(volumetricImage);
    extractFilter->SetExtractionRegion(sliceRegion);
    extractFilter->SetDirectionCollapseToIdentity(); // para mantener orientaciones 2D válidas

    try {
        extractFilter->Update();
    } catch (ExceptionObject &e) {
        cerr << "Volumetrics::setSliceAsMat: error extrayendo slice: " << e << "\n";
        slice = Mat();
        return;
    }

    // 4) Convertir el resultado (ITK Image<float,2>) a Mat (float 2D)
    auto itkSlice2D = extractFilter->GetOutput();
    auto region2D = itkSlice2D->GetLargestPossibleRegion();
    auto size2D = region2D.GetSize();
    int width = static_cast<int>(size2D[0]);
    int height = static_cast<int>(size2D[1]);

    // Creamos un Mat en 32-bit float
    Mat matFloat(height, width, CV_32FC1);

    // Iterar sobre cada píxel del slice ITK y copiarlo al Mat
    ImageRegionConstIterator<Image<float, 2>> it(itkSlice2D, region2D);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        auto idx = it.GetIndex();
        int x = static_cast<int>(idx[0]);
        int y = static_cast<int>(idx[1]);
        matFloat.at<float>(y, x) = it.Get();
    }

    // 5) Normalizar el matFloat a 8-bit (0-255) y asignar a this->slice
    double minVal, maxVal;
    minMaxLoc(matFloat, &minVal, &maxVal);
    if (maxVal - minVal <= 0.0) {
        // Volumen constante o error: llenamos con ceros
        slice = Mat::zeros(height, width, CV_8UC1);
    } else {
        matFloat.convertTo(slice,
                           CV_8UC1,
                           255.0 / (maxVal - minVal),
                           -minVal * 255.0 / (maxVal - minVal));
    }
}

Mat Volumetrics::getSliceAsMat() {
    return slice;
}
