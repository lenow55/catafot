#include "openglgrid.h"
#include <QOffscreenSurface>
#include <QMouseEvent>
#include "algorithms.h"

#define DEFAULT_POINT_SIZE          14.0
#define GRAPH_LINE_WIDTH            4.0

void OpenglGrid::OnFinishedChildThread()
{
    finishedLoadingBoldChars = true;
    finishedCreatingBuffers = true;
    update();
}

void OpenglGrid::onNValChanged(int value)
{
    if (state == State::StartState ||
        state == State::SelectedDeapprox)
        approx_count = value;
    if (state == State::ApproxedState ||
        state == State::SelectApprox)
    {
        approx_count = value;
        prepareMNK();
        prepareApproxeDRendering();
        update();
    }
}

void OpenglGrid::onSigValChanged(float value)
{
    if (state == State::SelectedDeapprox)
        if (pointSelectIndex != -1)
            sigmaApprox.replace(pointSelectIndex, value);
    if (state == State::SelectApprox)
    {
        if (pointSelectIndex != -1)
            sigmaApprox.replace(pointSelectIndex, value);
        prepareMNK();
        prepareApproxeDRendering();
    }
    update();
}

OpenglGrid::OpenglGrid(QWidget *parent)
    : QOpenGLWidget(parent), x_tiks(0), y_tiks(0), gridCoordStep(10)
{
    //  setAttribute(Qt::WA_TranslucentBackground); // делает виджет прозрачным прям
//    setFixedSize(SCR_WIDTH, SCR_HEIGHT);

//    screenDimension.setX(SCR_WIDTH);
//    screenDimension.setY(SCR_HEIGHT);
    gridOffset.setX(50.0f);
    gridOffset.setY(50.0f);
    gridStep.setX(100.0f);
    gridStep.setY(100.0f);

    checkFirstFrameTimeElapsed = false;
    finishedLoadingBoldChars = false;
    finishedCreatingBuffers = false;

    fractals = 200;
    realFractals = fractals;
    moveAccum = 0;
    state = State::StartState;
    pointSelectIndex = -1;
    approx_count = 1;
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

    // if (!programPoints.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader_points.glsl"))
    //     close();
    // //    if (!programPoints.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/gshader_points.glsl"))
    // //        close();
    // if (!programPoints.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader_points.glsl"))
    //     close();
    // if (!programPoints.link())
    //     close();
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

    // буфер входной линии
    approxBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    approxBuffer.create();

    ApproxVao.create();
    ApproxVao.bind();

    approxBuffer.bind();
    approxBuffer.allocate(50 * sizeof(QVector2D));
    int vertexLocation = programGrid.attributeLocation("vertex");
    programGrid.enableAttributeArray(vertexLocation);
    programGrid.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2);

    approxBuffer.release();
    ApproxVao.release();

    // буфер маски выделенных точек
    // sPointsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    // sPointsBuffer.create();

    // sPointsVao.create();
    // sPointsVao.bind();

    // sPointsBuffer.bind();
    // sPointsBuffer.allocate(50 * sizeof(QVector3D));
    // int colorLocation = programPoints.attributeLocation("color");
    // programPoints.enableAttributeArray(colorLocation);
    // programPoints.setAttributeBuffer(colorLocation, GL_FLOAT, 0, 3);

    // approxBuffer.bind();
    // vertexLocation = programPoints.attributeLocation("vertex");
    // programPoints.enableAttributeArray(vertexLocation);
    // programPoints.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2);

    // sPointsBuffer.release();
    // approxBuffer.release();
    // sPointsVao.release();
//    sPointsVao.release();

    // буфер выходной аппроксимированной линии
    approxeDBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    approxeDBuffer.create();

    ApproxeDVao.create();
    ApproxeDVao.bind();

    approxeDBuffer.bind();
    approxeDBuffer.allocate(2 * fractals * sizeof(Eigen::Vector2d));
    vertexLocation = programGrid.attributeLocation("vertex");
    programGrid.enableAttributeArray(vertexLocation);
    programGrid.setAttributeBuffer(vertexLocation, GL_DOUBLE, 0, 2);

    approxBuffer.release();
    ApproxeDVao.release();
}

void OpenglGrid::XAxisLabelsRendering()
{
    QVector3D fontColor(1.0f, 0.3f, 0.3f);
    programFont.bind();
    programFont.setUniformValue("mvp_matrix", getOrthoProjectionMatrix());
    programFont.setUniformValue("textColor", fontColor);
    programFont.setUniformValue("text", 0);
    for(int count = 0; count <= x_tiks; count++)
    {
        QString x_label = QString::number(count*gridCoordStep);
        fpObj->drawFontGeometry(
            &programFont,
            gridStep.x()*count+gridOffset.x(), 25.0f,
            x_label, 0.6f);
    }
    for(int count = 0; count <= y_tiks; count++)
    {
        QString x_label = QString::number(count*gridCoordStep);
        fpObj->drawFontLeftGeometry(
            &programFont,
            30.0f,
            gridStep.y()*count+gridOffset.y(),
            x_label, 0.6f);
    }
    programFont.release();
}

