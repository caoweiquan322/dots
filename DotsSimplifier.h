/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

/**
  * @file
  * @brief DotsSimplifier.h defines the DotsSimplifier class.
  * @author caoweiquan322
  * @date 2015/12/14
  * @version 1.0
  */
#ifndef DOTSSIMPLIFIER_H
#define DOTSSIMPLIFIER_H

#include <QObject>
#include<QString>

/**
 * @brief The DotsSimplifier class implements the trajectory simplification algorithm DOTS.
 *
 * DOTS is logogram of "Directed acyclic graph based Online Trajectory Simplification algorithm".
 * Please refer to my paper for details. The DOTS algorithm has several advantages:
 *
 * First, it works in an online manner. So it would adapts to streaming data well.
 * Second, it's a near-optimal algorithm. As a result, the accuracy is compared to those batch mode algorithms
 * like TS/MRPA.
 * Third, it solves not only the min-# problem but also (partially) the min-e problem.
 * Forth, the time cost is relatively low. The time complexity is O(N/M) for each input point. Note that N/M
 * represents the simplification or compression rate.
 */
class DotsSimplifier : public QObject
{
    Q_OBJECT
public:
    // Regular member methods.
    /**
     * @brief DotsSimplifier
     * @param parent
     */
    explicit DotsSimplifier(QObject *parent = 0);

    /**
     * @brief setParameters
     * @param lssdTh
     */
    void setParameters(double lssdTh);

    /**
     * @brief feedData
     * @param x
     * @param y
     * @param t
     */
    inline void feedData(double x, double y, double t)
    {
        //
    }

    /**
     * @brief readOutputData checks if the simplifier outputs any data after the recent feeds. Output data will be
     * stored in corresponding parameters if returned true.
     * @param x the x value to output.
     * @param y the y value to output.
     * @param t the timestamp to output.
     * @return true if there's output data, false otherwise.
     */
    inline bool readOutputData(double &x, double &y, double &t)
    {
        return false;
    }

    // Static methods.
    /**
     * @brief parseMOPSI
     * @param fileName
     * @param x
     * @param y
     * @param t
     */
    static void parseMOPSI(QString fileName, QVector<double> &x, QVector<double> &y, QVector<double> &t);

    /**
     * @brief mercatorProject
     * @param longitude
     * @param latitude
     * @param x
     * @param y
     */
    static void mercatorProject(QVector<double> &longitude, QVector<double> &latitude, QVector<double> &x,
                                QVector<double> &y);

    /**
     * @brief normalizeData
     * @param x
     */
    static void normalizeData(QVector<double> &x);

protected:
    /**
     * @brief lssdTh
     */
    double lssdTh;

    /**
     * @brief MOPSI_DATETIME_FORMAT
     */
    static const QString MOPSI_DATETIME_FORMAT;

signals:

public slots:
};

#endif // DOTSSIMPLIFIER_H
