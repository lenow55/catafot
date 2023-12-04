#include "openglgrid.h"
#include <QOffscreenSurface>
#include <QMouseEvent>
#include "algorithms.h"

#define DEFAULT_POINT_SIZE          14.0
#define GRAPH_LINE_WIDTH            3.0

void OpenglGrid::OnFinishedChildThread()
{
    finishedLoadingBoldChars = true;
    finishedCreatingBuffers = true;
    update();
}

void OpenglGrid::onAngleChanged(int value)
{
    qDebug() << "onAngleChanged: " << state;
    update();
}

void OpenglGrid::onRotateChanged(int value)
{
    qDebug() << "onRotateChanged: " << state;
    update();
}

void OpenglGrid::onCountMirrorsChanged(int value)
{
    countMirrors = value;
    qDebug() << "onCountMirrorsChanged: " << state;
    update();
}

void OpenglGrid::onLenChanged(int value)
{
    qDebug() << "onLenChanged: " << state;
    update();
}

OpenglGrid::OpenglGrid(QWidget *parent)
    : QOpenGLWidget(parent), x_tiks(0), y_tiks(0), gridCoordStep(10)
{
    gridOffset.setX(50.0f);
    gridOffset.setY(50.0f);
    gridStep.setX(100.0f);
    gridStep.setY(100.0f);

    checkFirstFrameTimeElapsed = false;
    finishedLoadingBoldChars = false;
    finishedCreatingBuffers = false;

    state = State::StartState;
    countMirrors = 1;
}

OpenglGrid::~OpenglGrid()
{
    if (fpObj->isRunning())
    {
        fpObj->quit();
        delete fpObj;
    }
}

QMatrix4x4 &OpenglGrid::getOrthoProjectionMatrix()
{
    return orthoProjection;
}

QMatrix4x4 &OpenglGrid::getCoordMatrix()
{
    return coordMatrix;
}

void OpenglGrid::setOrthoProjectionMatrix()
{
    orthoProjection.setToIdentity();
    orthoProjection.ortho(
        0.0f, static_cast<float>(screenDimension.x()),
        0.0f, static_cast<float>(screenDimension.y()),
        -5.0f, 5.0f);
}

void OpenglGrid::setCoordMatrix()
{
    coordMatrix = orthoProjection;
    coordMatrix.translate(gridOffset);
    invCoordMatrix = coordMatrix.inverted();
}

