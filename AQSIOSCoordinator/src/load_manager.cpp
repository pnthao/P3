
#include <iostream>
#include <list>
//#include<iterator>
#include "pthread.h"

#ifndef NODE_INFO_H_
#include "node_info.h"
#endif

#ifndef LOAD_MANAGER_H_
#include "load_manager.h"
#endif

using namespace std;

int LoadManager::addNode(int node_id){

	//check if the node exists
	//nodeID is not critical info, i.e., there is no possible read-write conflict situations: only one thread actually update this

	list<Node_Info>::const_iterator iter;
	for(iter = nodes.begin(); iter!= nodes.end(); iter++){
		if((*iter).nodeID ==node_id)
			return -1;
	}

	Node_Info node(this->num_of_classes);
	node.nodeID = node_id;
	pthread_mutex_lock(&mutexNodeInfos);
	nodes.push_back(node);
	pthread_mutex_unlock(&mutexNodeInfos);
	return 0;
}

int LoadManager::removeNode(int node_id){

	list<Node_Info>::iterator iter = nodes.begin();

	while(iter != nodes.end() && (*iter).nodeID != node_id){
		iter++;
	}
	if(iter == nodes.end()) //not found
		return -1;
	//found
	nodes.erase(iter);
	return 0;
}

int LoadManager::updateNodeInfo(int nodeID, char*msg){

	list<Node_Info>::iterator iter = nodes.begin();
	while(iter!=nodes.end() && (*iter).nodeID !=nodeID)
		iter++;

	if(iter == nodes.end()) return -1;
	//tell the node to extract the information and update itself
	if((*iter).update(msg)!=0) return -1;
	return 0;
}
bool LoadManager::isAllNodesReady(){
	list<Node_Info>::const_iterator iter = nodes.begin();
	while(iter!=nodes.end() && (*iter).isReady == true)
		iter++;
	if(iter==nodes.end()) return true;
	else return false;
}
