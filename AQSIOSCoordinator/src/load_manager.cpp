
#include <iostream>
#include <list>
#include <sstream>
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

	pthread_mutex_lock(&mutexNodeInfos);
	//check if the node exists

	list<Node_Info>::const_iterator iter;
	for(iter = nodes.begin(); iter!= nodes.end(); iter++){
		if((*iter).nodeID ==node_id){
			pthread_mutex_unlock(&mutexNodeInfos);
			return -1;
		}
	}

	Node_Info node(this->num_of_classes);
	node.nodeID = node_id;
	nodes.push_back(node);
	pthread_mutex_unlock(&mutexNodeInfos);
	return 0;
}

int LoadManager::removeNode(int node_id){

	pthread_mutex_lock(&mutexNodeInfos);
	list<Node_Info>::iterator iter = nodes.begin();

	while(iter != nodes.end() && (*iter).nodeID != node_id){
		iter++;
	}
	if(iter == nodes.end()){ //not found
		pthread_mutex_unlock(&mutexNodeInfos);
		return -1;
	}
	//found
	nodes.erase(iter);
	pthread_mutex_unlock(&mutexNodeInfos);
	return 0;
}

int LoadManager::updateNodeInfo(int nodeID, char*msg){

	pthread_mutex_lock(&mutexNodeInfos);
	list<Node_Info>::iterator iter = nodes.begin();
	while(iter!=nodes.end() && (*iter).nodeID !=nodeID)
		iter++;

	if(iter == nodes.end()){
		pthread_mutex_unlock(&mutexNodeInfos);
		return -1;
	}
	//tell the node to extract the information and update itself
	if((*iter).update(msg)!=0){
		pthread_mutex_unlock(&mutexNodeInfos);
		return -1;
	}
	pthread_mutex_unlock(&mutexNodeInfos);
	return 0;
}
bool LoadManager::isAllNodesReady(){

	pthread_mutex_lock(&mutexNodeInfos);
	list<Node_Info>::const_iterator iter = nodes.begin();
	while(iter!=nodes.end() && (*iter).isReady == true)
		iter++;
	if(iter==nodes.end()){
		pthread_mutex_unlock(&mutexNodeInfos);
		return true;
	}
	else {
		pthread_mutex_unlock(&mutexNodeInfos);
		return false;
	}
}
void LoadManager::redistributeLoad(std::map<int, std::string>* migrationDecision){
	pthread_mutex_lock(&mutexNodeInfos);
	//TODO: consider copying the list of node info to a snapshot for migration decision making
	//avoiding holding lock on the list for too long.
	//TODO: global load redistribution goes here
	//now for testing purpose only
	list<Node_Info>::const_iterator it, itMax, itMin;
	it = nodes.begin();
	itMax = it;
	itMin = it;
	it++;
	while(it!=nodes.end()){
		if((*it).getTotalCapacityUsage()> (*itMax).getTotalCapacityUsage())
			itMax = it;
		if((*it).getTotalCapacityUsage()< (*itMin).getTotalCapacityUsage())
			itMin = it;
		it++;
	}
	if(itMax!=itMin){
		//format QM,<destination ip>,<destination port>,<queryID1>,<queryID2>...,E)
		std::stringstream ss;
		ss<<"QM,";
		ss<<(*itMin).getNodeAddress()<<",";
		ss<<(*itMin).listeningPort<<",";
		//test moving all queries from max load to min load
		list<Node_Info::QueryInfo>::const_iterator qit;
		for(qit =  (*itMax).activeQueries.begin(); qit!=(*itMax).activeQueries.end(); qit++){
			ss<<(*qit).queryID<<",";
		}
		ss<<"E";
		migrationDecision->insert(std::pair<int,string>((*itMax).nodeID,ss.str()));
	}
	pthread_mutex_unlock(&mutexNodeInfos);
}