void OpenglGrid::initShaders()
{
    if (!programFont.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshaderFont30.glsl"))
        close();
    if (!programFont.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshaderFont30.glsl"))
        close();
    if (!programFont.link())
        close();
    //Font shaders
    if (!programGrid.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader_grid.glsl"))
        close();
    if (!programGrid.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader_grid.glsl"))
        close();
    if (!programGrid.link())
        close();
}

void OpenglGrid::createBuffers()
{
    // Создаем и загружаем данные в буфер вершин
    gridBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    gridBuffer.create();

    gridVao.create();
    gridVao.bind();

    gridBuffer.bind();
    gridBuffer.allocate(100 * sizeof(QVector2D));

    // Создаем и настраиваем VAO
    int gridVertexLocation = programGrid.attributeLocation("vertex");
    programGrid.enableAttributeArray(gridVertexLocation);
    programGrid.setAttributeBuffer(gridVertexLocation, GL_FLOAT, 0, 2);

    gridBuffer.release();
    gridVao.release();

    // буфер зеркала
    MirrorBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    MirrorBuffer.create();

    MirrorVao.create();
    MirrorVao.bind();

    MirrorBuffer.bind();
    MirrorBuffer.allocate(50 * sizeof(QVector2D));
    int mirrorVertexLocation = programGrid.attributeLocation("vertex");
    programGrid.enableAttributeArray(mirrorVertexLocation);
    programGrid.setAttributeBuffer(mirrorVertexLocation, GL_FLOAT, 0, 2);

    MirrorBuffer.release();
    MirrorVao.release();

    // буфер луча
    RayBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    RayBuffer.create();

    RayVao.create();
    RayVao.bind();

    RayBuffer.bind();
    RayBuffer.allocate(100 * sizeof(QVector2D));
    int rayVertexLocation = programGrid.attributeLocation("vertex");
    programGrid.enableAttributeArray(rayVertexLocation);
    programGrid.setAttributeBuffer(rayVertexLocation, GL_FLOAT, 0, 2);

    RayBuffer.release();
    RayVao.release();
}

void OpenglGrid::addRayPoint(const QVector2D &point)
{
    RayVao.bind();
    RayBuffer.bind();
    // добавляем точку в хранилище

    QVector<QVector2D> vVector;
    vVector.append(point);

    RayBuffer.write(
        verticesRay.size() * sizeof(QVector2D),
        vVector.data(),
        1 * sizeof(QVector2D)
        );

    verticesRay.append(point);

//    QVector<float> check;
//    check.resize(30);
//    approxBuffer.read(
//        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
//    qDebug() << "content Buf: " << check;

    RayBuffer.release();
    RayVao.release();
}

void OpenglGrid::genMirror(int count, float angle, float len)
{
    verticesMirror.append(QVector2D(10.0,10.0));
    bool side = true;
    float angle_big = angle * PI / 180.0;
    float angle_smal = (180.0f - angle) * PI / 180.0;
    for(int i = 1; i <= count*2; i++)
    {
        if(side)
            verticesMirror.append(
                QVector2D(
                    len*cos(angle_big)+verticesMirror.at(i-1).x(),
                    len*sin(angle_big)+verticesMirror.at(i-1).y()
                    )
                );
        else
            verticesMirror.append(
                QVector2D(
                    len*cos(angle_smal)+verticesMirror.at(i-1).x(),
                    len*sin(angle_smal)+verticesMirror.at(i-1).y()
                    )
                );
        side = side?false:true;
    }

    // qDebug() << verticesMirror;

    MirrorVao.bind();
    MirrorBuffer.bind();
    // добавляем точку в хранилище

    MirrorBuffer.write(
        0,
        verticesMirror.data(),
        verticesMirror.size() * sizeof(QVector2D)
        );

    //    QVector<float> check;
    //    check.resize(30);
    //    approxBuffer.read(
    //        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
    //    qDebug() << "content Buf: " << check;

    MirrorBuffer.release();
    MirrorVao.release();
}

void OpenglGrid::modifyRayPoint(const QVector2D &point)
{
    RayVao.bind();
    RayBuffer.bind();

    QVector<float> floatVector = {point.x(), point.y()};

    RayBuffer.write(
        pointSelectIndex * sizeof(QVector2D),
        floatVector.data(),
        1 * sizeof(QVector2D)
        );

    verticesRay.replace(pointSelectIndex, point);

    //    QVector<float> check;
    //    check.resize(30);
    //    approxBuffer.read(
    //        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
    //    qDebug() << "content Buf: " << check;

    RayBuffer.release();
    RayVao.release();
}

void OpenglGrid::prepareRayRendering()
{
    if(verticesRay.size() < 2)
        return;

    //флаг поиска отражения, повторяем цикл пока не
    //столкнёмся с отсутствием пересечения
    bool findingReflection = true;
    int startIndex = 2;

    // Удаление элементов с определенного индекса до конца
    verticesRay.erase(verticesRay.begin() + startIndex, verticesRay.end());

    int countMirrors = verticesMirror.size()-1;
    CrossResult crossResult;
    CrossResult nearestCrossResult;
    int mirrorIndex = -1;
    int lastReflectedMirror = -1;
    bool itFirstReflection = true;
    QPair<QVector2D,QVector2D> ray(
        verticesRay.at(0),
        verticesRay.at(1));
    QVector2D lastRay(ray.second-ray.first);
    while(findingReflection)
    {
        nearestCrossResult.flag = -1;
        nearestCrossResult.tau=10000;
        nearestCrossResult.t=10000;
        mirrorIndex = -1;
        for(int i = 0; i < countMirrors; i++)
        {
            // если это зеркало было на пред итерации, то пропускаем его
            if(lastReflectedMirror == i)
                continue;
            crossResult = cross_sem(
                QPair<QVector2D,QVector2D>(
                    verticesMirror.at(i),verticesMirror.at(i+1)),
                    ray
                );
            if(crossResult.flag > 1 && crossResult.tau < nearestCrossResult.tau && crossResult.tau>0)
            {
                nearestCrossResult=crossResult;
                mirrorIndex=i;
            }
        }
        if(nearestCrossResult.flag < 2)
        {
            lastRay.normalize();
            if(itFirstReflection)
                verticesRay.append(lastRay*2000+verticesRay.at(0));
            else
                verticesRay.append(lastRay*2000+nearestCrossResult.crossPoint);
            findingReflection=false;
        }
        else
        {
            if(itFirstReflection && nearestCrossResult.tau < 1)
                break;
            else
                itFirstReflection=false;
            lastReflectedMirror=mirrorIndex;
            verticesRay.append(nearestCrossResult.crossPoint);
            QVector3D V(
                verticesMirror.at(mirrorIndex+1)-verticesMirror.at(mirrorIndex),
                0);
            QVector3D rayV(lastRay,0);
            QVector3D W(0,0,1);
            QVector3D n_vec = normalVector(V,W);
            QVector3D M = mirrorVector(rayV, n_vec);
            QVector2D M2d(M);
            ray.first=nearestCrossResult.crossPoint;
            ray.second=nearestCrossResult.crossPoint+M2d;
            lastRay=M2d;
        }
    }

    // qDebug() << "V: " << verticesRay;
    RayVao.bind();
    RayBuffer.bind();

    RayBuffer.write(
        0,
        verticesRay.data(),
        verticesRay.size() * sizeof(QVector2D)
        );

    //    QVector<float> check;
    //    check.resize(30);
    //    approxBuffer.read(
    //        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
    //    qDebug() << "content Buf: " << check;

    RayBuffer.release();
    RayVao.release();
}

void OpenglGrid::paintRayRendering()
{
    QVector4D color(1.0f, 0.19f, 0.19f, 1.0f);

    programGrid.bind();
    RayVao.bind();
    int matrixLocation = programGrid.uniformLocation("matrix");
    int colorLocation = programGrid.uniformLocation("color");
    int depthLocation = programGrid.uniformLocation("depth");
    programGrid.setUniformValue(matrixLocation, getCoordMatrix());
    programGrid.setUniformValue(colorLocation, color);
    programGrid.setUniformValue(depthLocation, 1.0f);

    glLineWidth(GRAPH_LINE_WIDTH);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    if (verticesRay.size() > 1)
    {
        glDrawArrays(GL_LINE_STRIP, 0, verticesRay.size());
    }

    glDisable(GL_LINE_SMOOTH);

    glDisable(GL_DEPTH_TEST);
    glPointSize(DEFAULT_POINT_SIZE);
    if (verticesRay.size() < 2)
        glDrawArrays(GL_POINTS, 0, verticesRay.size());
    else
        glDrawArrays(GL_POINTS, 0, 2);
    glEnable(GL_DEPTH_TEST);

    RayVao.release();
    programGrid.release();
}

void OpenglGrid::paintMirrorRendering()
{
    QVector4D color(0.27f, 0.33f, 1.0f, 1.0f);

    programGrid.bind();
    MirrorVao.bind();
    int matrixLocation = programGrid.uniformLocation("matrix");
    int colorLocation = programGrid.uniformLocation("color");
    int depthLocation = programGrid.uniformLocation("depth");
    programGrid.setUniformValue(matrixLocation, getCoordMatrix());
    programGrid.setUniformValue(colorLocation, color);
    programGrid.setUniformValue(depthLocation, 0.0f);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

    if (verticesMirror.size() > 1)
    {
        glLineWidth(GRAPH_LINE_WIDTH);
        glDrawArrays(GL_LINE_STRIP, 0, verticesMirror.size());
    }
    glDisable(GL_LINE_SMOOTH);



    MirrorVao.release();
    programGrid.release();
}

void OpenglGrid::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(1, 1, 1, 1.0f);

    initShaders();
//    initApproxBuffer();

    frameTimeForFontLoad.start();

    QOpenGLContext *current = context();
    doneCurrent();

    fpObj = new FontProvider();

    // the background thread's context is shared from current
    QOpenGLContext *shared = fpObj->context;
    shared->setFormat(current->format());
    shared->setShareContext(current);
    shared->create();

    // must move the shared context to the background thread
    shared->moveToThread(fpObj);

    // setup the background thread's surface
    // must be created here in the main thread
    QOffscreenSurface *surface = fpObj->surface;
    surface->setFormat(shared->format());
    surface->create();

    // worker signal
    connect(fpObj, SIGNAL(started()), fpObj, SLOT(initializeFontProvider()));
    connect(fpObj, SIGNAL(finished_load()), this, SLOT(OnFinishedChildThread()));

    // must move the thread to itself
    fpObj->moveToThread(fpObj);

    // the worker can finally start
    fpObj->start();
}

void OpenglGrid::resizeGL(int w, int h)
{
    viewport = rect();
    screenDimension.setX(w);
    screenDimension.setY(h);
    setOrthoProjectionMatrix();
    setCoordMatrix();
    glViewport(
        0,
        0,
        w,
        h
        );

    prepareGridPositions();
    resised = true;
}

void OpenglGrid::paintGL()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!checkFirstFrameTimeElapsed)
    {
        int loadTime = frameTimeForFontLoad.elapsed();
        checkFirstFrameTimeElapsed = true;
        qDebug() << "Total load Time for First painGL call" << loadTime << "\n";
        createBuffers();
        genMirror(5, 5.0f, 300.0f);
    }
    if (resised)
    {
        prepareGLForGridRendering();
        resised = false;
    }


    paintGLHelperForGridRendering();
    paintMirrorRendering();
    if (state == State::RayPlaced ||
        state == State::LKMModifyRay
        )
        paintRayRendering();

    paintRayRendering();
    if(finishedLoadingBoldChars)
    {
        XAxisLabelsRendering();
    }
//    qDebug() << state;
}

void OpenglGrid::mouseMoveEvent(QMouseEvent *event)
{
    if (
        state == State::LKMModifyDeRay ||
        state == State::LKMModifyRay
        )
    {
        double x = double(
            event->x());
        double y = double(
            (viewport.height() - event->y()));
        pressedPos = QVector2D(x,y);

        pressedPos = pressedPos - gridOffset;
//        qDebug() << "movedPos"
//                 << pressedPos;

        modifyRayPoint(pressedPos);
        if (state == State::LKMModifyRay)
        {
            // prepareMNK();
            prepareRayRendering();
        }
        update();
    }
}

void OpenglGrid::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        double x = double(
            event->x());
        double y = double(
            (viewport.height() - event->y()));
        pressedPos = QVector2D(x,y);

        pressedPos = pressedPos - gridOffset;
        qDebug() << "plot_data->inverse_data_matrix * position"
                 << pressedPos;
        if (
            state == State::StartState)
        {
            int newSelectIndex = checkPointSelected(pressedPos,2);
            if (newSelectIndex == -1)
            {
                addRayPoint(pressedPos);
                if (verticesRay.size() < 2)
                {
                    state = State::LKMAddDeRay;
                }
                else
                {
                    pointSelectIndex = 1;
                    prepareRayRendering();
                    state = State::LKMModifyRay;
                }
                update();
            }
            else
            {
                pointSelectIndex = newSelectIndex;
                state = State::LKMModifyDeRay;
            }
        }
        if (
            state == State::RayPlaced)
        {
            int newSelectIndex = checkPointSelected(pressedPos,2);
            if (newSelectIndex != -1)
            {
                pointSelectIndex = newSelectIndex;
                state = State::LKMModifyRay;
            }
        }
    }
    else if (event->button() == Qt::MiddleButton)
    {
        if (
            state == State::StartState ||
            state == State::RayPlaced
            )
        {
            verticesRay.clear();
            pointSelectIndex = -1;
            state = State::StartState;
            update();
        }
    }
    QOpenGLWidget::mousePressEvent(event);
}

