// ExchComp.cxx: implementation of the cxxExchComp class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif
#include <iostream>				// std::cout std::cerr
#include <cassert>				// assert
#include <algorithm>			// std::sort
#include <float.h>

#include "Utils.h"				// define first
#include "Phreeqc.h"
#include "ExchComp.h"
#include "phqalloc.h"
#include "Dictionary.h"

#if defined(PHREEQCI_GUI)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cxxExchComp::cxxExchComp(PHRQ_io *io)
	//
	// default constructor for cxxExchComp 
	//
	: PHRQ_base(io)
{
	totals.type = cxxNameDouble::ND_ELT_MOLES;
	la = 0.0;
	charge_balance = 0.0;
	phase_proportion = 0.0;
	formula_z = 0.0;
}
cxxExchComp::~cxxExchComp()
{
}

void
cxxExchComp::dump_xml(std::ostream & s_oss, unsigned int indent) const
{
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Exch_Comp element and attributes

	s_oss << indent0 << "formula=\"" << this->formula << "\"" << "\n";
	s_oss << indent0 << "formula_z=\"" << this->
		formula_z << "\"" << "\n";
	s_oss << indent0 << "la=\"" << this->la << "\"" << "\n";
	s_oss << indent0 << "charge_balance=\"" << this->
		charge_balance << "\"" << "\n";
	if (this->phase_name.size() != 0)
	{
		s_oss << indent0 << "phase_name=\"" << this->phase_name << "\"" << "\n";
	}
	if (this->rate_name.size() != 0)
	{
		s_oss << indent0 << "rate_name=\"" << this->
			rate_name << "\"" << "\n";
	}
	s_oss << indent0 << "phase_proportion=\"" << this->
		phase_proportion << "\"" << "\n";

	// totals
	s_oss << indent0;
	s_oss << "<totals " << "\n";
	this->totals.dump_xml(s_oss, indent + 1);
}

void
cxxExchComp::dump_raw(std::ostream & s_oss, unsigned int indent) const
{
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Exch_Comp element and attributes
	//s_oss << indent1 << "# critical values" << "\n";

	// totals
	s_oss << indent0 << "# EXCHANGE_MODIFY candidate identifiers #\n";
	s_oss << indent0;
	s_oss << "-totals" << "\n";
	this->totals.dump_raw(s_oss, indent + 1);

	s_oss << indent0 << "-charge_balance          " << this->charge_balance << "\n";

	//s_oss << indent1 << "# Noncritical values" << "\n";
	s_oss << indent0 << "-la                      " << this->la << "\n";

	if (this->phase_name.size() != 0)
	{
		s_oss << indent0 << "-phase_name              " << this->phase_name << "\n";
	}
	if (this->rate_name.size() != 0)
	{
		s_oss << indent0 << "-rate_name               " << this->rate_name << "\n";
	}
	s_oss << indent0 << "-phase_proportion        " << this->phase_proportion << "\n";
	s_oss << indent0 << "-formula_z               " << this->formula_z << "\n";
}

