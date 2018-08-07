/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "AlgorithmComparison.h"
#include<QElapsedTimer>
#include<QDebug>
#include<QVector>
#include<QMap>
#include<QPair>
#include<QtMath>
#include<QElapsedTimer>
#include"Helper.h"
#include"mainwindow.h"
#include"DotsException.h"
#include"DotsSimplifier.h"
#include"DouglasPeuckerBatchSimplifier.h"
#include"PersistenceBatchSimplifier.h"
#include"SquishBatchSimplifier.h"
#include"OpwTrBatchSimplifier.h"
#include"OpwBatchSimplifier.h"

const int AlgorithmComparison::ERR_AVERAGE_SED = 0x01;
const int AlgorithmComparison::ERR_MAX_SED = 0x02;
const int AlgorithmComparison::ERR_MEAN_SED = 0x04;
const int AlgorithmComparison::ERR_AVERAGE_SSED_PER_POINT = 0x08;
const int AlgorithmComparison::ERR_MAX_LSSED = 0x10;
const int AlgorithmComparison::ERR_TIME_COST = 0x20;

const int AlgorithmComparison::ALG_DOTS = 0x01;
const int AlgorithmComparison::ALG_DOTS_CASCADE = 0x02;
const int AlgorithmComparison::ALG_DP = 0x04;
const int AlgorithmComparison::ALG_PERSISTENCE = 0x08;
const int AlgorithmComparison::ALG_SQUISH = 0x10;
const int AlgorithmComparison::ALG_MRPA = 0x20;
const int AlgorithmComparison::ALG_TS = 0x40;
const int AlgorithmComparison::ALG_OPWTR = 0x80;
const int AlgorithmComparison::ALG_OPW = 0x100;

const int AlgorithmComparison::ALG_DOTS_CASCADE_S_20 = 0x10000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_S_100 = 0x20000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_S_500 = 0x40000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_S_1000 = 0x80000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_S_5000 = 0x100000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_S_10000 = 0x200000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_S_50000 = 0x400000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_1_5 = 0x1000000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_2_0 = 0x2000000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_3_0 = 0x4000000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_4_0 = 0x8000000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_5_0 = 0x10000000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_6_0 = 0x20000000;
const int AlgorithmComparison::ALG_DOTS_CASCADE_T_7_0 = 0x40000000;

AlgorithmComparison::AlgorithmComparison(QObject *parent) : QObject(parent)
{

}

void AlgorithmComparison::evaluateAlgorithm(const QVector<double> &x, const QVector<double> &y, const QVector<double> &t,
                                            int algorithm,
                                            double compressMin, double compressMax, int numSteps,
                                            QMap<int, QVector<EvaluationPoint> > &errorToEval, int errorTypes)
{
    QVector<int> allAlgorithms;
    allAlgorithms<<ALG_DOTS<<ALG_DOTS_CASCADE<<ALG_DP<<ALG_PERSISTENCE<<ALG_SQUISH<<ALG_MRPA<<ALG_TS<<ALG_OPWTR<<ALG_OPW;
    allAlgorithms<<ALG_DOTS_CASCADE_S_20<<ALG_DOTS_CASCADE_S_100<<ALG_DOTS_CASCADE_S_500<<ALG_DOTS_CASCADE_S_1000
                <<ALG_DOTS_CASCADE_S_5000<<ALG_DOTS_CASCADE_S_10000<<ALG_DOTS_CASCADE_S_50000
               <<ALG_DOTS_CASCADE_T_1_5<<ALG_DOTS_CASCADE_T_2_0<<ALG_DOTS_CASCADE_T_3_0
              <<ALG_DOTS_CASCADE_T_4_0<<ALG_DOTS_CASCADE_T_5_0<<ALG_DOTS_CASCADE_T_6_0
             <<ALG_DOTS_CASCADE_T_7_0;
    QVector<int> allErrors;
    allErrors<<ERR_AVERAGE_SED<<ERR_MAX_SED<<ERR_MEAN_SED<<ERR_AVERAGE_SSED_PER_POINT<<ERR_MAX_LSSED<<ERR_TIME_COST;

    // Evaluation statistucs.
    QVector<double> xSum, ySum, tSum, x2Sum, y2Sum, t2Sum, xtSum, ytSum;
    calculateStatisticsForSSED(x, y, t, xSum, ySum, tSum, x2Sum, y2Sum, t2Sum, xtSum, ytSum);

    errorToEval.clear();
    double expStep = qPow(compressMax/compressMin, 1.0/((double)(numSteps-1)));
    double rate = compressMin;
    for (int i=0; i<numSteps; ++i)
    {
        //double rate = compressMin+(compressMax-compressMin)/(numSteps-1)*i;
        QVector<int> simplifiedIndex;
        double timeCost;
        try {
            //qDebug()<<"Start simplify"<<rate;
            timeCost = tryToSimplify(x, y, t, algorithm, rate, simplifiedIndex);
            //qDebug()<<"End simplify"<<rate;
        } catch (DotsException e) {
            qDebug()<<"Algoritm "<<algorithm<<" failed. Details: "<<e.getMessage();
            rate *= expStep;
            continue; // Go to next step.
        }

        double realRate = (double)(simplifiedIndex.count())/((double)x.count());
        foreach (int err, allErrors) {
            if (err & errorTypes) {
                if (!errorToEval.contains(err))
                    errorToEval[err] = QVector<EvaluationPoint>();

                if (err != ERR_TIME_COST)
                {
                    double simplifyError = evaluateResult(x, y, t, simplifiedIndex, err,
                                                          xSum, ySum, tSum, x2Sum, y2Sum, t2Sum, xtSum, ytSum);
                    //simplifyError = qLn(1.0+simplifyError);
                    errorToEval[err].append(EvaluationPoint(1.0/realRate, simplifyError));
                }
                else
                {
                    errorToEval[err].append(EvaluationPoint(1.0/realRate, timeCost));
                }
            }
        }
        rate *= expStep;
    }
}

