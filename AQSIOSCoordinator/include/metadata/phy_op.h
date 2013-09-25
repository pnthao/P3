#ifndef _PHY_OP_
#define _PHY_OP_

/**
 * @file     phy_op.h
 * @date     Aug. 21, 2004
 * @brief    Representation of physical operators in the system.
 */ 

#ifndef _CONSTANTS_
#include "common/constants.h"
#endif

#ifndef _TYPES_
#include "common/types.h"
#endif

#ifndef _AGGR_
#include "common/aggr.h"
#endif

#ifndef _OP_
#include "common/op.h"
#endif

#ifndef _CPP_OSTREAM_
#include <ostream>
#endif

#define MAX_NUM_CLIENTS 10

#include <list>
using namespace std;
namespace Physical {
	
	/**
	 * Representation of an attribute in a physical operator.  The member
	 * "input" identifies the input operator that produces the attribute,
	 * and "pos" identifies the position of the attribute in the output
	 * schema of the (input) operator.
	 */
	
	struct Attr {
		unsigned int input;
		unsigned int pos;
	};		
	
	/**
	 * Three kinds of expressions: constant values, attribute references,
	 * and complex expressions built using arithmetic operators.  Same
	 * as Logical::ExprKind.
	 */
	
	enum ExprKind {
		CONST_VAL, ATTR_REF, COMP_EXPR
	};
	
	/**
	 * Same as Logical::Expression, but now the definition of an attribute
	 * is different.
	 */
	struct Expr {
		ExprKind   kind;
		
		// output type of the expression
		Type       type; 
		
		union {
			int         ival; // type == INT && kind == CONST_VAL
			float       fval; // type == FLOAT && kind == CONST_VAL
			const char *sval; // type == CHAR && kind == CONST_VAL
			char        bval; // type == BYTE && kind == CONST_VAL
			
			struct {
				ArithOp  op;
				Expr    *left;
				Expr    *right;
			} COMP_EXPR;
			
			Attr   attr;
		} u;
	};
	
	struct BExpr {
		CompOp      op;
		Expr       *left;
		Expr       *right;
		BExpr      *next;  // next predicate in the conjunction
	};
	
	enum OperatorKind {
		// Selection
		PO_SELECT,
		
		// Projection
		PO_PROJECT,
		
		// Relation-Relation join
		PO_JOIN,
		
		// Stream-relation join
		PO_STR_JOIN,
		
		// Combinatin of a join followed by project
		PO_JOIN_PROJECT,

		// Combination of a str-join followed by project
		PO_STR_JOIN_PROJECT,
		
		// Group-by aggregation
		PO_GROUP_AGGR,
		
		// distinct 
		PO_DISTINCT,
		
		// row window operator
		PO_ROW_WIN,
		
		// range window op
		PO_RANGE_WIN,
				
		// partition window
		PO_PARTN_WIN,
		
		// Istream
		PO_ISTREAM,
		
		// Dstream
		PO_DSTREAM,
		
		// Rstream
		PO_RSTREAM,

		// Union
		PO_UNION,

		// Anti-semijoin
		PO_EXCEPT,
		
		// Source for a base stream
		PO_STREAM_SOURCE,
		
		// Base relation source
		PO_RELN_SOURCE,
		
		// "No-op" operator that sits on top of query views - other
		// queries that use the output of this query read from the query
		// source operator.  This is a dummy operator found at the
		// metadata level - when we generate the actual exec. operators we
		// will bypass it.
		PO_QUERY_SOURCE,
		
		// Output op that interfaces with the external world
		PO_OUTPUT,
		
		// Sink operator to consume tuples from unused ops
		PO_SINK,

		// System stream generator
		PO_SS_GEN,
		
		//drop operator by Thao Pham
		PO_DROP,
		PO_LWDROP,
		PO_HWDROP
		//end of drop operator by Thao Pham
	};	
	
	// Maximum number of inputs to an operator: we only consider unary and
	// binary operators now ...
	static const unsigned int MAX_IN_BRANCHING = 2;
	static const unsigned int MAX_GROUP_ATTRS = 10;
	static const unsigned int MAX_AGGR_ATTRS  = 10;

	// forward decl.
	struct Synopsis;
	struct Store;
	struct Queue;
	//query placement, by Thao Pham
	struct ExeInfo //the information about which node is executing the op and the corresponding load
	{
		double load_coefs;
		int nodeIDs;
	};
	struct Operator {
		
		/// indexes array PlanManagerImpl.ops
		unsigned int id;

		 //Query Class Scheduling by Lory Al Moakar
		 // the id of the query class that this operator belongs to
	 	 int query_class_id;
	  
		  //end of Query Class Scheduling by LAM
		