void
cxxExchComp::read_raw(CParser & parser, bool check)
{
	std::string str;

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;

	opt_save = CParser::OPT_ERROR;
	bool la_defined(false);
	bool charge_balance_defined(false);
	bool formula_z_defined(false);

	for (;;)
	{
		int opt = parser.get_option(vopts, next_char);
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}

		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_KEYWORD;
			// Allow return to Exchange for more processing
			break;

		case 0:				// formula
			warning_msg("-formula ignored. Defined with -component.");
			break;

		case 1:				// moles
			parser.warning_msg("-moles is an obsolete identifier");
			break;

		case 2:				// la
			if (!(parser.get_iss() >> this->la))
			{
				this->la = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for la.",
								 PHRQ_io::OT_CONTINUE);
			}
			la_defined = true;
			break;

		case 3:				// charge_balance
			if (!(parser.get_iss() >> this->charge_balance))
			{
				this->charge_balance = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for charge_balance.",
								 PHRQ_io::OT_CONTINUE);
			}
			charge_balance_defined = true;
			break;

		case 4:				// phase_name
			if (!(parser.get_iss() >> str))
			{
				this->phase_name.clear();
				parser.incr_input_error();
				parser.error_msg("Expected string value for phase_name.",
								 PHRQ_io::OT_CONTINUE);
			}
			else
			{
				this->phase_name = str;
			}
			break;

		case 5:				// rate_name
			if (!(parser.get_iss() >> str))
			{
				this->rate_name.clear();
				parser.incr_input_error();
				parser.error_msg("Expected string value for rate_name.",
								 PHRQ_io::OT_CONTINUE);
			}
			else
			{
				this->rate_name = str;
			}
			break;

		case 6:				// formula_z
			if (!(parser.get_iss() >> this->formula_z))
			{
				this->formula_z = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for formula_z.",
								 PHRQ_io::OT_CONTINUE);
			}
			formula_z_defined = true;
			break;

		case 7:				// phase_proportion
			if (!(parser.get_iss() >> this->phase_proportion))
			{
				this->phase_proportion = 0;
				parser.incr_input_error();
				parser.
					error_msg("Expected numeric value for phase_proportion.",
							  PHRQ_io::OT_CONTINUE);
			}
			break;

		case 8:				// totals
			if (this->totals.read_raw(parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected element name and molality for ExchComp totals.",
					 PHRQ_io::OT_CONTINUE);
			}
			opt_save = 8;
			break;

		case 9:				// formula_totals
			parser.warning_msg("-formula_totals is an obsolete identifier");
			break;
		}
		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	if (check)
	{
		// members that must be defined
		if (la_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("La not defined for ExchComp input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (charge_balance_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Charge_balance not defined for ExchComp input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (formula_z_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Formula_z not defined for ExchComp input.",
				PHRQ_io::OT_CONTINUE);
		}
	}
}
void
cxxExchComp::add(const cxxExchComp & addee, LDBLE extensive)
{
	LDBLE f1, f2;
	if (extensive == 0.0)
		return;
	if (addee.formula.size() == 0)
		return;
	// this and addee must have same formula
	// otherwise generate a new exchcomp with multiply
	f1 = 0.5;
	f2 = 0.5;

	if (this->formula.size() == 0 && addee.formula.size() == 0)
	{
		return;
	}
	assert(this->formula == addee.formula);
	assert(this->formula_z == addee.formula_z);
	if (this->formula.size() == 0 && addee.formula.size() != 0)
	{
		this->formula = addee.formula;
	}
	this->totals.add_extensive(addee.totals, extensive);
	this->la = f1 * this->la + f2 * addee.la;
	this->charge_balance += addee.charge_balance * extensive;

	//if (this->phase_name != addee.phase_name)
	if (Phreeqc::strcmp_nocase(this->phase_name.c_str(), addee.phase_name.c_str()) != 0)
	{
		std::ostringstream oss;
		oss <<
			"Cannot mix two exchange components with same formula and different related phases, "
			<< this->formula;
		error_msg(oss.str().c_str(), CONTINUE);
		return;
	}
	else if (this->phase_name.size() != 0)
	{
		this->phase_proportion =
			this->phase_proportion * f1 + addee.phase_proportion * f2;
	}
	//if (this->rate_name != addee.rate_name)
	if (Phreeqc::strcmp_nocase(this->rate_name.c_str(), addee.rate_name.c_str()) != 0)
	{
		std::ostringstream oss;
		oss <<
			"Cannot mix two exchange components with same formula and different related kinetics, "
			<< this->formula;
		error_msg(oss.str().c_str(), CONTINUE);
		return;
	}
	else if (this->rate_name.size() != 0)
	{
		this->phase_proportion =
			this->phase_proportion * f1 + addee.phase_proportion * f2;
	}
	if ((this->rate_name.size() != 0 && addee.phase_name.size() != 0) ||
		(this->phase_name.size() != 0 && addee.rate_name.size() != 0))
	{
		std::ostringstream oss;
		oss <<
			"Cannot mix exchange components related to phase with exchange components related to kinetics, "
			<< this->formula;
		error_msg(oss.str().c_str(), CONTINUE);
		return;
	}
}
void
cxxExchComp::multiply(LDBLE extensive)
{ 
	this->totals.multiply(extensive);
	this->charge_balance *= extensive;  
	//this->phase_proportion *= extensive;
}

void
cxxExchComp::Serialize(Dictionary & dictionary, std::vector < int >&ints, std::vector < double >&doubles)
{
	ints.push_back(dictionary.Find(this->formula));
	this->totals.Serialize(dictionary, ints, doubles);
	doubles.push_back(this->la);
	doubles.push_back(this->charge_balance);
	ints.push_back(dictionary.Find(this->phase_name));
	doubles.push_back(this->phase_proportion);
	ints.push_back(dictionary.Find(this->rate_name));
	doubles.push_back(this->formula_z);
}

void
cxxExchComp::Deserialize(Dictionary & dictionary, std::vector < int >&ints, 
	std::vector < double >&doubles, int &ii, int &dd)
{
	this->formula = dictionary.GetWords()[ints[ii++]];
	this->totals.Deserialize(dictionary, ints, doubles, ii, dd);
	this->la = doubles[dd++];
	this->charge_balance = doubles[dd++];
	this->phase_name = dictionary.GetWords()[ints[ii++]];
	this->phase_proportion = doubles[dd++];
	this->rate_name = dictionary.GetWords()[ints[ii++]];
	this->formula_z = doubles[dd++];
}


const std::vector< std::string >::value_type temp_vopts[] = {
	std::vector< std::string >::value_type("formula"),	            // 0 
	std::vector< std::string >::value_type("moles"),	            // 1
	std::vector< std::string >::value_type("la"),	                // 2 
	std::vector< std::string >::value_type("charge_balance"),	    // 3 
	std::vector< std::string >::value_type("phase_name"),	        // 4 
	std::vector< std::string >::value_type("rate_name"),	        // 5 
	std::vector< std::string >::value_type("formula_z"),	        // 6
	std::vector< std::string >::value_type("phase_proportion"),	    // 7 
	std::vector< std::string >::value_type("totals"),	            // 8
	std::vector< std::string >::value_type("formula_totals")	    // 9	
};									   
const std::vector< std::string > cxxExchComp::vopts(temp_vopts, temp_vopts + sizeof temp_vopts / sizeof temp_vopts[0]);
