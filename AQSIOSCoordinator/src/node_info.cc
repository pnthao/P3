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

#define aging_factor  0.2

using namespace std;
int Node_Info::set_capacity(int class_id, double capacity){
	if ((class_id < 0) or (class_id > this->number_of_classes))
		return -1;
	else
		this->capacity[class_id] = capacity;
	return 0;
}

int Node_Info::set_capacity_usage(int class_id, double capacity_usage){
	if ((class_id < 0) or (class_id > this->number_of_classes))
			return -1;
		else
			this->capacity_usage[class_id] = capacity_usage;
	return 0;

}

int Node_Info::set_local_priority(int class_id, double local_priority){
	if ((class_id < 0) or (class_id > this->number_of_classes))
			return -1;
		else
			this->local_priority[class_id] = local_priority;
	return 0;
}

void Node_Info::serialize(char *msg){

	//msg format: <header code>,<capacity list>,<capacity usage list>,<local priority list>
	//header code
	strcpy(msg, "NI,"); //indicate that the msg contains node's information regular report;
	//supposing the coordinator knows the number of existing classes, no need to embed that info into the msg
	//starting the capacity report
	char tempstr[64];
	for(int i=0;i<number_of_classes; i++){
		sprintf(tempstr,"%0.2f",capacity[i]);
		strcat(msg,tempstr);
		strcat(msg,",");
	}
	//capacity usage report
	for (int i=0;i<number_of_classes; i++){
		sprintf(tempstr,"%0.2f",capacity_usage[i]);
		strcat(msg, tempstr);
		strcat(msg,",");
	}
	//local-priority -- local priority might not need to be reported, since it does not change unless the coordinator request the change
	for (int i=0;i<number_of_classes; i++){
		sprintf(tempstr,"%0.2f",local_priority[i]);
		strcat(msg, tempstr);
		strcat(msg,",");
	}
	//end of msg - is it necessary?
	strcat(msg, "E");

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
				if(capacity[i] < 0.01 /*==0 does not always work for double*/)
					capacity[i] = newCapacity;
				else capacity[i] = capacity[i]*(1-aging_factor) + aging_factor*newCapacity;
				//skip the comma
				msg++;
				i++;
			}

			//extract the capacity usage of each class
			i=0;
			while(i<number_of_classes){
				double newCapUsage = strtod(msg, &msg);
				if(capacity_usage[i] < 0.01)
					capacity_usage[i] = newCapUsage;
				else capacity_usage[i] = capacity_usage[i]*(1-aging_factor) +aging_factor*newCapUsage;
				msg++;
				i++;
			}

			//extract the original priority

			i=0;
			while(i<number_of_classes){
				local_priority[i] = strtod(msg,&msg); //this is the current only, actually this might not be used at all
				msg++;
				i++;
			}
			//for testing purpose: print the node info
			for(i=0;i<number_of_classes; i++){
				printf("class %d: current capacity %0.2f, capacity usage %0.2f, local priority %0.2f \n", i, capacity[i], capacity_usage[i], local_priority[i]);
			}
		}
		if(msgType==0 || msgType ==2/*active query info only*/)
		{

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