void AlgorithmComparison::compareAlgorithms(QString fileName, int algorithmsToCompare, int errorTypesToCompare)
{
    // Parse file.
    QVector<double> x,y,t;
    QElapsedTimer timer;
    timer.start();
    if (fileName.endsWith(".txt")) // MOPSI dataset.
        Helper::parseMOPSI(fileName, x, y, t);
    else if (fileName.endsWith(".plt")) // GeoLife dataset.
        Helper::parseGeoLife(fileName, x, y, t);
    else
        Helper::parseMOPSI2(fileName, x, y, t);
    qDebug("Parsing file OK, time: %d ms.", timer.elapsed());

    // Evaluation.
    QVector<int> allAlgorithms;
    allAlgorithms<<ALG_DOTS<<ALG_DOTS_CASCADE<<ALG_DP<<ALG_PERSISTENCE<<ALG_SQUISH<<ALG_MRPA<<ALG_TS<<ALG_OPWTR<<ALG_OPW;
    allAlgorithms<<ALG_DOTS_CASCADE_S_20<<ALG_DOTS_CASCADE_S_100<<ALG_DOTS_CASCADE_S_500<<ALG_DOTS_CASCADE_S_1000
                <<ALG_DOTS_CASCADE_S_5000<<ALG_DOTS_CASCADE_S_10000<<ALG_DOTS_CASCADE_S_50000
               <<ALG_DOTS_CASCADE_T_1_5<<ALG_DOTS_CASCADE_T_2_0<<ALG_DOTS_CASCADE_T_3_0
              <<ALG_DOTS_CASCADE_T_4_0<<ALG_DOTS_CASCADE_T_5_0<<ALG_DOTS_CASCADE_T_6_0
             <<ALG_DOTS_CASCADE_T_7_0;
    QMap< QPair<int, int>, QVector<EvaluationPoint> > algErrToEval;
    foreach (int alg, allAlgorithms) {
        if (alg & algorithmsToCompare) // Select algorithms to evaluate.
        {
            qDebug("Evaluating algorithm: 0x%02x", alg);
            QMap<int, QVector<EvaluationPoint> > errorToEval;
            evaluateAlgorithm(x, y, t, alg,
                              0.01, 0.2, 10,
                              errorToEval, errorTypesToCompare);
            //qDebug()<<errorToEval.keys();
            foreach (int errorType, errorToEval.keys()) {
                algErrToEval[qMakePair(alg, errorType)] = errorToEval[errorType];
            }
        }
    }

    // Visualization.
    QVector<int> allErrors;
    allErrors<<ERR_AVERAGE_SED<<ERR_MAX_SED<<ERR_MEAN_SED<<ERR_AVERAGE_SSED_PER_POINT<<ERR_MAX_LSSED<<ERR_TIME_COST;
    QMap<int, Qt::GlobalColor> colorMap;
    colorMap[ALG_DOTS] = Qt::red;
    colorMap[ALG_DOTS_CASCADE] = Qt::green;
    colorMap[ALG_DP] = Qt::blue;
    colorMap[ALG_PERSISTENCE] = Qt::cyan;
    colorMap[ALG_SQUISH] = Qt::magenta;
    colorMap[ALG_OPWTR] = Qt::black;
    colorMap[ALG_OPW] = Qt::darkGray;
    colorMap[ALG_DOTS_CASCADE_S_20] = Qt::red;
    colorMap[ALG_DOTS_CASCADE_S_100] = Qt::green;
    colorMap[ALG_DOTS_CASCADE_S_500] = Qt::blue;
    colorMap[ALG_DOTS_CASCADE_S_1000] = Qt::cyan;
    colorMap[ALG_DOTS_CASCADE_S_5000] = Qt::magenta;
    colorMap[ALG_DOTS_CASCADE_S_10000] = Qt::black;
    colorMap[ALG_DOTS_CASCADE_S_50000] = Qt::darkGray;
    colorMap[ALG_DOTS_CASCADE_T_1_5] = Qt::red;
    colorMap[ALG_DOTS_CASCADE_T_2_0] = Qt::green;
    colorMap[ALG_DOTS_CASCADE_T_3_0] = Qt::blue;
    colorMap[ALG_DOTS_CASCADE_T_4_0] = Qt::cyan;
    colorMap[ALG_DOTS_CASCADE_T_5_0] = Qt::magenta;
    colorMap[ALG_DOTS_CASCADE_T_6_0] = Qt::black;
    colorMap[ALG_DOTS_CASCADE_T_7_0] = Qt::darkGray;
    QMap<int, QString> algNames;
    algNames[ALG_DOTS] = "Dots";
    algNames[ALG_DOTS_CASCADE] = "Dots Cascade";
    algNames[ALG_DP] = "Douglas Peucker";
    algNames[ALG_PERSISTENCE] = "Persistence";
    algNames[ALG_SQUISH] = "SQUISH";
    algNames[ALG_OPWTR] = "Opw-TR";
    algNames[ALG_OPW] = "Opw";
    algNames[ALG_DOTS_CASCADE_S_20] = "s=20";
    algNames[ALG_DOTS_CASCADE_S_100] = "s=100";
    algNames[ALG_DOTS_CASCADE_S_500] = "s=500";
    algNames[ALG_DOTS_CASCADE_S_1000] = "s=1000";
    algNames[ALG_DOTS_CASCADE_S_5000] = "s=5000";
    algNames[ALG_DOTS_CASCADE_S_10000] = "s=10000";
    algNames[ALG_DOTS_CASCADE_S_50000] = "s=50000";
    algNames[ALG_DOTS_CASCADE_T_1_5] = "t=1.5";
    algNames[ALG_DOTS_CASCADE_T_2_0] = "t=2.0";
    algNames[ALG_DOTS_CASCADE_T_3_0] = "t=3.0";
    algNames[ALG_DOTS_CASCADE_T_4_0] = "t=4.0";
    algNames[ALG_DOTS_CASCADE_T_5_0] = "t=5.0";
    algNames[ALG_DOTS_CASCADE_T_6_0] = "t=6.0";
    algNames[ALG_DOTS_CASCADE_T_7_0] = "t=7.0";
    QMap<int, QString> evaluationNames;
    evaluationNames[ERR_AVERAGE_SED] = "Average SED";
    evaluationNames[ERR_MAX_SED] = "Maximum SED";
    evaluationNames[ERR_MEAN_SED] = "Mean SED";
    evaluationNames[ERR_AVERAGE_SSED_PER_POINT] = "Average SSED";
    evaluationNames[ERR_MAX_LSSED] = "Maximum Local SSED";
    evaluationNames[ERR_TIME_COST] = "Time Cost";

    foreach (int errorType, allErrors) {
        if (errorType & errorTypesToCompare) {
            MainWindow *fig = new MainWindow;
            foreach (int alg, allAlgorithms) {
                if (alg & algorithmsToCompare) {
                    QVector<EvaluationPoint> eval = algErrToEval[qMakePair(alg, errorType)];
                    QVector<double> cr, err;
                    foreach (EvaluationPoint p, eval) {
                        cr.append(p.compressionRate);
                        err.append(p.error);
                    }
                    fig->plot(cr, err, colorMap[alg], algNames[alg]);
                }
            }
            fig->setWindowTitle(evaluationNames[errorType]);
            fig->show();
        }
    }

}

