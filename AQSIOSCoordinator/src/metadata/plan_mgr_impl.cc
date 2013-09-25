#ifndef _PLAN_MGR_IMPL_
#include "metadata/plan_mgr_impl.h"
#endif

#ifndef _DEBUG_
#include "common/debug.h"
#endif

#ifndef _SYS_STREAM_
#include "common/sys_stream.h"
#endif

#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace Metadata;
using namespace Physical;
using namespace std;

PlanManagerImpl::PlanManagerImpl(TableManager *tableMgr, ostream& _LOG)
	: LOG (_LOG)
{
	ASSERT (tableMgr);
	
	this -> tableMgr = tableMgr;
	this -> freeOps = 0;
	this -> usedOps = 0;
	this -> numExprs = 0;
	this -> numBExprs = 0;
	this -> numBaseTables = 0;
	this -> numOutputs = 0;
	this -> numTables = 0;
	this -> numQueries = 0;
	
	// Organize all the ops as a linked list
	init_ops ();
	
	//Query Class Scheduling by Lory Al Moakar
	this->current_query_class = 1;
	//end of part 1 of Query Class Scheduling by LAM

	//this is just to make to plan exacly the same as the one seen by the nodes
	// Create the operator that produces system stream (SS)
	createSSGen ();

}

void PlanManagerImpl::createSSGen ()
{
	Operator *op;
	
	op = new_op (PO_SS_GEN);
	ASSERT (op);
	
	// Schema of SysStream
	op -> numAttrs = SS_NUM_ATTRS;
	for (unsigned int a = 0 ; a < SS_NUM_ATTRS ; a++) {
		op -> attrTypes [a] = SS_ATTR_TYPES [a];
		op -> attrLen [a] = SS_ATTR_LEN [a];
	}
	
	op -> bStream = true;

	op -> u.SS_GEN.numOutput = 0;
	
	//Query Class Scheduling by Lory Al Moakar
	// assign the stream generator operator a high query class 
	// --> class 1
	op -> query_class_id = 1;
	//end of part 2 of Query Class Scheduling by LAM


	// Register the mapping from SysStream -> op
	ASSERT (numTables < MAX_TABLES);
	sourceOps [numTables].tableId = SS_ID;
	sourceOps [numTables].opId = op -> id;
	numTables ++;	
}

PlanManagerImpl::~PlanManagerImpl() {

}

/**
 * Create a new operator which acts as the operator-source for this
 * table.  Every plan that uses this base table involves this operator. 
 */ 

int PlanManagerImpl::addBaseTable(unsigned int tableId,
								  Interface::TableSource *source)
{
	Operator *op;	
	
	// The base table is a stream:
	if(tableMgr -> isStream(tableId)) {
		
		// Get a new stream source operator
		op = new_op (PO_STREAM_SOURCE);
		
		// We ran out of resources!
		if (!op) {
			LOG << "PlanManager: out of space for operators" << endl;
			return -1;
		}
		op -> u.STREAM_SOURCE.strId = tableId;
		op -> u.STREAM_SOURCE.srcId = numBaseTables;		
		op -> bStream = true;
	}
	
	// base table is a relation	
	else {
		
		// new relation source operator
		op = new_op (PO_RELN_SOURCE);
		
		// We ran out of resources!
		if (!op) {
			LOG << "PlanManager: out of space for operators" << endl;
			return -1;
		}
		
		op -> u.RELN_SOURCE.relId = tableId;
		op -> u.RELN_SOURCE.srcId = numBaseTables;
		op -> bStream = false;
	}
	
	// Output schema of the operator = schema of the stream
	op -> numAttrs = tableMgr -> getNumAttrs (tableId);
	
	if (op -> numAttrs > MAX_ATTRS) {
		LOG << "PlanManager: too many attributes in the table" << endl;
		return -1;
	}
	
	for (unsigned int a = 0 ; a < op -> numAttrs ; a++) {
		op -> attrTypes [a] = tableMgr -> getAttrType (tableId, a);		
		op -> attrLen [a] = tableMgr -> getAttrLen (tableId, a);
	}
	
	// Store the tableSource
	if (numBaseTables >= MAX_TABLES) {
		LOG << "PlanManager: out of space for base tables" << endl;
		return -1;
	}
	baseTableSources [numBaseTables ++] = source;

#ifdef _DM_
	// Have we seen this guy before?
	for (unsigned int t = 0 ; t < numTables ; t++) 
		ASSERT (sourceOps [t].tableId != tableId);
#endif	
	
	// Store the fact that "op" is the source operator for tableId
	if(numTables >= MAX_TABLES) {
		LOG << "PlanManager: out of space for tables" << endl;
		return -1;
	}
	sourceOps [numTables].tableId = tableId;
	sourceOps [numTables].opId = op -> id;
	numTables ++;
	
	return 0;	
}

