/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "DotsSimplifier.h"
#include"Helper.h"
#include"DotsException.h"
#include<QFile>
#include<QDateTime>
#include<QVector>
#include<QDebug>
#include<QtMath>

const QString DotsSimplifier::MOPSI_DATETIME_FORMAT("yyyy-MM-ddHH:mm:ss");
const double DotsSimplifier::SCALE_FACTOR_PRECISION = 1e-4;

DotsSimplifier::DotsSimplifier(QObject *parent) : QObject(parent)
{
    lssdTh = 10000.0;
    lssdUpperBound = lssdTh*2.0;
    resetInternalData();
}

void DotsSimplifier::setParameters(double lssdTh, double k)
{
    this->lssdTh = lssdTh;
    lssdUpperBound = lssdTh*k;
}

void DotsSimplifier::resetInternalData()
{
    // Clear data points and internal statistics.
    ptx.clear();
    pty.clear();
    ptt.clear();
    xSum.clear();
    ySum.clear();
    tSum.clear();
    x2Sum.clear();
    y2Sum.clear();
    t2Sum.clear();
    xtSum.clear();
    ytSum.clear();

    // Clear structures for DAG construction and optimization.
    vK.clear();
    vL.clear();
    pathK.clear();
//    vD.clear();
//    vE.clear();
    issed.clear();
    parents.clear();
    terminated.clear();
    numTerminated = 0;
    simplifiedIndex.clear();

    // Input/output queue position.
//    currentLayer = -1;
    inputCount = 0;
    outputCount = 0;

    // Finish flag.
    finished = false;
}

void DotsSimplifier::finish()
{
    if (!finished)
    {
        finished = true;
        directedAcyclicGraphSearch();
    }
}

void DotsSimplifier::parseMOPSI(QString fileName, QVector<double> &x, QVector<double> &y, QVector<double> &t)
{
    // Check if file name is null or empty.
    Helper::checkNotNullNorEmpty("fileName", fileName);
    try
    {
        // Open file in TEXT mode.
        QFile file(fileName.trimmed());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            DotsException(QString("Open file %1 error.").arg(fileName)).raise();

        QVector<double> longitude, latitude;
        x.clear();
        y.clear();
        t.clear();
        while(!file.atEnd())
        {
            // Read the file line by line.
            QByteArray line = file.readLine().trimmed();
            if(line.isEmpty())
                continue;
            auto parts = line.split(' ');

            // In case where the line is malformed.
            if(parts.count() != 4)
            {
                DotsException("Malformed line found.").raise();
            }

            // Store the parsed data without cleaning it.
            latitude.append(parts[0].toDouble());
            longitude.append(parts[1].toDouble());
            t.append((double)QDateTime::fromString(parts[2]+parts[3], MOPSI_DATETIME_FORMAT).toTime_t());
        }
        // Do mercator projection on the parsed longitude/latitude.
        mercatorProject(longitude, latitude, x, y);

        // Normalize data by first value of each array.
        normalizeData(x, false);
        normalizeData(y, false);
        normalizeData(t, false);
    }
    catch (DotsException &e)
    {
        e.raise();
    }
    catch (QException &)
    {
        DotsException("Error occured when parsing trajectory file.").raise();
    }
}

// Problem with points whose latitude nears pi/2 was fixed.
void DotsSimplifier::mercatorProject(QVector<double> &longitude, QVector<double> &latitude, QVector<double> &x,
                                     QVector<double> &y)
{
    // Check if input position array is of the same size.
    Helper::checkIntEqual(longitude.count(), latitude.count());
    try
    {
        // Clear output variables.
        x.clear();
        y.clear();
        // Define the constant MERCATOR projection limits.
        const double MERCATOR_LATITUDE_LB = 2.5*2.0-M_PI/2;
        const double MERCATOR_LATITUDE_UB = 87.5*2.0-M_PI/2;
        int pointCount = longitude.count();
        if(pointCount<=0)
        {
            return;
        }

        // Calculate the average scale factor.
        const double earthRadius = 6378100.0;
        double rx, ry, sf=0;// Longitude/latitude in radian.
        for(int i=0; i<pointCount; ++i)
        {
            sf += 1.0/qCos(qDegreesToRadians(latitude[i]));
        }
        sf/=pointCount;
        sf = qRound(sf/SCALE_FACTOR_PRECISION)*SCALE_FACTOR_PRECISION;

        // Do projection.
        for(int i=0; i<pointCount; ++i)
        {
            rx = qDegreesToRadians(longitude[i]);
            ry = qDegreesToRadians(Helper::limitVal(latitude[i], MERCATOR_LATITUDE_LB, MERCATOR_LATITUDE_UB));
            ry = qLn(qFabs(qTan(ry)+1.0/qCos(ry)));

            x.append(rx*earthRadius/sf);
            y.append(ry*earthRadius/sf);
        }
    }
    catch(DotsException &e)
    {
        e.raise();
    }
    catch (QException &)
    {
        DotsException("Error occured when doing mercator projection.").raise();
    }
}

void DotsSimplifier::normalizeData(QVector<double> &x, bool byMean)
{
    // Check if array is empty.
    if (x.empty())
        return;

    // Get the calibrate value.
    double cal = 0;
    if (byMean)
    {
        foreach (double px, x) {
            cal += px;
        }
        cal /= x.count();
    }
    else
    {
        cal = x[0];
    }

    // Update data.
    for (int i=0; i<x.count(); ++i)
    {
        x[i] = x[i]-cal;
    }
}


