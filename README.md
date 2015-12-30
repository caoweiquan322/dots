### DOTS - Directed acyclic graph based Online Trajectory Simplification algorithm

Please refer to my paper for details. The DOTS algorithm has several advantages:
* It works in an online manner. So it would adapts to streaming data well.
* It's a near-optimal algorithm. As a result, the accuracy is compared to those batch mode algorithms like TS/MRPA.
* It solves not only the min-# problem but also (partially) the min-e problem.
* The time cost is relatively low. The time complexity is O(N/M) for each input point. Note that N/M represents the simplification or compression rate.