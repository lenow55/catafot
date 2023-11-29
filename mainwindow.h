#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    Ui::MainWindow *ui;
    int sig_scale;

signals:
    void nChanged(int);
    void sigChanged(float);

public slots:
    void onNSliderChange(int);
    void onSigSliderChange(int);
    void setSigSlider(float);
    void setNSliderRangeLabel(int,int,int);
};

#endif // MAINWINDOW_H
