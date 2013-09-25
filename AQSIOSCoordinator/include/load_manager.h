/*
 * load_manager.h -- global load manager
 *
 *  Created on: Feb 1, 2013
 *      Author: thao pham
 */
#include <iostream>
#include <list>
#include <pthread.h>
#ifndef NODE_INFO_H_
#include "node_info.h"
#endif

#ifndef LOAD_MANAGER_H_
#define LOAD_MANAGER_H_
#endif


using namespace std;

class LoadManager{
private:
	list<Node_Info> nodes;
	int num_of_classes;
	pthread_mutex_t mutexNodeInfos;
public:
	LoadManager(int num_classes){
		this->num_of_classes = num_classes;
		pthread_mutex_init(&mutexNodeInfos, NULL);
	}
	int addNode(int nodeID); //a object of this class is shared between the load management thread and the Coordinator thread, so mutex is needed
	int removeNode(int nodeID);
	int updateNodeInfo(int nodeID, char* msg);
	//check if all the nodes is ready to execute
	bool isAllNodesReady();
	//this class also contains function that redistribute workload;
};