void AlgorithmComparison::compareAlgorithmsByFolder(QString folderName, QString suffix,
                                                    int algorithmsToCompare, int errorTypesToCompare)
{
    // Collect all files under the folder.
    QStringList filesPath;
    collectFilesOfFolder(folderName, filesPath, suffix);
    qDebug()<<"The folder "<<folderName<<" contains "<<filesPath.count()<<" files.";

    // Process each file with specified suffix.
    int numTest = 0;
    QVector<int> allAlgorithms;
    allAlgorithms<<ALG_DOTS<<ALG_DOTS_CASCADE<<ALG_DP<<ALG_PERSISTENCE<<ALG_SQUISH<<ALG_MRPA<<ALG_TS<<ALG_OPWTR<<ALG_OPW;
    allAlgorithms<<ALG_DOTS_CASCADE_S_20<<ALG_DOTS_CASCADE_S_100<<ALG_DOTS_CASCADE_S_500<<ALG_DOTS_CASCADE_S_1000
                <<ALG_DOTS_CASCADE_S_5000<<ALG_DOTS_CASCADE_S_10000<<ALG_DOTS_CASCADE_S_50000
               <<ALG_DOTS_CASCADE_T_1_5<<ALG_DOTS_CASCADE_T_2_0<<ALG_DOTS_CASCADE_T_3_0
              <<ALG_DOTS_CASCADE_T_4_0<<ALG_DOTS_CASCADE_T_5_0<<ALG_DOTS_CASCADE_T_6_0
             <<ALG_DOTS_CASCADE_T_7_0;
    QMap< QPair<int, int>, QVector<EvaluationPoint> > algErrToEval;
    double crStart = 0.01, crEnd= 0.2;
    int crSteps = 10;
    double avgDistance=0, avgSpeed=0;
    int numDistance = 0, numSpeed = 0;
    foreach (QString fileName, filesPath) {
        // Parse file.
        QVector<double> x,y,t;
        QElapsedTimer timer;
        timer.start();
        try {
        if (fileName.endsWith(".txt")) // MOPSI dataset.
            Helper::parseMOPSI(fileName, x, y, t);
        else if (fileName.endsWith(".plt")) // GeoLife dataset.
            Helper::parseGeoLife(fileName, x, y, t);
        else if (fileName.endsWith(".csv")) // MIT Single Camera View.
            Helper::parseMitScv(fileName, x, y, t);
        else
            Helper::parseMOPSI2(fileName, x, y, t);
        } catch (DotsException e) {
            qDebug("Parsing file %s Error.", fileName.toStdString().c_str());
            continue;
        }

        qDebug("Parsing file %s OK, time: %d ms.", fileName.toStdString().c_str(), timer.elapsed());
        if (x.count() < 100)continue;
//        for (int i=0; i<x.count(); ++i) {
//            x[i] += 0.0*(qrand()/((double)RAND_MAX)-0.5);
//            y[i] += 0.0*(qrand()/((double)RAND_MAX)-0.5);
//        }
        double singleDistance = 0;
        for (int i=1; i<x.count(); ++i) {
            double d = qSqrt((x[i]-x[i-1])*(x[i]-x[i-1])+(y[i]-y[i-1])*(y[i]-y[i-1]));
            double dt = t[i]-t[i-1];
            //qDebug()<<d<<", "<<dt<<", "<<(d/dt*3.6);
            if (d/dt*3.6 < 150)
                singleDistance += d;
            if (d/dt*3.6 > 150 || d/dt*3.6 < 1)continue;

            avgSpeed += (d/dt);
            numSpeed++;
        }
        avgDistance += singleDistance;
        numDistance++;

        // Evaluation.
        foreach (int alg, allAlgorithms) {
            if (alg & algorithmsToCompare) // Select algorithms to evaluate.
            {
                qDebug("Evaluating algorithm: 0x%02x", alg);
                QMap<int, QVector<EvaluationPoint> > errorToEval;
                evaluateAlgorithm(x, y, t, alg,
                                  crStart, crEnd, crSteps,
                                  errorToEval, errorTypesToCompare);
                //qDebug()<<errorToEval.keys();
                foreach (int errorType, errorToEval.keys()) {
                    QPair<int, int> _key = qMakePair(alg, errorType);
                    if (!algErrToEval.contains(_key))
                        algErrToEval[_key] = errorToEval[errorType];
                    else {
                        foreach (EvaluationPoint _point, errorToEval[errorType]) {
                            algErrToEval[_key].append(_point);
                        }
                    }
                }
            }
        }

        ++numTest;
        qDebug()<<"Processing "<<numTest;
        if (numTest > 1000)break;
    }
//    qDebug()<<"Average distance:"<<avgDistance/numDistance;
//    qDebug()<<"Average speed: "<<avgSpeed/numSpeed;
//    return;

    // Post-processing the results.
    double expStep = qPow(crEnd/crStart, 1.0/((double)(crSteps-1)));
    foreach (auto _key, algErrToEval.keys()) {
        QVector<EvaluationPoint> _val = algErrToEval[_key], _nval;
        double errs[crSteps], weights[crSteps];
        for (int k=0; k<crSteps; ++k) {
            errs[k] = 0;
            weights[k] = 0;
        }
        foreach (EvaluationPoint _point, _val) {
            double floatIndex = qLn(1/_point.compressionRate/crStart)/qLn(expStep);
            int intIndex = qMin(crSteps-1, qMax(0, qFloor(floatIndex+0.5)));
            double w = qFabs(floatIndex-intIndex);
            if (w>0.5)
                w=0;
            else {
                w=0.5-w;
                errs[intIndex] += (w*_point.error);
                weights[intIndex] += w;
            }
        }
        for (int k=0; k<crSteps; ++k)
            if (weights[k] > 20.0) // We must ensure that there's enough samples for each evaluation point.
                _nval << EvaluationPoint(1/(crStart*qPow(expStep, k)), errs[k]/weights[k]);
        algErrToEval[_key] = _nval;
    }

    // Visualization.
    QVector<int> allErrors;
    allErrors<<ERR_AVERAGE_SED<<ERR_MAX_SED<<ERR_MEAN_SED<<ERR_AVERAGE_SSED_PER_POINT<<ERR_MAX_LSSED<<ERR_TIME_COST;
    QMap<int, Qt::GlobalColor> colorMap;
    colorMap[ALG_DOTS] = Qt::red;
    colorMap[ALG_DOTS_CASCADE] = Qt::green;
    colorMap[ALG_DP] = Qt::blue;
    colorMap[ALG_PERSISTENCE] = Qt::cyan;
    colorMap[ALG_SQUISH] = Qt::magenta;
    colorMap[ALG_OPWTR] = Qt::black;
    colorMap[ALG_OPW] = Qt::darkGray;
    colorMap[ALG_DOTS_CASCADE_S_20] = Qt::red;
    colorMap[ALG_DOTS_CASCADE_S_100] = Qt::green;
    colorMap[ALG_DOTS_CASCADE_S_500] = Qt::blue;
    colorMap[ALG_DOTS_CASCADE_S_1000] = Qt::cyan;
    colorMap[ALG_DOTS_CASCADE_S_5000] = Qt::magenta;
    colorMap[ALG_DOTS_CASCADE_S_10000] = Qt::black;
    colorMap[ALG_DOTS_CASCADE_S_50000] = Qt::darkGray;
    colorMap[ALG_DOTS_CASCADE_T_1_5] = Qt::red;
    colorMap[ALG_DOTS_CASCADE_T_2_0] = Qt::green;
    colorMap[ALG_DOTS_CASCADE_T_3_0] = Qt::blue;
    colorMap[ALG_DOTS_CASCADE_T_4_0] = Qt::cyan;
    colorMap[ALG_DOTS_CASCADE_T_5_0] = Qt::magenta;
    colorMap[ALG_DOTS_CASCADE_T_6_0] = Qt::black;
    colorMap[ALG_DOTS_CASCADE_T_7_0] = Qt::darkGray;
    QMap<int, QString> algNames;
    algNames[ALG_DOTS] = "Dots";
    algNames[ALG_DOTS_CASCADE] = "Dots Cascade";
    algNames[ALG_DP] = "Douglas Peucker";
    algNames[ALG_PERSISTENCE] = "Persistence";
    algNames[ALG_SQUISH] = "SQUISH";
    algNames[ALG_OPWTR] = "Opw-TR";
    algNames[ALG_OPW] = "Opw";
    algNames[ALG_DOTS_CASCADE_S_20] = "s=20";
    algNames[ALG_DOTS_CASCADE_S_100] = "s=100";
    algNames[ALG_DOTS_CASCADE_S_500] = "s=500";
    algNames[ALG_DOTS_CASCADE_S_1000] = "s=1000";
    algNames[ALG_DOTS_CASCADE_S_5000] = "s=5000";
    algNames[ALG_DOTS_CASCADE_S_10000] = "s=10000";
    algNames[ALG_DOTS_CASCADE_S_50000] = "s=50000";
    algNames[ALG_DOTS_CASCADE_T_1_5] = "t=1.5";
    algNames[ALG_DOTS_CASCADE_T_2_0] = "t=2.0";
    algNames[ALG_DOTS_CASCADE_T_3_0] = "t=3.0";
    algNames[ALG_DOTS_CASCADE_T_4_0] = "t=4.0";
    algNames[ALG_DOTS_CASCADE_T_5_0] = "t=5.0";
    algNames[ALG_DOTS_CASCADE_T_6_0] = "t=6.0";
    algNames[ALG_DOTS_CASCADE_T_7_0] = "t=7.0";
    QMap<int, QString> evaluationNames;
    evaluationNames[ERR_AVERAGE_SED] = "Average SED";
    evaluationNames[ERR_MAX_SED] = "Maximum SED";
    evaluationNames[ERR_MEAN_SED] = "Mean SED";
    evaluationNames[ERR_AVERAGE_SSED_PER_POINT] = "Average SSED";
    evaluationNames[ERR_MAX_LSSED] = "Maximum Local SSED";
    evaluationNames[ERR_TIME_COST] = "Time Cost";

    foreach (int errorType, allErrors) {
        if (errorType & errorTypesToCompare) {
            MainWindow *fig = new MainWindow;
            foreach (int alg, allAlgorithms) {
                if (alg & algorithmsToCompare) {
                    QVector<EvaluationPoint> eval = algErrToEval[qMakePair(alg, errorType)];
                    QVector<double> cr, err;
                    foreach (EvaluationPoint p, eval) {
                        cr.append(p.compressionRate);
                        err.append(p.error);
                        qDebug("%d, %d, %.8f, %.8f", alg, errorType, p.compressionRate, p.error);
                    }
                    fig->plot(cr, err, colorMap[alg], algNames[alg]);
                }
            }
            fig->setWindowTitle(evaluationNames[errorType]);
            fig->show();
        }
    }

}

