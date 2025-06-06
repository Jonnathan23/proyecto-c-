#include <MainWindow.h>
#include <iostream> // solo si quieres imprimir mensajes de error
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

namespace Utils {
bool isChecked(Ui::MainWindow *ui) {
    return ui->chUseImageProcessed->isChecked();
}

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

    return processedSlice;
}
} // namespace Utils