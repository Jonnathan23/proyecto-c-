#include <iostream>
#include <itkExtractImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkNiftiImageIOFactory.h>
#include <opencv2/imgproc.hpp>

#include "helpers/Volumetrics.h"

using namespace std;
using namespace itk;
using namespace cv;

Volumetrics::Volumetrics() {}
Volumetrics::~Volumetrics() {}

/**
 * @brief Cargar un volumen NIfTI
 * @param path Ruta del volumen NIfTI
 * @param type Tipo de volumen a cargar
 * @return true si se pudo cargar el volumen, false si no
 */
bool Volumetrics::loadVolumetric(string path, string type) {
    NiftiImageIOFactory::RegisterOneFactory();

    // Definir el tipo de lector (imagen 3D float)
    using ReaderType = ImageFileReader<VolumetricImageType>;
    ReaderType::Pointer reader = ReaderType::New();

    reader->SetFileName(path);

    // Intentar leer el volumen. Si falla, atrapar la excepción y retornar false.
    try {
        reader->Update();
    } catch (ExceptionObject &e) {
        cerr << "Error al leer el volumen NIfTI: " << e << endl;
        return false;
    }

    if (type == "mask") {
        volumetricImageMask = reader->GetOutput();
        return true;
    }

    volumetricImage = reader->GetOutput();
    return true;
}

/**
 * @brief Procesar un slice resaltando en color la zona afectada, metodo principal
 * @return Mat con el slice resaltado
 */
Mat Volumetrics::processSlice(Mat sliceToProcess) {
    if (slice.empty() || sliceMask.empty()) {
        cerr << "Volumetrics::processSlice: slice o sliceMask vacíos.\n";
        return Mat();
    }

    Mat sliceProcessed = (sliceToProcess.empty()) ? slice.clone() : sliceToProcess.clone();

    Mat colorSlice;
    cvtColor(sliceProcessed, colorSlice, COLOR_GRAY2BGR);

    Mat maksFloat;
    sliceMask.convertTo(maksFloat, CV_32F, 1.0f / 255.0f);

    const int rows = colorSlice.rows;
    const int cols = colorSlice.cols;
    Vec3b color;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            float alpha = maksFloat.at<float>(y, x);

            if (alpha <= 0.0f) {
                continue;
            }
            color = colorSlice.at<Vec3b>(y, x);

            uchar originalB = color[0];
            uchar originalG = color[1];
            uchar originalR = color[2];

            float blendedR = (1.0f - alpha) * originalR + alpha * 255.0f;
            uchar newR = cv::saturate_cast<uchar>(blendedR);
            // Asignar de vuelta en el Mat de color
            colorSlice.at<cv::Vec3b>(y, x) = cv::Vec3b(originalB, originalG, newR);
        }
    }

    return colorSlice;
}

/**
 * @brief Aplicar umbral a un slice
 * @param sliceProcessed Slice procesado por el metodo principal
 * @param umbral Umbral a aplicar
 */
Mat Volumetrics::aplyThreshold(cv::Mat sliceProcessed, double umbral) {

    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();
    Mat grayImage;
    Mat thresholded;

    imageToProcess.channels() == 3 ? cvtColor(imageToProcess, grayImage, COLOR_BGR2GRAY) : imageToProcess.copyTo(grayImage);

    threshold(grayImage, thresholded, umbral, 255, THRESH_BINARY);
    return thresholded;
}

/**
 * @brief Aplicar umbral binario a un slice
 */
Mat Volumetrics::aplyUmbralBinary() {
    Mat imageToProcess = sliceMask.clone();

    if (imageToProcess.channels() == 1) {
        Mat tmp;
        cvtColor(imageToProcess, tmp, COLOR_GRAY2BGR);
        imageToProcess = tmp;
    }

    Mat imageHSV;
    cvtColor(imageToProcess, imageHSV, COLOR_BGR2HSV);

    Scalar lowerBoundHSV(0, 0, 60);
    Scalar upperBoundHSV(180, 30, 70);

    Mat maskBinary;
    inRange(imageHSV, lowerBoundHSV, upperBoundHSV, maskBinary);

    // TODO: Redactar en el informe que si devuelve una imagen negra no hay zona "Muerta"

    return maskBinary;
}

