//============================================================================
// Name        : AQSIOSCoordinator.cpp
// Author      : Thao Pham
// Version     :
// Copyright   : Your copyright notice
// Description : workload distribution coordinator
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <thread_db.h>
#include <errno.h>
#include <iostream>
#include <arpa/inet.h>

#ifndef _SCRIPT_FILE_READER_
#include "script_file_reader.h"
#endif

#ifndef _FILE_SOURCE_
#include "file_source.h"
#endif

#ifndef _GEN_OUTPUT_
#include "gen_output.h"
#endif

#ifndef _SERVER_
#include "interface/server.h"
#endif

#ifndef _ERROR_
#include "interface/error.h"
#endif

#ifndef AQSIOSCOORDINATOR_H_
#include "AQSIOSCoordinator.h"
#endif

#ifndef _PHY_OP_
#include "phy_op.h"
#endif

pthread_t threadID_loadMigration;
int isUpdateReady;
pthread_mutex_t mutexUpdateReady;
pthread_cond_t condUpdateReady;

int serverfd, portno; //listening socket and port number of server
pthread_mutex_t mutex_fdset;
int bStop;
int sockfds[MAX_NUM_CLIENTS];
LoadManager *load_mgr;
//the global query network, which is a linked list of registered operators
Physical::Operator* regOps =0;

//this is for script file loadder
static const unsigned int MAX_TABLE_SPEC = 1024;//256;
static char tableSpecBuf [MAX_TABLE_SPEC + 1];
static unsigned int tableSpecLen;

static const unsigned int MAX_QUERY = 1024;//512;
static char query [MAX_QUERY + 1];
static unsigned int queryLen;

static const int MAX_OUTPUT = 5000;//100;
static fstream queryOutput [MAX_OUTPUT];
static int numOutput;

static const unsigned int MAX_SOURCES = 500;//50;
static FileSource *sources [MAX_SOURCES];
static unsigned int numSources;

static GenOutput *outputs [MAX_OUTPUT];


using namespace std;
int main(int argc, char *argv[]) {

	if (argc < 4) {
		printf("ERROR, please provide Coordinator port number, number of classes, and the script file\n");
		return -1;
	}
	//initialize global variables
	bStop = 0;
	portno = atoi(argv[1]);
	int num_of_classes = atoi( argv[2]);

	char* script_file = argv[3];

	//load the global query network
	loadQueryNetwork(script_file,regOps);

	//initializing

	//init the mutexes and conditional variable

	pthread_mutex_init(&mutex_fdset, NULL);
	pthread_mutex_init(&mutexUpdateReady, NULL);
	pthread_cond_init(&condUpdateReady, NULL);

	isUpdateReady = 0;

	//initialize the socket fd array
	for (int i=0; i<MAX_NUM_CLIENTS;i++){
		sockfds[i]=0;
	}

	//create the global load manager object
	load_mgr = new LoadManager(num_of_classes);

	//start server listening
	if(startServerListensing()<0) return -1;

	//add the server listening socket
	for(int i=0;i<MAX_NUM_CLIENTS;i++){
		if(sockfds[i]==0){
			sockfds[i]=serverfd;
			break;
		}
	}

	//handle contrl-C from user input
	signal(SIGINT, sigproc);


	//create another thread to monitor client's load status and calculate migration decision
	pthread_attr_t migration_thread_attr;
	pthread_attr_init(&migration_thread_attr);
	pthread_attr_setdetachstate(&migration_thread_attr,PTHREAD_CREATE_JOINABLE);

	printf("creating load migration management thread \n");

	pthread_create(&threadID_loadMigration, &migration_thread_attr, manageMigration, NULL);

	//wait for and process connection request/msg from clients;
	receivingClientReport();

	stopServer();
	return 0;

}


void sigproc(int)
{
	signal(SIGINT,sigproc);

	// NOTE some versions of UNIX will reset signal to default
	//after each call. So for portability reset signal each time
	printf("preparing to close coordinator...\n");
	//stop server, including closing the listening socket
	stopServer();
	exit(0);

}

int loadQueryNetwork(char *applnScriptFile, Physical::Operator* &plan){

	int rc;
	Server     *server;
	ofstream logStr;
	char *logFile = "/home/thao/workspace/p3/log";
	logStr.open(logFile,ofstream::out);

	server = Server::newServer(logStr);

	if(!server)
		return -1;

	// Register application
	if((rc = register_appln (server, applnScriptFile, plan)) != 0){
		delete server;
		return -1;
	}

	delete server;
	return 0;

}