		/// next & prev pointers to arrange the operators as a linked list
		/// for resource management purposes
		Operator *next;
		Operator *prev;
		
		/// Type of operator
		OperatorKind  kind;
		
		/// Number of attributes in the output schema of the operator
		unsigned int numAttrs;
		
		/// Types of the various attributes in the output schema
		Type  attrTypes [MAX_ATTRS];
		
		/// Length of string attributes in the output schema
		unsigned int attrLen [MAX_ATTRS];
		
		/// Does the operator produce a stream
		bool bStream;
		
		/// Operators reading off from this operator
		Operator *outputs [MAX_OUT_BRANCHING];
		
		/// number of output operators
		unsigned int numOutputs;
		
		/// Operators which form the input to this operator
		Operator *inputs  [MAX_IN_BRANCHING];
		
		/// number of input operators
		unsigned int numInputs;
		
		//load coeficient; in the first implementation, each stream source has this info updated
		//from a node, assumed that there is no sharing at all either between or within the classes.
		//in the future, when sharing is enabled, two changes need to be considered:
		//First: the operator at the shared points needs to have its load coeficient updated
		//Second: the operator can be executed at multiple nodes (i.e., the sharing are split.
		//in such cases, load coef, and also node ID, have to become a list instead of a single value
		/*list<double> load_coefs;
		list<int> nodeIDs;*/
		list<ExeInfo> exeInfo;

		union {
			
			struct {
				// Id assigned by tableMgr
				unsigned int strId; 
				
				// Id of the tableSource operator
				unsigned int srcId;

				double input_rate;
			} STREAM_SOURCE;
			
			struct {
				// Id assigned by tableMgr
				unsigned int relId; 
				
				// Id of the tableSource operator
				unsigned int srcId;
				
			} RELN_SOURCE;
			
			struct {
				unsigned int outputId; // [[ Explanation ]]
				unsigned int queryId;
			} OUTPUT;
			
			struct {
				BExpr       *pred;
			} SELECT;
			
			struct {
				
				// project expressions
				Expr  *projs [MAX_ATTRS];
			} PROJECT;
			
			struct {
				
				// My   output   schema    is   concatenation   of   first
				// numOuterAttrs  from left  input and  numInnerAttrs from
				// right  input. (Assert(numOuterAttrs +  numInnerAttrs ==
				// numAttrs).  We  store this information  at construction
				// time since the schemas of the input operators can later
				// "expand"
				
				unsigned int numOuterAttrs;				
				unsigned int numInnerAttrs;
				
				// Join predicate
				BExpr *pred;
				
			} JOIN;
			
			struct {
				// My output schema is concatenation of first
				// numOuterAttrs from left input and numInnerAttrs from
				// right input. (Assert(numOuterAttrs + numInnerAttrs ==
				// numAttrs)
				
				unsigned int numOuterAttrs;				
				unsigned int numInnerAttrs;
				
				// Join predicate
				BExpr *pred;
				
			} STR_JOIN;
			
			struct {
				// Project expressions
				Expr *projs [MAX_ATTRS];
				
				// Join predicate
				BExpr *pred;
				
			} JOIN_PROJECT;
			
			struct {
				// Output projections
				Expr *projs [MAX_ATTRS];
				
				// Join predicate
				BExpr *pred;

			} STR_JOIN_PROJECT;
			
			struct {
				// grouping attributes
				Attr         groupAttrs [MAX_GROUP_ATTRS];
				
				// aggregated attributes
				Attr         aggrAttrs [MAX_AGGR_ATTRS];
				
				// aggregation function
				AggrFn       fn [MAX_AGGR_ATTRS];
				
				// ...
				unsigned int numGroupAttrs;
				
				// ...
				unsigned int numAggrAttrs;
				
			} GROUP_AGGR;
			
			struct {
				// Window size
				unsigned int numRows;
				
			} ROW_WIN;
			
			struct {
				// window size
				unsigned int timeUnits;
				
			} RANGE_WIN;
			
			struct {
				// Partitioning attributes
				Attr partnAttrs [MAX_GROUP_ATTRS];
				
				// ...
				unsigned int numPartnAttrs;
				
				// Window size
				unsigned int numRows;
				
			} PARTN_WIN;
			
			struct {				
				unsigned int numOutput;
			} SS_GEN;
			//specific parameters and data strutures for drop operator, by Thao Pham
			struct {
				unsigned int dropPercent; //percent of tuples to be drop
			//	bool active; //whether this drop is active or not -- drop percent = 0 means not active
				OperatorKind kind; //lightweight--PO_LWDROP or heavyweight--PO_HWDROP 
			} DROP;
			//end of parameters and data strutures for drop operator, by Thao Pham
		} u;

	};
	//overloading the << operator
	std::ostream &operator << (std::ostream &out, Operator *op);

}

#endif
