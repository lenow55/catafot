#include "openglgrid.h"

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
