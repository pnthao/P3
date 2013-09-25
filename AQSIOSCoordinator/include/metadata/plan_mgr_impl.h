#ifndef _PLAN_MGR_IMPL_
#define _PLAN_MGR_IMPL_

/**
 * @file       plan_mgr_impl.h
 * @date       Aug. 21, 2004
 * @brief      
 */

#include <ostream>

#ifndef _PLAN_MGR_
#include "metadata/plan_mgr.h"
#endif

#ifndef _PHY_OP_
#include "metadata/phy_op.h"
#endif

/*#ifndef _AEVAL_
#include "execution/internals/aeval.h"
#endif

#ifndef _BEVAL_
#include "execution/internals/beval.h"
#endif
*/
using namespace Physical;

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
	
	class PlanManagerImpl : public PlanManager {
	private:
		
		/// System log
		std::ostream& LOG;
		
		/// System-wide table manager
		TableManager *tableMgr;		
		
		//----------------------------------------------------------------------
		// Constants affecting the system resources allocated for storing
		// various meta information.
		//----------------------------------------------------------------------
		
		/// Maximum number of physical operators permitted in the system
		static const unsigned int MAX_OPS = 5000;//2000;
		/// Maximum number of query outputs from the system
		static const unsigned int MAX_OUTPUT = 1000;//50;		
		
		/// Maximum number of expression over all the queries
		static const unsigned int MAX_EXPRS = 5000;//1000;
		
		//----------------------------------------------------------------------
		// System Execution state: operators, queues, plan ...
		//----------------------------------------------------------------------
		
		/// Physical operators in the system.
		Operator ops [MAX_OPS];
		
		/// Pool of free operators (organized as a linked list)
		Operator *freeOps;
		
		/// Pool of active (used) operators organized as a linked list
		Operator *usedOps;
		
		/// Expressions that occur within operators
		Expr exprs [MAX_EXPRS];
		unsigned int numExprs;
		
		/// Boolean expressions that occur within operators
		BExpr bexprs [MAX_EXPRS];
		unsigned int numBExprs;		
		
		//----------------------------------------------------------------------
		// System Input/Output
		//----------------------------------------------------------------------
		
		/// Number of base (input) stream / relations
		unsigned int numBaseTables;
		
		/// Source objects to get tuples from base tables.
		Interface::TableSource *baseTableSources [MAX_TABLES];		
		
		/// Number of output points in the system.
		unsigned int numOutputs;
		
		/// QueryOutput's are interfaces dsms uses to produce query results
		Interface::QueryOutput *queryOutputs [MAX_OUTPUT];
		
		//----------------------------------------------------------------------
		// "Naming" related:
		// 
		// The output of some subqueries form the input of other queries.
		// A tableId that occurs in a plan could refer to a base
		// stream/relation or to the result of another subquery.  We
		// handle these two cases uniformly by storing a "source" operator
		// for each tableId that produces the tuples of the
		// stream/relation corresponding to the table.
		//----------------------------------------------------------------------
		
		/// Number of base tables + intermediate (named) tables
		unsigned int numTables;
		
		/// Structure that stores tableId -> opId mapping
		struct {
			unsigned int tableId;
			unsigned int opId;
		} sourceOps [MAX_TABLES];
		
		/// Number of queries registered so far.
		unsigned int numQueries;
		
		/// For each query, we store the operator that produces the outptu
		/// of the query
		struct {
			unsigned int queryId;
			unsigned int opId;
		} queryOutOps [MAX_QUERIES];
		
	public:
		PlanManagerImpl(TableManager *tableMgr, std::ostream& LOG);
		virtual ~PlanManagerImpl();		
		
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
		
		int addBaseTable (unsigned int tableId,
						  Interface::TableSource *source);		
		
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
		
		int addLogicalQueryPlan (unsigned int queryId,
								 Logical::Operator *logPlan,
								 Interface::QueryOutput *output);

		
		/**
		 * Indicate that the result of a previously registered query will
		 * be referenced in future queries using tableId.
		 * 
		 * @param  queryId       Identifier for a query
		 * @param  tableId       Identifier for the output of query queryId
		 * @return               0 (success), !0 (failure)
		 */ 
		
		int map(unsigned int queryId, unsigned int tableId);
		
		/**
		 * This  method  is  usually called   after  all  the  queries
		 * have  been registered.  It performs optimizations across
		 * subqueries.
		 */
		
		int optimize_plan ();
		Physical::Operator* getPlan();

		/**
		 * (Debug routine) 
		 */
		void printPlan();


		int printStat ();

		//Operator Sharing By Lory Al Moakar
		/**
		 * this method traverses the physical plan and finds 
		 * replicated operators
		 * it combines them into combined shared operators;
		 * @return true if successful false otherwise
		 */
		bool findSharedOps();
		//end of part 1 of operator sharing by LAM

		//HR implementation by Lory Al Moakar

		//Query Class Scheduling by Lory Al Moakar
		/**
		 * this method goes over the operators that belong to a certain 
		 * query and attach the id of the query class that this query 
		 * belongs to. This is done here since inside 
		 * addLogicalQueryPlan is the last method where the operators are 
		 * treated as part of the query and not as individual units.
		 * @return               0 (success), !0 (failure)
		 */
		int addQueryClasses ( Physical::Operator * phyOp);
		
		//end of Query Class Scheduling by LAM
		
	private:
		//---------------------------------------------------------------------
		// Operators management routines:
		//
		// Routines to keep track of the list of available operators
		// (ops), which operators are free, which are not,  allocate
		// operators during plan generation
		//---------------------------------------------------------------------
		
		/**
		 * Organize the set of all operators into a linked list of "free"
		 * operators.  this linked list is used 
		 */
		void init_ops ();
		
		/**
		 * Allocate a new Physical::Operator from the pool of available unused
		 * operators (ops).  Return 0 if we do not have a spare operator.
		 */
		Operator *new_op(OperatorKind kind);
		
		/**
		 * Free up one of the operators currently in use so that it can
		 * be used later.
		 */
		void free_op (Physical::Operator *op);
		
		//----------------------------------------------------------------------
		// Other resource management routines: expressions, boolean
		// expressions, strings, etc.
		//----------------------------------------------------------------------
		
		/**
		 * Allocate a new boolean expression from the pool of available
		 * boolean expressions (bexprs).  Return 0 if we do not have a
		 * spare one.
		 */
		BExpr *new_bexpr();

		/**
		 * Allocate a new arithmetic expression from the pool of available
		 * arithmetic expressions (exprs).  Return 0 if we do not have a
		 * spare one.
		 */		
		Expr *new_expr();
		
		/**
		 * Make a local copy of the string.  This is used to make local
		 * copy of string constants occurring in queries.  The memory for
		 * these constants was allocated within parser, which will be used
		 * to future queries ...
		 */ 
		
		char *copyString(const char *string);
		
		//----------------------------------------------------------------------
		
		/**
		 * Generate a physical plan from a logical plan.  This is called
		 * from within addLogicalQueryPlan		 
		 */ 
		int genPhysicalPlan(Logical::Operator *logPlan,
							Physical::Operator *&phyPlan);
		
		
		//----------------------------------------------------------------------
		//
		// Routines to construct various kinds of physical operators.
		// These routines are called from within genPhysicalPlan method.
		// All these routines take in two parameters:
		//
		// 1. A logical plan (logPlan)
		// 2. An array of physical plans (phyChildPlans)
		// 
		// The phyChildPlans[i] is the physical plan for the i-th subtree
		// below the root node of the logPlan.  The root node of the
		// logPlan is SELECT for mk_select () routine, PROJECT for
		// mk_project() routine, and so on.
		//
		// Each routine returns the physical plan for the entire logPlan -
		// it constructs the physical operator for the root and attaches
		// the phyChildPlans as child nodes of the root.
		//
		// All these routines are implemented in gen_phy_plan.cc
		//----------------------------------------------------------------------
		
		// PO_STREAM_SOURCE
		int mk_stream_source (Logical::Operator *logPlan,
							  Physical::Operator **phyChildPlans,
							  Physical::Operator *&phyPlan);
		
		// PO_RELN_SOURCE
		int mk_reln_source (Logical::Operator *logPlan,
							Physical::Operator **phyChildPlans,
							Physical::Operator *&phyPlan);
		
		// PO_ROW_WIN
		int mk_row_win (Logical::Operator *logPlan,
						Physical::Operator **phyChildPlans,
						Physical::Operator *&phyPlan);
		
		// PO_RANGE_WIN
		int mk_range_win (Logical::Operator *logPlan,
						  Physical::Operator **phyChildPlans,
						  Physical::Operator *&phyPlan);

		// PO_NOW_WIN
		int mk_now_win (Logical::Operator *logPlan,
						Physical::Operator **phyChildPlans,
						Physical::Operator *&phyPlan);

		// PO_PARTN_WIN
		int mk_partn_win (Logical::Operator *logPlan,
						  Physical::Operator **phyChildPlans,
						  Physical::Operator *&phyPlan);
		
		// PO_SELECT
		int mk_select (Logical::Operator *logPlan,
					   Physical::Operator **phyChildPlans,
					   Physical::Operator *&phyPlan);
		
		// PO_PROJECT
		int mk_project (Logical::Operator *logPlan,
						Physical::Operator **phyChildPlans,
						Physical::Operator *&phyPlan);

		// PO_JOIN
		int mk_join (Logical::Operator *logPlan,
					 Physical::Operator **phyChildPlans,
					 Physical::Operator *&phyPlan);
		
		// PO_STR_JOIN
		int mk_str_join (Logical::Operator *logPlan,
						 Physical::Operator **phyChildPlans,
						 Physical::Operator *&phyPlan);
		
		// PO_GROUP_AGGR
		int mk_group_aggr (Logical::Operator *logPlan,
						   Physical::Operator **phyChildPlans,
						   Physical::Operator *&phyPlan);

		// PO_DISTINCT
		int mk_distinct (Logical::Operator *logPlan,
						 Physical::Operator **phyChildPlans,
						 Physical::Operator *&phyPlan);

		// PO_ISTREAM
		int mk_istream (Logical::Operator *logPlan,
						Physical::Operator **phyChildPlans,
						Physical::Operator *&phyPlan);
		
		// PO_DSTREAM
		int mk_dstream (Logical::Operator *logPlan,
						Physical::Operator **phyChildPlans,
						Physical::Operator *&phyPlan);
		
		// PO_RSTREAM
		int mk_rstream (Logical::Operator *logPlan,
						Physical::Operator **phyChildPlans,
						Physical::Operator *&phyPlan);

		// PO_UNION
		int mk_union (Logical::Operator *logPlan,
					  Physical::Operator **phyChildPlans,
					  Physical::Operator *&phyPlan);
		
		// PO_EXCEPT
		int mk_except (Logical::Operator *logPlan,
					   Physical::Operator **phyChildPlans,
					   Physical::Operator *&phyPlan);
		
		// PO_QUERY_SOURCE
		int mk_qry_src (Physical::Operator *phyPlan,
						Physical::Operator *& qrySrc);
		
		// PO_OUTPUT
		int mk_output (Physical::Operator *phyPlan,
					   Physical::Operator *& output);

		// PO_SINK
		int mk_sink (Operator *op, Operator *&sink);
		
		//PO_DROP, by Thao Pham
		int mk_drop (Operator *child, Operator *&drop);
		//end of PO_DROP, by Thao Pham
		
		//---------------------------------------------------------------------
		// Help routines used during the physical plan generation
		//---------------------------------------------------------------------
		
		/// Update child to reflect that parent reads from the child 
		int addOutput (Operator *child, Operator *parent);		
		
		//----------------------------------------------------------------------
		//
		// Routines to transform objects from Logical:: namespace to
		// Physical:: namespace.  These are used within the Physiclal
		// operator construction routines above.
		//
		// Implemented in gen_phy_plan.cc
		//----------------------------------------------------------------------
		
		/// Logical::Attr -> Physical::Attr
		Physical::Attr transformAttr (Logical::Attr, Logical::Operator *);
		
		/// Logical::BExpr -> Physical::BExpr
		Physical::BExpr *transformBExpr (Logical::BExpr, Logical::Operator *);
		
		/// Logical::Expr -> Physical::Expr
		Physical::Expr *transformExpr (Logical::Expr *, Logical::Operator *);
		
		//----------------------------------------------------------------------
		// Plan transformation routines:
		//
		// These routines are used to perform transformations over the
		// global plan (cutting across all the queries).  These are called
		// from within 
		//----------------------------------------------------------------------
		
		/**
		 * Remove all the query source operators from the overall query
		 * plan.  See comment within addLogicalQueryPlan() definition to
		 * understand the rationale for having query sources.  
		 */
		
		int removeQuerySources ();

		/**
		 * Add sink operators to drain away output from operators with
		 * no other operator reading their inputs
		 */
		int addSinks ();
		
		/**
		 * Wherever possible, merge two select operators that apply
		 * different predicates into one select operator that applies all
		 * the predicates of both the select operators.  Also a select
		 * above a join is merged into the join.
		 */ 
		int mergeSelects ();
		
		/**
		 * Whenever possible merge a project above a join into the join.
		 */
		int mergeProjectsToJoin ();
		
		/**
		 * Each group by aggr. function requires some "internal"
		 * aggregations either to compute other aggregation functions
		 * (e.g., avg requires count) or for its implementation.
		 */
		
		int addIntAggrs ();
		
		// defined in gen_phy_plan.cc
		int mk_dummy_project (Operator *child, Operator *&project);
		
		//----------------------------------------------------------------------
		// Instantiation routines
		//----------------------------------------------------------------------
		
	/*	struct EvalContextInfo {
			TupleLayout *st_layout;
			ConstTupleLayout *ct_layout;
		};*/
		
		static const unsigned int SCRATCH_ROLE = 0;
		static const unsigned int CONST_ROLE = 1;
		
		//------------------------------------------------------------
		// Monitor related routines
		//------------------------------------------------------------
		
		void createSSGen ();
		

		/*int update_ss_gen (Physical::Operator *p_op,
						   Physical::Operator **opList, unsigned int,
						   Physical::Queue **queueList, unsigned int,
						   Physical::Store **storeList, unsigned int,
						   Physical::Synopsis **synList, unsigned int);
						   */
	};
}

#endif
