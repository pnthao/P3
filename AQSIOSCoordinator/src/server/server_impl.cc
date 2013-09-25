/**
 * @file       server_impl.cc
 * @date       May 22, 2004
 * @brief      Implementation of the STREAM server.
 */

/// debug

#include <stdio.h>
#include <iostream>
using namespace std;


#ifndef _ERROR_
#include "interface/error.h"
#endif

#ifndef _DEBUG_
#include "common/debug.h"
#endif

#ifndef _SERVER_IMPL_
#include "server/server_impl.h"
#endif

#ifndef _PARSER_
#include "parser/parser.h"
#endif

#ifndef _NODES_
#include "parser/nodes.h"
#endif

#ifndef _QUERY_
#include "querygen/query.h"
#endif

#ifndef _LOGOP_
#include "querygen/logop.h"
#endif


#ifndef _PARAMS_
#include "server/params.h"
#endif

#ifdef _DM_
#include "querygen/query_debug.h"
#include "parser/nodes_debug.h"
#include "querygen/logop_debug.h"
#endif

using namespace std;

// Helper functions ...
static int registerRelation (NODE *parseTree,
							 Metadata::TableManager *tableMgr,
							 unsigned int &relId);

static int registerStream (NODE *parseTree,
						   Metadata::TableManager *tableMgr,
						   unsigned int &strId);


ServerImpl::ServerImpl(ostream &_LOG)
	: LOG (_LOG)
{
	state             = S_INIT;
	tableMgr          = 0;
	qryMgr            = 0;
	planMgr           = 0;
	//load manager, by Thao Pham
	//loadMgr 		  = 0;
	
	// Set default values of various server params
	MEMORY            = MEMORY_DEFAULT;
	QUEUE_SIZE        = QUEUE_SIZE_DEFAULT;
	SHARED_QUEUE_SIZE = SHARED_QUEUE_SIZE_DEFAULT;
	INDEX_THRESHOLD   = INDEX_THRESHOLD_DEFAULT;
	SCHEDULER_TIME    = SCHEDULER_TIME_DEFAULT;
	CPU_SPEED         = CPU_SPEED_DEFAULT;
	
	// Response Time Calculation By Lory Al Moakar
	TIME_UNIT         = TIME_UNIT_DEFAULT; 
	//end of part 1 of response time calculation by LAM
	
	//load managing, by Thao Pham
	INPUT_RATE_TIME_UNIT = INPUT_RATE_TIME_UNIT_DEFAULT;
	HEADROOM_FACTOR = HEADROOM_FACTOR_DEFAULT;
	DELAY_TOLERANCE = DELAY_TOLERANCE_DEFAULT;
	INITIAL_GAP = INITIAL_GAP_DEFAULT;
	NEXT_GAP = NEXT_GAP_DEFAULT;
	LOAD_MANAGER = LOAD_MANAGER_DEFAULT;
	
	//end of load managing, by Thao Pham
	//Lottery scheduling by Lory Al Moakar
	SCHEDULING_POLICY = ROUND_ROBIN_SCHED;
	// end of part 2 of lottery scheduling by LAM

	
	
	//Query Class Scheduling by Lory Al Moakar
	N_QUERY_CLASSES = N_QUERY_CLASSES_DEFAULT;
	QUERY_CLASS_PRIORITIES[0] = DEFAULT_QC_PRIORITY;
	current_query_class = 1;
	HONOR_PRIORITIES = HONOR_PRIORITIES_DEFAULT;
	STEADY_STATE = STEADY_STATE_DEFAULT;
	QUOTA = QUOTA_DEFAULT;
	SHARING_AMONG_CLASSES = SHARING_AMONG_CLASSES_D;

	//end of part 1 of Query Class Scheduling by LAM
	
	//end of comment out by Thao Pham, ignore these for now
	
	pthread_mutex_init (&mutex, NULL);
	pthread_cond_init (&mainThreadWait, NULL);
	pthread_cond_init (&interruptThreadWait, NULL);
}

ServerImpl::~ServerImpl()
 {
	if (logPlanGen)
		delete logPlanGen;

	if (semInterpreter)
		delete semInterpreter;
	
	if (tableMgr)
		delete tableMgr;
	
	if (planMgr)
		delete planMgr;
	
	if (qryMgr)
		delete qryMgr;

	//load manager, by Thao Pham
	/*if(scheduler->loadMgrs)
		delete scheduler->loadMgrs;
		//end of load manager, by Thao Pham*/
	
}


/**
 * Begin the specification of an application.
 *
 * Server should be in INIT state when this method id called.  It moves to
 * S_APP_SPEC state when it returns from this method.  
 *
 * Also server starts up:
 *
 * 1. Table manager to manage named streams and relations (input &
 *    derived)
 */

