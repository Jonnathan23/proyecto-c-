#include "MainWindow.h"


using namespace cv;
using namespace std;

/**
 * @brief Constructor de la clase MainWindow
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      currentSliceIndex(0),
      outputFolder("output") {
    // 1) Cargar la interfaz generada por uic
    ui->setupUi(this);

    //|----------| |CONFIGURACIÓN INICIAL DE WIDGETS | |----------|
    //* ComboBox de Brats
    ui->cbImageBrats->addItem("---- Seleccione un volumen ----");
    ui->cbImageBrats->addItem("brats0");
    ui->cbImageBrats->addItem("brats2");
    ui->cbImageBrats->addItem("brats3");

    //* Slider de slices deshabilitado hasta que carguemos un volumen
    ui->slSliceNumber->setMinimum(0);
    ui->slSliceNumber->setMaximum(0);
    ui->slSliceNumber->setValue(0);
    ui->slSliceNumber->setEnabled(false);

    //* ComboBox de efectos de visión
    ui->cbAplyEffect->addItem("---- Seleccione un efecto ----");
    ui->cbAplyEffect->addItem("Ninguno");
    ui->cbAplyEffect->addItem("Threshold");
    ui->cbAplyEffect->addItem("ContrastStretch");
    ui->cbAplyEffect->addItem("UmbralBinary");
    ui->cbAplyEffect->addItem("BitwiseAND");
    ui->cbAplyEffect->addItem("BitwiseOR");
    ui->cbAplyEffect->addItem("BitwiseXOR");
    ui->cbAplyEffect->addItem("Canny");
    ui->cbAplyEffect->addItem("Brightness");

    // Filtros de suavizado añadidos
    ui->cbAplyEffect->addItem("MeanFilter");
    ui->cbAplyEffect->addItem("GaussianFilter");
    ui->cbAplyEffect->addItem("MedianFilter");
    ui->cbAplyEffect->addItem("BilateralFilter");

    // Morfologicas
    ui->cbAplyEffect->addItem("Erosion");
    ui->cbAplyEffect->addItem("Dilation");
    ui->cbAplyEffect->addItem("Opening");
    ui->cbAplyEffect->addItem("Closing");

    // histograma
    ui->cbAplyEffect->addItem("HistogramEqualization");

    // Investigado
    ui->cbAplyEffect->addItem("Emboss");

    //* Botones “Guardar Imagen” y “Generar Video” deshabilitados al inicio
    ui->btSaveImage->setEnabled(false);
    ui->btGenerateVideo->setEnabled(false);

    //* TextEdit “txtVideoImages” inicialmente vacío
    ui->txtVideoImages->setPlainText("");
}

MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief Función que se ejecuta cuando se pulsa el botón “CargarVolumen”
 */
void MainWindow::on_btLoadImage_clicked() {
    // Obtener opción seleccionada (“brats0”, “brats2”, “brats3”)
    QString qOption = ui->cbImageBrats->currentText();
    string optionUser = qOption.toStdString();

    // Verificar que exista en el mapa global allBratsMap
    if (allBratsMap.count(optionUser) == 0) {
        ui->statusbar->showMessage("Opción inválida: " + qOption);
        return;
    }
    BratsPaths paths = allBratsMap.at(optionUser);

    // Cargar volumen FLAIR
    bool okFlair = volumetrics.loadVolumetric(paths.standar, "flair");
    if (!okFlair) {
        ui->statusbar->showMessage("Error cargando FLAIR: " + QString::fromStdString(paths.standar));
        return;
    }

    // Cargar volumen MÁSCARA
    bool okMask = volumetrics.loadVolumetric(paths.mask, "mask");
    if (!okMask) {
        ui->statusbar->showMessage("Error cargando MASK: " + QString::fromStdString(paths.mask));
        return;
    }

    // Obtener profundidad total (cantidad de slices en Z)
    size_t depth = volumetrics.getDepth();
    if (depth == 0) {
        ui->statusbar->showMessage("Volumen sin profundidad (depth = 0).");
        return;
    }

    // Configurar slider: rango [0 .. depth-1], habilitarlo y fijar valor 0
    ui->slSliceNumber->setEnabled(true);
    ui->slSliceNumber->setMinimum(0);
    ui->slSliceNumber->setMaximum(static_cast<int>(depth) - 1);
    ui->slSliceNumber->setValue(0);

    // Mostrar “0” en la etiqueta lbSliceNum
    ui->lbSliceNum->setText("0");

    // Extraer y mostrar slice Z=0

    volumetrics.setSliceAsMat();
    volumetrics.setSliceMaskAsMat();
    currentSlice = volumetrics.getSliceAsMat();
    currentMask = volumetrics.getSliceMaskAsMat();
    showSliceOnLabel(currentSlice, ui->lbSliceImage);

    // Limpiar label procesado (aún no hay)
    ui->lbSliceImageProcessed->setText("Sin procesar");
    ui->lbSliceImageProcessed->setPixmap(QPixmap());
    processedSlice.release();

    // Habilitar botones de “Guardar Imagen” y “Generar Video”
    ui->btSaveImage->setEnabled(true);
    ui->btGenerateVideo->setEnabled(true);

    ui->statusbar->showMessage("Volumen cargado correctamente.");
}

