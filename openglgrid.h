#ifndef OPENGLGRID_H
#define OPENGLGRID_H

#include <QWidget>
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

#define PI 3.14159265

class OpenglGrid : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public slots:
    void OnFinishedChildThread();
    void onAngleChanged(int);
    void onRotateChanged(int);
    void onCountMirrorsChanged(int);
    void onLenChanged(int);

signals:

public:
    OpenglGrid(QWidget *parent = 0);
    ~OpenglGrid();

    QMatrix4x4&		getOrthoProjectionMatrix();
    void			setOrthoProjectionMatrix();

    void            setCoordMatrix();
    QMatrix4x4      &getCoordMatrix();

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
        LKMAddDeRay,
        LKMModifyDeRay,
        RayPlaced,
        LKMModifyRay,
    };
    // инициализации вспомогательных
    void                			initShaders();
    void                            createBuffers();
    void                            prepareGridPositions();
    void                            prepareGLForGridRendering();

    void                            prepareApproxeDRendering();

    void                            genMirror(int count, float angle, float len);

    void                            addRayPoint(const QVector2D &point);
    void                            modifyRayPoint(const QVector2D &point);
    int                             checkPointSelected(const QVector2D &, int stop=-1);

    void                            XAxisLabelsRendering();
    void                            paintGLHelperForGridRendering();

    void                            paintMirrorRendering();
    void                            paintRayRendering();

    QRect                           viewport;

    QMatrix4x4						orthoProjection;
    QMatrix4x4						mirrorModelMatrix;

    QMatrix4x4						coordMatrix;
    QMatrix4x4						invCoordMatrix;

    QVector2D						screenDimension;

    QOpenGLShaderProgram			programFont;
    QOpenGLShaderProgram			programGrid;

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
    bool                            resised; // надо для перерисовки сетки

    QOpenGLBuffer                   MirrorBuffer;
    QOpenGLVertexArrayObject        MirrorVao;
    QVector<QVector2D>              initVerticesMirror; // начальные позиции точек зеркала
    QVector<QVector2D>              verticesMirror; //повёрнутые и перенесённые позиции зеркала


    QOpenGLBuffer                   RayBuffer;
    QOpenGLVertexArrayObject        RayVao;
    QVector<QVector2D>              verticesRay;

    bool                            finishedCreatingBuffers;

    int                             countMirrors;

    QVector2D                       pressedPos;
    int                             pointSelectIndex;

    State                           state;
};
#endif // OPENGLGRID_H