void AlgorithmComparison::collectFilesOfFolder(QString folderName, QStringList& fileNames, QString suffix) {
    QDir dir(folderName);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folderList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    QString fileName;
    foreach (QFileInfo file, fileList) {
        fileName = file.absoluteFilePath();
        if (fileName.endsWith(suffix)) {
            //qDebug()<<"File: "<<fileName;
            fileNames << fileName;
        }
    }

    foreach (QFileInfo folder, folderList) {
        //qDebug()<<"Folder: "<<folder.absoluteFilePath();
        collectFilesOfFolder(folder.absoluteFilePath(), fileNames, suffix);
    }
}

double AlgorithmComparison::tryToSimplify(const QVector<double> &x, const QVector<double> &y, const QVector<double> &t,
                                        int algorithm, double compressRate, QVector<int> &simplifiedIndex)
{
    QVector<int> supported, binFind;
    supported<<ALG_DOTS<<ALG_DOTS_CASCADE<<ALG_DP<<ALG_PERSISTENCE<<ALG_SQUISH<<ALG_OPWTR<<ALG_OPW;
    supported<<ALG_DOTS_CASCADE_S_20<<ALG_DOTS_CASCADE_S_100<<ALG_DOTS_CASCADE_S_500<<ALG_DOTS_CASCADE_S_1000
            <<ALG_DOTS_CASCADE_S_5000<<ALG_DOTS_CASCADE_S_10000<<ALG_DOTS_CASCADE_S_50000
           <<ALG_DOTS_CASCADE_T_1_5<<ALG_DOTS_CASCADE_T_2_0<<ALG_DOTS_CASCADE_T_3_0
          <<ALG_DOTS_CASCADE_T_4_0<<ALG_DOTS_CASCADE_T_5_0<<ALG_DOTS_CASCADE_T_6_0
         <<ALG_DOTS_CASCADE_T_7_0;
    binFind<<ALG_DOTS<<ALG_DOTS_CASCADE<<ALG_DP<<ALG_PERSISTENCE<<ALG_OPWTR<<ALG_OPW;
    binFind<<ALG_DOTS_CASCADE_S_20<<ALG_DOTS_CASCADE_S_100<<ALG_DOTS_CASCADE_S_500<<ALG_DOTS_CASCADE_S_1000
          <<ALG_DOTS_CASCADE_S_5000<<ALG_DOTS_CASCADE_S_10000<<ALG_DOTS_CASCADE_S_50000
         <<ALG_DOTS_CASCADE_T_1_5<<ALG_DOTS_CASCADE_T_2_0<<ALG_DOTS_CASCADE_T_3_0
        <<ALG_DOTS_CASCADE_T_4_0<<ALG_DOTS_CASCADE_T_5_0<<ALG_DOTS_CASCADE_T_6_0
       <<ALG_DOTS_CASCADE_T_7_0;
    if (!supported.contains(algorithm))
        DotsException(QString("Algorithm %x not supported.").arg(algorithm)).raise();
    QMap<int, double> defaultParam;
    defaultParam[ALG_DOTS] = 1000.0;
    defaultParam[ALG_DOTS_CASCADE] = 1000.0;
    defaultParam[ALG_DP] = 10.0;
    defaultParam[ALG_PERSISTENCE] = 100.0;
    defaultParam[ALG_OPWTR] = 100.0;
    defaultParam[ALG_OPW] = 100.0;

    QElapsedTimer timer;
    double timeCost = 0.0;

    if (binFind.contains(algorithm))
    {
        double param;
        if (defaultParam.contains(algorithm))
            param = defaultParam[algorithm];
        else
            param = 1000.0;
        double lb, ub;
        double cr = 0.0;
        param*=2;
        int maxItr=20;
        while (cr < compressRate && (--maxItr)>0) {
            param/=2;
            timer.start();
            cr = generalSimplify(x, y, t, algorithm, param, simplifiedIndex);
            timeCost = (double)timer.nsecsElapsed()/(1.0e9);
            //qDebug()<<algorithm<<", "<<cr<<", "<<compressRate<<", "<<param;
        }
        lb = param;
        param/=2;
        maxItr = 20;
        while (cr >= compressRate && (--maxItr)>0) {
            param*=2;
            timer.start();
            cr = generalSimplify(x, y, t, algorithm, param, simplifiedIndex);
            timeCost = (double)timer.nsecsElapsed()/(1.0e9);
            //qDebug()<<algorithm<<", "<<cr<<", "<<compressRate<<", "<<param;
        }
        ub = param;
        double tolerance = 0.1*compressRate;

        maxItr = 30;
        while (qFabs(compressRate-cr) > tolerance && ub>lb+1e-3 && (--maxItr)>0)
        {
            param = (lb+ub)/2;
            timer.start();
            cr = generalSimplify(x, y, t, algorithm, param, simplifiedIndex);
            timeCost = (double)timer.nsecsElapsed()/(1.0e9);
            if (cr <= compressRate-tolerance/2)
            {
                ub = param;
            }
            else if (cr >= compressRate+tolerance/2)
            {
                lb = param;
            }
            else
            {
                // Will break soon.
                break;
            }
            //qDebug()<<algorithm<<", "<<cr<<", "<<compressRate<<", "<<param<<ub<<","<<(ub-lb);
        }
        // Simplified contains the results.
        //qDebug("General %d simplified count: %d", algorithm, simplifiedIndex.count());
    }
    else // SQUISH
    {
        //qDebug("SQUISH simplified count: %d", qRound(x.count()*compressRate));
        timer.start();
        SquishBatchSimplifier::simplifyByIndex(x, y, t, simplifiedIndex, qRound(x.count()*compressRate));
        timeCost = timer.nsecsElapsed()/(1.0e9);
    }
    return timeCost;
}

