// src/MainWindow.cpp
#include "MainWindow.h"
#include "helpers/DirectionImages.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      currentSliceIndex(0),
      outputFolder("") // aún no elegimos carpeta de salida
{
    // 1) Cargar la interfaz generada por uic
    ui->setupUi(this);

    //------------------------------------------------------------------
    // 2) CONFIGURACIÓN INICIAL DE WIDGETS
    //------------------------------------------------------------------

    // 2.1) ComboBox de Brats (identificadores “brats0”, “brats2”, “brats3”)
    ui->cbImageBrats->addItem("brats0");
    ui->cbImageBrats->addItem("brats2");
    ui->cbImageBrats->addItem("brats3");

    // 2.2) Slider de slices deshabilitado hasta que carguemos un volumen
    ui->slSliceNumber->setMinimum(0);
    ui->slSliceNumber->setMaximum(0);
    ui->slSliceNumber->setValue(0);
    ui->slSliceNumber->setEnabled(false);

    // 2.3) ComboBox de efectos de visión
    ui->cbAplyEffect->addItem("Ninguno");
    ui->cbAplyEffect->addItem("Threshold");
    ui->cbAplyEffect->addItem("ContrastStretch");
    ui->cbAplyEffect->addItem("UmbralBinary");
    ui->cbAplyEffect->addItem("BitwiseAND");
    ui->cbAplyEffect->addItem("BitwiseOR");
    ui->cbAplyEffect->addItem("BitwiseXOR");
    ui->cbAplyEffect->addItem("Canny");
    ui->cbAplyEffect->addItem("Brightness");

    // 2.4) Botones “Guardar Imagen” y “Generar Video” deshabilitados al inicio
    ui->btSaveImage->setEnabled(false);
    ui->btGenerateVideo->setEnabled(false);

    // 2.5) TextEdit “txtVideoImages” inicialmente vacío
    ui->txtVideoImages->setPlainText("");
}

MainWindow::~MainWindow() {
    delete ui;
}

//------------------------------------------------------------------------------
// SLOT: Cuando se pulsa “CargarVolumen” (btLoadImage)
//------------------------------------------------------------------------------
void MainWindow::on_btLoadImage_clicked() {
    // 1) Obtener opción seleccionada (“brats0”, “brats2”, “brats3”)
    QString qOption = ui->cbImageBrats->currentText();
    std::string optionUser = qOption.toStdString();

    // 2) Verificar que exista en el mapa global allBratsMap
    if (allBratsMap.count(optionUser) == 0) {
        ui->statusbar->showMessage("Opción inválida: " + qOption);
        return;
    }
    BratsPaths paths = allBratsMap.at(optionUser);

    // 3) Cargar volumen FLAIR
    bool okFlair = volumetrics.loadVolumetric(paths.standar, "flair");
    if (!okFlair) {
        ui->statusbar->showMessage("Error cargando FLAIR: " + QString::fromStdString(paths.standar));
        return;
    }

    // 4) Cargar volumen MÁSCARA
    bool okMask = volumetrics.loadVolumetric(paths.mask, "mask");
    if (!okMask) {
        ui->statusbar->showMessage("Error cargando MASK: " + QString::fromStdString(paths.mask));
        return;
    }

    // 5) Obtener profundidad total (cantidad de slices en Z)
    size_t depth = volumetrics.getDepth();
    if (depth == 0) {
        ui->statusbar->showMessage("Volumen sin profundidad (depth = 0).");
        return;
    }

    // 6) Configurar slider: rango [0 .. depth-1], habilitarlo y fijar valor 0
    ui->slSliceNumber->setEnabled(true);
    ui->slSliceNumber->setMinimum(0);
    ui->slSliceNumber->setMaximum(static_cast<int>(depth) - 1);
    ui->slSliceNumber->setValue(0);

    // 7) Mostrar “0” en la etiqueta lbSliceNum
    ui->lbSliceNum->setText("0");

    // 8) Extraer y mostrar slice Z=0
    currentSliceIndex = 0;
    volumetrics.setSliceAsMat(currentSliceIndex);
    volumetrics.setSliceMaskAsMat(currentSliceIndex);
    currentSlice = volumetrics.getSliceAsMat();
    currentMask = volumetrics.getSliceMaskAsMat();
    showSliceOnLabel(currentSlice, ui->lbSliceImage);

    // 9) Limpiar label procesado (aún no hay)
    ui->lbSliceImageProcessed->setText("Sin procesar");
    ui->lbSliceImageProcessed->setPixmap(QPixmap());
    processedSlice.release();

    // 10) Habilitar botones de “Guardar Imagen” y “Generar Video”
    ui->btSaveImage->setEnabled(true);
    ui->btGenerateVideo->setEnabled(true);

    ui->statusbar->showMessage("Volumen cargado correctamente.");
}

