#include "mainwindow.h"
#include "ui_mainwindow.h"
// #include "openglgrid.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , ui(new Ui::MainWindow)
    , sig_scale(100)

{
    ui->setupUi(this);
    // connect(
    //     ui->n_Slider,
    //     SIGNAL(sliderMoved(int)),
    //     this,
    //     SLOT(onNSliderChange(int)));
    // connect(
    //     this,
    //     SIGNAL(nChanged(int)),
    //     ui->openGLWidget,
    //     SLOT(onNValChanged(int)));
    // connect(
    //     ui->openGLWidget,
    //     SIGNAL(setNRangeVal(int,int,int)),
    //     this,
    //     SLOT(setNSliderRangeLabel(int,int,int)));

    // connect(
    //     ui->openGLWidget,
    //     SIGNAL(setSigEnabled(bool)),
    //     ui->sig_Slider,
    //     SLOT(setEnabled(bool)));
    // connect(
    //     ui->sig_Slider,
    //     SIGNAL(sliderMoved(int)),
    //     this,
    //     SLOT(onSigSliderChange(int)));
    // connect(
    //     this,
    //     SIGNAL(sigChanged(float)),
    //     ui->openGLWidget,
    //     SLOT(onSigValChanged(float)));
    // connect(
    //     ui->openGLWidget,
    //     SIGNAL(setSigValue(float)),
    //     this,
    //     SLOT(setSigSlider(float)));
}

void MainWindow::onNSliderChange(int sliderValue)
{
    ui->n_label->setText(QString("N: %1").arg(sliderValue));
    emit nChanged(sliderValue);
}

void MainWindow::onSigSliderChange(int sliderValue)
{
    float normalValue = (float)sliderValue/sig_scale;
    ui->sig_label->setText(QString("Sig: %1").arg(normalValue));
    emit sigChanged(normalValue);
}

void MainWindow::setSigSlider(float normalValue)
{
    int scaledValue = (int)(normalValue*sig_scale);
    ui->sig_label->setText(QString("Sig: %1").arg(normalValue));
    ui->sig_Slider->setValue(scaledValue);
}

void MainWindow::setNSliderRangeLabel(int min, int max, int value)
{
    ui->n_label->setText(QString("N: %1").arg(value));
    ui->n_Slider->setValue(value);
    ui->n_Slider->setRange(min,max);
}
