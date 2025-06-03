#include <iostream>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkNiftiImageIOFactory.h> // Para registrar la fábrica NIfTI
#include <string>

#include <opencv2/opencv.hpp>

#include "helpers/DirectionImages.h"
#include "helpers/Volumetrics.h"

using namespace std;
using namespace itk;
using namespace cv;

int main() {

    const string brats000 = DirectionImages0::brats0Flair;
    const string brats000Mask = DirectionImages0::brats0Seg;

    const string brats002 = DirectionImages2::brats2Flair;
    const string brats002Mask = DirectionImages2::brats2Seg;

    cout << "Mostrar Ruta: " << brats000 << endl;

    Volumetrics volumetrics;
    Volumetrics volumetrics2;

    const bool result = volumetrics.loadVolumetric(brats000);
    const bool resultMask = volumetrics.loadVolumetric(brats000Mask, "mask");

    if (!result) {
        cout << "Error al cargar el volumen" << endl;
        return 0;
    }

    if (!resultMask) {
        cout << "Error al cargar las mascaras" << endl;
    }

    cout << "Volumen cargado correctamente :)" << endl;

    volumetrics.setSliceAsMat(65);
    Mat secondSlice = volumetrics.getSliceAsMat();

    namedWindow("Ventana_65", WINDOW_AUTOSIZE);
    imshow("Ventana_65", secondSlice);

    volumetrics.setSliceMaskAsMat(65);
    Mat secondSliceMask = volumetrics.getSliceMaskAsMat();

    namedWindow("Ventana_65_mask", WINDOW_AUTOSIZE);
    imshow("Ventana_65_mask", secondSliceMask);


    waitKey(0);

    return 0;
}