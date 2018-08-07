/* Copyright © 2015 DynamicFatty. All Rights Reserved. */
#ifndef OPWBATCHSIMPLIFIER_H
#define OPWBATCHSIMPLIFIER_H

#include <QObject>

class OpwBatchSimplifier : public QObject
{
    Q_OBJECT
public:
    explicit OpwBatchSimplifier(QObject *parent = 0);

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

#endif // OPWBATCHSIMPLIFIER_H
