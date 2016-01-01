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
#include<QVector>
#include"DotsException.h"

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
     * @brief DotsSimplifier is the default constructor.
     * @param parent is the QT parent object.
     */
    explicit DotsSimplifier(QObject *parent = 0);

    /**
     * @brief setParameters specifies DOTS settings for trajectory simplification.
     * @param lssdTh is the LSSD threshold for DAG searching.
     * @param k is the factor for upper bound. LSSD that exceeds lssdTh*k would be ignored by DAG searching.
     * @param maxVkSize is the maximum size of each layer of DAG tree.
     */
    void setParameters(double lssdTh, double k = 2.0, int maxVkSize = 1e6);

    /**
     * @brief resetInternalData resets all internal data structures for DOTS algorithm.
     */
    void resetInternalData();

    /**
     * @brief feedData feeds a 2D spatio temporary point to DOTS.
     * @param x is the x position.
     * @param y is the y position.
     * @param t is the timestamp.
     */
    inline void feedData(double x, double y, double t)
    {
        if (finished)
            DotsException("Feeding data is NOT allowed after the simplifier finished. "\
                          "Suggest calling resetInternalData() first.").raise();

        // Store data.
        ptx.append(x);
        pty.append(y);
        ptt.append(t);
        // Update statistics.
        if (xSum.empty())
        {
            xSum.append(x);
            ySum.append(y);
            tSum.append(t);
            x2Sum.append(x*x);
            y2Sum.append(y*y);
            t2Sum.append(t*t);
            xtSum.append(x*t);
            ytSum.append(y*t);

            // Setup the initial vK set {0}.
            vK.append(0);
            terminated.append(false);
            numTerminated = 0;

            // Setup the following-path for viterbi decoding.
            QVector<int> path;
            path.append(0);
            pathK.append(path);

            // Set input/output queue.
            inputCount = 1;
            outputCount = 0;
            simplifiedIndex.append(0);
        }
        else
        {
            int count = xSum.count()-1;
            xSum.append(xSum[count]+x);
            ySum.append(ySum[count]+y);
            tSum.append(tSum[count]+t);
            x2Sum.append(x2Sum[count]+x*x);
            y2Sum.append(y2Sum[count]+y*y);
            t2Sum.append(t2Sum[count]+t*t);
            xtSum.append(xtSum[count]+x*t);
            ytSum.append(ytSum[count]+y*t);
        }
        // Initialize issed&parents.
        issed.append(0);
        parents.append(-1);
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
        // Run DAG search to produce potentially more output data.
        if (!finished && outputCount >= simplifiedIndex.count())
            directedAcyclicGraphSearch();

        // Retrieve one data.
        if (outputCount < simplifiedIndex.count())
        {
            int idx = simplifiedIndex[outputCount];
            x = ptx[idx];
            y = pty[idx];
            t = ptt[idx];
            ++outputCount;
            return true;
        }
        // No output yet.
        return false;
    }

    /**
     * @brief getSimplifiedIndex retrieves index of the i-th point of simplified trajectory.
     * @param i the point number of simplified trajectory to retrieve.
     * @return index of the i-th point.
     */
    inline int getSimplifiedIndex(int i)
    {
        if (i<0 || i>=simplifiedIndex.count())
            DotsException(QString("Index %1 is out of range [0, %2)").arg(i).arg(simplifiedIndex.count())).raise();

        return simplifiedIndex.at(i);
    }

    /**
     * @brief finish sets the finish flag for DOTS algorithm. No more data could be feeded after calling this method.
     */
    void finish();