int register_appln (Server       *server,
		const char   *applnScriptFile, Physical::Operator* &plan)
{
	int rc;

	ScriptFileReader           *scriptReader;
	ScriptFileReader::Command   command;
	unsigned int                queryId;
	bool                        bQueryValid;
	bool                        bQueryIdValid;
	bool                        bTableSpecValid;
	FileSource                 *source;
	GenOutput                  *output;

	// Reader to interpret the script file
	scriptReader = new ScriptFileReader(applnScriptFile);

	// Get the server ready for the barge of commands
	if((rc = server -> beginAppSpecification()) != 0)
		return rc;

	bQueryIdValid = false;
	bTableSpecValid = false;
	bQueryValid = false;
	numOutput = 0;
	numSources = 0;


	// Process all commands
	while (true) {

		if ((rc = scriptReader -> getNextCommand(command)) != 0)
			return rc;

		// No more commands
		if(!command.desc)
			break;

		switch (command.type) {

		case ScriptFileReader::TABLE:

			// Copy the table specification to my local state
			tableSpecLen = command.len;
			if (tableSpecLen > MAX_TABLE_SPEC)
				return -1;
			strncpy (tableSpecBuf, command.desc, tableSpecLen+1);
			bTableSpecValid = true;

			break;

		case ScriptFileReader::SOURCE:

			// Previous command should have been a table specification.
			if (!bTableSpecValid)
				return -1;

			// Create a source for this table
			if (numSources >= MAX_SOURCES)
				return -1;

			source = sources[numSources++] = new FileSource (command.desc);

			// register the table
			if((rc = server -> registerBaseTable(tableSpecBuf,
					tableSpecLen,
					source)) != 0) {
				cout << "Error registering base table" << endl;
				return rc;
			}

			// Reset the table specification buffer
			bTableSpecValid = false;

			break;

		case ScriptFileReader::QUERY:

			queryLen = command.len;
			if (queryLen > MAX_QUERY)
				return -1;

			strncpy (query, command.desc, queryLen+1);
			bQueryValid = true;
			break;

		case ScriptFileReader::DEST:

			if (!bQueryValid)
				return -1;

			if (numOutput >= MAX_OUTPUT)
				return -1;

			queryOutput[numOutput].open (command.desc, std::ios_base::out);

			// Create a generic query output
			output = outputs[numOutput] =
					new GenOutput (queryOutput[numOutput]);
			numOutput++;

			// Register the query
			if((rc = server -> registerQuery(query,
					queryLen,
					output,
					queryId)) != 0){
				cout << "Error registering query" << endl;
				cout << "Query: " << command.desc << endl;
				return rc;
			}

			bQueryValid = false;

			break;

			//Query Class Scheduling by Lory Al Moakar
			// recognize the query class command and pass to the server
		case ScriptFileReader:: QCLASS:
			server -> current_query_class =  atoi(command.desc);
			cout<< "Current class " << server -> current_query_class<<endl;
			break;
			//end of Query Class Scheduling by LAM


		case ScriptFileReader::VQUERY:

			// Register query
			if ((rc = server -> registerQuery (command.desc,
					command.len,
					0,
					queryId)) != 0) {
				cout << "Error registering query" << endl;
				cout << "Query: " << command.desc << endl;
				return rc;
			}

			bQueryIdValid = true;

			break;

		case ScriptFileReader::VTABLE:

			if (!bQueryIdValid)
				return -1;

			if ((rc = server -> registerView (queryId,
					command.desc,
					command.len)) != 0) {
				cout << "Error registering view" << endl;
				cout << "View: " << command.desc << endl;
				return rc;
			}

			bQueryIdValid = false;
			break;

		default:

			// unknown command type
			return -1;
		}
	}

	if (bQueryValid || bQueryIdValid || bTableSpecValid)
		return -1;

	if((rc = server -> endAppSpecification()) != 0)
		return rc;

	plan = server->getPlan();

	delete scriptReader;

	return 0;
}

