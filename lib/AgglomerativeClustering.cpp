#include "AgglomerativeClustering.hpp"
#include <iostream>
#include <mutex>
#include <omp.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

using namespace std;

mutex cppMutex;

AgglomerativeClustering::AgglomerativeClustering (vector<Point*> points) {
	this->clusters = this->fromPointsToClusters(points);
}

Cluster* AgglomerativeClustering::start () {

	while(this->clusters.size() != 1) {

		vector<Cluster*> clusters_aux (0);
		int i = 0;

		while(i < this->clusters.size()) {
			Cluster* nearest = this->clusters[i]->findNearest(this->clusters);

			if(nearest->findNearest(this->clusters) == this->clusters[i]) {
				
				Cluster* cluster = new Cluster(nearest, this->clusters[i]);

				if(!cluster->isThere(clusters_aux)){

					clusters_aux.push_back(cluster);

				}

			} else {

				clusters_aux.push_back(clusters[i]);

			}

			i++;
		}

		this->clusters = clusters_aux;
	}

	return this->clusters[0];
}

struct Clusterize {
	
	vector<Cluster*> in_clusters;
	vector<Cluster*>* out_clusters;

	void operator () (const tbb::blocked_range<int>& range) const {

		for(int i = range.begin(); i != range.end(); i++) {

			Cluster* nearest = in_clusters[i]->findNearest(in_clusters);

			if(nearest->findNearest(in_clusters) == in_clusters[i]) {

				Cluster* cluster = new Cluster(nearest, in_clusters[i]);
				(*out_clusters)[i] = cluster;

			} else {

				(*out_clusters)[i] = in_clusters[i];

			}
		// 	cppMutex.lock();
		// 	cout << "dentro do loop sou o " << i << endl;
		// 	cppMutex.unlock();
		}
	
	}

};

Cluster* AgglomerativeClustering::TBBStart () {
	
	while(this->clusters.size() != 1) {
	// int teste = 1;
	// while(teste) {

		vector<Cluster*> clusters_aux (this->clusters.size());

		Clusterize clusterize;
		
		clusterize.in_clusters = this->clusters;
		clusterize.out_clusters = &clusters_aux;

		tbb::parallel_for(tbb::blocked_range<int>(0, this->clusters.size()),clusterize);

		this->clusters = clusters_aux;

		this->removeRepetitions();

		// cout << "fora do for só apareço uma vez" << endl;
		// teste = 0;
	}

	return this->clusters[0];

}

// void funcTeste (int i){

// 	cppMutex.lock();
// 	cout << " ola eu sou o " << i << endl;
// 	cppMutex.unlock();
// }


Cluster* AgglomerativeClustering::cpp11Start () {

	// vector<thread> ths;
	int teste = 0;
	while(this->clusters.size() != 1) {
	// while(teste != 1) {
		vector<thread> ths;
		vector<Cluster*> clusters_aux (0);
		for(int i = 0; i < this->clusters.size(); i++) {
		// for(int i = 0; i < 10; i++) {

			ths.push_back(thread(&cppClusterize, &(this->clusters), &(clusters_aux), i));

			// ths.push_back(thread(&funcTeste,i));
		
		}

		for(auto& th : ths) {
			th.join();
		}

		// teste = 1;
		this->clusters = clusters_aux;
		this->removeRepetitions();

	}

	return this->clusters[0];

}

void cppClusterize (vector<Cluster*>* clusters, vector<Cluster*>* clusters_aux, int i) {

	
	
	Cluster* nearest = (*clusters)[i]->findNearest((*clusters));

	if(nearest->findNearest((*clusters)) == (*clusters)[i]) {

		Cluster* cluster = new Cluster(nearest,(*clusters)[i]);
		cppMutex.lock();
		(*clusters_aux).push_back(cluster);
		cppMutex.unlock();

	} else {

		cppMutex.lock();
		(*clusters_aux).push_back((*clusters)[i]);
		cppMutex.unlock();

	}

}

Cluster* AgglomerativeClustering::openMPStart (int numThreads) {

	omp_set_dynamic(0); // permite controlar o numero de threads
	omp_set_num_threads(numThreads);
	while(this->clusters.size() != 1) {

		vector<Cluster*> clusters_aux (0);

		#pragma omp parallel for shared(clusters_aux)
		for (int i = 0; i < this-> clusters.size(); i++) {

			Cluster* nearest = this->clusters[i]->findNearest(this->clusters);

			if(nearest->findNearest(this->clusters) == this->clusters[i]) {

				Cluster* cluster = new Cluster(nearest,this->clusters[i]);
				#pragma omp critical(dataupdate) 
				{
					clusters_aux.push_back(cluster);
				}
			} else {
				#pragma omp critical(dataupdate)
				{
					clusters_aux.push_back(this->clusters[i]);
				}
			}
			// cout << "threads = " << omp_get_num_threads() << endl;
		}

		#pragma omp barrier

		this->clusters = clusters_aux;
		this->removeRepetitions();
		
	}
	
	return this->clusters[0];

}



Cluster* AgglomerativeClustering::cilkStart (int numThreads) {


	// char numero = numThreads + '0';
	// __cilkrts_set_param("nworkers",&numero);
	// export CILK_NWORKERS=4

	
	// cout << " t1 " << numThreads << " t2 " << numero << endl;
	while (this->clusters.size() != 1) {
	// int teste = 1;
	// while (teste) {

		vector<Cluster*> clusters_aux (0);

		cilk_for(int i = 0; i < this->clusters.size(); i++) {
		// cilk_for(int i = 0; i < 10; i++) {
			// funcTesteCilk(i);

			Cluster* nearest = this->clusters[i]->findNearest(this->clusters);

			if(nearest->findNearest(this->clusters) == this->clusters[i]) {

				Cluster* cluster = new Cluster(nearest, this->clusters[i]);

				tbbMutex.lock();
				clusters_aux.push_back(cluster);
				tbbMutex.unlock();
			} else {
				
				tbbMutex.lock();
				clusters_aux.push_back(this->clusters[i]);
				tbbMutex.unlock();

			}
			// tbbMutex.lock();
			// cout << "fora da função eu sou o " << i << endl;
			// tbbMutex.unlock();

			// cout << "threads = " << __cilkrts_get_nworkers() << endl;
		}

		this->clusters = clusters_aux;

		this->removeRepetitions();
		// teste  = 0;
	}

	return this->clusters[0];

}


vector<Cluster*> AgglomerativeClustering::fromPointsToClusters (vector<Point*> points) {

	vector<Cluster*> newClusters (0);

	for (int i = 0; i < points.size(); i++) {

		newClusters.push_back(new Cluster(points[i]));
		
	}

	return newClusters;

}

vector<Cluster*> AgglomerativeClustering::getClusters () {

	return this->clusters;

}

void AgglomerativeClustering::removeRepetitions () {

	for (int i = 0; i < this->clusters.size(); i++) {

		Cluster* cluster = this->clusters[i];
		vector<int> toRemove (0);

		for (int j = 0; j < this->clusters.size(); ++j) {

			if((j != i) && (cluster->getPoint()->x == this->clusters[j]->getPoint()->x) && 
				(cluster->getPoint()->y == this->clusters[j]->getPoint()->y)) {
				toRemove.push_back(j);
			}
			
		}
		for (int i = toRemove.size() - 1; i > -1; i--) {

			this->clusters.erase((this->clusters.begin() + toRemove[i]));
			
		}

	}

}