int ServerImpl::beginAppSpecification() 
{
	// State transition
	if(state != S_INIT)
		return INVALID_USE_ERR;
	state = S_APP_SPEC;
	
	// Table manager
	tableMgr = new Metadata::TableManager();
	if(!tableMgr)
		return INTERNAL_ERR;

	// Query Manager
	qryMgr = new Metadata::QueryManager(LOG);
	if (!qryMgr)
		return INTERNAL_ERR;
	
	// Plan manager
	planMgr = Metadata::PlanManager::newPlanManager(tableMgr, LOG);
	if(!planMgr)
		return INTERNAL_ERR;
	
	
	planMgr-> sharingAmongClasses = SHARING_AMONG_CLASSES;
	
	
	// Semantic Interpreter
	semInterpreter = new SemanticInterpreter(tableMgr);
	if(!semInterpreter)
		return INTERNAL_ERR;
	
	// Logical plan generator
	logPlanGen = new LogPlanGen (tableMgr);
	if (!logPlanGen)
		return INTERNAL_ERR;
	
	return 0;
}

/**
 * Register a table (stream or a relation):
 *
 * The informatin about a table is encoded as a string.  This string is
 * first parsed using the parser.
 */

int ServerImpl::registerBaseTable(const char *tableInfo,
								  unsigned int tableInfoLen,
								  Interface::TableSource *input)
{
	int rc;
	NODE  *parseTree;
	unsigned int tableId;
	//printf("Table registered\n");
	// Tables can be registered only in S_APP_SPEC mode (before
	// endApplicationSpec() has been called)
	if (state != S_APP_SPEC)
		return INVALID_USE_ERR;	
	
	// Sanity check
	if (!tableInfo)
		return INVALID_PARAM_ERR;
	
	// Parse table info
	parseTree = Parser::parseCommand(tableInfo, tableInfoLen);
	if(!parseTree)
		return PARSE_ERR;
	
	// Store the parsed info with the table manager
	if(parseTree -> kind == N_REL_SPEC) {
		if ((rc = registerRelation (parseTree, tableMgr, tableId)) != 0)
			return rc;
	}
	else if(parseTree -> kind == N_STR_SPEC) {
		if ((rc = registerStream (parseTree, tableMgr, tableId)) != 0)
			return rc;
	}
	
	// Unknown command: parse error
	else {
		return PARSE_ERR;
	}
	
	// Inform the plan manager about the new table
	if ((rc = planMgr -> addBaseTable (tableId, input)) != 0)
		return rc;
	
	return 0;	
}

/**
 *  [[ To be implemented ]] 
 */ 
int ServerImpl::registerQuery (const char *querySpec, 
							   unsigned int querySpecLen,
							   Interface::QueryOutput *output,
							   unsigned int &queryId)
{	
	NODE *parseTree;
	Semantic::Query semQuery;
	Logical::Operator *logPlan;
	int rc;
	
	// This method can be called only in S_APP_SPEC state
	if (state != S_APP_SPEC)
		return INVALID_USE_ERR;
	
	// Query String -> Parse Tree
	parseTree = Parser::parseCommand(querySpec, querySpecLen);
	if (!parseTree) {
		return PARSE_ERR;
	}
	
	// Parse Tree -> Semantic Query
	if((rc = semInterpreter -> interpretQuery (parseTree, semQuery)) != 0) {
		return rc;
	}

	/// Debug
#ifdef _DM_
	//printQuery (cout, semQuery, tableMgr);
#endif
	
	// At this point there are no errors in the query (syntactic or
	// semantic), so we register the query and get an internal id for it
	if ((rc = qryMgr -> registerQuery (querySpec, querySpecLen, queryId))
		!= 0) {
		return rc;
	}
	
	
	
	// SemanticQuery -> logical plan
	if ((rc = logPlanGen -> genLogPlan (semQuery, logPlan)) != 0) {
		return rc;
	}
	
#ifdef _DM_
	//cout << endl << endl << logPlan << endl;
#endif
	
	//Query Class Scheduling by Lory Al Moakar
	//copy current_class to the plan manager
	// so that when the phsical plan is generated the operators
	// are aware of the query class that these operators
	// belong to
	planMgr-> current_query_class = this->current_query_class;

	//end of Query Class Scheduling by LAM

	printf("query registered\n");

	// Logical Plan -> Physical Plan (stored with plan manager)
	if ((rc = planMgr -> addLogicalQueryPlan (queryId, logPlan, output)) != 0){
		return rc;
	}
	
#ifdef _DM_
	//cout << endl << endl;
	//planMgr -> printPlan();
#endif
	
	return 0;
}

