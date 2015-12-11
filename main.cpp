/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include <QCoreApplication>
#include <QDebug>
#include "DotsSimplifier.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DotsSimplifier simplifier;
    qDebug()<<simplifier.objectName()<<"\n";

    return 0;//a.exec();
}

