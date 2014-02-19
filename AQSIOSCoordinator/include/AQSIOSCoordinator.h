/*
 * AQSIOSCoordinator.h
 *
 *  Created on: Sep 25, 2012
 *      Author: thao
 */
#include <thread_db.h>
#ifndef AQSIOSCOORDINATOR_H_
#define AQSIOSCOORDINATOR_H_

#ifndef LOAD_MANAGER_H_
#include "load_manager.h"
#endif

#ifndef _SERVER_
#include "interface/server.h"
#endif

#define MAX_NUM_CLIENTS 10
//global variables


//the thread managing the load migration of the client nodes
extern pthread_t threadID_loadMigration;
//the thread listening and receiving client's report will set this variable to inform
//the load migration manager that update is available for a new decision to be considered
extern int isUpdateReady;
extern pthread_mutex_t mutexUpdateReady;
extern pthread_cond_t condUpdateReady;

extern int serverfd, portno; //listening socket and port number of server
extern pthread_mutex_t mutex_fdset;
extern int bStop;
//set of client socket fds
extern int sockfds_clients[MAX_NUM_CLIENTS];
//the load manager object
extern LoadManager *load_mgr;
extern Physical::Operator* regOps;

void sigproc(int);
int startServerListensing();
int acceptingConn();
int stopServer();
int receivingClientReport(); //the procedure of the thread checking client's sockets and receiving node info updates if any
int handleClientReport(int clientfd);
int loadQueryNetwork(char *applnScriptFile, Physical::Operator* &plan);
int register_appln (Server* server, const char   *applnScriptFile, Physical::Operator* &plan);
static void* manageMigration(void* arg);

#endif