//------------------------------------------------------------------------------
// SLOT: Cuando se mueve el slider slSliceNumber
//------------------------------------------------------------------------------
void MainWindow::on_slSliceNumber_valueChanged(int value) {
    currentSliceIndex = value;
    ui->lbSliceNum->setText(QString::number(value));

    // Extraer slice y máscara en el nuevo índice
    volumetrics.setSliceAsMat(currentSliceIndex);
    volumetrics.setSliceMaskAsMat(currentSliceIndex);
    currentSlice = volumetrics.getSliceAsMat();
    currentMask = volumetrics.getSliceMaskAsMat();

    // Mostrar slice original
    showSliceOnLabel(currentSlice, ui->lbSliceImage);

    // Si “Usar Imagen Destacada” está marcado, mostrar processedSlice
    if (ui->chUseImageProcessed->isChecked() && !processedSlice.empty()) {
        showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
    } else {
        ui->lbSliceImageProcessed->setText("Sin procesar");
        ui->lbSliceImageProcessed->setPixmap(QPixmap());
    }
}

//------------------------------------------------------------------------------
// SLOT: Cuando se cambia el combo cbAplyEffect
//------------------------------------------------------------------------------
void MainWindow::on_cbAplyEffect_currentIndexChanged(int /*index*/) {
    // Si no hay slice ni máscara, no hacemos nada
    if (currentSlice.empty() || currentMask.empty()) {
        return;
    }

    // 1) Generar slice coloreado con máscara (resaltada)
    processedSlice = volumetrics.processSlice();

    // 2) Obtener el nombre del efecto seleccionado
    QString fx = ui->cbAplyEffect->currentText();
    std::string effectName = fx.toStdString();

    // 3) Aplicar el filtro correspondiente
    if (effectName == "Threshold") {
        processedSlice = volumetrics.aplyThreshold(processedSlice, 55.0);
    } else if (effectName == "ContrastStretch") {
        processedSlice = volumetrics.aplyContratstStreching(processedSlice);
    } else if (effectName == "UmbralBinary") {
        processedSlice = volumetrics.aplyUmbralBinary();
    } else if (effectName == "BitwiseAND") {
        processedSlice = volumetrics.aplyBitWiseOperation(processedSlice, "AND");
    } else if (effectName == "BitwiseOR") {
        processedSlice = volumetrics.aplyBitWiseOperation(processedSlice, "OR");
    } else if (effectName == "BitwiseXOR") {
        processedSlice = volumetrics.aplyBitWiseOperation(processedSlice, "XOR");
    } else if (effectName == "Canny") {
        processedSlice = volumetrics.aplyCanny(processedSlice);
    } else if (effectName == "Brightness") {
        processedSlice = volumetrics.adjustBrightness(processedSlice);
    }
    // Si es “Ninguno”, processedSlice ya es solo la máscara resaltada

    // 4) Si el checkbox está marcado, mostrar processedSlice
    if (ui->chUseImageProcessed->isChecked()) {
        showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
    }
}