double AlgorithmComparison::generalSimplify(const QVector<double> &x, const QVector<double> &y, const QVector<double> &t,
                                          int algorithm, double param, QVector<int> &simplifiedIndex)
{
    switch (algorithm) {
    case ALG_DOTS:
        DotsSimplifier::batchDotsByIndex(x, y, t, simplifiedIndex, param);
        break;
    case ALG_DOTS_CASCADE:
        DotsSimplifier::batchDotsCascadeByIndex(x, y, t, simplifiedIndex, param);
        break;
    case ALG_DP:
        DouglasPeuckerBatchSimplifier::simplifyByIndex(x, y, simplifiedIndex, param);
        break;
    case ALG_PERSISTENCE:
        PersistenceBatchSimplifier::simplifyByIndexCascade(x, y, simplifiedIndex, param);
        break;
    case ALG_OPWTR:
        OpwTrBatchSimplifier::simplifyByIndex(x, y, t, simplifiedIndex, param);
        //qDebug("CR: %.6f", (double)(simplifiedIndex.count())/((double)(x.count())));
        break;
    case ALG_OPW:
        OpwBatchSimplifier::simplifyByIndex(x, y, t, simplifiedIndex, param);
        break;
    case ALG_DOTS_CASCADE_S_20:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 20.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_S_100:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_S_500:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 500.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_S_1000:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 1000.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_S_5000:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 5000.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_S_10000:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 10000.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_S_50000:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 50000.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_T_1_5:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 1.2);
        break;
    case ALG_DOTS_CASCADE_T_2_0:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 1.4);
        break;
    case ALG_DOTS_CASCADE_T_3_0:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 1.8);
        break;
    case ALG_DOTS_CASCADE_T_4_0:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 2.0);
        break;
    case ALG_DOTS_CASCADE_T_5_0:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 4.0);
        break;
    case ALG_DOTS_CASCADE_T_6_0:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 6.0);
        break;
    case ALG_DOTS_CASCADE_T_7_0:
        DotsSimplifier::batchDotsCascadeByIndexOptions(x, y, t, simplifiedIndex, param, 100.0, 7.0);
        break;
    default:
        DotsException(QString("Algorithm %x not supported.").arg(algorithm)).raise();
        break;
    }
    return (double)(simplifiedIndex.count())/((double)(x.count()));
}