void OpenglGrid::PointsLabelsRendering()
{
    qDebug() << "PointsLabelsRendering";
}

void OpenglGrid::addApproxPoint(const QVector2D &point)
{
    ApproxVao.bind();
    approxBuffer.bind();
    // добавляем точку в хранилище

    QVector<QVector2D> vVector;
    vVector.append(point);

    approxBuffer.write(
        verticesApprox.size() * sizeof(QVector2D),
        vVector.data(),
        1 * sizeof(QVector2D)
        );

    verticesApprox.append(point);

    //устанавливаем начальное значение аномальности для точки
    sigmaApprox.append(0.00f);

//    QVector<float> check;
//    check.resize(30);
//    approxBuffer.read(
//        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
//    qDebug() << "content Buf: " << check;

    approxBuffer.release();
    ApproxVao.release();
}

void OpenglGrid::genMirror(int count, float angle, float len)
{
    verticesApprox.append(QVector2D(10.0,10.0));
    bool side = true;
    float angle_big = angle * PI / 180.0;
    float angle_smal = (180.0f - angle) * PI / 180.0;
    for(int i = 1; i <= count*2; i++)
    {
        if(side)
            verticesApprox.append(
                QVector2D(
                    len*cos(angle_big)+verticesApprox.at(i-1).x(),
                    len*sin(angle_big)+verticesApprox.at(i-1).y()
                    )
                );
        else
            verticesApprox.append(
                QVector2D(
                    len*cos(angle_smal)+verticesApprox.at(i-1).x(),
                    len*sin(angle_smal)+verticesApprox.at(i-1).y()
                    )
                );
        side = side?false:true;
    }

    qDebug() << verticesApprox;

    QPair<QVector2D,QVector2D> ray(QVector2D(300,100),QVector2D(400,100));
    CrossResult result;
    result = cross_sem(
        QPair<QVector2D,QVector2D>(
            verticesApprox.at(2),verticesApprox.at(3)),
            ray
        );

    QVector3D V(verticesApprox.at(3)-verticesApprox.at(2),0);
    QVector3D W(0,0,1);
    QVector3D n_vec = normalVector(V,W);

    QVector3D rayV = ray.second - ray.first;

    QVector3D M = mirrorVector(rayV, n_vec);
    qDebug() << "V: " << V << "\n" << "M: " << M;

    ApproxVao.bind();
    approxBuffer.bind();
    // добавляем точку в хранилище

    approxBuffer.write(
        0,
        verticesApprox.data(),
        verticesApprox.size() * sizeof(QVector2D)
        );

    //устанавливаем начальное значение аномальности для точки
    // sigmaApprox.append(0.00f);

    //    QVector<float> check;
    //    check.resize(30);
    //    approxBuffer.read(
    //        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
    //    qDebug() << "content Buf: " << check;

    approxBuffer.release();
    ApproxVao.release();
}

void OpenglGrid::modifyApproxPoint(const QVector2D &point)
{
    ApproxVao.bind();
    approxBuffer.bind();

    QVector<float> floatVector = {point.x(), point.y()};

    approxBuffer.write(
        pointSelectIndex * sizeof(QVector2D),
        floatVector.data(),
        1 * sizeof(QVector2D)
        );

    verticesApprox.replace(pointSelectIndex, point);

    //    QVector<float> check;
    //    check.resize(30);
    //    approxBuffer.read(
    //        0, check.data() ,verticesApprox.size()*sizeof(QVector2D));
    //    qDebug() << "content Buf: " << check;

    approxBuffer.release();
    ApproxVao.release();
}

void OpenglGrid::setPointSelection(int newIndex)
{
    qDebug() << "setPointSelection";
}

void OpenglGrid::prepareApproxeDRendering()
{
    qDebug() << "prepareApproxeDRendering";
}

void OpenglGrid::paintApproxeDRendering()
{
    qDebug() << "paintApproxeDRendering";
}

void OpenglGrid::paintApproxLineRendering()
{
    QVector4D color(0.27f, 0.33f, 1.0f, 1.0f);

    programGrid.bind();
    ApproxVao.bind();
    int matrixLocation = programGrid.uniformLocation("matrix");
    int colorLocation = programGrid.uniformLocation("color");
    int depthLocation = programGrid.uniformLocation("depth");
    programGrid.setUniformValue(matrixLocation, getCoordMatrix());
    programGrid.setUniformValue(colorLocation, color);
    programGrid.setUniformValue(depthLocation, 0.0f);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

    if (verticesApprox.size() > 1)
    {
        glLineWidth(GRAPH_LINE_WIDTH);
        glDrawArrays(GL_LINE_STRIP, 0, verticesApprox.size());
    }
    glDisable(GL_LINE_SMOOTH);


    ApproxVao.release();
    programGrid.release();
}

void OpenglGrid::paintApproxPointsRendering()
{
    qDebug() << "paintApproxPointsRendering ";
}

