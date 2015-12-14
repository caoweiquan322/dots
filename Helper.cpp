/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include "Helper.h"
#include "DotsException.h"

Helper::Helper(QObject *parent) : QObject(parent)
{
}

void Helper::checkNotNullNorEmpty(QString name, QString value)
{
    if (value.isNull() || value.trimmed().isEmpty())
    {
        DotsException(QString("%1 must NOT be null nor empty.").arg(name)).raise();
    }
}

void Helper::checkIntEqual(int a, int b)
{
    if(a!=b)
    {
        DotsException(QString("Expected equal values but got %1 and %2.").arg(a).arg(b)).raise();
    }
}

