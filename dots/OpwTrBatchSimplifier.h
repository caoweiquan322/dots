/* Copyright © 2015 DynamicFatty. All Rights Reserved. */
#ifndef OPWTRBATCHSIMPLIFIER_H
#define OPWTRBATCHSIMPLIFIER_H

#include <QObject>

class OpwTrBatchSimplifier : public QObject
{
    Q_OBJECT
public:
    explicit OpwTrBatchSimplifier(QObject *parent = 0);

    static void simplify(const QVector<double> x, const QVector<double> y, const QVector<double> t,
                         QVector<double> &ox, QVector<double> &oy, QVector<double> &ot,
                         double thresh);

    static void simplifyByIndex(const QVector<double> x, const QVector<double> y, const QVector<double> t,
                                QVector<int> &outIndex,
                                double thresh);

protected:
    static void internalSimplify(const QVector<double> x, const QVector<double> y, const QVector<double> t,
                                 QVector<int> &outIndex, int startIndex, int endIndex,
                                 double threshQuad);

signals:

public slots:
};

#endif // OPWTRBATCHSIMPLIFIER_H