//Query Class Scheduling by Lory Al Moakar
int PlanManagerImpl::addQueryClasses ( Physical::Operator * phyOp) {
 
  //is this an operator that is not shared or is shared with a 
  //lower priority class ? --> assign it the higher priority
  if ( phyOp-> query_class_id == 0 || 
       phyOp-> query_class_id > current_query_class)
    phyOp-> query_class_id = current_query_class;
  //we already assigned this operator a higher query class
  //--> we also assigned its input --> return success
  else 
    return 0;
  int rc = 0;
  //if no more inputs to scan return 0
  if ( phyOp->numInputs <= 0 )
    return 0; 
  
  //printf("%d \n",phyOp->query_class_id);
  
  for ( int i = 0; i < phyOp->numInputs ; i ++ ) {
   	
   
    rc = addQueryClasses( phyOp->inputs[i]);
     
    if ( rc != 0 ) 
      return rc;
  }
  return 0;
}

//end of part 3 of Query Class Scheduling by LAM

int PlanManagerImpl::addLogicalQueryPlan (unsigned int queryId,
										  Logical::Operator *logPlan,
										  Interface::QueryOutput *output)
{
	int rc;
	Physical::Operator *phyPlan, *rootOp;
	Physical::Operator *outputOp;
	Physical::Operator *querySource;
	
	// generate the physical plan for the query
	if ((rc = genPhysicalPlan (logPlan, phyPlan)) != 0)
		return rc;

	// root operator for the plan
	rootOp = phyPlan;

	// Generate  a query-source  operator  for this  query.  Query  source
	// operator  is a no-op  operator that  sits on  top of  queries.  any
	// other query that  uses the result of this query  reads off from the
	// query source operator.  This is  a dummy operator found only at the
	// metadata  level.    When  we  instantiate   the  actual  executable
	// operators, we  bypass this dummy operator.   This operator prevents
	// certain incorrect optimizations  (e.g., pushing selections from one
	// query to another) from happening.   
	if ((rc = mk_qry_src (phyPlan, querySource)) != 0)
		return rc;
	
	// Store the <queryId, querySrc> pair - this is used if the queryId is
	// an  intermediate query  whose results  are used  by  other queries.
	// Then we can "attach" the base of other queries to this operator.	
	if (numQueries >= MAX_QUERIES) {
		LOG << "PlanManagerImpl:: too many queries" << endl;
		return -1;
	}
	
	queryOutOps [numQueries].queryId = queryId;
	queryOutOps [numQueries].opId = querySource -> id;
	numQueries ++;
	
	// If the the output of this query is required externally (parameter
	// output is not null), we need to create a specific operator that
	// interfaces outside the system.
	if (output) {
		if (numOutputs >= MAX_OUTPUT) {
			LOG << "PlanManagerImpl:: too many outputs" << endl;
			return -1;
		}
		
		// Create an output operator that reads from the query
		if ((rc = mk_output (phyPlan, outputOp)) != 0)
			return rc;
		
		queryOutputs [numOutputs] = output;		
		outputOp -> u.OUTPUT.outputId = numOutputs;
		outputOp -> u.OUTPUT.queryId = queryId;
		numOutputs ++;
		
		//moved inside by Thao Pham, it might not correct to put it outside in the case of virtual query
	
		//Query Class Scheduling by Lory Al Moakar
		//attach the query class id to operators of this query
		
		addQueryClasses ( outputOp );
		
		//end of part 4 of Query Class Scheduling by LAM
		
		//end of "moved inside by Thao Pham"
	
	}
	
	
	return 0;
}

