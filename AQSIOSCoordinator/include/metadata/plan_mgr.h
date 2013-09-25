#ifndef _PLAN_MGR_
#define _PLAN_MGR_

/**
 * @file       plan_mgr.h
 * @date       Aug. 21, 2004
 * @brief      
 */

#include <ostream>

#ifndef _LOGOP_
#include "querygen/logop.h"
#endif

#ifndef _QUERY_OUTPUT_
#include "interface/query_output.h"
#endif

#ifndef _TABLE_SOURCE_
#include "interface/table_source.h"
#endif

#ifndef _TABLE_MGR_
#include "metadata/table_mgr.h"
#endif

#ifndef _PHY_OP_
#include "metadata/phy_op.h"
#endif

namespace Metadata {

	/**
	 * Plan manager  manages the physical  level entities in the  system -
	 * the operators, the queues, synopses, the query plans, etc.
	 *	 
	 * Once   the  query  plans   are  finalized   it  also   handles  the
	 * instantiation  of  physical  entities  (creating  the  objects  for
	 * operators, queues etc.)
	 *
	 * Note that by query plan, we mean the global plan involving all
	 * subqueries with one or more inputs and one or more outputs.
	 */
	
	class PlanManager {
	public:
		virtual ~PlanManager () {}
		
		/**
		 * Indicate to  the plan manager that  the table (stream/relation)
		 * represented by  tableId is  a base input  (stream/relation).  A
		 * table could also be  a intermediate stream/relation produced by
		 * a query,  and this is indicated  to the plan  manager using the
		 * map method.
		 *
		 * @param     tableId     system-wide id for the stream/relation
		 *
		 * @param     source      source object for the stream/relation
		 *
		 * @return                0 (success), !0 (failure)		 
		 */ 
		
		virtual int addBaseTable (unsigned int tableId,
								  Interface::TableSource *source) = 0;		
		
		/**
		 * Register a query with a  logical query plan.  The logical query
		 * plan is converted  to a physical query plan  within this method
		 * and stored by the plan  manager.  Every table referenced in the
		 * logical query plan should  have been "registered" with the plan
		 * manager earlier  using addSource()  (for base table)  method or
		 * map method (for intermediate tables).
		 * 
		 * @param   queryId      Identifier for the (sub)query which is
		 *                       registered.  Assigned by QueryManager
		 *
		 * @param   logPlan      logical query plan for the subquery.
		 *
		 * @return               0 (success), !0 (failure)
		 */ 
		
		virtual int addLogicalQueryPlan(unsigned int queryId,
										Logical::Operator *logPlan,
										Interface::QueryOutput *output) = 0;

		
		/**
		 * Indicate that the result of a previously registered query will
		 * be referenced in future queries using tableId.
		 * 
		 * @param  queryId       Identifier for a query
		 * @param  tableId       Identifier for the output of query queryId
		 * @return               0 (success), !0 (failure)
		 */ 
		
		virtual int map(unsigned int queryId, unsigned int tableId) = 0;
		
		/**
		 * This  method  is  usually called   after  all  the  queries
		 * have  been registered.  It performs optimizations across
		 * subqueries.
		 */
		
		virtual int optimize_plan () = 0;
		virtual Physical::Operator* getPlan()=0;
		
		 /* Factory method to construct a new plan manager.
		 */		
		static PlanManager *newPlanManager(TableManager *tableMgr,
										   std::ostream& LOG);
		
		//Query Class Scheduling by Lory Al Moakar
		// a variable used to keep track of the query class  
		// for the query whose physical plan is currently being 
		// generated
		int current_query_class;
		
		
		//added in order to signify if there is sharing of regular ops
		// ( non- source ops ) among query classes or not
		bool sharingAmongClasses;

		//end of Query Class Scheduling by LAM
		

		virtual void printPlan() = 0;		


		/// debug
		virtual int printStat () = 0;
		
	};
}

#endif
