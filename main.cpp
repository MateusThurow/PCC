#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include "./lib/Utils.hpp"
#include "./lib/Point.hpp"
#include "./lib/Cluster.hpp"
#include "./lib/AgglomerativeClustering.hpp"
#include "tbb/tbb.h"
#include "tbb/tick_count.h"


using namespace std;

int main(int argc, char const *argv[])
{
	tbb::tick_count tick_start, tick_end;
	vector<Point*> points;
	AgglomerativeClustering* AC;
	Cluster* c;
	clock_t start, end;
	int num_threads = 1;
	int num_points = 10;
	int times = 0;

	// cout << "Points,Threads,SequentialTime,OPENMP,TBB,CILK_PLUS,C++11" << endl;

	for(num_points = 10; num_points < 100; num_points = num_points *10){
		
		points = generatePoints(num_points);
		
		for (num_threads = 2; num_threads < 10; num_threads = num_threads *2){

			tbb::task_scheduler_init init(num_threads);

			for(times = 0; times < 10; times++) {


				cout << num_points << "," << num_threads << ",";
				// AC = new AgglomerativeClustering(points);
				// start = clock();
				// c = AC->start();
				// end = clock();

				// // cout << ((double) end-start) / CLOCKS_PER_SEC << ",";
				// cout << "Cluster Seq" << endl << c->getName() << endl;
				
				// AC = new AgglomerativeClustering(points);
				// start = clock();
				// c = AC->openMPStart(num_threads);
				// end = clock();

				// cout << ((double) end - start) / CLOCKS_PER_SEC << "," <<endl;
				// cout << "Cluster OPMP" << endl << c->getName() << endl;

				// AC = new AgglomerativeClustering(points);
				// start = clock();
				// // tick_start = tbb::tick_count::now();
				// c = AC->TBBStart();
				// // tick_end = tbb::tick_count::now();
				// end = clock();

				// // cout << ((double) end - start) / CLOCKS_PER_SEC << ",";
				// // cout << (tick_end - tick_start).seconds() << ",";
				// cout << "Cluster  TBB" << endl << c->getName() << endl;
				// // 
				
				AC = new AgglomerativeClustering(points);
				start = clock();
				c = AC->cilkStart(num_threads);
				end = clock();

				cout << ((double) end - start) / CLOCKS_PER_SEC << ",";
				// cout << "Cluster Cilk" << endl << c->getName() << endl;
				// // 
				// AC = new AgglomerativeClustering(points);
				// start = clock();
				// c = AC->cpp11Start();
				// end = clock();

				// cout << "Cluster CPP11" << endl << c->getName() << endl;
				// 
				// cout << ((double) end - start) / CLOCKS_PER_SEC << endl;
			}

		}

	}	

	return 0;
}