void OpenglGrid::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (
            state == State::LKMAddDeRay ||
            state == State::LKMModifyDeRay
            )
            state = State::StartState;
        if (
            state == State::LKMModifyRay
            )
            state = State::RayPlaced;
        pointSelectIndex = -1;
    }

    QOpenGLWidget::mouseReleaseEvent(event);
}

//Вовращает индекс точки, которую выделили, если не вделили,
//то -1 ставит
//есть ограничение на количество точек, по которому смотреть
int OpenglGrid::checkPointSelected(const QVector2D &position, int stop)
{
    QVector<QVector2D>::iterator itBegin = verticesRay.begin();
    QVector<QVector2D>::iterator itEnd = verticesRay.end();
    int stoper=verticesRay.size();
    if(stop > 0)
        stoper = stop;

    int newSelectionIndex = -1;
    // Цикл с использованием итераторов
    for (QVector<QVector2D>::iterator it = itBegin; it != itEnd; ++it)
    {
        if(stoper == 0)
            break;
        if(abs(it->x()-position.x()) < DEFAULT_POINT_SIZE/2 &&
            abs(it->y()-position.y()) < DEFAULT_POINT_SIZE/2)
            newSelectionIndex = std::distance(itBegin, it);
        stoper--;
    }
    return newSelectionIndex;
}
