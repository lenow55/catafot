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
    void countChanged(int);
    void rotateChanged(int);
    void angleChanged(int);

public slots:
    void onCountSliderMoved(int);
    void onRotateSliderMoved(int);
    void onAngleSliderMoved(int);
    // void onCountSliderChange(int);
    // void onRotateSliderChange(int);
    // void onAngleSliderChange(int);
};

#endif // MAINWINDOW_H
