/*
 * node_info.h
 *
 *  Created on: Jan 10, 2013
 *      Author: thaopham
 */

#ifndef NODE_INFO_H_
#define NODE_INFO_H_
#endif /* NODE_INFO_H_ */

#include <vector>
#include <list>
#include <string>
using namespace std;
class Node_Info{
struct QueryInfo{
	int queryID;
	double queryLoad;
};
private:
	int number_of_classes;
	vector<double> capacities;
	vector<double> capacity_usages; //list of capacity usage of the classes ( = 0 if the class is not exist in the current node)
	vector<double> local_priorities;

public:
	list<QueryInfo> activeQueries; //list of active queries running on this node
	bool isReady; //a ready node is the one that can start executing, pending starting timestamp from the coordinator
	int nodeID; //sockfd can be used as node ID;
	/*the port that this node uses to listen to connection from other nodes which want
 * migrate a query to this node. The AQSIOS node will informs the coordinator of this port
 * in the first message it sends to the coordinator
	 */

	int listeningPort;

	Node_Info(int number_of_classes)
	{
		this->number_of_classes = number_of_classes;

		for(int i=0;i<number_of_classes; i++){
			capacities.push_back(0.0);
			capacity_usages.push_back(0.0);
			local_priorities.push_back(0.0);
		}

		isReady = false;
		nodeID = 0;
		listeningPort = 0;
	}

	~Node_Info()
	{

	}

	int update(char * msg); //extract the node info from the msg sent from client and update the moving average;
	int updateLoadInfo(char*msg);
	int updateListeningPortNo(char *msg);
	int set_capacity(int class_id, double capacity); //class_id starts from 0;
	int set_capacity_usage(int class_id, double capacity_usage);
	int set_local_priority(int class_id, double priority);
	double getTotalCapacityUsage() const;
	std::string getNodeAddress() const;
};
