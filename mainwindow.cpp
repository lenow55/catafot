#include "mainwindow.h"
#include "ui_mainwindow.h"
// #include "openglgrid.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , ui(new Ui::MainWindow)
    , len_scale(10)

{
    ui->setupUi(this);
    connect(
        ui->rot_Slider,
        SIGNAL(sliderMoved(int)),
        this,
        SLOT(onRotateSliderMoved(int)));
    connect(
        this,
        SIGNAL(rotateChanged(int)),
        ui->openGLWidget,
        SLOT(onRotateChanged(int)));

    connect(
        ui->angle_Slider,
        SIGNAL(sliderMoved(int)),
        this,
        SLOT(onAngleSliderMoved(int)));
    connect(
        this,
        SIGNAL(angleChanged(int)),
        ui->openGLWidget,
        SLOT(onAngleChanged(int)));

    connect(
        ui->countSlider,
        SIGNAL(sliderMoved(int)),
        this,
        SLOT(onCountSliderMoved(int)));
    connect(
        this,
        SIGNAL(countChanged(int)),
        ui->openGLWidget,
        SLOT(onCountMirrorsChanged(int)));

    connect(
        ui->lenSlider,
        SIGNAL(sliderMoved(int)),
        this,
        SLOT(onLenSliderMoved(int)));
    connect(
        this,
        SIGNAL(lenChanged(int)),
        ui->openGLWidget,
        SLOT(onLenChanged(int)));
}

void MainWindow::onCountSliderMoved(int sliderValue)
{
    ui->countLabel->setText(QString("Count: %1").arg(sliderValue));
    emit countChanged(sliderValue);
}

void MainWindow::onRotateSliderMoved(int sliderValue)
{
    ui->rot_label->setText(QString("Rot:\n%1").arg(sliderValue));
    emit rotateChanged(sliderValue);
}

void MainWindow::onAngleSliderMoved(int sliderValue)
{
    ui->angle_label->setText(QString("Angle:\n%1").arg(sliderValue));
    emit angleChanged(sliderValue);
}

void MainWindow::onLenSliderMoved(int sliderValue)
{
    int value = sliderValue*len_scale;
    ui->lenlabel->setText(QString("Len: %1").arg(sliderValue));
    emit lenChanged(value);
}
