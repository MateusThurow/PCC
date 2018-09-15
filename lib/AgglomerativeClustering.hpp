#include <vector>
#include <mutex>	
#include <thread>
#include "Cluster.hpp"
#include "Point.hpp"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/mutex.h"
#include "cilk/cilk.h"

using namespace std;

#ifndef agglomerative_class
#define agglomerative_class

// mutex cppMutex;

class AgglomerativeClustering {

private:

	vector<Cluster*> clusters;
	vector<Cluster*> fromPointsToClusters (vector<Point*> points);
	tbb::mutex tbbMutex;


public:

	AgglomerativeClustering(vector<Point*> points);
	Cluster* start();
	Cluster* TBBStart();
	Cluster* openMPStart(int numThreads);
	Cluster* cilkStart(int numThreads);
	Cluster* cpp11Start();
	vector<Cluster*> getClusters();
	void removeRepetitions();
	
};
	
	void funcTeste(int i);

	void cppClusterize(vector<Cluster*>* clusters, vector<Cluster*>* clusters_aux, int i);

#endif