void AlgorithmComparison::calculateStatisticsForSSED(
        const QVector<double> &x, const QVector<double> &y, const QVector<double> &t,
        QVector<double> &xSum, QVector<double> &ySum, QVector<double> &tSum,
        QVector<double> &x2Sum, QVector<double> &y2Sum, QVector<double> &t2Sum,
        QVector<double> &xtSum, QVector<double> &ytSum)
{
    xSum.clear();
    ySum.clear();
    tSum.clear();
    x2Sum.clear();
    y2Sum.clear();
    t2Sum.clear();
    xtSum.clear();
    ytSum.clear();

    xSum.append(x.at(0));
    ySum.append(y.at(0));
    tSum.append(t.at(0));
    x2Sum.append(x.at(0)*x.at(0));
    y2Sum.append(y.at(0)*y.at(0));
    t2Sum.append(t.at(0)*t.at(0));
    xtSum.append(x.at(0)*t.at(0));
    ytSum.append(y.at(0)*t.at(0));
    double px, py, pt;
    for (int i=0; i<x.count()-1; ++i)
    {
        px = x.at(i);
        py = y.at(i);
        pt = t.at(i);
        xSum.append(xSum[i]+px);
        ySum.append(ySum[i]+py);
        tSum.append(tSum[i]+pt);
        x2Sum.append(x2Sum[i]+px*px);
        y2Sum.append(y2Sum[i]+py*py);
        t2Sum.append(t2Sum[i]+pt*pt);
        xtSum.append(xtSum[i]+px*pt);
        ytSum.append(ytSum[i]+py*pt);
    }
}

