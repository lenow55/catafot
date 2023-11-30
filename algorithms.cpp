#include "algorithms.h"
#include <Eigen/Dense>

// Получение пересечения прямых, несущих отрезки
// и проверка пересечения отрезков.
// Возвращает:
// -1 отрезки параллельны.
// 0, точка - прямые пересекаются, но отрезки нет.
// 1, точка - точка пересечения лежит на отрезке cd.
// 2, точка - точка пересечения лежит на отрезке ab.
// 3, точка - точка пересечения лежит на обоих отрезках.
QPair<int,QVector2D> cross_sem(
    const QPair<QVector2D, QVector2D> &ab,
    const QPair<QVector2D, QVector2D> &cd)
{
    QVector2D V = ab.second - ab.first;
    QVector2D CD = cd.first - cd.second;
    Eigen::Matrix2d M;
    if (M.determinant() == 0)
        return QPair<int,QVector2D>(-1, QVector2D());
    M(0,0) = V.x();
    M(0,1) = V.y();
    M(1,0) = CD.x();
    M(1,1) = CD.y();
    M = M.inverse();
    QVector2D CA = cd.first - ab.first;
    Eigen::Vector2d t_tau = Eigen::Vector2d(CA.x(), CA.y())*M;
    QVector2D q_point = ab.first + V*t_tau.x();
    int flag = 0;
    if ((t_tau.y() <= 1.0 && t_tau.y() >=0))
        flag++;
    if (t_tau.x() <= 1.0 && t_tau.x() >=0)
        flag+=2;
    return QPair<int,QVector2D>(flag, q_point);
}