Mat Volumetrics::aplyContratstStreching(Mat sliceProcessed) {

    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    vector<Mat> separateChannels;

    imageToProcess.channels() == 1 ? separateChannels.push_back(imageToProcess) : split(imageToProcess, separateChannels);

    for (size_t i = 0; i < separateChannels.size(); i++) {
        double minVal = 0.0, maxVal = 0.0;
        minMaxLoc(separateChannels[i], &minVal, &maxVal);

        if (minVal == maxVal) {
            continue;
        }

        double escala = 255.0 / (maxVal - minVal);
        double desplazamiento = -minVal * escala;

        Mat canalStretched;
        separateChannels[i].convertTo(canalStretched, CV_8U, escala, desplazamiento);

        // Reemplazamos el canal original por la versión estirada
        separateChannels[i] = canalStretched;
    }

    // Reconstruir la imagen final a partir de esos canales
    Mat finalImage;
    if (separateChannels.size() == 1) {
        // Solo había un canal (gris)
        finalImage = separateChannels[0];
    } else {
        // Volvemos a mezclar B, G y R
        merge(separateChannels, finalImage);
    }

    return finalImage;
}

Mat Volumetrics::aplyBitWiseOperation(Mat sliceProcessed1, string type) {

    Mat imageToProcess = (sliceProcessed1.empty()) ? slice.clone() : sliceProcessed1.clone();

    if (imageToProcess.empty() || sliceMask.empty()) {
        return Mat();
    }

    if (imageToProcess.channels() == 1) {
        cvtColor(imageToProcess, imageToProcess, COLOR_GRAY2BGR);
    }

    Mat maskColor = sliceMask.clone();
    if (maskColor.channels() == 1) {
        cvtColor(maskColor, maskColor, COLOR_GRAY2BGR);
    }

    Mat result;
    if (type == "NOT") {
        bitwise_not(imageToProcess, result);
        return result;
    }

    if (type == "AND") {
        bitwise_and(imageToProcess, maskColor, result);
        return result;
    }

    if (type == "OR") {
        bitwise_or(imageToProcess, maskColor, result);
        return result;
    }

    bitwise_xor(imageToProcess, maskColor, result);
    return result;
}

Mat Volumetrics::aplyCanny(Mat sliceProcessed) {
    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    Mat grayImage;

    if (imageToProcess.channels() == 1) {
        cvtColor(imageToProcess, grayImage, COLOR_GRAY2BGR);
    } else {
        grayImage = imageToProcess.clone();
    }

    Mat blurredSlice;
    GaussianBlur(grayImage, blurredSlice, cv::Size(5, 5), 1.5);

    // Definir umbrales para Canny
    double lowerThreshold = 50.0;  // Umbral inferior
    double upperThreshold = 150.0; // Umbral superior

    // Aplicar detector de bordes Canny
    Mat edges;
    Canny(blurredSlice, edges, lowerThreshold, upperThreshold);

    return edges;
}

Mat Volumetrics::adjustBrightness(Mat sliceProcessed) {

    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    int valueBrightness = 50;

    Mat adjustedImage;
    addWeighted(imageToProcess, 1.0, imageToProcess, 0.0, valueBrightness, adjustedImage);

    return adjustedImage;
}

//* |------------| | Suavizado | |------------|

/**
 * @brief Aplica un filtro de promedio a la imagen
 * @param sliceProcessed Slice procesado por el metodo principal
 */
Mat Volumetrics::aplyMeanFilter(Mat sliceProcessed, int kernelSize) {
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    // Forzar tamaño impar mínimo de kernel
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    Mat result;
    // blur() aplica promedio en cada vecindario de tamaño (kernelSize × kernelSize)
    blur(imageToProcess, result, cv::Size(kernelSize, kernelSize));
    return result;
}

/**
 * @brief Aplica un filtro gaussiano a la imagen
 * @param sliceProcessed Slice procesado por el metodo principal
 */
Mat Volumetrics::aplyGaussianFilter(Mat sliceProcessed, int kernelSize, double sigmaX) {
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    // Forzar tamaño impar mínimo de kernel
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    Mat result;
    // GaussianBlur(src, dst, Size(k,k), sigmaX, sigmaY=0, bordes por defecto)
    GaussianBlur(imageToProcess, result, cv::Size(kernelSize, kernelSize), sigmaX);
    return result;
}

/**
 * @brief Aplica un filtro de mediana a la imagen
 * @param sliceProcessed Slice procesado por el metodo principal
 */
Mat Volumetrics::aplyMedianFilter(Mat sliceProcessed, int kernelSize) {
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    // Forzar tamaño impar mínimo de kernel (>=3)
    if (kernelSize < 3) {
        kernelSize = 3;
    }
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    Mat result;
    // medianBlur() hace una ordenación de valores en la vecindad y elige la mediana
    medianBlur(imageToProcess, result, kernelSize);
    return result;
}

