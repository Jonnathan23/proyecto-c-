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

    string optionUser = "brats0";
    if (allBratsMap.count(optionUser) == 0) {
        cerr << "Opción inválida: " << optionUser << endl;
        return EXIT_FAILURE;
    }
    // 3) Obtener copia del struct con las rutas
    BratsPaths paths = allBratsMap.at(optionUser);

    cout << "Rutas cargadas: " << paths.standar << endl;

    Volumetrics volumetrics;

    // 4) Cargar la imagen “flair”
    if (!volumetrics.loadVolumetric(paths.standar, "flair")) {
        cerr << "Error al cargar FLAIR: " << paths.standar << endl;
        return EXIT_FAILURE;
    }
    cout << "FLAIR cargado: " << paths.mask << endl;

    // 5) Cargar la máscara “seg”
    if (!volumetrics.loadVolumetric(paths.mask, "mask")) {
        cerr << "Error al cargar MÁSCARA: " << paths.mask << endl;
        return EXIT_FAILURE;
    }
    cout << "Máscara cargada: " << paths.mask << endl;

    // Extraer y procesar un slice
    int sliceIndex = 60;
    volumetrics.setSliceAsMat(sliceIndex);
    volumetrics.setSliceMaskAsMat(sliceIndex);

    Mat imageProcessed = volumetrics.processSlice();
    Mat imageMask = volumetrics.getSliceMaskAsMat();
    Mat originalImage = volumetrics.getSliceAsMat();

    //* Filters
    Mat threshold = volumetrics.aplyThreshold(); // listo
    Mat contrast = volumetrics.aplyContratstStreching(originalImage); // listo
    Mat binary = volumetrics.aplyUmbralBinary(); //listo
    Mat bitwise = volumetrics.aplyBitWiseOperation( Mat(),"AND"); //listo
    Mat canny = volumetrics.aplyCanny();
    //Mat brightness = volumetrics.adjustBrightness(imageProcessed);

    if (imageProcessed.empty()) {
        cerr << "Error en processSlice()\n";
        return EXIT_FAILURE;
    }


    namedWindow("imageProcessed", WINDOW_AUTOSIZE);
    imshow("imageProcessed", imageProcessed);

    namedWindow("Original", WINDOW_AUTOSIZE);
    imshow("Original", originalImage);

    namedWindow("Mask", WINDOW_AUTOSIZE);
    imshow("Mask", imageMask);

    namedWindow("Threshold", WINDOW_AUTOSIZE);
    imshow("Threshold", threshold);

    
    namedWindow("Contrast", WINDOW_AUTOSIZE);
    imshow("Contrast", contrast);

    namedWindow("Binary", WINDOW_AUTOSIZE);
    imshow("Binary", binary);

    namedWindow("Bitwise", WINDOW_AUTOSIZE);
    imshow("Bitwise", bitwise);

    namedWindow("Canny", WINDOW_AUTOSIZE);
    imshow("Canny", canny);
/*
    namedWindow("Brightness", WINDOW_AUTOSIZE);
    */


    waitKey(0);

    return 0;
}