int ServerImpl::registerView(unsigned int queryId,
							 const char *tableInfo,
							 unsigned int tableInfoLen)
{
	int rc;
	NODE  *parseTree;
	unsigned int tableId;	
	
	// This method can be called only in S_APP_SPEC state
	if (state != S_APP_SPEC)
		return INVALID_USE_ERR;

	// Sanity check
	if (!tableInfo)
		return INVALID_PARAM_ERR;

	// Parse table info
	parseTree = Parser::parseCommand(tableInfo, tableInfoLen);
	if(!parseTree)
		return PARSE_ERR;
	
	// Store the parsed info with the table manager
	if(parseTree -> kind == N_REL_SPEC) {
		if ((rc = registerRelation (parseTree, tableMgr, tableId)) != 0)
			return rc;
	}
	
	else if(parseTree -> kind == N_STR_SPEC) {
		if ((rc = registerStream (parseTree, tableMgr, tableId)) != 0)
			return rc;
	}
	
	// Unknown command: parse error
	else {
		return PARSE_ERR;
	}
	
	// Indicate the mapping from query -> tableId to the plan manager
	if ((rc = planMgr -> map (queryId, tableId)) != 0)
		return rc;
	
	return 0;
}

/**
 * [[ to be implemented ]]
 */ 
int ServerImpl::endAppSpecification() 
{
	int rc;

	if (state != S_APP_SPEC)
		return INVALID_USE_ERR;
	
	if ((rc = planMgr -> optimize_plan ()) != 0)
		return rc;
	
	state = S_PLAN_GEN;

	return 0;
}

static int registerRelation (NODE *parseTree,
							 Metadata::TableManager *tableMgr,
							 unsigned int &relId)
{	
	int rc;
	
	const char   *relName;
	NODE         *attrList;	
	NODE         *attrSpec;
 	const char   *attrName;
	Type          attrType;
	unsigned int  attrLen;
	
	// Relation name
	relName = parseTree -> u.REL_SPEC.rel_name;
	ASSERT(relName);	
	
	if((rc = tableMgr -> registerRelation(relName, relId)) != 0)
		return rc;
	
	attrList = parseTree -> u.REL_SPEC.attr_list;
	ASSERT(attrList);
	
	// Process each attribute in the list
	while (attrList) {		
		ASSERT(attrList -> kind == N_LIST);		
		
		// Attribute specification
		attrSpec = attrList -> u.LIST.curr;
		
		ASSERT(attrSpec);
		ASSERT(attrSpec -> kind == N_ATTR_SPEC);
		
		attrName = attrSpec -> u.ATTR_SPEC.attr_name;
		attrType = attrSpec -> u.ATTR_SPEC.type;

		switch (attrType) {
		case INT:
			attrLen = INT_SIZE; break;			
		case FLOAT:
			attrLen = FLOAT_SIZE; break;
		case CHAR:
			attrLen = attrSpec -> u.ATTR_SPEC.len; break;			
		case BYTE:
			attrLen = BYTE_SIZE; break;
		default:
			ASSERT (0);
			break;
		}
		
		if((rc = tableMgr -> addTableAttribute(relId, attrName, 
											   attrType, attrLen)) != 0) {
			tableMgr -> unregisterTable (relId);
			return rc;
		}
		
		// next attribute
		attrList = attrList -> u.LIST.next;
	}
	
	return 0;
}

static int registerStream (NODE *parseTree,
						   Metadata::TableManager *tableMgr,
						   unsigned int &strId)
{
	int rc;
	
	const char   *strName;
	NODE         *attrList;	
	NODE         *attrSpec;
 	const char   *attrName;
	Type          attrType;
	unsigned int  attrLen;

	// Stream name
	strName = parseTree -> u.STR_SPEC.str_name;
	ASSERT(strName);
	
	if((rc = tableMgr -> registerStream(strName, strId)) != 0)
		return rc;
	
	attrList = parseTree -> u.STR_SPEC.attr_list;
	ASSERT(attrList);
	
	// Process each attribute in the list
	while (attrList) {
		
		ASSERT(attrList -> kind == N_LIST);
		
		// Attribute specification
		attrSpec = attrList -> u.LIST.curr;
		
		ASSERT(attrSpec);
		ASSERT(attrSpec -> kind == N_ATTR_SPEC);
		
		attrName = attrSpec -> u.ATTR_SPEC.attr_name;
		attrType = attrSpec -> u.ATTR_SPEC.type;
		
		switch (attrType) {
		case INT:
			attrLen = INT_SIZE; break;			
		case FLOAT:
			attrLen = FLOAT_SIZE; break;
		case CHAR:
			attrLen = attrSpec -> u.ATTR_SPEC.len; break;			
		case BYTE:
			attrLen = BYTE_SIZE; break;
		default:
			ASSERT (0);
			break;
		}
		
		if((rc = tableMgr -> addTableAttribute(strId, attrName, 
											   attrType, attrLen)) != 0) {
			tableMgr -> unregisterTable (strId);
			return rc;
		}
		
		// next attribute
		attrList = attrList -> u.LIST.next;
	}
	
	return 0;
}

Server *Server::newServer(std::ostream& LOG) 
{
	return new ServerImpl(LOG);
}

//Load shedding by Thao Pham: add shedding log file info
int ServerImpl::setSheddingLogFile(const char *sheddingLogFile)
{
	this->sheddingLogFile =sheddingLogFile; 
	return 0;
}

Physical::Operator* ServerImpl::getPlan(){
	//this function should be called only when the physical plan has been fully constructed
	return planMgr->getPlan();
}


