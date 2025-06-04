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
Mat Volumetrics::processSlice() {
    if (slice.empty() || sliceMask.empty()) {
        cerr << "Volumetrics::processSlice: slice o sliceMask vacíos.\n";
        return Mat();
    }

    Mat colorSlice;
    cvtColor(slice, colorSlice, COLOR_GRAY2BGR);

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

    cvtColor(imageToProcess, grayImage, COLOR_BGR2GRAY);

    threshold(grayImage, thresholded, umbral, 255, THRESH_BINARY);
    return thresholded;
}

/**
 * @brief Aplicar umbral binario a un slice
 */
Mat Volumetrics::aplyUmbralBinary() {
    Mat imageToProcess = sliceMask.clone();

    Mat imageHSV;
    cvtColor(imageToProcess, imageHSV, COLOR_BGR2HSV);

    cv::Scalar lowerBoundHSV(0, 0, 60);
    cv::Scalar upperBoundHSV(180, 30, 70);

    cv::Mat maskBinary;
    cv::inRange(imageHSV, lowerBoundHSV, upperBoundHSV, maskBinary);

    return maskBinary;
}

Mat Volumetrics::aplyContratstStreching(Mat sliceProcessed) {

    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();

    std::vector<cv::Mat> separateChannels;
    if (sliceProcessed.channels() == 1) {
        // Imagen ya en escala de grises
        separateChannels.push_back(sliceProcessed);
    } else {
        // Imagen color: separamos en B, G y R
        cv::split(sliceProcessed, separateChannels);
    }

    for (size_t i = 0; i < separateChannels.size(); i++) {
        double minVal = 0.0, maxVal = 0.0;
        // Encontrar mínimo y máximo en el canal i
        cv::minMaxLoc(separateChannels[i], &minVal, &maxVal);
        // Si todos los píxeles tienen el mismo valor, no hay nada que estirar
        if (minVal == maxVal) {
            // Dejamos ese canal tal cual
            continue;
        }
        // Calculamos escala y desplazamiento
        double escala = 255.0 / (maxVal - minVal);
        double desplazamiento = -minVal * escala;
        // Convertimos el canal en un nuevo Mat estirado
        cv::Mat canalStretched;
        separateChannels[i].convertTo(canalStretched, CV_8U, escala, desplazamiento);
        // Reemplazamos el canal original por el canal estirado
        separateChannels[i] = canalStretched;
    }

    cv::Mat finalImage;
    if (separateChannels.size() == 1) {
        // Solo había un canal (gris)
        finalImage = separateChannels[0];
    } else {
        // Volvemos a mezclar B, G y R
        cv::merge(separateChannels, finalImage);
    }

    return finalImage;
}

Mat Volumetrics::aplyBitWiseOperation(Mat sliceProcessed1, string type) {

    Mat imageToProcess = (sliceProcessed1.empty()) ? slice.clone() : sliceProcessed1.clone();

    Mat result;
    if (type == "NOT") {
        bitwise_not(imageToProcess, result);
        return result;
    }

    if (type == "AND") {
        bitwise_and(imageToProcess, sliceMask, result);
        return result;
    }

    if (type == "OR") {
        bitwise_or(imageToProcess, sliceMask, result);
        return result;
    }

    bitwise_xor(imageToProcess, sliceMask, result);
    return result;
}

Mat Volumetrics::aplyCanny(Mat sliceProcessed) {
    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();

    Mat grayImage;

    cvtColor(imageToProcess, grayImage, COLOR_BGR2GRAY);

    Mat blurredSlice;
    GaussianBlur(grayImage, blurredSlice, cv::Size(5, 5), 1.5);

    // 4) Definir umbrales para Canny
    double lowerThreshold = 50.0;  // Umbral inferior
    double upperThreshold = 150.0; // Umbral superior

    // 5) Aplicar detector de bordes Canny
    Mat edges;
    Canny(blurredSlice, edges, lowerThreshold, upperThreshold);

    return edges;
}

Mat Volumetrics::adjustBrightness(Mat sliceProcessed) {

    Mat imageToProcess = (sliceProcessed.empty()) ? slice.clone() : sliceProcessed.clone();

    int valueBrightness = 50;

    Mat adjustedImage;
    addWeighted(imageToProcess, 1.0, Mat(), 0.0, valueBrightness, adjustedImage);
}


//* |------------| | Sets | |------------|

/**
 * @brief Setear la región de un slice en Z = sliceIndex y lo guarda en this->slice
 * @param sliceIndex Índice Z del slice a extraer
 */
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
 * @param sliceIndex Índice Z del slice a extraer
 */
void Volumetrics::setSliceMaskAsMat(int sliceIndex) {
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

//* |------------| | Gets | |------------|

Mat Volumetrics::getSliceAsMat() {
    return slice;
}

Mat Volumetrics::getSliceMaskAsMat() {
    return sliceMask;
}