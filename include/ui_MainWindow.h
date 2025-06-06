/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow {
  public:
    QWidget *centralwidget;
    QPushButton *btLoadImage;
    QLabel *lbSliceNum;
    QLabel *label;
    QSlider *slSliceNumber;
    QComboBox *cbAplyEffect;
    QCheckBox *chUseImageProcessed;
    QLabel *label_2;
    QComboBox *cbImageBrats;
    QTextEdit *txtVideoImages;
    QPushButton *btGenerateVideo;
    QPushButton *btSaveImage;
    QLabel *lbSliceImageProcessed;
    QLabel *lbSliceImage;
    QLabel *label_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow) {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        MainWindow->setLayoutDirection(Qt::LeftToRight);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        btLoadImage = new QPushButton(centralwidget);
        btLoadImage->setObjectName(QString::fromUtf8("btLoadImage"));
        btLoadImage->setGeometry(QRect(20, 90, 121, 31));
        lbSliceNum = new QLabel(centralwidget);
        lbSliceNum->setObjectName(QString::fromUtf8("lbSliceNum"));
        lbSliceNum->setGeometry(QRect(300, 20, 21, 17));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(250, 20, 41, 17));
        slSliceNumber = new QSlider(centralwidget);
        slSliceNumber->setObjectName(QString::fromUtf8("slSliceNumber"));
        slSliceNumber->setGeometry(QRect(240, 60, 160, 20));
        slSliceNumber->setOrientation(Qt::Horizontal);
        cbAplyEffect = new QComboBox(centralwidget);
        cbAplyEffect->setObjectName(QString::fromUtf8("cbAplyEffect"));
        cbAplyEffect->setGeometry(QRect(470, 100, 151, 32));
        chUseImageProcessed = new QCheckBox(centralwidget);
        chUseImageProcessed->setObjectName(QString::fromUtf8("chUseImageProcessed"));
        chUseImageProcessed->setGeometry(QRect(450, 30, 181, 21));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(480, 70, 131, 17));
        cbImageBrats = new QComboBox(centralwidget);
        cbImageBrats->setObjectName(QString::fromUtf8("cbImageBrats"));
        cbImageBrats->setGeometry(QRect(20, 40, 171, 32));
        txtVideoImages = new QTextEdit(centralwidget);
        txtVideoImages->setObjectName(QString::fromUtf8("txtVideoImages"));
        txtVideoImages->setGeometry(QRect(320, 100, 104, 31));
        btGenerateVideo = new QPushButton(centralwidget);
        btGenerateVideo->setObjectName(QString::fromUtf8("btGenerateVideo"));
        btGenerateVideo->setGeometry(QRect(195, 100, 111, 33));
        btSaveImage = new QPushButton(centralwidget);
        btSaveImage->setObjectName(QString::fromUtf8("btSaveImage"));
        btSaveImage->setGeometry(QRect(250, 440, 131, 41));
        lbSliceImageProcessed = new QLabel(centralwidget);
        lbSliceImageProcessed->setObjectName(QString::fromUtf8("lbSliceImageProcessed"));
        lbSliceImageProcessed->setGeometry(QRect(360, 170, 251, 211));
        lbSliceImage = new QLabel(centralwidget);
        lbSliceImage->setObjectName(QString::fromUtf8("lbSliceImage"));
        lbSliceImage->setGeometry(QRect(30, 180, 281, 191));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 10, 181, 17));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 29));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow) {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        btLoadImage->setText(QCoreApplication::translate("MainWindow", "CargarVolumen", nullptr));
        lbSliceNum->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Slice:", nullptr));
        chUseImageProcessed->setText(QCoreApplication::translate("MainWindow", "Resaltar Imagen", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Efectos Vision:", nullptr));
        btGenerateVideo->setText(QCoreApplication::translate("MainWindow", "Generar Video", nullptr));
        btSaveImage->setText(QCoreApplication::translate("MainWindow", "Guardar Imagen", nullptr));
        lbSliceImageProcessed->setText(QCoreApplication::translate("MainWindow", "Sin volumen cargado", nullptr));
        lbSliceImage->setText(QCoreApplication::translate("MainWindow", "Sin volumen cargado", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Selecione una Imagen nii", nullptr));
    } // retranslateUi
};

namespace Ui {
class MainWindow : public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