/**
 * @brief Función que se ejecuta cuando se mueve el slider "slSliceNumber"
 * @param value Nuevo valor del slider
 */
void MainWindow::on_slSliceNumber_valueChanged(int value) {
    volumetrics.setSliceIndex(value);
    ui->lbSliceNum->setText(QString::number(value));

    // Extraer slice y máscara en el nuevo índice
    volumetrics.setSliceAsMat();
    volumetrics.setSliceMaskAsMat();
    currentSlice = volumetrics.getSliceAsMat();
    currentMask = volumetrics.getSliceMaskAsMat();

    // Mostrar slice original
    showSliceOnLabel(currentSlice, ui->lbSliceImage);

    // Si “Usar Imagen Destacada” está marcado, mostrar processedSlice
    if (Utils::isChecked(ui) && !processedSlice.empty()) {
        showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
    } else {
        ui->lbSliceImageProcessed->setText("Sin procesar");
        ui->lbSliceImageProcessed->setPixmap(QPixmap());
    }

    processedSlice = Utils::isChecked(ui) ? volumetrics.processSlice() : volumetrics.getSliceAsMat();
    processedSlice = Utils::aplyFilter(volumetrics, processedSlice, volumetrics.getEffectName());

    showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
}

/**
 * @brief Función que se ejecuta cuando se cambia el combo cbAplyEffect
 * @param index Nuevo valor del combo
 */
void MainWindow::on_cbAplyEffect_currentIndexChanged(int /*index*/) {
    // Si no hay slice ni máscara, no hacemos nada
    if (currentSlice.empty() || currentMask.empty()) {
        return;
    }

    QString fx = ui->cbAplyEffect->currentText();
    volumetrics.setEffectName(fx.toStdString());

    processedSlice = Utils::isChecked(ui) ? volumetrics.processSlice() : volumetrics.getSliceAsMat();
    processedSlice = Utils::aplyFilter(volumetrics, processedSlice, volumetrics.getEffectName());

    showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
}

/**
 * @brief Función que se ejecuta cuando se marca/desmarca el checkbox chUseImageProcessed
 * @param checked Nuevo valor del checkbox
 */
void MainWindow::on_chUseImageProcessed_toggled(bool checked) {
    if (!processedSlice.empty()) {
        if (checked) {
            processedSlice = (volumetrics.getEffectName().empty()) ? volumetrics.processSlice() : Utils::aplyFilter(volumetrics, volumetrics.processSlice(), volumetrics.getEffectName());
        } else {
            processedSlice = (volumetrics.getEffectName().empty()) ? volumetrics.getSliceAsMat() : Utils::aplyFilter(volumetrics, volumetrics.getSliceAsMat(), volumetrics.getEffectName());
        }

        showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
    }
}

/**
 * @brief Función que se ejecuta cuando se pulsa el botón btSaveImage
 * @details Guarda la imagen actual en la carpeta de salida
 */
