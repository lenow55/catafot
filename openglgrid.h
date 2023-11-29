#ifndef OPENGLGRID_H
#define OPENGLGRID_H

#include <QWidget>
#include <iomanip>
#include <QOpenGLWidget>
#include "fontprovider.h"
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QGenericMatrix>
#include <QtGlobal>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QThread>
#include <QElapsedTimer>
#include <qmath.h>

#include <Eigen/Dense>
//const unsigned int SCR_WIDTH_OFFSET = 100;
//const unsigned int SCR_HEIGHT_OFFSET = 100;

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 780;

#define MAX_SIG_SLIDER 100
#define PI 3.14159265

class OpenglGrid : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public slots:
    void OnFinishedChildThread();
    void onNValChanged(int);
    void onSigValChanged(float);

signals:
    void setSigEnabled(bool enabled);
    void setNRangeVal(int min, int max, int val);
    void setSigValue(float val);

public:
    OpenglGrid(QWidget *parent = 0);
    ~OpenglGrid();

    QMatrix4x4&		getOrthoProjectionMatrix();

    void			setOrthoProjectionMatrix();



    void			paintGLHelperForAxisLabelsRendering();
    void            paintGLHelperForGridRendering();
    void            prepareGLForGridRendering();

    void            setCoordMatrix();
    QMatrix4x4      &getCoordMatrix();
    void            prepareMNK();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private:
    enum State {
        StartState,
        LKMAddDeapprox,
        LKMModifyDeapprox,
        SelectedDeapprox,
        ApproxedState,
        SelectApprox,
        LKMModifyApprox,
    };

    void                            addApproxPoint(const QVector2D &point);
    void                            genMirror(int count, float angle, float len);
    void                            modifyApproxPoint(const QVector2D &point);
    int                             checkPointSelected(const QVector2D &);
    void                            setPointSelection(int newIndex);

    void                			initShaders();
    void                            createBuffers();
    void                            prepareGridPositions();
    void                            initPointsBuffer();

    QRect viewport;

    QMatrix4x4						orthoProjection;
    QMatrix4x4						mirrorModelMatrix;

    QMatrix4x4						coordMatrix;
    QMatrix4x4						invCoordMatrix;

    QVector2D						screenDimension;

    QOpenGLShaderProgram			programFont;
    QOpenGLShaderProgram			programGrid;
    QOpenGLShaderProgram			programPoints;

    QThread							threadCons;

    FontProvider					*fpObj;
    QElapsedTimer       			frameTimeForFontLoad;
    bool							checkFirstFrameTimeElapsed;
    bool							finishedLoadingBoldChars;

    QOpenGLBuffer                   gridBuffer;
    QOpenGLVertexArrayObject        gridVao;
    QVector2D                       gridOffset;
    QVector2D                       gridStep; //100px = 10rem
    QVector<QVector2D>              verticesGrid;
    int                             y_tiks;
    int                             x_tiks;
    int                             gridCoordStep;

    QOpenGLBuffer                   approxBuffer;
    QOpenGLVertexArrayObject        ApproxVao;
    QVector<QVector2D>              verticesApprox;
    QVector<float>                  sigmaApprox;

    QOpenGLBuffer                   sPointsBuffer;
    QOpenGLVertexArrayObject        sPointsVao;
//    QVector<int>                    selectedPointsMask;
    int                             pointSelectIndex;

    QOpenGLBuffer                   approxeDBuffer;
    QOpenGLVertexArrayObject        ApproxeDVao;
    Eigen::Matrix<long double,
                  Eigen::Dynamic,
                  Eigen::Dynamic>   S;
    int                             approx_count;
    int                             fractals;
    int                             realFractals;
    bool                            startApprox;
    bool                            finishedCreatingBuffers;

    int                             moveAccum;
    State                           state;
    QVector2D                       pressedPos;


    void                            XAxisLabelsRendering();
    void                            PointsLabelsRendering();

    void                            paintApproxLineRendering();
    void                            prepareApproxeDRendering();
    void                            paintApproxeDRendering();
    void                            paintApproxPointsRendering();
    bool                            resised; // надо для перерисовки сетки
};
#endif // OPENGLGRID_H
