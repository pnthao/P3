#ifndef _PHY_OP_
#include "metadata/phy_op.h"
#endif

using namespace Physical;
using namespace std;

std::ostream& Physical::operator << (std::ostream &out, Operator *op)
{
	// Operator type:
	out << op -> id << ". ";

	switch (op -> kind) {
	case PO_SELECT:
		out << "Select";
		break;
	case PO_PROJECT:
		out << "Project";
		break;
	case PO_JOIN:
		out << "Join";
		break;
	case PO_JOIN_PROJECT:
		out << "Join (+Project)";
		break;

	case PO_STR_JOIN:
		out << "StreamJoin";
		break;

	case PO_STR_JOIN_PROJECT:
		out << "StreamJoin (+Project)";
		break;

	case PO_ISTREAM:
		out << "Istream";
		break;

	case PO_DSTREAM:
		out << "Dstream";
		break;

	case PO_RSTREAM:
		out << "Rstream";
		break;

	case PO_STREAM_SOURCE:
		out << "StreamSource";
		break;

	case PO_ROW_WIN:
		out << "RowWin";
		break;

	case PO_RANGE_WIN:
		out << "RangeWin";
		break;

	case PO_GROUP_AGGR:
		out << "Groupby Aggr";
		break;

	case PO_DISTINCT:
		out << "Distinct";
		break;

	case PO_RELN_SOURCE:
		out << "RelationSource";
		break;

	case PO_PARTN_WIN:
		out << "PartitionWindow";
		break;

	case PO_QUERY_SOURCE:
		out << "QuerySource";
		break;

	case PO_OUTPUT:
		out << "Output";
		break;

	case PO_SINK:
		out << "Sink";
		break;

	case PO_UNION:
		out << "Union";
		break;

	case PO_EXCEPT:
		out << "Except";
		break;

	case PO_SS_GEN:
		out << "SSGen";
		break;
		//drop operator, by Thao Pham
	case PO_DROP:
		out << "Drop";
		break;

		//drop operator, by Thao Pham
	default:
		out << "(Unknown)";
		break;
	}

	out << ": " << endl;

	// Stream/Reln
	out << "Output: ";
	if (op -> bStream)
		out << "Stream";
	else
		out << "Relation";
	out << endl;

	// Schema information
	out << "Schema: <";

	for (unsigned int a = 0 ; a < op -> numAttrs ; a++) {

		out << op -> attrTypes [a];

		if (op -> attrTypes [a] == CHAR) {
			out << "("
					<< op -> attrLen [a]
					                  << ")";
		}

		if (a < op -> numAttrs - 1)
			out << ", ";
	}
	out << ">" << endl;

	// Input information:
	out << "Input: ";

	for (unsigned int i = 0 ; i < op -> numInputs ; i++) {
		out << op -> inputs [i] -> id;

		if (i < op -> numInputs - 1)
			out << ", ";
	}
	out << endl;

	return out;
}
