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

    static inline double limitVal(double val, double lb, double ub)
    {
        if (val<lb)
            val = lb;
        if (val>ub)
            val = ub;
        return val;
    }

signals:

public slots:
};

#endif // HELPER_H