void MainWindow::on_btSaveImage_clicked() {
    // 1) Si no hay slice cargado, salimos
    if (currentSlice.empty()) {
        ui->statusbar->showMessage("No hay slice para guardar.");
        return;
    }

    // 2) Si no se definió carpeta de salida, la pedimos con un diálogo
    if (outputFolder.isEmpty()) {
        QString dir = QFileDialog::getExistingDirectory(
            this,
            "Seleccione carpeta de salida",
            ".", // carpeta inicial
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (dir.isEmpty()) {
            ui->statusbar->showMessage("No se seleccionó carpeta de salida.");
            return;
        }
        outputFolder = dir;
        ui->txtVideoImages->setPlainText(outputFolder);
    }

    // 3) Decidir qué imagen guardar: si el checkbox está marcado y processedSlice existe, uso ese
    Mat toSave;
    toSave = (Utils::isChecked(ui) && !processedSlice.empty()) ? processedSlice : currentSlice;

    // 4) Formar nombre de archivo: “slice_<Z>.png”
    int index = volumetrics.getSliceIndex();

    QString name = QString("slice_%1_%2.png").arg(index).arg(currentSliceIndex);
    QString fullPath = QDir(outputFolder).filePath(name);

    // 5) Guardar con imwrite
    bool success = imwrite(fullPath.toStdString(), toSave);
    if (!success) {
        ui->statusbar->showMessage("Error al guardar imagen en " + fullPath);
        return;
    }

    ui->statusbar->showMessage("Imagen guardada: " + fullPath + ", generando gráficos estadísticos...");

    // Generar datos estadisticos
    // Obtener la máscara y extraer valores dentro de ella
    cv::Mat mask = volumetrics.getSliceMaskAsMat();
    if (mask.empty()) {
        ui->statusbar->showMessage("No se encontró máscara para generar estadísticas.");
        return;
    }

    // 7) Convertir toSave a escala de grises si tuviera 3 canales
    cv::Mat graySlice;
    if (toSave.channels() == 3) {
        cv::cvtColor(toSave, graySlice, cv::COLOR_BGR2GRAY);
    } else {
        graySlice = toSave;
    }

    // 8) Recopilar valores de píxel en un vector
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
        ui->statusbar->showMessage("La máscara no cubre ningún píxel en el slice.");
        return;
    }

    // 9) Guardar estos valores en un CSV en ./tmp/valores.csv
    QString tmpDir = QCoreApplication::applicationDirPath() + "/tmp";
    QDir().mkpath(tmpDir); // crea tmp/ si no existe

    QString csvPath = tmpDir + "/valores.csv";
    {
        QFile file(csvPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            ui->statusbar->showMessage("No se pudo crear archivo CSV en tmp/valores.csv.");
            return;
        }
        QTextStream out(&file);
        for (int v : valores) {
            out << v << "\n";
        }
        file.close();
    }

    // 10) Invocar el script Python que genera gráficos y reporte de estadísticas
    QString scriptPython = "python3";
    QString scriptFile = QCoreApplication::applicationDirPath() + "/scripts/main.py";
    QStringList argumentos;
    argumentos << scriptFile << csvPath << outputFolder;

    QProcess process;
    process.start(scriptPython, argumentos);
    bool terminó = process.waitForFinished(10000); // esperamos hasta 10s
    if (!terminó || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        ui->statusbar->showMessage("Error al ejecutar el script Python.");
        QString err = process.readAllStandardError();
        qDebug() << "Python STDERR:" << err;
        return;
    }

    // 11) Mostrar en ventana modal los gráficos generados:
    //     asumimos que el script guardó: boxplot.png, histograma.png, estadisticos_basicos.png, reporte_estadisticas.txt
    QString boxplotPath = outputFolder + "/boxplot.png";
    QString histoPath = outputFolder + "/histograma.png";
    QString barrasPath = outputFolder + "/estadisticos_basicos.png";
    QString reportePath = outputFolder + "/reporte_estadisticas.txt";

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Resultados Estadísticos");
    QTabWidget *tabs = new QTabWidget(dialog);

    // 11a) Pestaña de reporte de texto
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

    // 11b) Pestaña de boxplot
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

    // 11c) Pestaña de histograma
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

    // 11d) Pestaña de gráficos de barras
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

    // Layout del diálogo
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabs);

    QPushButton *botonCerrar = new QPushButton("Cerrar");
    connect(botonCerrar, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(botonCerrar);

    dialog->setLayout(layout);
    dialog->resize(640, 480);
    dialog->exec();
}

/**
 * @brief Función que se ejecuta cuando se pulsa el botón btGenerateVideo
 */