void OpenglGrid::paintGLHelperForAxisLabelsRendering()
{
    QVector3D fontColor(1.0f, 0.3f, 0.3f);
    programFont.bind();
    programFont.setUniformValue("mvp_matrix", getOrthoProjectionMatrix());
    programFont.setUniformValue("textColor", fontColor);
    programFont.setUniformValue("text", 0);

    QString stringToDisplay = QString::fromUtf8("0");
    fpObj->drawFontGeometry(&programFont,50.0f, 25.0f, stringToDisplay, 0.6f);
    fpObj->drawFontGeometry(&programFont,30.0f, 50.0f, stringToDisplay, 0.6f);
    programFont.release();
}

void OpenglGrid::paintGLHelperForGridRendering()
{
    QVector4D color(0.35f, 0.35f, 0.35f, 1.0f);

    programGrid.bind();
    gridVao.bind();
    int matrixLocation = programGrid.uniformLocation("matrix");
    int colorLocation = programGrid.uniformLocation("color");
    int depthLocation = programGrid.uniformLocation("depth");
    programGrid.setUniformValue(matrixLocation, getOrthoProjectionMatrix());
    programGrid.setUniformValue(colorLocation, color);
    programGrid.setUniformValue(depthLocation, 0.0f);


    glLineWidth(2.0);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(5, 0x9999);
    glDrawArrays(GL_LINES, 0, verticesGrid.size());
    glDisable(GL_LINE_STIPPLE);

    gridVao.release();
    programGrid.release();
}

void OpenglGrid::prepareGLForGridRendering()
{
    gridVao.bind();
    gridBuffer.bind();

    gridBuffer.write(0, verticesGrid.constData(), verticesGrid.size() * sizeof(QVector2D));

//    QVector<float> check;
//    check.resize(300);
//    gridBuffer.read(
//        0, check.data() ,verticesGrid.size()*sizeof(QVector2D));
//    qDebug() << "content Buf: " << check;
    gridBuffer.release();
    gridVao.release();
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
        genMirror(5, 45.0f, 50.0f);
    }
    if (resised)
    {
        prepareGLForGridRendering();
        resised = false;
    }


    paintGLHelperForGridRendering();
    paintApproxLineRendering();
    if (state == State::SelectApprox ||
        state == State::ApproxedState ||
        state == State::LKMModifyApprox
        )
        paintApproxeDRendering();

    // paintApproxPointsRendering();
    if(finishedLoadingBoldChars)
    {
        XAxisLabelsRendering();
        // PointsLabelsRendering();
    }
//    qDebug() << state;
}

void OpenglGrid::mouseMoveEvent(QMouseEvent *event)
{
//     if (
//         state == State::LKMModifyDeapprox ||
//         state == State::LKMModifyApprox
//         )
//     {
//         double x = double(
//             event->x());
//         double y = double(
//             (viewport.height() - event->y()));
//         pressedPos = QVector2D(x,y);

//         pressedPos = pressedPos - gridOffset;
// //        qDebug() << "movedPos"
// //                 << pressedPos;

//         modifyApproxPoint(pressedPos);
//         if (state == State::LKMModifyApprox)
//         {
//             prepareMNK();
//             prepareApproxeDRendering();
//         }
//         update();
//     }
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
    }
}

void OpenglGrid::mouseReleaseEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseReleaseEvent(event);
}

//Вовращает индекс точки, которую выделили, если не вделили,
//то -1 ставит
int OpenglGrid::checkPointSelected(const QVector2D &position)
{
    QVector<QVector2D>::iterator itBegin = verticesApprox.begin();
    QVector<QVector2D>::iterator itEnd = verticesApprox.end();

    int newSelectionIndex = -1;
    // Цикл с использованием итераторов
    for (QVector<QVector2D>::iterator it = itBegin; it != itEnd; ++it)
    {
        if(abs(it->x()-position.x()) < DEFAULT_POINT_SIZE/2 &&
            abs(it->y()-position.y()) < DEFAULT_POINT_SIZE/2)
            newSelectionIndex = std::distance(itBegin, it);
    }

    return newSelectionIndex;
}

void OpenglGrid::prepareGridPositions()
{
    verticesGrid.clear();
    y_tiks = screenDimension.y() / static_cast<int>(gridStep.y());
    for(int i = 0; i <= y_tiks; i++){
        verticesGrid
            << QVector2D(
                   gridOffset.x(),
                   gridOffset.y()+gridStep.y()*i)
            << QVector2D(
                   screenDimension.x(),
                   gridOffset.y()+gridStep.y()*i);
    }

    x_tiks = screenDimension.x() / static_cast<int>(gridStep.x());
    for(int i = 0; i <= x_tiks; i++){
        verticesGrid
            << QVector2D(
                   gridOffset.x()+gridStep.x()*i,
                   gridOffset.y())
            << QVector2D(
                   gridOffset.x()+gridStep.x()*i,
                   screenDimension.y());
    }
}

void OpenglGrid::prepareMNK()
{
    qDebug() << "prepareMNK";
}