protected:
    /**
     * @brief directedAcyclicGraphSearch does a DAG search among the feeded spatio-temporal 2D data. The output queue
     * would be updated when appropriate.
     */
    inline void directedAcyclicGraphSearch()
    {
        int numPoints = ptx.count();
        if (finished)
        {
            // Construct the DAG completely.
            while (true)
            {
                if (inputCount>=numPoints)
                    break;

                // Move v* points to vL.
                for (int i=inputCount; i<numPoints; ++i)
                {
                    // Update vK if parent was not assigned yet.
                    if (parents.at(i) < 0)
                    {
                        for (int j=0;j<vK.count(); ++j)
                        {
                            int jIndex = vK.at(j);
                            if (!terminated.at(j))
                            {
                                double distance = getLSSD(jIndex,i);
                                if (distance < lssdTh)
                                {
                                    vL.append(i);
                                    issed[i] = issed[jIndex]+distance;
                                    parents[i] = jIndex;
                                    break;
                                }
                                else if (distance > lssdUpperBound)
                                {
                                    terminated[j] = true;
                                    ++numTerminated;

                                    // Check if all vK terminated.
                                    if (needUpdateVK())
                                    {
                                        break;
                                    }
                                }
                            } // if (!terminated.at(j))
                        } // for (int j=0;j<vK.count(); ++j)
                    } // if (parents.at(i) < 0)

                    // Update inputCount for consequent none v* points.
                    if(parents.at(i)>=0 && inputCount == i)
                    {
                        // Update start point of DAG search.
                        ++inputCount;
                    }

                    // Terminate loop early if we need to update vK.
                    if (needUpdateVK())
                        break;
                } // for (int i=inputCount; i<numPoints; ++i)

                // Minimize ISSED.
                minimizeISSED();

                // Force swap vK/vL no matter we need update vK.
                updateVK();

                // Omit viterbi decoding!!
            }

            // Decode the simplified data from back to front.
            QVector<int> temp;
            int idx = numPoints-1;
            int currentIndex = -1;
            if (!simplifiedIndex.empty())
                currentIndex = simplifiedIndex[simplifiedIndex.count()-1];
            while(idx>currentIndex)
            {
                temp.append(idx);
                idx = parents.at(idx);
            }
            for (int k=temp.count()-1; k>=0; --k)
                simplifiedIndex.append(temp.at(k));
        } // if (finished)
        else
        {
            while (true)
            {
                bool vKUpdated = false;
                if (inputCount>=numPoints)
                    break;

                // Move v* points to vL.
                for (int i=inputCount; i<numPoints; ++i)
                {
                    // Update vK if parent was not assigned yet.
                    if (parents.at(i) < 0)
                    {
                        for (int j=0;j<vK.count(); ++j)
                        {
                            int jIndex = vK.at(j);
                            if (!terminated.at(j))
                            {
                                double distance = getLSSD(jIndex,i);
                                if (distance < lssdTh)
                                {
                                    vL.append(i);
                                    issed[i] = issed[jIndex]+distance;
                                    parents[i] = jIndex;

                                    // Check if vL exceeds max size.
                                    if (needUpdateVK())
                                    {
                                        // Minimize ISSED.
                                        minimizeISSED();

                                        // Swap vK/vL
                                        updateVK();

                                        // Viterbi decoding.
                                        viterbiDecode();

                                        // Break search loop.
                                        vKUpdated = true;
                                    }
                                    break;
                                }
                                else if (distance > lssdUpperBound)
                                {
                                    terminated[j] = true;
                                    ++numTerminated;

                                    // Check if all vK terminated.
                                    if (needUpdateVK())
                                    {
                                        // Minimize ISSED.
                                        minimizeISSED();

                                        // Swap vK/vL
                                        updateVK();

                                        // Viterbi decoding.
                                        viterbiDecode();

                                        // Break search loop.
                                        vKUpdated = true;
                                        break;
                                    }
                                }
                            } // if (!terminated.at(j))
                        } // for (int j=0;j<vK.count(); ++j)
                    } // if (parents.at(i) < 0)

                    // Update inputCount for consequent none v* points.
                    if(parents.at(i)>=0 && inputCount == i)
                    {
                        // Update start point of DAG search.
                        ++inputCount;
                    }

                    if (vKUpdated)
                        break;
                } // for (int i=inputCount; i<numPoints; ++i)

                // Stop DAG search if no new vK generated.
                if (!vKUpdated)
                    break;
            }
        }
    }

    /**
     * @brief getLSSD calculates the LSSD (logogram of Local integral Square Synchronous Euclidean Distance ) between
     * two points indexed by fst and lst.
     * @param fst is index of the first point.
     * @param lst is index of the second point.
     * @return the LSSD distance.
     */
    inline double getLSSD(int fst, int lst)
    {
        if (fst+1>=lst)
            return 0;
        if (fst<0 || lst>=xSum.count())
            DotsException(QString("Index out of bound error.")).raise();

        int plst = lst-1;
        double c1x = ptx[fst]*ptt[lst]-ptx[lst]*ptt[fst];
        double c2x = c1x*c1x;
        double c3x = ptt[lst]-ptt[fst];
        double c4x = c3x*c3x;
        double c5x = ptx[lst]-ptx[fst];
        double c6x = c5x*c5x;

        double c1y = pty[fst]*ptt[lst]-pty[lst]*ptt[fst];
        double c2y = c1y*c1y;
        double c3y = c3x;
        double c4y = c3y*c3y;
        double c5y = pty[lst]-pty[fst];
        double c6y = c5y*c5y;

        double distance = (plst-fst)*c2x/c4x
                + c6x/c4x*(t2Sum[plst]-t2Sum[fst])
                + (x2Sum[plst]-x2Sum[fst])
                + 2*c1x*c5x/c4x*(tSum[plst]-tSum[fst])
                - 2*c1x/c3x*(xSum[plst]-xSum[fst])
                - 2*c5x/c3x*(xtSum[plst]-xtSum[fst])
                + (plst-fst)*c2y/c4y
                + c6y/c4y*(t2Sum[plst]-t2Sum[fst])
                + (y2Sum[plst]-y2Sum[fst])
                + 2*c1y*c5y/c4y*(tSum[plst]-tSum[fst])
                - 2*c1y/c3y*(ySum[plst]-ySum[fst])
                - 2*c5y/c3y*(ytSum[plst]-ytSum[fst]);
        return distance;
    }

    /**
     * @brief needUpdateVK Checks if we need to swap Vl and Vk sets.
     * @return true if a swap operation is necessary, false otherwise.
     */
    inline bool needUpdateVK()
    {
        return (vK.count() == numTerminated || vL.count()>= maxVkSize);
    }

    /**
     * @brief updateVK swaps Vl and Vk sets and updates the DAG paths from root node to each elements of current Vk set.
     */
    inline void updateVK()
    {
        // Update following-path.
        QVector<QVector<int>> newPath;
        for (int k=0; k<vL.count(); ++k)
        {
            int indexK = -1;
            int parentPos = parents.at(vL.at(k));
            for (int m=0; m<vK.count(); ++m)
            {
                if (vK.at(m) == parentPos)
                {
                    indexK = m;
                    break;
                }
            }

            QVector<int> p = pathK.at(indexK);
            p.append(vL.at(k));
            newPath.append(p);
        }
        pathK = newPath;

        // Update vK set.
        vK = vL;
        terminated.resize(vK.count());
        for (int k=0; k<terminated.count(); ++k)
            terminated[k] = false;
        numTerminated = 0;
        vL.clear();
    }

    /**
     * @brief minimizeISSED minimizes the total error from root node to each element of Vl set. The minimization is
     * done by choosing the best parents of Vl elements among Vk elements.
     */
    inline void minimizeISSED()
    {
        foreach (int i, vL) {
            double minDistance = issed.at(i);
            double minParent = parents.at(i);
            foreach (int j, vK) {
                double distance = issed.at(j) + getLSSD(j,i);
                if (distance<minDistance)
                {
                    minDistance = distance;
                    minParent = j;
                }
            }
            issed[i] = minDistance;
            parents[i] = minParent;
        }
    }

    /**
     * @brief viterbiDecode decodes output indices of simplified points in a viterbi-like manner.
     */
    inline void viterbiDecode()
    {
        int inputLayer = pathK.at(0).count();
        int startLayer = simplifiedIndex.count();
        while (startLayer < inputLayer) {
            bool equal = true;
            int reference = pathK.at(0).at(startLayer);
            foreach (QVector<int> path, pathK) {
                if (path.at(startLayer) != reference)
                {
                    equal = false;
                    break;
                }
            }
            if (!equal)
                break;

            simplifiedIndex.append(reference);
            ++startLayer;
        }
    }

protected:
    // DOTS settings.
    /**
     * @brief lssdTh
     */
    double lssdTh;
    double lssdUpperBound;
    int maxVkSize;

    // Input sequence.
    QVector<double> ptx, pty, ptt;

    // DOTS algorithm internal data.
    QVector<double> xSum, ySum, tSum, x2Sum, y2Sum, t2Sum, xtSum, ytSum;
    QVector<double> vK,vL;
    QVector<bool> terminated;
    int numTerminated;
    QVector<QVector<int>> pathK;
    QVector<double> issed;
    QVector<int> parents;

    // Output sequence.
    QVector<int> simplifiedIndex;
    int inputCount;
    int outputCount;

    // Indicates if the input got EOF. No more data could be input after this flag was set.
    bool finished;

signals:

public slots:
};

#endif // DOTSSIMPLIFIER_H