int startServerListensing(){

	//socket type: connection-based, stream-based and reliable - tcp connection
	sockaddr_in serv_addr;

	serverfd = socket(AF_INET, SOCK_STREAM, 0); //stream-based TCP/IP

	if (serverfd < 0){
		printf("ERROR opening socket");
		return -1;
	}
	memset((char*)&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(serverfd, (sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0){
		printf("ERROR on binding \n");
		return -1;
	}
	printf("listening on port %d \n", portno);
	//listen to connection request from AQSIOS nodes, maximum 5 pending requests
	listen(serverfd,5);

	return 0;
}
int stopServer(){

	pthread_mutex_lock(&mutexUpdateReady);
	bStop =1;
	pthread_cond_signal(&condUpdateReady);
	pthread_mutex_unlock(&mutexUpdateReady);

	pthread_join(threadID_loadMigration, NULL);
	pthread_mutex_destroy(&mutex_fdset);
	pthread_mutex_destroy(&mutexUpdateReady);
	pthread_cond_destroy(&condUpdateReady);

	close(serverfd);
	return 0;
}

int acceptingConn(){

	socklen_t clilen;
	sockaddr_in cli_addr;

	clilen = sizeof(cli_addr);
	int newsockfd = accept(serverfd,
			(struct sockaddr *) &cli_addr,
			&clilen);
	if (newsockfd < 0){
		printf("ERROR on accept");
		return -1;
	}
	else{
		//add to the set of client socket, mutex needed here to avoid conflict with the load migration threads
		//which might need to access this set
		pthread_mutex_lock(&mutex_fdset);
		for(int i=0;i<MAX_NUM_CLIENTS;i++){
			if(sockfds[i]==0){
				sockfds[i]=newsockfd;
				break;
			}
		}
		//unlock the mutex protecting the fd set so that the other threads can select the set.
		pthread_mutex_unlock(&mutex_fdset);

		//add the info to the load manager object to manage, mutex used inside the call
		load_mgr->addNode(newsockfd);

		//testing only: printing the IP address of the connected client
		/*sockaddr_in peeraddr;
		socklen_t peerlen = sizeof(peeraddr);
		getpeername(newsockfd,(sockaddr*) &peeraddr, &peerlen);
		char str[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &peeraddr.sin_addr.s_addr, str, INET_ADDRSTRLEN );
		printf("accepted a new connection: %s: %d \n", str, peeraddr.sin_port);
		 */
	}
	return 0;
}
int receivingClientReport(){

	while(1){ //this will stop with a Ctrl+C

		//read the current set of client sockets into a temporary set, trying not to block the other threads
		//using this set for too long.
		fd_set readfds;
		FD_ZERO(&readfds);
		int temp_sockfds_clients[MAX_NUM_CLIENTS];
		int max_fd =0;
		//copy the currently opened socket to the readable fd set
		pthread_mutex_lock(&mutex_fdset);
		for(int i=0;i<MAX_NUM_CLIENTS;i++){
			temp_sockfds_clients[i] = sockfds[i]; // we copy it to a temp array, so that we can release the sockfds_clients for other threads
			if(sockfds[i]>0){
				FD_SET(sockfds[i],&readfds);
				if(sockfds[i]>max_fd) max_fd = sockfds[i];
			}
		}
		pthread_mutex_unlock(&mutex_fdset);

		int activity = select(max_fd+1,&readfds, NULL, NULL, NULL); //timeout is null: wait indefinitely

		if(activity<0 && errno!=EINTR){
			printf("select error");
			return -1;
		}

		//if there is a coming connection request
		if(FD_ISSET(serverfd,&readfds)){
			acceptingConn();
		}

		//check the other sockets (client sockets)
		for(int i=0;i<MAX_NUM_CLIENTS;i++){
			if(FD_ISSET(temp_sockfds_clients[i],&readfds)&&temp_sockfds_clients[i]!=serverfd){//the socket is readable

				//read the incoming message and handle it
				if(handleClientReport(temp_sockfds_clients[i])<0){
					pthread_mutex_lock(&mutex_fdset);
					if(sockfds[i]==temp_sockfds_clients[i]){
						close(sockfds[i]);
						sockfds[i]=0;
					}
					pthread_mutex_unlock(&mutex_fdset);
					printf("client leaving\n");
				}
			}

		}
	}
	return 0;

}
int handleClientReport(int clientfd){
	char msg[512];
	int result = read(clientfd,msg,512);
	if(result ==0)//the socket is closed from client side{
		return -1;
	else{
		pthread_mutex_lock(&mutexUpdateReady);
		load_mgr->updateNodeInfo(clientfd,msg);
		isUpdateReady = 1;
		pthread_cond_signal(&condUpdateReady);
		pthread_mutex_unlock(&mutexUpdateReady);
	}
	return 0;

}
static void* manageMigration(void* arg){

	pthread_mutex_lock(&mutexUpdateReady); //TODO: double check here
	//testing purpose only
	int count =0;
	while(!bStop /*&& !isUpdateReady*/)
	{
		pthread_cond_wait(&condUpdateReady,&mutexUpdateReady);
		if(isUpdateReady){
			//check the load condition on all client nodes, make migration decision, and send notifications to clients
			//who need to migrate

			//release the mutexUpdateReady, the LoadManager's methods has it own mutex to sync.
			//the read and write on NodeInfo list.
			isUpdateReady = 0;
			pthread_mutex_unlock(&mutexUpdateReady);

			//testing purpose only: sleep for a while before making the decision:
			sleep(10);
			std::map<int, std::string> migrationDecision;
			load_mgr->redistributeLoad(&migrationDecision);
			//send the migration decision to the nodes that are the sources of the migration
			std::map<int,string>::const_iterator it;
			for(it = migrationDecision.begin(); it!=migrationDecision.end(); it++){
				if(count ==0){ //for testing purpose, now send this only once
					int n = write((*it).first,(*it).second.c_str(),strlen((*it).second.c_str())+1);
					count++;
					cout<<(*it).second<<endl;
					if (n<0){
						printf("error sending migration decision to node %d \n", (*it).first);
					}
				}

			}
			pthread_mutex_lock(&mutexUpdateReady);
		}
	}
	pthread_mutex_unlock(&mutexUpdateReady);
	cout<<"migration coordination thread is about to exit"<<endl;
	return 0;
}