/** @brief Aplica un filtro bilateral a la imagen */
Mat Volumetrics::aplyBilateralFilter(Mat sliceProcessed, int diameter, double sigmaColor, double sigmaSpace) {
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    if (imageToProcess.empty()) {
        return Mat();
    }

    // diameter debe ser >=1
    if (diameter < 1) {
        diameter = 1;
    }

    Mat result;
    // bilateralFilter(src, dst, d, sigmaColor, sigmaSpace)
    bilateralFilter(imageToProcess, result, diameter, sigmaColor, sigmaSpace);
    return result;
}

//* |------------| | Sets | |------------|

/**
 * @brief Setear la región de un slice en Z = sliceIndex y lo guarda en this->slice
 */
void Volumetrics::setSliceAsMat() {
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

    // Definir la región 3D para extraer un slice en Z = sliceIndex
    ImageRegion<3> sliceRegion;
    {
        // Tomamos el índice y el tamaño de la región entera
        auto start3D = region3D.GetIndex();
        auto sizeRegion3D = region3D.GetSize();

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

    // 4) Convertir el result (ITK Image<float,2>) a Mat (float 2D)
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

    //  Normalizar el matFloat a 8-bit (0-255) y asignar a this->slice
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

/**
 * @brief Extrae un slice del volumen de máscaras y lo guarda en this->sliceMask
 */
void Volumetrics::setSliceMaskAsMat() {
    // 1) Verificar que volumetricImageMask no sea nulo
    if (!volumetricImageMask) {
        cerr << "Volumetrics::setSliceMaskAsMat: volumetricImageMask no está cargado.\n";
        sliceMask = cv::Mat();
        return;
    }

    // 2) Comprobar rango en Z
    auto region3D = volumetricImageMask->GetLargestPossibleRegion();
    auto size3D = region3D.GetSize();
    size_t depth = size3D[2];

    if (sliceIndex < 0 || static_cast<size_t>(sliceIndex) >= depth) {
        cerr << "Volumetrics::setSliceMaskAsMat: índice fuera de rango (Z = "
             << sliceIndex << ", depth = " << depth << ").\n";
        sliceMask = cv::Mat();
        return;
    }

    // 3) Definir región 3D para extraer solo un slice en Z = sliceIndex
    ImageRegion<3> sliceRegion;
    {
        auto start3D = region3D.GetIndex();
        auto sizeRegion = region3D.GetSize();

        sizeRegion[2] = 0;
        start3D[2] = sliceIndex;

        sliceRegion.SetSize(sizeRegion);
        sliceRegion.SetIndex(start3D);
    }

    // 4) Extraer el slice 2D de la máscara
    using ExtractFilterType = ExtractImageFilter<VolumetricImageType, Image<float, 2>>;
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetInput(volumetricImageMask);
    extractFilter->SetExtractionRegion(sliceRegion);
    extractFilter->SetDirectionCollapseToIdentity();

    try {
        extractFilter->Update();
    } catch (ExceptionObject &e) {
        cerr << "Volumetrics::setSliceMaskAsMat: error al extraer mask slice: " << e << "\n";
        sliceMask = cv::Mat();
        return;
    }

    // 5) Convertir ITK slice (float 2D) a cv::Mat<float>
    auto itkSlice2D = extractFilter->GetOutput();
    auto region2D = itkSlice2D->GetLargestPossibleRegion();
    auto size2D = region2D.GetSize();
    int width = static_cast<int>(size2D[0]);
    int height = static_cast<int>(size2D[1]);

    cv::Mat matFloatMask(height, width, CV_32FC1);
    ImageRegionConstIterator<Image<float, 2>> it(itkSlice2D, region2D);
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        auto idx = it.GetIndex();
        int x = static_cast<int>(idx[0]);
        int y = static_cast<int>(idx[1]);
        matFloatMask.at<float>(y, x) = it.Get();
    }

    // 6) Normalizar a 8-bit y guardar en sliceMask
    double minVal, maxVal;
    cv::minMaxLoc(matFloatMask, &minVal, &maxVal);
    if (maxVal - minVal <= 0.0) {
        sliceMask = cv::Mat::zeros(height, width, CV_8UC1);
    } else {
        matFloatMask.convertTo(
            sliceMask,
            CV_8UC1,
            255.0 / (maxVal - minVal),
            -minVal * 255.0 / (maxVal - minVal));
    }
}

/**
 * @brief Establece el sliceIndex
 */
void Volumetrics::setSliceIndex(int index) {
    this->sliceIndex = index;
}

/**
 * @brief Establece el nombre de la tecnica de visión artificial
 */
void Volumetrics::setEffectName(string effectName) {
    this->effectName = effectName;
}

//* |------------| | Morfologicas | |------------|
Mat Volumetrics::aplyErosion(Mat sliceProcessed, int kernelSize) {
    // 1.1) Seleccionar la imagen a procesar
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    // 1.2) Si está vacía, retorno Mat vacía
    if (imageToProcess.empty()) {
        return Mat();
    }

    // 1.3) Asegurar que kernelSize sea impar y ≥ 1
    if (kernelSize < 1) {
        kernelSize = 1;
    }
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    // 1.4) Crear elemento estructurante rectangular
    Mat structElem = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize), cv::Point(-1, -1));
    // 1.5) Aplicar erosión
    Mat result;
    erode(imageToProcess, result, structElem);

    return result;
}

