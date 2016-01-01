/* Copyright © 2015 DynamicFatty. All Rights Reserved. */

#include "DotsSimplifier.h"
#include"Helper.h"
#include"DotsException.h"
#include<QVector>

DotsSimplifier::DotsSimplifier(QObject *parent) : QObject(parent)
{
    lssdTh = 10000.0;
    lssdUpperBound = lssdTh*2.0;
    maxVkSize = 1e6;
    resetInternalData();
}

void DotsSimplifier::setParameters(double lssdTh, double k, int maxVkSize)
{
    this->lssdTh = lssdTh;
    lssdUpperBound = lssdTh*k;
    this->maxVkSize = maxVkSize;
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
    terminated.clear();
    numTerminated = 0;
    pathK.clear();
    issed.clear();
    parents.clear();

    // Input/output queue position.
    simplifiedIndex.clear();
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
        // Run DAG search once again to finish the simplification work.
        directedAcyclicGraphSearch();
    }
}
