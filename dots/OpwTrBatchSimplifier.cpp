/* Copyright © 2015 DynamicFatty. All Rights Reserved. */
#include "OpwTrBatchSimplifier.h"
#include "Helper.h"
#include "DotsException.h"
#include <QDebug>

OpwTrBatchSimplifier::OpwTrBatchSimplifier(QObject *parent) : QObject(parent)
{

}

void OpwTrBatchSimplifier::simplify(const QVector<double> x, const QVector<double> y, const QVector<double> t,
                                    QVector<double> &ox, QVector<double> &oy, QVector<double> &ot,
                                    double thresh)
{
    // Validates input parameters.
    Helper::checkIntEqual(x.count(), y.count());
    Helper::checkIntEqual(x.count(), t.count());
    Helper::checkPositive("thresh", thresh);

    QVector<int> simplifiedIndex;
    simplifyByIndex(x, y, t, simplifiedIndex, thresh);
    Helper::slice(x, simplifiedIndex, ox);
    Helper::slice(y, simplifiedIndex, oy);
    Helper::slice(t, simplifiedIndex, ot);
}

void OpwTrBatchSimplifier::simplifyByIndex(const QVector<double> x, const QVector<double> y, const QVector<double> t,
                                           QVector<int> &outIndex, double thresh)
{
    Helper::checkPositive("#points - 2", (double)(x.count()-2));
    outIndex.clear();
    internalSimplify(x, y, t, outIndex, 0, 2, thresh*thresh);
//    foreach (int idx, outIndex) {
//        qDebug("Out: %d", idx);
//    }
}

void OpwTrBatchSimplifier::internalSimplify(const QVector<double> x, const QVector<double> y,
                                            const QVector<double> t,
                                            QVector<int> &outIndex, int startIndex, int endIndex,
                                            double threshQuad)
{
    if (endIndex<=startIndex) {
        DotsException("EndIndex <= StartIndex is not expected.").raise();
    } else if (endIndex == startIndex+1 && endIndex >= x.count()) {
        outIndex << startIndex << endIndex;
//        qDebug()<<endIndex<<", "<<startIndex;
//        qDebug()<<endIndex<<", "<<endIndex;
        return;
    } else {
        int stopIndex = -1;
        double r = 0.0, syncX, syncY, d;
        while (endIndex < x.count()) {
            double t1 = t.at(startIndex);
            double t2 = t.at(endIndex);
            double x1 = x.at(startIndex);
            double x2 = x.at(endIndex);
            double y1 = y.at(startIndex);
            double y2 = y.at(endIndex);
            for (int k=startIndex+1; k<endIndex; ++k) {
                double tk = t.at(k);
                double xk = x.at(k);
                double yk = y.at(k);
                r = (tk-t1)/(t2-t1);
                syncX = x1 + (x2-x1)*r;
                syncY = y1 + (y2-y1)*r;
                d = (xk-syncX)*(xk-syncX)+(yk-syncY)*(yk-syncY);
                if (d>threshQuad) {
                    stopIndex = k;
                    break;
                }
            }
            if (stopIndex >= 0)
                break;
            ++endIndex;
        }
        if (stopIndex < 0) { // There's no breaking point.
            outIndex << startIndex << (endIndex-1);
//            qDebug()<<(endIndex-1)<<", "<<startIndex;
//            qDebug()<<(endIndex-1)<<", "<<(endIndex-1);
            return;
        } else { // Start new simplification from the breaking point.
            outIndex << startIndex;
//            qDebug()<<endIndex<<", "<<startIndex;
            internalSimplify(x, y, t, outIndex, stopIndex, endIndex, threshQuad);
            return;
        }
    }
}
