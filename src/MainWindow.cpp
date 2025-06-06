// src/MainWindow.cpp
#include "MainWindow.h"
#include "helpers/DirectionImages.h"
#include "utils/Utils.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

using namespace cv;
using namespace std;

/**
 * @brief Constructor de la clase MainWindow
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      currentSliceIndex(0),
      outputFolder("output") // aún no elegimos carpeta de salida
{
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
    // 1) Obtener opción seleccionada (“brats0”, “brats2”, “brats3”)
    QString qOption = ui->cbImageBrats->currentText();
    string optionUser = qOption.toStdString();

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

    volumetrics.setSliceAsMat();
    volumetrics.setSliceMaskAsMat();
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
    if (ui->chUseImageProcessed->isChecked() && !processedSlice.empty()) {
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

    processedSlice = ui->chUseImageProcessed->isChecked() ? volumetrics.processSlice() : volumetrics.getSliceAsMat();
    processedSlice = Utils::aplyFilter(volumetrics, processedSlice, volumetrics.getEffectName());

    showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
}


/**
 * @brief Función que se ejecuta cuando se marca/desmarca el checkbox chUseImageProcessed
 * @param checked Nuevo valor del checkbox
 */
void MainWindow::on_chUseImageProcessed_toggled(bool checked) {
    if (checked && !processedSlice.empty()) {                
        showSliceOnLabel(processedSlice, ui->lbSliceImageProcessed);
    } else {
        ui->lbSliceImageProcessed->setText("Sin procesar");
        ui->lbSliceImageProcessed->setPixmap(QPixmap());
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
    if (ui->chUseImageProcessed->isChecked() && !processedSlice.empty()) {
        toSave = processedSlice;
    } else {
        toSave = currentSlice;
    }

    // 4) Formar nombre de archivo: “slice_<Z>.png”
    QString nombre = QString("slice_%1.png").arg(currentSliceIndex);
    QString fullPath = QDir(outputFolder).filePath(nombre);

    // 5) Guardar con imwrite
    bool success = imwrite(fullPath.toStdString(), toSave);
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
    string effectName = fx.toStdString();

    // 5) Iterar sobre cada slice Z = 0..depth-1
    for (size_t z = 0; z < depth; ++z) {
        // TODO Corregir la generacion para el video
        // volumetrics.setSliceAsMat(static_cast<int>(z));
        // volumetrics.setSliceMaskAsMat(static_cast<int>(z));
        volumetrics.setSliceAsMat();
        volumetrics.setSliceMaskAsMat();
        Mat sliceOut = volumetrics.processSlice();
        sliceOut = Utils::aplyFilter(volumetrics, sliceOut, effectName);

        // 6) Nombre de archivo: “slice_000.png” (rellena con ceros)
        QString nombre = QString("slice_%1.png").arg(z, 3, 10, QChar('0'));
        QString fullPath = QDir(outputFolder).filePath(nombre);

        // TODO Los slices deben formar un video
    }

    ui->statusbar->showMessage(
        QString("Secuencia de %1 imágenes guardada en %2").arg(depth).arg(outputFolder));
}

//------------------------------------------------------------------------------
// Convierte Mat (CV_8UC1 o CV_8UC3) a QImage (copia) para mostrarlo en QLabel
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Muestra un Mat en un QLabel, escalándolo para ajustarse al tamaño del label
//------------------------------------------------------------------------------
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