int PlanManagerImpl::map (unsigned int queryId, unsigned int tableId)
{
	Operator *queryOutOp;
	
	// Get the output operator for the query
	queryOutOp = 0;	
	for (unsigned int q = 0 ; q < numQueries ; q++) {
		if (queryOutOps [q].queryId == queryId) {
			queryOutOp = ops + queryOutOps [q].opId;						
			break;
		}
	}

	// We did not find an outut operator for this query. (this should
	// never happen during correct execution
	if (!queryOutOp)
		return -1;
   	
	// We want to make sure that the schema of the table and the query are
	// identical.
	
	if (tableMgr -> getNumAttrs (tableId) != queryOutOp -> numAttrs)				
		return -1;
	
	for (unsigned int a = 0 ; a < queryOutOp -> numAttrs ; a++) {
		
		if (tableMgr -> getAttrType (tableId, a) !=
			queryOutOp -> attrTypes [a]) {
			return -1;
		}
		
		if (tableMgr -> getAttrLen (tableId, a) !=
			queryOutOp -> attrLen [a]) {
			LOG << "Reg len: " << tableMgr -> getAttrLen (tableId, a)
				<< endl;
			LOG << "Qry len: " << queryOutOp -> attrLen [a]
				<< endl;
			
			return -1;
		}
	}
	
	if (numTables >= MAX_TABLES) {
		LOG << "PlanManagerImpl:: too many sources" << endl;
		return -1;
	}		
	
	sourceOps [numTables].tableId = tableId;
	sourceOps [numTables].opId = queryOutOp -> id;
	numTables ++;
	
	return 0;
}

/**
 * [[ To be implemented ]]
 */
int PlanManagerImpl::optimize_plan ()
{
	int rc;
	
	LOG << endl << "Optimizing plan" << endl << endl;
	
	// remove all query sources
	if((rc = removeQuerySources()) != 0)
		return rc;
	
	if ((rc = addSinks ()) != 0)
		return rc;
	
	//LOG << endl << endl;
	//printPlan();
	
	if ((rc = mergeSelects ()) != 0)
		return rc;

	//LOG << endl << endl;
	//printPlan();
	//Operator Sharing By Lory Al Moakar

	findSharedOps(); 

	LOG << endl << endl;
	LOG << "-------------------------------------------------------" << endl;
	LOG << "-----------AFTER FINDING SHARED OPERATORS--------------" << endl;
	LOG << "-------------------------------------------------------" << endl;

	//printPlan();

	//end of part 1 of operator sharing by LAM
	
	if ((rc = mergeProjectsToJoin()) != 0)
		return rc;


	//end of inserting inactive drop operator, by Thao Pham


	//LOG << endl << endl;
	printPlan();

	return 0;
}

void PlanManagerImpl::printPlan()
{
	LOG << "PrintPlan: begin" << endl;
	Operator *op = usedOps;

	LOG <<
		"---------------------------- OPERATORS -----------------------"
		<< endl;
	
	while (op) {
		LOG << "Op: " << op -> id << endl;
		LOG << op << endl;

		//Query Class Scheduling by Lory Al Moakar
		//added to display in the log file the query_class_id of the operator
		LOG << "Query class: " << op->query_class_id << "\n"<<endl;
		//end of part 6 Query Class Scheduling by LAM

		op = op -> next;
	}

	LOG << "PrintPlan: end" << endl;
	return;
}

