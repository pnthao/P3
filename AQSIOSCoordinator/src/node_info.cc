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

	//trim the msg header
	char buf[5];
	int len = strchr(msg,',') - msg;
	strncpy(buf,msg,len);
	buf[len]=0;

	if(strcmp(buf,"NI")!=0)
		return -1; // not a valid node info message

	//extract the current capacity of each class
	msg = msg+len+1;
	int i = 0;
	while(i<number_of_classes){
		//after this call msg points to the next comma
		capacity[i] = capacity[i]*(1-aging_factor) + aging_factor*strtod(msg, &msg);
		//skip the comma
		msg++;
		i++;
	}

	//extract the capacity usage of each class
	i=0;
	while(i<number_of_classes){
		capacity_usage[i] = capacity_usage[i]*(1-aging_factor) +aging_factor*strtod(msg, &msg);
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

	return 0;
}

