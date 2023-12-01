#include "algorithms.h"
#include <Eigen/Dense>

// Получение пересечения прямых, несущих отрезки
// и проверка пересечения отрезков.
// Возвращает: int:флаг; QVector2D:точка пересечения;
//             double: значение t; double: значение tau;
// -1  отрезки параллельны.
// 0 - прямые пересекаются, но отрезки нет.
// 1 - точка пересечения лежит на отрезке cd.
// 2 - точка пересечения лежит на отрезке ab.
// 3 - точка пересечения лежит на обоих отрезках.
const CrossResult cross_sem(
    const QPair<QVector2D, QVector2D> &ab,
    const QPair<QVector2D, QVector2D> &cd)
{
    QVector2D V = ab.second - ab.first;
    QVector2D CD = cd.first - cd.second;
    Eigen::Matrix2d M;
    M(0,0) = V.x();
    M(0,1) = V.y();
    M(1,0) = CD.x();
    M(1,1) = CD.y();
    CrossResult result;
    result.flag=-1;
    result.crossPoint=QVector2D();
    result.t=0;
    result.tau=0;
    if (M.determinant() == 0)
        return result;
    Eigen::Matrix2d MI = M.inverse();
    QVector2D CA = cd.first - ab.first;
    Eigen::RowVector2d t_tau = Eigen::RowVector2d(CA.x(), CA.y())*MI;
    result.crossPoint = ab.first + V*t_tau.x();
    result.flag = 0;
    if ((t_tau.y() <= 1.0 && t_tau.y() >=0))
        result.flag++;
    if (t_tau.x() <= 1.0 && t_tau.x() >=0)
        result.flag+=2;
    result.t = t_tau.x();
    result.tau = t_tau.y();
    return result;
}
