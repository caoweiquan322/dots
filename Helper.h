/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#ifndef HELPER_H
#define HELPER_H

#include <QObject>

/**
 * @brief The Helper class provides some utilities for the package. E.g. parameter checking, logging, etc.
 */
class Helper : public QObject
{
    Q_OBJECT
protected:
    /**
     * @brief Helper is the protected constructor to prevent from instancing this class.
     * @param parent is the parent of this QObject.
     */
    explicit Helper(QObject *parent = 0);

public:
    /**
     * @brief checkNotNullNorEmpty validates the specified value. A DotsException would be raised if value is null
     * or empty.
     * @param name is the name of the variable to check.
     * @param value is the value of the variable to check.
     */
    static void checkNotNullNorEmpty(QString name, QString value);

    /**
     * @brief checkIntEqual checks if the specified integers are equal or not. A DotsException would be raised if a does
     * not equal to b.
     * @param a is the 1st integer to check.
     * @param b is the 2nd integer to check.
     */
    static void checkIntEqual(int a, int b);

    /**
     * @brief limitVal truncates val to be within range [lb, ub].
     * @param val is the value to limit.
     * @param lb is the lower bound.
     * @param ub is the upper bound.
     * @return the truncated value.
     */
    static inline double limitVal(double val, double lb, double ub)
    {
        if (val<lb)
            val = lb;
        if (val>ub)
            val = ub;
        return val;
    }

    /**
     * @brief parseMOPSI parses a MOPSI data file. Reference to http://cs.joensuu.fi/mopsi for details of
     * MOPSI dataset.
     * @param fileName is the file name of MOPSI format.
     * @param x is the x values of trajectory points.
     * @param y is the y values of trajectory points.
     * @param t is the timestamps of trajectory points.
     */
    static void parseMOPSI(QString fileName, QVector<double> &x, QVector<double> &y, QVector<double> &t);

    /**
     * @brief mercatorProject does mercator projection on the longitude/latitude pairs.
     * @param longitude is the longitude of positions.
     * @param latitude is the latitude of positions.
     * @param x is the x values of positions.
     * @param y is the y values of positions.
     */
    static void mercatorProject(QVector<double> &longitude, QVector<double> &latitude, QVector<double> &x,
                                QVector<double> &y);

    /**
     * @brief normalizeData normalizes data array by sutracts values by the mean or the first value. Method behavior
     * is controlled by parameter byMean.
     * @param x is the data array
     * @param byMean specifies by which value to subtracts each element. Set it to true (DEFAULT) if use the mean
     * value and false if use the first value.
     */
    static void normalizeData(QVector<double> &x, bool byMean = true);

protected:
    /**
     * @brief MOPSI_DATETIME_FORMAT
     */
    static const QString MOPSI_DATETIME_FORMAT;
    static const double SCALE_FACTOR_PRECISION;

signals:

public slots:
};

#endif // HELPER_H
