#if !defined(ISOLUTION_H_INCLUDED)
#define ISOLUTION_H_INCLUDED

#include <cassert>				// assert
#include <map>					// std::map
#include <string>				// std::string
#include <list>					// std::list
#include <vector>				// std::vector
#include <set>					// std::set
#include <iostream>
#include <sstream>
#include <fstream>

#include "ISolutionComp.h"
#include "PHRQ_base.h"
#include "NameDouble.h"
#include "global_structures.h"
class cxxISolution: public PHRQ_base
{

  public:
	cxxISolution(PHRQ_io *io=NULL);
	cxxISolution(const cxxISolution *is);
	virtual ~cxxISolution();
	std::string Get_units() const                     {return units;}
	void Set_units(std::string l_units)               {units = l_units;}
	void Set_units(const char * l_units)
	{
		if (l_units != NULL)
			this->units = std::string(l_units);
		else
			this->units.clear();
	}
	const char * Get_default_pe() const                {return default_pe;}
	void Set_default_pe(const char * pe)               {default_pe = pe;}
	bool Get_calc_density(void)                        {return this->calc_density;}
	void Set_calc_density(bool calc)                   {this->calc_density = calc;}
	std::map < std::string, cxxISolutionComp > &Get_comps(void) {return this->comps;}
	const std::map < std::string, cxxISolutionComp > &Get_comps(void)const {return this->comps;}
	void Set_comps(std::map < std::string, cxxISolutionComp > &c) {this->comps = c;}
	std::map<std::string, CReaction>& Get_pe_reactions(void) { return this->pe_reactions; }
	void Set_pe_reactions(std::map < std::string, CReaction >& pe) { this->pe_reactions = pe; }

  protected:
	friend class cxxISolutionComp;	// for this->pe access
	std::string units;
	bool calc_density;
	std::map < std::string, cxxISolutionComp > comps;
	std::map <std::string, CReaction> pe_reactions;
	const char * default_pe;
};

#endif // !defined(ISOLUTION_H_INCLUDED)
