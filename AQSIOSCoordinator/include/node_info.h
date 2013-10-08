/*
 * node_info.h
 *
 *  Created on: Jan 10, 2013
 *      Author: thaopham
 */

#ifndef NODE_INFO_H_
#define NODE_INFO_H_

#endif /* NODE_INFO_H_ */
class Node_Info{
private:
	int number_of_classes;
	double* capacity;
	double* capacity_usage; //list of capacity usage of the classes ( = 0 if the class is not exist in the current node)
	double* local_priority;

public:
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
		capacity = new double[number_of_classes];
		capacity_usage = new double[number_of_classes];
		local_priority = new double[number_of_classes];
		for(int i=0;i<number_of_classes; i++){
			capacity[i] = 0;
			capacity_usage[i] = 0;
			local_priority[i] = 0;
		}
		isReady = false;
	}

	~Node_Info()
	{
		delete[] capacity;
		delete[] capacity_usage;
		delete[] local_priority;
	}
	void serialize(char* msg); //serialize the node info into string recognizable to the client.
	int update(char * msg); //extract the node info from the msg sent from client and update the moving average;
	int updateLoadInfo(char*msg);
	int updateListeningPortNo(char *msg);
	int set_capacity(int class_id, double capacity); //class_id starts from 0;
	int set_capacity_usage(int class_id, double capacity_usage);
	int set_local_priority(int class_id, double priority);
};
