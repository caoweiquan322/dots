/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include <QCoreApplication>
#include<QDebug>
#include<QException>
#include<QVector>
#include<QElapsedTimer>
#include"DotsSimplifier.h"
#include"Helper.h"
#include"DotsException.h"

void batchDots(DotsSimplifier &simplifier, QVector<double> &x, QVector<double> &y, QVector<double> &t,
               QVector<double> &ox, QVector<double> &oy, QVector<double> &ot)
{
    int pointCount = x.count();
    double px, py, pt;

    simplifier.resetInternalData();
    for(int i=0; i<pointCount; ++i)
    {
        // Feed one point.
        simplifier.feedData(x[i], y[i], t[i]);
        // Check if there's output data.
        if(simplifier.readOutputData(px, py, pt))
        {
            ox.append(px);
            oy.append(py);
            ot.append(pt);
            qDebug("Input: %4d, output: %4d (%4d), Delay: %3d", i+1, simplifier.getSimplifiedIndex(ox.count()-1)+1,
                   ox.count(), i-simplifier.getSimplifiedIndex(ox.count()-1));
        }
    }
//    qDebug("=====> Finished <=======");
    simplifier.finish();
    while (simplifier.readOutputData(px, py, pt)) {
        ox.append(px);
        oy.append(py);
        ot.append(pt);
        qDebug("Input: %4d, output: %4d (%4d), Delay: %3d", pointCount, simplifier.getSimplifiedIndex(ox.count()-1)+1,
               ox.count(), pointCount-1-simplifier.getSimplifiedIndex(ox.count()-1));
    }
}

/**
 * @brief main is the entry point of the whole application.
 * @param argc is number of arguments transfered to the application by invoker.
 * @param argv represents the arguments trasnfered to the application by invoker.
 * @return 0 if the application exits normally, non-zero otherwise.
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    try
    {
        QString dataFileName = "E:\\Research_2015\\盲信号处理DOTS论文\\oritxt\\0001.txt";
        DotsSimplifier simplifier;
        // Set the simplification tolerance to 3km.
        simplifier.setParameters(3000.0);
        QVector<double> x,y,t;
        QElapsedTimer timer;
        // Parse the MOPSI data file as trajectory x/y/t.
        timer.start();
        Helper::parseMOPSI(dataFileName, x, y, t);
        qDebug("Parsing file OK, time: %d ms.", timer.elapsed());

        // Construct singular data.
//        x.clear();
//        y.clear();
//        t.clear();
//        for (int i=0; i<1000; ++i)
//        {
//            x.append(i);
//            y.append(i);
//            t.append(i);
//        }

        // Simplify the parsed trajectory.
        timer.start();
        QVector<double> ox, oy, ot;
        for (int i=0;i<1;++i)
        {
            ox.clear();
            oy.clear();
            ot.clear();
            batchDots(simplifier, x, y, t, ox, oy, ot);
        }
        qDebug("Original curve size: %d, output curve size: %d, time: %d ms", x.count(), ox.count(),
               timer.elapsed());
//        foreach (int idx, simplifier.simplifiedIndex) {
//            qDebug("%d", idx+1);
//        }
        qDebug("\nPress any key to continue...");
    }
    catch (DotsException &e)
    {
        qDebug()<<"ERROR: "<<e.getMessage();
    }
    catch (QException &)
    {
        qDebug()<<"ERROR: Unknown exception.";
    }

    return 0;//a.exec();
}

