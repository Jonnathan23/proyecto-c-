#include <iostream>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkNiftiImageIOFactory.h> // Para registrar la fábrica NIfTI
#include <string>

#include <opencv2/opencv.hpp>

#include "DirectionImages.h"
#include "Volumetrics.h"

using namespace std;
using namespace itk;
using namespace cv;

int main() {

    const string brats000 = DirectionImages0::brats0Flair;
    const string brats002 = DirectionImages2::brats2Flair;

    cout << "Mostrar Ruta: " << brats000 << endl;

    Volumetrics volumetrics;
    Volumetrics volumetrics2;

    const bool result = volumetrics.loadVolumetric(brats000);
    const bool result2 = volumetrics2.loadVolumetric(brats002);

    if (!result && !result2) {
        cout << "Error al cargar el volumen" << endl;
    }

    cout << "Volumen cargado correctamente :)" << endl;

    volumetrics.setSliceAsMat(45);
    Mat slice = volumetrics.getSliceAsMat();

    namedWindow("Ventana", WINDOW_AUTOSIZE);
    imshow("Ventana", slice);

    volumetrics.setSliceAsMat(65);
    Mat secondSlice = volumetrics.getSliceAsMat();

    namedWindow("Ventana_65", WINDOW_AUTOSIZE);
    imshow("Ventana_65", secondSlice);


    volumetrics2.setSliceAsMat(45);
    Mat slice2 = volumetrics2.getSliceAsMat();

    namedWindow("Ventana2", WINDOW_AUTOSIZE);
    imshow("Ventana2", slice2);

    volumetrics2.setSliceAsMat(65);
    Mat secondSlice2 = volumetrics2.getSliceAsMat();

    namedWindow("Ventana2_65", WINDOW_AUTOSIZE);
    imshow("Ventana2_65", secondSlice2);

    waitKey(0);

    return 0;
}