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

    cout << "Rutas cargadas: "<<paths.flair << endl;

    Volumetrics volumetrics;

    // 4) Cargar la imagen “flair”
    if (!volumetrics.loadVolumetric(paths.flair, "flair")) {
        cerr << "Error al cargar FLAIR: " << paths.flair << endl;
        return EXIT_FAILURE;
    }
    cout << "FLAIR cargado: " << paths.flair << endl;

    // 5) Cargar la máscara “seg”
    if (!volumetrics.loadVolumetric(paths.seg, "mask")) {
        cerr << "Error al cargar MÁSCARA: " << paths.seg << endl;
        return EXIT_FAILURE;
    }
    cout << "Máscara cargada: " << paths.seg << endl;

    // 6) Extraer y procesar un slice (Z=0)
    int sliceIndex = 60;
    volumetrics.setSliceAsMat(sliceIndex);
    volumetrics.setSliceMaskAsMat(sliceIndex);

    Mat resultado = volumetrics.processSlice();
    if (resultado.empty()) {
        cerr << "Error en processSlice()\n";
        return EXIT_FAILURE;
    }

    namedWindow("Resultado", WINDOW_AUTOSIZE);
    imshow("Resultado", resultado);
    waitKey(0);

    return 0;
}