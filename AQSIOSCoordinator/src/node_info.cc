/*
 * node_info.cc
 *
 *  Created on: Jan 10, 2013
 *      Author: thaopham
 */
#ifndef NODE_INFO_H_
#include "node_info.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<iostream>
#include <arpa/inet.h>

#define aging_factor  0.2

using namespace std;
int Node_Info::set_capacity(int class_id, double capacity){
	if ((class_id < 0) or (class_id > this->number_of_classes))
		return -1;
	else
		this->capacities[class_id] = capacity;
	return 0;
}

int Node_Info::set_capacity_usage(int class_id, double capacity_usage){
	if ((class_id < 0) or (class_id > this->number_of_classes))
		return -1;
	else
		this->capacity_usages[class_id] = capacity_usage;
	return 0;

}

int Node_Info::set_local_priority(int class_id, double local_priority){
	if ((class_id < 0) or (class_id > this->number_of_classes))
		return -1;
	else
		this->local_priorities[class_id] = local_priority;
	return 0;
}

int Node_Info::update(char* msg){

	//check the header of the message
	if(strncmp(msg,"NI",2)==0)//node's load info
		return updateLoadInfo(msg);
	if(strncmp(msg,"P",1)==0) // port number
		return updateListeningPortNo(msg);
	return -1;
}
int Node_Info::updateLoadInfo(char* msg){
	//trim the msg header
	int len = strchr(msg,',') - msg;
	//the next value is the info type
	msg = msg+len+1;
	int msgType = strtod(msg,&msg);
	//skip the comma
	msg++;
	if(msgType==0/*all*/||msgType ==1/*class level info*/){
		//extract the current capacity of each class
		int i = 0;
		while(i<number_of_classes){
			//after this call msg points to the next comma
			double newCapacity = strtod(msg, &msg);
			if(capacities[i] < 0.01 /*==0 does not always work for double*/)
				capacities[i] = newCapacity;
			else capacities[i] = capacities[i]*(1-aging_factor) + aging_factor*newCapacity;
			//skip the comma
			msg++;
			i++;
		}

		//extract the capacity usage of each class
		i=0;
		while(i<number_of_classes){
			double newCapUsage = strtod(msg, &msg);
			if(capacity_usages[i] < 0.01)
				capacity_usages[i] = newCapUsage;
			else capacity_usages[i] = capacity_usages[i]*(1-aging_factor) +aging_factor*newCapUsage;
			msg++;
			i++;
		}

		//extract the original priority

		i=0;
		while(i<number_of_classes){
			local_priorities[i] = strtod(msg,&msg); //this is the current only, actually this might not be used at all
			msg++;
			i++;
		}
		//for testing purpose: print the node info
		for(i=0;i<number_of_classes; i++){
			printf("class %d: current capacity %0.2f, capacity usage %0.2f, local priority %0.2f \n", i, capacities[i], capacity_usages[i], local_priorities[i]);
		}
	}
	if(msgType==0 || msgType ==2/*active query info only*/)
	{
		while(strcmp(msg,"E")!=0)//not end of message yet
		{
			//queryID
			int queryID = strtol(msg,&msg, 10);
			msg++;
			//query load (i.e., query's capacity usage
			double queryLoad = strtod(msg, &msg);
			msg++;
			std::list<QueryInfo>::iterator it = activeQueries.begin();
			while(it !=activeQueries.end()&&(*it).queryID != queryID)
				it++;
			if(it!=activeQueries.end()) //the entry already existed
			{
				if((*it).queryLoad <0.01)
					(*it).queryLoad = queryLoad;
				else
					(*it).queryLoad = (*it).queryLoad*(1-aging_factor)+aging_factor*queryLoad;
				//testing
				printf("queryID: %d; accumulated load: %0.2f; current load: %0.2f\n", (*it).queryID,(*it).queryLoad, queryLoad);
			}
			else {//has not existed, add to the list
				QueryInfo queryInfo;
				queryInfo.queryID = queryID;
				queryInfo.queryLoad = queryLoad;
				activeQueries.push_back(queryInfo);
			}

		}
	}
	return 0;
}

int Node_Info::updateListeningPortNo(char* msg)
{
	int len = strchr(msg,',') - msg;
	//the next value after the comma is the port number
	msg = msg+len+1;
	listeningPort = strtod(msg,&msg);
	//testing
	printf("node's listening port: %d \n",listeningPort);
	return 0;
}

double Node_Info::getTotalCapacityUsage()const {

	double totalCapacityUsage =0;

	for(int i=0;i<number_of_classes;i++){
		totalCapacityUsage += capacity_usages[i];
	}
	return totalCapacityUsage;
}
std::string Node_Info::getNodeAddress() const{
	sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	getpeername(nodeID,(sockaddr*) &peeraddr, &peerlen);
	char str[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &peeraddr.sin_addr.s_addr, str, INET_ADDRSTRLEN );
	return str;
}