/**
 * @brief Aplica dilatación
 */
Mat Volumetrics::aplyDilation(Mat sliceProcessed, int kernelSize) {
    // 2.1) Seleccionar la imagen a procesar
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    // 2.2) Si está vacía, retorno Mat vacía
    if (imageToProcess.empty()) {
        return Mat();
    }

    // 2.3) Asegurar que kernelSize sea impar y ≥ 1
    if (kernelSize < 1) {
        kernelSize = 1;
    }
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    // 2.4) Crear elemento estructurante rectangular
    Mat structElem = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize), cv::Point(-1, -1));
    // 2.5) Aplicar dilatación
    Mat result;
    dilate(imageToProcess, result, structElem);

    return result;
}

/**
 * @brief Aplica la técnica de apertura - erosión seguida de dilatación
 */
Mat Volumetrics::aplyOpening(Mat sliceProcessed, int kernelSize) {
    // 3.1) Seleccionar la imagen a procesar
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    // 3.2) Si está vacía, retorno Mat vacía
    if (imageToProcess.empty()) {
        return Mat();
    }

    // 3.3) Asegurar que kernelSize sea impar y ≥ 1
    if (kernelSize < 1) {
        kernelSize = 1;
    }
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    // 3.4) Crear elemento estructurante rectangular
    Mat structElem = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize), cv::Point(-1, -1));
    // 3.5) Aplicar apertura (erosión + dilatación)
    Mat result;
    morphologyEx(imageToProcess, result, MORPH_OPEN, structElem);

    return result;
}

/**
 * @brief Cierre - dilatación seguida de erosión
 */
Mat Volumetrics::aplyClosing(Mat sliceProcessed, int kernelSize) {
    // 4.1) Seleccionar la imagen a procesar
    Mat imageToProcess = sliceProcessed.empty() ? slice.clone() : sliceProcessed.clone();

    // 4.2) Si está vacía, retorno Mat vacía
    if (imageToProcess.empty()) {
        return Mat();
    }

    // 4.3) Asegurar que kernelSize sea impar y ≥ 1
    if (kernelSize < 1) {
        kernelSize = 1;
    }
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    // 4.4) Crear elemento estructurante rectangular
    Mat structElem = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize), cv::Point(-1, -1));
    // 4.5) Aplicar cierre (dilatación + erosión)
    Mat result;
    morphologyEx(imageToProcess, result, MORPH_CLOSE, structElem);

    return result;
}

//* |------------| | Gets | |------------|

/**
 * @brief Devuelve la profundidad del volumen
 */
size_t Volumetrics::getDepth() const {
    if (!volumetricImage) {
        return 0;
    }
    auto region3D = volumetricImage->GetLargestPossibleRegion();
    auto size3D = region3D.GetSize();
    return size3D[2];
}

/**
 * @brief Devuelve el slice
 */
Mat Volumetrics::getSliceAsMat() {
    return slice;
}

/**
 * @brief Devuelve la sliceMask
 */
Mat Volumetrics::getSliceMaskAsMat() {
    return sliceMask;
}

/**
 * @brief Devuelve el nombre de la tecnica de visión artificial que se está usando
 */
string Volumetrics::getEffectName() const {
    return effectName;
}

/**
 * @brief Devuelve el sliceIndex
 */
int Volumetrics::getSliceIndex() const {
    return sliceIndex;
}