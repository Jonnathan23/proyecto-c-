#include <iostream>
#include <itkExtractImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkNiftiImageIOFactory.h>
#include <opencv2/imgproc.hpp>

#include "Volumetrics.h"

using namespace std;
using namespace itk;

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