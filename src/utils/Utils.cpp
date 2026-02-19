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

bool generateStatistics(Volumetrics &volumetrics, const cv::Mat &slice, const QString &outputFolder, QWidget *parent) {
    // 1) Obtener la máscara desde volumetrics
    cv::Mat mask = volumetrics.getSliceMaskAsMat();
    if (mask.empty()) {
        // No hay máscara → fallamos
        return false;
    }

    // 2) Convertir 'slice' a escala de grises si viene en color
    cv::Mat graySlice;
    if (slice.channels() == 3) {
        cv::cvtColor(slice, graySlice, COLOR_BGR2GRAY);
    } else {
        graySlice = slice;
    }

    // 3) Recopilar valores de píxel dentro de la máscara
    std::vector<int> valores;
    valores.reserve(graySlice.rows * graySlice.cols);
    for (int y = 0; y < mask.rows; ++y) {
        const uchar *ptrMask = mask.ptr<uchar>(y);
        const uchar *ptrGray = graySlice.ptr<uchar>(y);
        for (int x = 0; x < mask.cols; ++x) {
            if (ptrMask[x] > 0) {
                valores.push_back(static_cast<int>(ptrGray[x]));
            }
        }
    }
    if (valores.empty()) {
        // La máscara cubre cero píxeles
        return false;
    }

    // 4) Guardar valores en CSV en ./tmp/valores.csv
    QString tmpDir = QCoreApplication::applicationDirPath() + "/tmp";
    QDir().mkpath(tmpDir); // crea tmp/ si no existe

    QString csvPath = tmpDir + "/valores.csv";
    {
        QFile file(csvPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        QTextStream out(&file);
        for (int v : valores) {
            out << v << "\n";
        }
        file.close();
    }

    // 5) Invocar el script Python (scripts/main.py) con python3
    QString scriptPython = "python3";
    QString scriptFile = QCoreApplication::applicationDirPath() + "/scripts/main.py";
    QStringList argumentos;
    argumentos << scriptFile << csvPath << outputFolder;

    QProcess process;
    process.start(scriptPython, argumentos);
    bool terminó = process.waitForFinished(10000); // 10 s de timeout
    if (!terminó || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QString err = process.readAllStandardError();
        qDebug() << "Error Python:" << err;
        return false;
    }

    // 6) Preparar rutas a los archivos generados por Python
    QString boxplotPath = outputFolder + "/boxplot.png";
    QString histoPath = outputFolder + "/histograma.png";
    QString barrasPath = outputFolder + "/estadisticos_basicos.png";
    QString reportePath = outputFolder + "/reporte_estadisticas.txt";

    // 7) Mostrar un QDialog modal con pestañas para cada gráfico y el reporte
    QDialog dlg(parent);
    dlg.setWindowTitle("Resultados Estadísticos");

    QTabWidget *tabs = new QTabWidget(&dlg);

    // 7a) Pestaña “Reporte” (texto)
    {
        QTextEdit *txt = new QTextEdit;
        txt->setReadOnly(true);
        QFile ftxt(reportePath);
        if (ftxt.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString contenido = ftxt.readAll();
            txt->setPlainText(contenido);
            ftxt.close();
        } else {
            txt->setPlainText("No se encontró reporte de texto.");
        }
        tabs->addTab(txt, "Reporte");
    }

    // 7b) Pestaña “Boxplot”
    {
        QLabel *lbl = new QLabel;
        if (QFile::exists(boxplotPath)) {
            QPixmap pix(boxplotPath);
            lbl->setPixmap(pix.scaled(600, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            lbl->setText("No se encontró boxplot.png");
        }
        tabs->addTab(lbl, "Boxplot");
    }

    // 7c) Pestaña “Histograma”
    {
        QLabel *lbl = new QLabel;
        if (QFile::exists(histoPath)) {
            QPixmap pix(histoPath);
            lbl->setPixmap(pix.scaled(600, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            lbl->setText("No se encontró histograma.png");
        }
        tabs->addTab(lbl, "Histograma");
    }

    // 7d) Pestaña “Básicos”
    {
        QLabel *lbl = new QLabel;
        if (QFile::exists(barrasPath)) {
            QPixmap pix(barrasPath);
            lbl->setPixmap(pix.scaled(600, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            lbl->setText("No se encontró estadisticos_basicos.png");
        }
        tabs->addTab(lbl, "Básicos");
    }

    // Botón “Cerrar” y layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabs);

    QPushButton *botonCerrar = new QPushButton("Cerrar");
    QObject::connect(botonCerrar, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(botonCerrar);

    dlg.setLayout(layout);
    dlg.resize(640, 480);
    dlg.exec();

    return true;
}
} // namespace Utils