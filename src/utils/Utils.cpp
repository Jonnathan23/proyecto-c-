#include <MainWindow.h>
#include <iostream> // solo si quieres imprimir mensajes de error
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

namespace Utils {

/**
 * @brief Verifica si el checkbox de usar la imagen procesada esta marcado
 */
bool isChecked(Ui::MainWindow *ui) {
    return ui->chUseImageProcessed->isChecked();
}

/**
 * @brief Aplica el efecto seleccionado a la imagen
 * @param volumetrics Objeto Volumetrics
 * @param processedSlice Imagen a la que se le va a aplicar el efecto
 * @param effectName Nombre del efecto - "Threshold", "ContrastStretch", "UmbralBinary", "BitwiseAND", "BitwiseOR", "BitwiseXOR"
 * @return Imagen con el efecto
 */
Mat aplyFilter(Volumetrics volumetrics, Mat processedSlice, std::string effectName) {

    if (effectName == "Threshold") {
        processedSlice = volumetrics.aplyThreshold(processedSlice, 55.0);
        return processedSlice;
    }

    if (effectName == "ContrastStretch") {
        processedSlice = volumetrics.aplyContratstStreching(processedSlice);
        return processedSlice;
    }

    if (effectName == "UmbralBinary") {
        processedSlice = volumetrics.aplyUmbralBinary();
        return processedSlice;
    }
    if (effectName == "BitwiseAND") {
        processedSlice = volumetrics.aplyBitWiseOperation(processedSlice, "AND");
        return processedSlice;
    }

    if (effectName == "BitwiseOR") {
        processedSlice = volumetrics.aplyBitWiseOperation(processedSlice, "OR");
        return processedSlice;
    }

    if (effectName == "BitwiseXOR") {
        processedSlice = volumetrics.aplyBitWiseOperation(processedSlice, "XOR");
        return processedSlice;
    }

    if (effectName == "Canny") {
        processedSlice = volumetrics.aplyCanny(processedSlice);
        return processedSlice;
    }

    if (effectName == "Brightness") {
        processedSlice = volumetrics.adjustBrightness(processedSlice);
        return processedSlice;
    }

    if (effectName == "MeanFilter") {
        // Usa tamaño de kernel por defecto = 5
        processedSlice = volumetrics.aplyMeanFilter(processedSlice);
        return processedSlice;
    }

    if (effectName == "GaussianFilter") {
        // Usa kernelSize=5, sigmaX=1.0 por defecto
        processedSlice = volumetrics.aplyGaussianFilter(processedSlice);
        return processedSlice;
    }

    if (effectName == "MedianFilter") {
        // Usa kernelSize por defecto = 5
        processedSlice = volumetrics.aplyMedianFilter(processedSlice);
        return processedSlice;
    }

    if (effectName == "BilateralFilter") {
        // Usa diameter=9, sigmaColor=75.0, sigmaSpace=75.0 por defecto
        processedSlice = volumetrics.aplyBilateralFilter(processedSlice);
        return processedSlice;
    }

    if (effectName == "Erosion") {
        processedSlice = volumetrics.aplyErosion(processedSlice, 3);
        return processedSlice;
    }

    if (effectName == "Dilation") {
        processedSlice = volumetrics.aplyDilation(processedSlice, 3);
        return processedSlice;
    }

    if (effectName == "Opening") {
        processedSlice = volumetrics.aplyOpening(processedSlice, 3);
        return processedSlice;
    }

    if (effectName == "Closing") {
        processedSlice = volumetrics.aplyClosing(processedSlice, 3);
        return processedSlice;
    }

    if (effectName == "HistogramEqualization") {
        processedSlice = volumetrics.aplyHistogramEqualization(processedSlice);
        return processedSlice;
    }

    if (effectName == "Emboss") {
        processedSlice = volumetrics.aplyEmbossFilter(processedSlice);
        return processedSlice;
    }

    return processedSlice;
}
} // namespace Utils