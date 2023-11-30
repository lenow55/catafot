#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QPair>
#include <QVector2D>

int cross_sem(
    const QPair<QVector2D, QVector2D> &ab,
    const QPair<QVector2D, QVector2D> &cd,
    float &t,
    float &tau,
    QVector2D &q_point
    );

#endif // ALGORITHMS_H
