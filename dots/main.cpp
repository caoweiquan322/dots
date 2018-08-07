/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include<QApplication>
#include<QDebug>
#include<QException>
#include<QVector>
#include<QElapsedTimer>
#include"DotsSimplifier.h"
#include"Helper.h"
#include"DotsException.h"
#include"PersistenceBatchSimplifier.h"
#include"DouglasPeuckerBatchSimplifier.h"
#include"SquishBatchSimplifier.h"
#include "OpwTrBatchSimplifier.h"
#include"OpwBatchSimplifier.h"
#include"mainwindow.h"
#include"AlgorithmComparison.h"
#include<QtMath>

/**
 * @brief main is the entry point of the whole application.
 * @param argc is number of arguments transfered to the application by invoker.
 * @param argv represents the arguments trasnfered to the application by invoker.
 * @return 0 if the application exits normally, non-zero otherwise.
 */
int main(int argc, char *argv[])
{
    // The application.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QApplication::setGraphicsSystem("raster");
#endif
    QApplication a(argc, argv);

    try
    {
        int algorithms = 0x19f;//0x7f000001;//
        int errTypes = 0x3f;
        QString suffix, folderPath;
        folderPath = "/Users/fatty/Code/GeoLife1.3/Data/";
        suffix = ".plt";
//        folderPath = "/Users/fatty/Code/MopsiDataset";
//        suffix = "";
//        folderPath = "/Users/fatty/Code/mit_scv";
//        suffix = "csv";
//        AlgorithmComparison::compareAlgorithms(
//                    "/Users/fatty/Code/MopsiDataset/1/1416074584000", //"../../../../test_files/r6.txt",
//                    algorithms, errTypes);
        AlgorithmComparison::compareAlgorithmsByFolder(
                    folderPath, suffix,
                    algorithms, errTypes);
        return a.exec();

        // Parse the MOPSI/GeoLife data file as trajectory x/y/t.
        QVector<double> x,y,t;
        QElapsedTimer timer;
        timer.start();
        QString dataFileName = "../../../../test_files/r6.txt";
        dataFileName = "/Users/fatty/Code/GeoLife1.3/Data/000/Trajectory/20090403011657.plt";
        //dataFileName = "/Users/fatty/Code/MopsiDataset/1/1416074584000";
        if (dataFileName.endsWith(".txt")) // MOPSI dataset.
            Helper::parseMOPSI(dataFileName, x, y, t);
        else if (dataFileName.endsWith(".plt")) // GeoLife dataset.
            Helper::parseGeoLife(dataFileName, x, y, t);
        else
            Helper::parseMOPSI2(dataFileName, x, y, t);
        qDebug("Parsing file OK, time: %d ms.", timer.elapsed());

        // Construct singular data for testing.
//        x = Helper::range<double>(0, 1000, 1);
//        y = Helper::range<double>(0, 1000, 1);
//        t = Helper::range<double>(0, 1000, 1);
//        int numK = x.count()-x.count()/7;
//        for (int i=0; i<numK; ++i) {
//            x.removeLast();
//            y.removeLast();
//            t.removeLast();
//        }

        // Simplify the parsed trajectory.
        QVector<double> dotsX, dotsY, dotsT, dotsCascadeX, dotsCascadeY, dotsCascadeT,
                persistenceX, persistenceY, persistenceT, DPX, DPY, DPT,
                squishX, squishY, squishT, opwX, opwY, opwT;
        timer.start();
        DotsSimplifier::batchDots(x, y, t, dotsX, dotsY, dotsT, 40000);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "DOTS", x.count(), dotsX.count(), timer.elapsed());
        timer.start();
        DotsSimplifier::batchDotsCascade(x, y, t, dotsCascadeX, dotsCascadeY, dotsCascadeT, 85000);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "DOTS CASCADE", x.count(), dotsCascadeX.count(), timer.elapsed());
        timer.start();
        PersistenceBatchSimplifier::simplifyCascade(x, y, persistenceX, persistenceY, 65);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "PERSISTENCE", x.count(), persistenceX.count(), timer.elapsed());
        timer.start();
        DouglasPeuckerBatchSimplifier::simplify(x, y, DPX, DPY, 2.0);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "DP", x.count(), DPX.count(), timer.elapsed());
        timer.start();
        SquishBatchSimplifier::simplify(x, y, t, squishX, squishY, squishT, 108);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "SQUISH", x.count(), squishX.count(), timer.elapsed());
        timer.start();
        OpwTrBatchSimplifier::simplify(x, y, t, opwX, opwY, opwT, 132);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "OpwTr", x.count(), opwX.count(), timer.elapsed());
        timer.start();
        OpwBatchSimplifier::simplify(x, y, t, opwX, opwY, opwT, 55);
        qDebug("%15s - Original curve size: %d, output curve size: %d, time: %d ms",
               "OpwTr", x.count(), opwX.count(), timer.elapsed());

        MainWindow figure;
        figure.plot(x, y, Qt::blue);
        figure.plot(dotsX, dotsY, Qt::red);
        figure.plot(dotsCascadeX, dotsCascadeY, Qt::green);
        figure.plot(persistenceX, persistenceY, Qt::cyan);
        figure.plot(DPX, DPY, Qt::black);
        figure.plot(squishX, squishY, Qt::magenta);
        figure.plot(opwX, opwY, Qt::darkGreen);
        figure.show();
        qDebug("\nPress any key to continue...");
        return 0;//a.exec();
    }
    catch (DotsException &e)
    {
        qDebug()<<"ERROR: "<<e.getMessage();
    }
    catch (QException &)
    {
        qDebug()<<"ERROR: Unknown exception.";
    }

    return a.exec();
}