void MainWindow::on_btGenerateVideo_clicked() {
    // 1) Si no hay volumen cargado, salimos
    if (currentSlice.empty()) {
        ui->statusbar->showMessage("No hay volumen cargado para generar video.");
        return;
    }

    // 2) Si no se definió carpeta de salida, la pedimos
    if (outputFolder.isEmpty()) {
        QString dir = QFileDialog::getExistingDirectory(
            this,
            "Seleccione carpeta de salida para los slices",
            ".",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (dir.isEmpty()) {
            ui->statusbar->showMessage("No se seleccionó carpeta de salida.");
            return;
        }
        outputFolder = dir;
        ui->txtVideoImages->setPlainText(outputFolder);
    }

    // 3) Obtener profundidad total (número de slices)
    size_t depth = volumetrics.getDepth();
    if (depth == 0) {
        ui->statusbar->showMessage("Volumen sin profundidad (depth = 0).");
        return;
    }

    // 4) Obtener el nombre del efecto y si usar processedSlice
    QString fx = ui->cbAplyEffect->currentText();
    string effectName = fx.toStdString();

    QString qNumberSlicesToVideo = ui->txtVideoImages->toPlainText();
    string slicesToVideo = qNumberSlicesToVideo.toStdString();
    numberSlicesToVideo = atoi(slicesToVideo.c_str()) / 2;

    int currentIndex = static_cast<int>(volumetrics.getSliceIndex());
    int beginSlice = (currentIndex - numberSlicesToVideo) < 0 ? 0 : (currentIndex - numberSlicesToVideo);
    int endSlice = (currentIndex + numberSlicesToVideo) > static_cast<int>(depth) ? static_cast<int>(depth) : (currentIndex + numberSlicesToVideo);

    // 6) Extraer la primera imagen para conocer tamaño (width, height)
    volumetrics.setSliceIndex(beginSlice);
    volumetrics.setSliceAsMat();
    volumetrics.setSliceMaskAsMat();

    Mat firstFrame = Utils::isChecked(ui) ? volumetrics.processSlice() : volumetrics.getSliceAsMat();
    firstFrame = Utils::aplyFilter(volumetrics, firstFrame, effectName);

    if (firstFrame.empty()) {
        ui->statusbar->showMessage("No se pudo obtener el primer slice para el video.");
        return;
    }

    // Asegurarse de que el primer frame esté en BGR (VideoWriter espera color)
    cv::Mat colorFirst;
    if (firstFrame.channels() == 1) {
        cv::cvtColor(firstFrame, colorFirst, cv::COLOR_GRAY2BGR);
    } else {
        colorFirst = firstFrame;
    }

    int frameWidth = colorFirst.cols;
    int frameHeight = colorFirst.rows;

    // 7) Crear el archivo de video (.mp4)
    //    - Usamos el codec H.264 (mp4v suele funcionar en la mayoría de instalaciones)
    //    - Elegimos, por ejemplo, 10 fps (puedes modificar ese valor si lo deseas)
    QString videoName = QDir(outputFolder).filePath("output_video.mp4");
    std::string videoPath = videoName.toStdString();
    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    double fps = 10.0;

    cv::VideoWriter writer;
    writer.open(videoPath, fourcc, fps, cv::Size(frameWidth, frameHeight), /*isColor=*/true);
    if (!writer.isOpened()) {
        ui->statusbar->showMessage("No se pudo crear el archivo de video: " + videoName);
        return;
    }

    // 8) Iterar sobre cada slice y escribirlo al video
    for (int index = beginSlice; index <= endSlice; ++index) {
        volumetrics.setSliceIndex(index);
        volumetrics.setSliceAsMat();
        volumetrics.setSliceMaskAsMat();

        cv::Mat sliceOut = Utils::isChecked(ui) ? volumetrics.processSlice() : volumetrics.getSliceAsMat();
        sliceOut = Utils::aplyFilter(volumetrics, sliceOut, effectName);

        if (sliceOut.empty()) {
            continue;
        }

        // Convertir a BGR si es que está en gris
        cv::Mat colorFrame;
        if (sliceOut.channels() == 1) {
            cv::cvtColor(sliceOut, colorFrame, cv::COLOR_GRAY2BGR);
        } else {
            colorFrame = sliceOut;
        }

        // Redimensionar sólo si difiere del tamaño inicial (opcional, pero mantendremos fijo)
        if (colorFrame.cols != frameWidth || colorFrame.rows != frameHeight) {
            cv::resize(colorFrame, colorFrame, cv::Size(frameWidth, frameHeight));
        }

        // Escribir el fotograma en el video
        writer.write(colorFrame);
    }

    writer.release();
    ui->statusbar->showMessage(QString("Video generado correctamente en %1").arg(videoName));
}

/**
 * @brief Convierte Mat (CV_8UC1 o CV_8UC3) a QImage (copia) para mostrarlo en QLabel
 */
QImage MainWindow::cvMatToQImage(const Mat &mat) {
    if (mat.empty()) {
        return QImage();
    }
    if (mat.type() == CV_8UC1) {
        QImage img(mat.data, mat.cols, mat.rows,
                   static_cast<int>(mat.step),
                   QImage::Format_Grayscale8);
        return img.copy();
    } else if (mat.type() == CV_8UC3) {
        Mat rgb;
        cvtColor(mat, rgb, COLOR_BGR2RGB);
        QImage img(rgb.data, rgb.cols, rgb.rows,
                   static_cast<int>(rgb.step),
                   QImage::Format_RGB888);
        return img.copy();
    }
    return QImage();
}

/**
 * @brief Muestra un Mat en un QLabel, escalándolo para ajustarse al tamaño del label
 */
void MainWindow::showSliceOnLabel(const Mat &mat, QLabel *label) {
    QImage img = cvMatToQImage(mat);
    if (img.isNull()) {
        label->setText("Sin imagen");
        label->setPixmap(QPixmap());
        return;
    }
    QPixmap pix = QPixmap::fromImage(img);
    QSize lblSize = label->size();
    QPixmap scaled = pix.scaled(lblSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setPixmap(scaled);
    label->setText("");
}
