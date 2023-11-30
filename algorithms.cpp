#include "algorithms.h"
#include <Eigen/Dense>

int cross_sem(
    const QPair<QVector2D, QVector2D> &ab,
    const QPair<QVector2D, QVector2D> &cd,
    float &t, float &tau, QVector2D &q_point)
{
    QVector2D V = ab.second - ab.first;
    Eigen::Matrix2d M;
    return 0;
}
