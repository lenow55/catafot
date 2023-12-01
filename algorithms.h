#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QPair>
#include <QVector2D>

struct CrossResult {
    int flag;
    QVector2D crossPoint;
    double t;
    double tau;
};

const CrossResult cross_sem(
    const QPair<QVector2D, QVector2D> &ab,
    const QPair<QVector2D, QVector2D> &cd
    );

#endif // ALGORITHMS_H
