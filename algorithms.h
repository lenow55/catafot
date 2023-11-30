#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QPair>
#include <QVector2D>

QPair<int,QVector2D> cross_sem(
    const QPair<QVector2D, QVector2D> &ab,
    const QPair<QVector2D, QVector2D> &cd
    );

#endif // ALGORITHMS_H