double AlgorithmComparison::evaluateResult(
        const QVector<double> &x, const QVector<double> &y, const QVector<double> &t,
        const QVector<int> simplifiedIndex, int errorType,
        const QVector<double> &xSum, const QVector<double> &ySum, const QVector<double> &tSum,
        const QVector<double> &x2Sum, const QVector<double> &y2Sum, const QVector<double> &t2Sum,
        const QVector<double> &xtSum, const QVector<double> &ytSum)
{
    Helper::checkIntEqual(simplifiedIndex.first(), 0);
    Helper::checkIntEqual(simplifiedIndex.last(), x.count()-1);

    if (errorType == ERR_AVERAGE_SSED_PER_POINT || errorType == ERR_MAX_LSSED)
    {
        QVector<double> SSEDs;
        for(int i=0; i<simplifiedIndex.count()-1; ++i)
        {
            SSEDs.append(getLSSD(simplifiedIndex.at(i), simplifiedIndex.at(i+1),
                                 x, y, t,
                                 xSum, ySum, tSum, x2Sum, y2Sum, t2Sum, xtSum, ytSum));
        }
        if (errorType == ERR_AVERAGE_SSED_PER_POINT)
        {
            return Helper::sum(SSEDs)/((double)x.count());
        }
        else
        {
            return Helper::max(SSEDs);
        }
    }
    else if (errorType == ERR_AVERAGE_SED || errorType == ERR_MAX_SED || errorType == ERR_MEAN_SED)
    {
        QVector<double> SEDs;
        SEDs.append(0.0);
        for(int i=0; i<simplifiedIndex.count()-1; ++i)
        {
            int _start = simplifiedIndex.at(i);
            int _end = simplifiedIndex.at(i+1);
            for (int j=_start+1; j<_end; ++j)
            {
                double k = (t.at(j) - t.at(_start))/(t.at(_end)-t.at(_start));
                double px = (1.0-k)*x.at(_start) + k*x.at(_end);
                double py = (1.0-k)*y.at(_start) + k*y.at(_end);
                double sed = (x.at(j)-px)*(x.at(j)-px)+(y.at(j)-py)*(y.at(j)-py);
                SEDs.append(qSqrt(sed));
            }
            SEDs.append(0.0);
        }
        if (errorType == ERR_AVERAGE_SED)
            return Helper::sum(SEDs)/((double)SEDs.count());
        else if (errorType == ERR_MAX_SED)
            return Helper::max(SEDs);
        else
            return Helper::mean(SEDs);
    }
    else
    {
        DotsException(QString("Evaluation type code %x not supported.").arg(errorType)).raise();
    }
    return -1.0;
}