//------------------------------------------------------------------------------
// SLOT: Cuando se marca/desmarca el checkbox chUseImageProcessed
//------------------------------------------------------------------------------
void MainWindow::on_chUseImageProcessed_toggled(bool checked) {
    if (checked && !processedSlice.empty()) {
        showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
    } else {
        ui->lbSliceImageProcessed->setText("Sin procesar");
        ui->lbSliceImageProcessed->setPixmap(QPixmap());
    }
}

//------------------------------------------------------------------------------
// SLOT: Cuando se pulsa “Guardar Imagen” (btSaveImage)
//------------------------------------------------------------------------------
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
    cv::Mat toSave;
    if (ui->chUseImageProcessed->isChecked() && !processedSlice.empty()) {
        toSave = processedSlice;
    } else {
        toSave = currentSlice;
    }

    // 4) Formar nombre de archivo: “slice_<Z>.png”
    QString nombre = QString("slice_%1.png").arg(currentSliceIndex);
    QString fullPath = QDir(outputFolder).filePath(nombre);

    // 5) Guardar con imwrite
    bool success = cv::imwrite(fullPath.toStdString(), toSave);
    if (!success) {
        ui->statusbar->showMessage("Error al guardar imagen en " + fullPath);
        return;
    }

    ui->statusbar->showMessage("Imagen guardada: " + fullPath);
}

//------------------------------------------------------------------------------
// SLOT: Cuando se pulsa “Generar Video” (btGenerateVideo)
//------------------------------------------------------------------------------
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
    std::string effectName = fx.toStdString();

    // 5) Iterar sobre cada slice Z = 0..depth-1
    for (size_t z = 0; z < depth; ++z) {
        volumetrics.setSliceAsMat(static_cast<int>(z));
        volumetrics.setSliceMaskAsMat(static_cast<int>(z));
        cv::Mat sliceOut = volumetrics.processSlice();

        // Aplicar filtro adicional según effectName
        if (effectName == "Threshold") {
            sliceOut = volumetrics.aplyThreshold(sliceOut, 55.0);
        } else if (effectName == "ContrastStretch") {
            sliceOut = volumetrics.aplyContratstStreching(sliceOut);
        } else if (effectName == "UmbralBinary") {
            sliceOut = volumetrics.aplyUmbralBinary();
        } else if (effectName == "BitwiseAND") {
            sliceOut = volumetrics.aplyBitWiseOperation(sliceOut, "AND");
        } else if (effectName == "BitwiseOR") {
            sliceOut = volumetrics.aplyBitWiseOperation(sliceOut, "OR");
        } else if (effectName == "BitwiseXOR") {
            sliceOut = volumetrics.aplyBitWiseOperation(sliceOut, "XOR");
        } else if (effectName == "Canny") {
            sliceOut = volumetrics.aplyCanny(sliceOut);
        } else if (effectName == "Brightness") {
            sliceOut = volumetrics.adjustBrightness(sliceOut);
        }
        // Si es “Ninguno”, sliceOut es la máscara coloreada

        // 6) Nombre de archivo: “slice_000.png” (rellena con ceros)
        QString nombre = QString("slice_%1.png").arg(z, 3, 10, QChar('0'));
        QString fullPath = QDir(outputFolder).filePath(nombre);

        // 7) Guardar sliceOut
        bool saved = cv::imwrite(fullPath.toStdString(), sliceOut);
        if (!saved) {
            ui->statusbar->showMessage("Error guardando slice " + QString::number(z));
            return;
        }
    }

    ui->statusbar->showMessage(
        QString("Secuencia de %1 imágenes guardada en %2").arg(depth).arg(outputFolder));
}

//------------------------------------------------------------------------------
// Convierte cv::Mat (CV_8UC1 o CV_8UC3) a QImage (copia) para mostrarlo en QLabel
//------------------------------------------------------------------------------
QImage MainWindow::cvMatToQImage(const cv::Mat &mat) {
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

//------------------------------------------------------------------------------
// Muestra un cv::Mat en un QLabel, escalándolo para ajustarse al tamaño del label
//------------------------------------------------------------------------------
void MainWindow::showSliceOnLabel(const cv::Mat &mat, QLabel *label) {
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