int PlanManagerImpl::printStat ()
{
	int rc;
	
#ifdef _MONITOR_

	// Operator times
	Operator *op = usedOps;
	double timeTaken;
	int lastOutTs;
	double total = 0;
	
	while (op) {

		ASSERT (op -> instOp);
		if ((rc = op -> instOp -> getDoubleProperty (Monitor::OP_TIME_USED,
													 timeTaken)) != 0)
			return rc;

		if ((rc = op -> instOp -> getIntProperty (Monitor::OP_LAST_OUT_TS,
												  lastOutTs)) != 0)
			return rc;
		
		total += timeTaken;
		LOG << "Operator [" << op -> id << "]"
			<< " Time = " 
			<< timeTaken
			<< " Lot = "
			<< lastOutTs
			<< endl;
		
		op = op -> next;
	}
	
	LOG << "Total Time: " << total << endl;
	
	// Store memories
	int maxPages, numPages;
	for (unsigned int s = 0 ; s < numStores ; s++) {
		ASSERT (stores [s].instStore);
		if ((rc = stores [s].instStore ->
			 getIntProperty (Monitor::STORE_MAX_PAGES, maxPages)) != 0)
			return rc;
		
		if ((rc = stores [s].instStore ->
			 getIntProperty (Monitor::STORE_NUM_PAGES, numPages)) != 0)
			return rc;
		
		
		LOG << "Store [" << s << "]: <"
			<< maxPages
			<< ","
			<< numPages
			<< ">";
		
		LOG << endl;
	}

#if 0	
	int numEntries, numBuckets, numNonMt;
	for (unsigned int i = 0 ; i < numIndexes ; i++) {

		if ((rc = indexes [i] -> getIntProperty 
			 (Monitor::HINDEX_NUM_BUCKETS, numBuckets)) != 0)
			return rc;
		if ((rc = indexes [i] -> getIntProperty
			 (Monitor::HINDEX_NUM_NONMT_BUCKETS, numNonMt)) != 0)
			return rc;
		if ((rc = indexes [i] -> getIntProperty
			 (Monitor::HINDEX_NUM_ENTRIES, numEntries)) != 0)
			return rc;

		LOG << "Index [" << i << "]: <"
			<< numBuckets
			<< ","
			<< numNonMt
			<< ","
			<< numEntries
			<< ">"
			<< endl;
	}	
	
	// Synopsis sizez
	int maxTuples, numTuples;	
	for (unsigned int s = 0 ; s < numSyns ; s++) {

		switch (syns[s].kind) {
		case REL_SYN:
			ASSERT (syns [s].u.relSyn);
			rc = syns [s].u.relSyn ->
				getIntProperty (Monitor::SYN_MAX_TUPLES, maxTuples);
			if (rc != 0) return rc;
			
			rc = syns [s].u.relSyn ->
				getIntProperty (Monitor::SYN_NUM_TUPLES, numTuples);
			if (rc != 0) return rc;			
			
			break;
			
		case WIN_SYN:
			ASSERT (syns [s].u.winSyn);
			rc = syns [s].u.winSyn ->
				getIntProperty (Monitor::SYN_MAX_TUPLES, maxTuples);
			if (rc != 0) return rc;
			
			rc = syns [s].u.winSyn ->
				getIntProperty (Monitor::SYN_NUM_TUPLES, numTuples);
			if (rc != 0) return rc;
			break;
			
		case LIN_SYN:
			ASSERT (syns [s].u.linSyn);
			rc = syns [s].u.linSyn ->
				getIntProperty (Monitor::SYN_MAX_TUPLES, maxTuples);
			if (rc != 0) return rc;
			
			rc = syns [s].u.linSyn ->
				getIntProperty (Monitor::SYN_NUM_TUPLES, numTuples);
			if (rc != 0) return rc;
			break;
			
		case PARTN_WIN_SYN:
			ASSERT (syns [s].u.pwinSyn);
			rc = syns [s].u.pwinSyn ->
				getIntProperty (Monitor::SYN_MAX_TUPLES, maxTuples);
			if (rc != 0) return rc;
			
			rc = syns [s].u.pwinSyn ->
				getIntProperty (Monitor::SYN_NUM_TUPLES, numTuples);
			if (rc != 0) return rc;
			break;
			
		default:
			ASSERT (0);
			break;
		}

		LOG << "Synopsis [" << s << "]: <"
			<< maxTuples
			<< ","
			<< numTuples
			<< ">"
			;
		
		LOG << endl;					
	}
#endif

	
#endif
	
	return 0;
}

Physical::Operator* PlanManagerImpl::getPlan(){
	return usedOps;
}
