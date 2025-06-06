#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <QString>
#include <opencv2/opencv.hpp>
#include "helpers/Volumetrics.h"
#include "ui_MainWindow.h"  // Header generado por uic

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btLoadImage_clicked();
    void on_slSliceNumber_valueChanged(int value);
    void on_cbAplyEffect_currentIndexChanged(int index);
    void on_chUseImageProcessed_toggled(bool checked);
    void on_btSaveImage_clicked();
    void on_btGenerateVideo_clicked();

private:
    Ui::MainWindow *ui;       // Puntero a la UI generada por uic
    Volumetrics volumetrics;  // Objeto para carga y filtros

    int currentSliceIndex;
    int numberSlicesToVideo = 0;
    bool useImageProcessed = false;

    cv::Mat currentSlice;
    cv::Mat currentMask;
    cv::Mat processedSlice;

    QString outputFolder;     // Carpeta donde guardaremos imágenes

    QImage cvMatToQImage(const cv::Mat &mat);
    void showSliceOnLabel(const cv::Mat &mat, QLabel *label);
};
