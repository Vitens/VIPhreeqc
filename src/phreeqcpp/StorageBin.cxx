// StorageBin.cxx: implementation of the cxxStorageBin class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif
#include <fstream>
#include <iostream>				// std::cout std::cerr
#include <cassert>				// assert
#include <algorithm>			// std::sort

#include "Utils.h"				// define first
#include "Phreeqc.h"
#include "NameDouble.h"
#include "StorageBin.h"
#include "SSassemblage.h"
#include "Solution.h"
#include "Exchange.h"
#include "GasPhase.h"
#include "cxxKinetics.h"
#include "PPassemblage.h"
#include "SS.h"
#include "SSassemblage.h"
#include "Surface.h"
#include "cxxMix.h"
#include "Reaction.h"
#include "Temperature.h"
#include "phqalloc.h"
#include "Use.h"

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
cxxStorageBin::cxxStorageBin(PHRQ_io *io)
:
PHRQ_base(io)
{
	// default constructor for cxxStorageBin 
	this->system.Set_io(io);
	this->system.Initialize();
}

cxxStorageBin::cxxStorageBin(cxxUse &use_ref, PHRQ_io *io)
:
PHRQ_base(io)
{
	this->system.Set_io(io);
	this->system.Initialize();
	// Solution
	if (use_ref.Get_solution_ptr() != NULL)
	{
		this->Set_Solution(use_ref.Get_solution_ptr()->Get_n_user(), use_ref.Get_solution_ptr());
	}

	// Exchange
	if (use_ref.Get_exchange_ptr() != NULL)
	{
		this->Set_Exchange(use_ref.Get_exchange_ptr()->Get_n_user(), use_ref.Get_exchange_ptr());
	}

	// gas_phase
	if (use_ref.Get_gas_phase_ptr() != NULL)
	{
		this->Set_GasPhase(use_ref.Get_gas_phase_ptr()->Get_n_user(), use_ref.Get_gas_phase_ptr());
	}

	// kinetics
	if (use_ref.Get_kinetics_ptr() != NULL)
	{
		this->Set_Kinetics(use_ref.Get_kinetics_ptr()->Get_n_user(), use_ref.Get_kinetics_ptr());
	}

	// pp_assemblage
	if (use_ref.Get_pp_assemblage_ptr() != NULL)
	{
		this->Set_PPassemblage(use_ref.Get_pp_assemblage_ptr()->Get_n_user(), use_ref.Get_pp_assemblage_ptr());
	}

	// ss_assemblage
	if (use_ref.Get_ss_assemblage_ptr() != NULL)
	{
		this->Set_SSassemblage(use_ref.Get_ss_assemblage_ptr()->Get_n_user(), use_ref.Get_ss_assemblage_ptr());
	}

	// surface
	if (use_ref.Get_surface_ptr() != NULL)
	{
		this->Set_Surface(use_ref.Get_surface_ptr()->Get_n_user(), use_ref.Get_surface_ptr());
	}

	// mix
	if (use_ref.Get_mix_ptr() != NULL)
	{
		this->Set_Mix(use_ref.Get_mix_ptr()->Get_n_user(), use_ref.Get_mix_ptr());
	}

	// reaction
	if (use_ref.Get_reaction_ptr() != NULL)
	{
		this->Set_Reaction(use_ref.Get_reaction_ptr()->Get_n_user(), use_ref.Get_reaction_ptr());
	}

	// reaction temperature
	if (use_ref.Get_temperature_ptr() != NULL)
	{
		this->Set_Temperature(use_ref.Get_temperature_ptr()->Get_n_user(), use_ref.Get_temperature_ptr());
	}

	// reaction pressure
	if (use_ref.Get_pressure_ptr() != NULL)
	{
		this->Set_Pressure(use_ref.Get_pressure_ptr()->Get_n_user(), use_ref.Get_pressure_ptr());
	}
}
cxxStorageBin::~cxxStorageBin()
{
}
void
cxxStorageBin::Add(cxxStorageBin &src, int n)
{
	// Solution
	if (src.Get_Solution(n) != NULL)
	{
		this->Set_Solution(n, src.Get_Solution(n));
	}

	// Exchange
	if (src.Get_Exchange(n) != NULL)
	{
		this->Set_Exchange(n, src.Get_Exchange(n));
	}

	// gas_phase
	if (src.Get_GasPhase(n) != NULL)
	{
		this->Set_GasPhase(n, src.Get_GasPhase(n));
	}

	// kinetics
	if (src.Get_Kinetics(n) != NULL)
	{
		this->Set_Kinetics(n, src.Get_Kinetics(n));
	}

	// pp_assemblage
	if (src.Get_PPassemblage(n) != NULL)
	{
		this->Set_PPassemblage(n, src.Get_PPassemblage(n));
	}

	// ss_assemblage
	if (src.Get_SSassemblage(n) != NULL)
	{
		this->Set_SSassemblage(n, src.Get_SSassemblage(n));
	}

	// surface
	if (src.Get_Surface(n) != NULL)
	{
		this->Set_Surface(n, src.Get_Surface(n));
	}

	// mix
	if (src.Get_Mix(n) != NULL)
	{
		this->Set_Mix(n, src.Get_Mix(n));
	}

	// reaction
	if (src.Get_Reaction(n) != NULL)
	{
		this->Set_Reaction(n, src.Get_Reaction(n));
	}

	// reaction temperature
	if (src.Get_Temperature(n) != NULL)
	{
		this->Set_Temperature(n, src.Get_Temperature(n));
	}

	// reaction pressure
	if (src.Get_Pressure(n) != NULL)
	{
		this->Set_Pressure(n, src.Get_Pressure(n));
	}
}
void
cxxStorageBin::Add_uz(cxxStorageBin &uzbin)
{
	cxxMix mx;
	mx.Add(0, 1.0);
	mx.Add(1, 1.0);

	// Solution

	// Exchange
	{
		std::map<int, cxxExchange>::iterator it_uz = uzbin.Get_Exchangers().begin();
		std::map<int, cxxExchange> temp_map;
		for (; it_uz != uzbin.Get_Exchangers().end(); it_uz++)
		{
			int n_user = it_uz->second.Get_n_user();
			std::map < int, cxxExchange >::iterator it_sz = this->Exchangers.find(n_user);
			if (it_sz == this->Exchangers.end())
			{
				this->Exchangers[n_user] = it_uz->second;
			}
			else
			{
				temp_map[0] = it_uz->second;
				temp_map[1] = it_sz->second;
				cxxExchange temp_entity(temp_map, mx, n_user);
				this->Exchangers[n_user] = temp_entity;
			}
		}
	}

	// gas_phase
	{
		std::map<int, cxxGasPhase>::iterator it_uz = uzbin.Get_GasPhases().begin();
		std::map<int, cxxGasPhase> temp_map;
		for (; it_uz != uzbin.Get_GasPhases().end(); it_uz++)
		{
			int n_user = it_uz->second.Get_n_user();
			std::map < int, cxxGasPhase >::iterator it_sz = this->GasPhases.find(n_user);
			if (it_sz == this->GasPhases.end())
			{
				this->GasPhases[n_user] = it_uz->second;
			}
			else
			{
				temp_map[0] = it_uz->second;
				temp_map[1] = it_sz->second;
				cxxGasPhase temp_entity(temp_map, mx, n_user);
				this->GasPhases[n_user] = temp_entity;
			}
		}
	}

	// kinetics
	{
		std::map<int, cxxKinetics>::iterator it_uz = uzbin.Get_Kinetics().begin();
		std::map<int, cxxKinetics> temp_map;
		for (; it_uz != uzbin.Get_Kinetics().end(); it_uz++)
		{
			int n_user = it_uz->second.Get_n_user();
			std::map < int, cxxKinetics >::iterator it_sz = this->Kinetics.find(n_user);
			if (it_sz == this->Kinetics.end())
			{
				this->Kinetics[n_user] = it_uz->second;
			}
			else
			{
				temp_map[0] = it_uz->second;
				temp_map[1] = it_sz->second;
				cxxKinetics temp_entity(temp_map, mx, n_user);
				this->Kinetics[n_user] = temp_entity;
			}
		}
	}

	// pp_assemblage
	{
		std::map<int, cxxPPassemblage>::iterator it_uz = uzbin.Get_PPassemblages().begin();
		std::map<int, cxxPPassemblage> temp_map;
		for (; it_uz != uzbin.Get_PPassemblages().end(); it_uz++)
		{
			int n_user = it_uz->second.Get_n_user();
			std::map < int, cxxPPassemblage >::iterator it_sz = this->PPassemblages.find(n_user);
			if (it_sz == this->PPassemblages.end())
			{
				this->PPassemblages[n_user] = it_uz->second;
			}
			else
			{
				temp_map[0] = it_uz->second;
				temp_map[1] = it_sz->second;
				cxxPPassemblage temp_entity(temp_map, mx, n_user);
				this->PPassemblages[n_user] = temp_entity;
			}
		}
	}

	// ss_assemblage
	{
		std::map<int, cxxSSassemblage>::iterator it_uz = uzbin.Get_SSassemblages().begin();
		std::map<int, cxxSSassemblage> temp_map;
		for (; it_uz != uzbin.Get_SSassemblages().end(); it_uz++)
		{
			int n_user = it_uz->second.Get_n_user();
			std::map < int, cxxSSassemblage >::iterator it_sz = this->SSassemblages.find(n_user);
			if (it_sz == this->SSassemblages.end())
			{
				this->SSassemblages[n_user] = it_uz->second;
			}
			else
			{
				temp_map[0] = it_uz->second;
				temp_map[1] = it_sz->second;
				cxxSSassemblage temp_entity(temp_map, mx, n_user);
				this->SSassemblages[n_user] = temp_entity;
			}
		}
	}

	// surface
	{
		std::map<int, cxxSurface>::iterator it_uz = uzbin.Get_Surfaces().begin();
		std::map<int, cxxSurface> temp_map;
		for (; it_uz != uzbin.Get_Surfaces().end(); it_uz++)
		{
			int n_user = it_uz->second.Get_n_user();
			std::map < int, cxxSurface >::iterator it_sz = this->Surfaces.find(n_user);
			if (it_sz == this->Surfaces.end())
			{
				this->Surfaces[n_user] = it_uz->second;
			}
			else
			{
				temp_map[0] = it_uz->second;
				temp_map[1] = it_sz->second;
				cxxSurface temp_entity(temp_map, mx, n_user);
				this->Surfaces[n_user] = temp_entity;
			}
		}
	}

	// mix

	// reaction

	// reaction temperature

	// reaction pressure
}
void
cxxStorageBin::Copy(int destination, int source)
{
	if (destination == source)
		return;
	this->Remove(destination);
	// Solution
	{
		std::map < int, cxxSolution >::iterator it = this->Solutions.find(source);
		if (it != this->Solutions.end())
		{
			this->Set_Solution(destination, &(it->second));
		}
	}

	// Exchange
	{
		std::map < int, cxxExchange >::iterator it = this->Exchangers.find(source);
		if (it != this->Exchangers.end())
		{
			this->Set_Exchange(destination, &(it->second));
		}
	}

	// gas_phase
	{
		std::map < int, cxxGasPhase >::iterator it = this->GasPhases.find(source);
		if (it != this->GasPhases.end())
		{
			this->Set_GasPhase(destination, &(it->second));
		}
	}
	// kinetics
	{
		std::map < int, cxxKinetics >::iterator it = this->Kinetics.find(source);
		if (it != this->Kinetics.end())
		{
			this->Set_Kinetics(destination, &(it->second));
		}
	}
	// pp_assemblage
	{
		std::map < int, cxxPPassemblage >::iterator it = this->PPassemblages.find(source);
		if (it != this->PPassemblages.end())
		{
			this->Set_PPassemblage(destination, &(it->second));
		}
	}
	// ss_assemblage
	{
		std::map < int, cxxSSassemblage >::iterator it = this->SSassemblages.find(source);
		if (it != this->SSassemblages.end())
		{
			this->Set_SSassemblage(destination, &(it->second));
		}
	}
	// surface
	{
		std::map < int, cxxSurface >::iterator it = this->Surfaces.find(source);
		if (it != this->Surfaces.end())
		{
			this->Set_Surface(destination, &(it->second));
		}
	}
	// mix
	{
		std::map < int, cxxMix >::iterator it =	this->Mixes.find(source);
		if (it != this->Mixes.end())
		{
			this->Set_Mix(destination, &(it->second));
		}
	}
	// reaction
	{
		std::map < int, cxxReaction >::iterator it = this->Reactions.find(source);
		if (it != this->Reactions.end())
		{
			this->Set_Reaction(destination, &(it->second));
		}
	}
	// reaction temperature
	{
		std::map < int, cxxTemperature >::iterator it = this->Temperatures.find(source);
		if (it != this->Temperatures.end())
		{
			this->Set_Temperature(destination, &(it->second));
		}
	}

	// reaction pressure
	{
		this->Set_Pressure(destination, Utilities::Rxn_find(this->Pressures, source));
	}
}

cxxSolution *
cxxStorageBin::Get_Solution(int n_user)
{
	if (this->Solutions.find(n_user) != this->Solutions.end())
	{
		return (&(this->Solutions.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_Solution(int n_user, cxxSolution * entity)
{
	if (entity == NULL)
		return;
	Solutions[n_user] = *entity;
	Solutions.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Solution(int n_user, cxxSolution & entity)
{
	Solutions[n_user] = entity;
	Solutions.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Solution(int n_user)
{
	Solutions.erase(n_user);
}

cxxExchange *
cxxStorageBin::Get_Exchange(int n_user)
{
	if (this->Exchangers.find(n_user) != this->Exchangers.end())
	{
		return (&(this->Exchangers.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_Exchange(int n_user, cxxExchange * entity)
{
	if (entity == NULL)
		return;
	Exchangers[n_user] = *entity;
	Exchangers.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Exchange(int n_user, cxxExchange & entity)
{
	Exchangers[n_user] = entity;
	Exchangers.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Exchange(int n_user)
{
	Exchangers.erase(n_user);
}

cxxPPassemblage *
cxxStorageBin::Get_PPassemblage(int n_user)
{
	if (this->PPassemblages.find(n_user) != this->PPassemblages.end())
	{
		return (&(this->PPassemblages.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_PPassemblage(int n_user, cxxPPassemblage * entity)
{
	if (entity == NULL)
		return;
	PPassemblages[n_user] = *entity;
	PPassemblages.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_PPassemblage(int n_user, cxxPPassemblage & entity)
{
	PPassemblages[n_user] = entity;
	PPassemblages.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_PPassemblage(int n_user)
{
	PPassemblages.erase(n_user);
}

cxxGasPhase *
cxxStorageBin::Get_GasPhase(int n_user)
{
	if (this->GasPhases.find(n_user) != this->GasPhases.end())
	{
		return (&(this->GasPhases.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_GasPhase(int n_user, cxxGasPhase * entity)
{
	if (entity == NULL)
		return;
	GasPhases[n_user] = *entity;
	GasPhases.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_GasPhase(int n_user, cxxGasPhase & entity)
{
	GasPhases[n_user] = entity;
	GasPhases.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_GasPhase(int n_user)
{
	GasPhases.erase(n_user);
}

cxxSSassemblage *
cxxStorageBin::Get_SSassemblage(int n_user)
{
	if (this->SSassemblages.find(n_user) != this->SSassemblages.end())
	{
		return (&(this->SSassemblages.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_SSassemblage(int n_user, cxxSSassemblage * entity)
{
	if (entity == NULL)
		return;
	SSassemblages[n_user] = *entity;
	SSassemblages.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_SSassemblage(int n_user, cxxSSassemblage & entity)
{
	SSassemblages[n_user] = entity;
	SSassemblages.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_SSassemblage(int n_user)
{
	SSassemblages.erase(n_user);
}

cxxKinetics *
cxxStorageBin::Get_Kinetics(int n_user)
{
	if (this->Kinetics.find(n_user) != this->Kinetics.end())
	{
		return (&(this->Kinetics.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_Kinetics(int n_user, cxxKinetics * entity)
{
	if (entity == NULL)
		return;
	Kinetics[n_user] = *entity;
	Kinetics.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Kinetics(int n_user, cxxKinetics & entity)
{
	Kinetics[n_user] = entity;
	Kinetics.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Kinetics(int n_user)
{
	Kinetics.erase(n_user);
}

cxxSurface *
cxxStorageBin::Get_Surface(int n_user)
{
	if (this->Surfaces.find(n_user) != this->Surfaces.end())
	{
		return (&(this->Surfaces.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_Surface(int n_user, cxxSurface * entity)
{
	if (entity == NULL)
		return;
	Surfaces[n_user] = *entity;
	Surfaces.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Surface(int n_user, cxxSurface & entity)
{
	Surfaces[n_user] = entity;
	Surfaces.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Surface(int n_user)
{
	Surfaces.erase(n_user);
}

cxxMix *
cxxStorageBin::Get_Mix(int n_user)
{
	if (this->Mixes.find(n_user) != this->Mixes.end())
	{
		return (&(this->Mixes.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_Mix(int n_user, cxxMix * entity)
{
	if (entity == NULL)
		return;
	Mixes[n_user] = *entity;
	Mixes.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Mix(int n_user, cxxMix & entity)
{
	Mixes[n_user] = entity;
	Mixes.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Mix(int n_user)
{
	Mixes.erase(n_user);
}

cxxReaction *
cxxStorageBin::Get_Reaction(int n_user)
{
	if (this->Reactions.find(n_user) != this->Reactions.end())
	{
		return (&(this->Reactions.find(n_user)->second));
	}
	return (NULL);
}
void 
cxxStorageBin::Set_Reaction(int n_user, cxxReaction * entity)
{
	if (entity == NULL)
		return;
	Reactions[n_user] = *entity;
	Reactions.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Reaction(int n_user, cxxReaction & entity)
{
	Reactions[n_user] = entity;
	Reactions.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Reaction(int n_user)
{
	Reactions.erase(n_user);
}

cxxTemperature *
cxxStorageBin::Get_Temperature(int n_user)
{
	if (this->Temperatures.find(n_user) != this->Temperatures.end())
	{
		return (&(this->Temperatures.find(n_user)->second));
	}
	return (NULL);
}

void 
cxxStorageBin::Set_Temperature(int n_user, cxxTemperature * entity)
{
	if (entity == NULL)
		return;
	Temperatures[n_user] = *entity;
	Temperatures.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Set_Temperature(int n_user, cxxTemperature & entity)
{
	Temperatures[n_user] = entity;
	Temperatures.find(n_user)->second.Set_n_user_both(n_user);
}
void 
cxxStorageBin::Remove_Temperature(int n_user)
{
	Temperatures.erase(n_user);
}

cxxPressure *
cxxStorageBin::Get_Pressure(int n_user)
{
	return Utilities::Rxn_find(this->Pressures, n_user);
}

void
cxxStorageBin::Set_Pressure(int n_user, cxxPressure * entity)
{
	if (entity == NULL)
		return;
	Pressures[n_user] = *entity;
	Pressures.find(n_user)->second.Set_n_user_both(n_user);
}
void
cxxStorageBin::Set_Pressure(int n_user, cxxPressure & entity)
{
	Pressures[n_user] = entity;
	Pressures.find(n_user)->second.Set_n_user_both(n_user);
}
void 

cxxStorageBin::Remove_Pressure(int n_user)
{
	Pressures.erase(n_user);
}

std::map < int, cxxSolution > &
cxxStorageBin::Get_Solutions()
{
	return this->Solutions;
}
std::map < int, cxxExchange > &
cxxStorageBin::Get_Exchangers()
{
	return this->Exchangers;
}
std::map < int, cxxGasPhase > &
cxxStorageBin::Get_GasPhases() 
{
	return this->GasPhases;
}
std::map < int, cxxKinetics > &
cxxStorageBin::Get_Kinetics()
{
	return this->Kinetics;
}
std::map < int, cxxPPassemblage > &
cxxStorageBin::Get_PPassemblages()
{
	return this->PPassemblages;
}
std::map < int, cxxSSassemblage > &
cxxStorageBin::Get_SSassemblages()
{
	return this->SSassemblages;
}
std::map < int, cxxSurface > &
cxxStorageBin::Get_Surfaces()
{
	return this->Surfaces;
}
std::map < int, cxxMix > &
cxxStorageBin::Get_Mixes()
{
	return this->Mixes;
}
std::map < int, cxxReaction > &
cxxStorageBin::Get_Reactions()
{
	return this->Reactions;
}
std::map < int, cxxTemperature > &
cxxStorageBin::Get_Temperatures()
{
	return this->Temperatures;
}
std::map < int, cxxPressure > &
cxxStorageBin::Get_Pressures()
{
	return this->Pressures;
}
void
cxxStorageBin::dump_raw(std::ostream & s_oss, unsigned int indent) const
{
	// Dump all data
	s_oss.precision(DBL_DIG - 1);

	// Solutions
	Utilities::Rxn_dump_raw(Solutions, s_oss, indent);

	// Exchange
	Utilities::Rxn_dump_raw(Exchangers, s_oss, indent);

	// Gas Phases
	Utilities::Rxn_dump_raw(GasPhases, s_oss, indent);

	// Kinetics
	Utilities::Rxn_dump_raw(Kinetics, s_oss, indent);

	// PPassemblage
	Utilities::Rxn_dump_raw(PPassemblages, s_oss, indent);

	// SSassemblage
	Utilities::Rxn_dump_raw(SSassemblages, s_oss, indent);

	// Surface
	Utilities::Rxn_dump_raw(Surfaces, s_oss, indent);

	// Mix
	Utilities::Rxn_dump_raw(Mixes, s_oss, indent);

	// Reactions
	Utilities::Rxn_dump_raw(Reactions, s_oss, indent);

	// Temperature
	Utilities::Rxn_dump_raw(Temperatures, s_oss, indent);
}

void
cxxStorageBin::dump_raw(std::ostream & s_oss, int n, unsigned int indent, int *n_out)
{
	// Dump one user number, optionally change number from n to n_out
	int n_user_local = (n_out != NULL) ? *n_out : n;
	s_oss.precision(DBL_DIG - 1);

	// Solutions
	if (this->Get_Solution(n) != NULL)
	{
		this->Get_Solution(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Exchange
	if (this->Get_Exchange(n) != NULL)
	{
		this->Get_Exchange(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Gas Phases
	if (this->Get_GasPhase(n) != NULL)
	{
		this->Get_GasPhase(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Kinetics
	if (this->Get_Kinetics(n) != NULL)
	{
		this->Get_Kinetics(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// PPassemblage
	if (this->Get_PPassemblage(n) != NULL)
	{
		this->Get_PPassemblage(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// SSassemblage
	if (this->Get_SSassemblage(n) != NULL)
	{
		this->Get_SSassemblage(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Surface
	if (this->Get_Surface(n) != NULL)
	{
		this->Get_Surface(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Mix
	if (this->Get_Mix(n) != NULL)
	{
		this->Get_Mix(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Reaction
	if (this->Get_Reaction(n) != NULL)
	{
		this->Get_Reaction(n)->dump_raw(s_oss, indent, &n_user_local);
	}

	// Temperature
	if (this->Get_Temperature(n) != NULL)
	{
		this->Get_Temperature(n)->dump_raw(s_oss, indent, &n_user_local);
	}
}

void
cxxStorageBin::dump_raw_range(std::ostream & s_oss, int start, int end, unsigned int indent) const
{
	// Dump all data
	s_oss.precision(DBL_DIG - 1);

	// Solutions
	Utilities::Rxn_dump_raw_range(Solutions, s_oss, start, end, indent);

	// Exchange
	Utilities::Rxn_dump_raw_range(Exchangers, s_oss, start, end, indent);

	// Gas Phases
	Utilities::Rxn_dump_raw_range(GasPhases, s_oss, start, end, indent);

	// Kinetics
	Utilities::Rxn_dump_raw_range(Kinetics, s_oss, start, end, indent);

	// PPassemblage
	Utilities::Rxn_dump_raw_range(PPassemblages, s_oss, start, end, indent);

	// SSassemblage
	Utilities::Rxn_dump_raw_range(SSassemblages, s_oss, start, end, indent);

	// Surface
	Utilities::Rxn_dump_raw_range(Surfaces, s_oss, start, end, indent);

	// Mix
	Utilities::Rxn_dump_raw_range(Mixes, s_oss, start, end, indent);

	// Reactions
	Utilities::Rxn_dump_raw_range(Reactions, s_oss, start, end, indent);

	// Temperature
	Utilities::Rxn_dump_raw_range(Temperatures, s_oss, start, end, indent);
}

void
cxxStorageBin::read_raw(CParser & parser)
{
	PHRQ_io::LINE_TYPE i;
	while ((i =
			parser.check_line("StorageBin read_raw", false, true, true,
							  true)) != PHRQ_io::LT_KEYWORD)
	{
		if (i == PHRQ_io::LT_EOF)
			return;				// PHRQ_io::LT_EOF;
	}

	for (;;)
	{
		switch (parser.next_keyword())
		{
		case Keywords::KEY_END:
		case Keywords::KEY_NONE:
			goto END_OF_SIMULATION_INPUT;
			break;
		case Keywords::KEY_SOLUTION_RAW:
			{
				cxxSolution entity(this->Get_io());
				entity.read_raw(parser);
				Solutions[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_SOLUTION_MODIFY:
			{
				Utilities::SB_read_modify(this->Solutions, parser);
			}
			break;
		case Keywords::KEY_EXCHANGE_RAW:
			{
				cxxExchange entity(this->Get_io());
				entity.read_raw(parser);
				Exchangers[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_EXCHANGE_MODIFY:
			{
				Utilities::SB_read_modify(this->Exchangers, parser);
			}
			break;
		case Keywords::KEY_GAS_PHASE_RAW:
			{
				cxxGasPhase entity(this->Get_io());
				entity.read_raw(parser);
				GasPhases[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_GAS_PHASE_MODIFY:
			{
				Utilities::SB_read_modify(this->GasPhases, parser);
			}
			break;
		case Keywords::KEY_KINETICS_RAW:
			{
				cxxKinetics entity(this->Get_io());
				entity.read_raw(parser);
				Kinetics[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_KINETICS_MODIFY:
			{
				Utilities::SB_read_modify(this->Kinetics, parser);
			}
			break;
		case Keywords::KEY_EQUILIBRIUM_PHASES_RAW:
			{
				cxxPPassemblage entity(this->Get_io());
				entity.read_raw(parser);
				PPassemblages[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_EQUILIBRIUM_PHASES_MODIFY:
			{
				Utilities::SB_read_modify(this->PPassemblages, parser);
			}
			break;
		case Keywords::KEY_SOLID_SOLUTIONS_RAW:
			{
				cxxSSassemblage entity(this->Get_io());
				entity.read_raw(parser);
				SSassemblages[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_SOLID_SOLUTIONS_MODIFY:
			{
				Utilities::SB_read_modify(this->SSassemblages, parser);
			}
			break;
		case Keywords::KEY_SURFACE_RAW:
			{
				cxxSurface entity(this->Get_io());
				entity.read_raw(parser);
				Surfaces[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_SURFACE_MODIFY:
			{
				Utilities::SB_read_modify(this->Surfaces, parser);
			}
			break;
		case Keywords::KEY_REACTION_TEMPERATURE_RAW:
			{
				cxxTemperature entity(this->Get_io());
				entity.read_raw(parser);
				Temperatures[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_REACTION_RAW:
			{
				cxxReaction entity;
				entity.read_raw(parser, true);
				Reactions[entity.Get_n_user()] = entity;
			}
			break;
		case Keywords::KEY_REACTION_MODIFY:
		{
			Utilities::SB_read_modify(this->Reactions, parser);
		}
		break;
		case Keywords::KEY_MIX_RAW:
			{
				cxxMix entity;
				entity.read_raw(parser);
				Mixes[entity.Get_n_user()] = entity;
			}
			break;
		default:
			{
				for (;;)
				{
					PHRQ_io::LINE_TYPE lt;
					lt = parser.check_line("read_raw", false, true, true, false);
					if (lt == PHRQ_io::LT_KEYWORD)
						break;
					if (lt == PHRQ_io::LT_EOF)
						goto END_OF_SIMULATION_INPUT;
				}
			}
			break;
		}
	}

  END_OF_SIMULATION_INPUT:
	return;						//PHRQ_io::LT_OK;
}

int
cxxStorageBin::read_raw_keyword(CParser & parser)
{
	PHRQ_io::LINE_TYPE i;
	int entity_number = -999;

	switch (parser.next_keyword())
	{
	case Keywords::KEY_NONE:
	case Keywords::KEY_END:
		while ((i =
				parser.check_line("StorageBin read_raw_keyword", false, true,
								  true, true)) != PHRQ_io::LT_KEYWORD)
		{
			if (i == PHRQ_io::LT_EOF)
				break;			// PHRQ_io::LT_EOF;
		}
		break;

	case Keywords::KEY_SOLUTION_RAW:
		{
			cxxSolution entity(this->Get_io());
			entity.read_raw(parser);
			Solutions[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_EXCHANGE_RAW:
		{
			cxxExchange entity(this->Get_io());
			entity.read_raw(parser);
			Exchangers[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_GAS_PHASE_RAW:
		{
			cxxGasPhase entity(this->Get_io());
			entity.read_raw(parser);
			GasPhases[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_KINETICS_RAW:
		{
			cxxKinetics entity(this->Get_io());
			entity.read_raw(parser);
			Kinetics[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_EQUILIBRIUM_PHASES_RAW:
		{
			cxxPPassemblage entity(this->Get_io());
			entity.read_raw(parser);
			PPassemblages[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_SOLID_SOLUTIONS_RAW:
		{
			cxxSSassemblage entity(this->Get_io());
			entity.read_raw(parser);
			SSassemblages[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_SURFACE_RAW:
		{
			cxxSurface entity(this->Get_io());
			entity.read_raw(parser);
			Surfaces[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_REACTION_TEMPERATURE_RAW:
		{
			cxxTemperature entity(this->Get_io());
			entity.read_raw(parser);
			Temperatures[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_REACTION_RAW:
		{
			cxxReaction entity;
			entity.read_raw(parser, true);
			Reactions[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	case Keywords::KEY_MIX_RAW:
		{
			cxxMix entity;
			entity.read_raw(parser);
			Mixes[entity.Get_n_user()] = entity;
			entity_number = entity.Get_n_user();
		}
		break;

	default:
		break;
	}
	return (entity_number);		//PHRQ_io::LT_OK;
}

void
cxxStorageBin::Remove(int n)
{
	// Solution
	this->Solutions.erase(n);

	// Exchanger
	this->Exchangers.erase(n);

	// GasPhase
	this->GasPhases.erase(n);

	// Kinetics
	this->Kinetics.erase(n);

	// PPassemblage
	this->PPassemblages.erase(n);

	// SSassemblage
	this->SSassemblages.erase(n);

	// Surface
	this->Surfaces.erase(n);

	// Mixes
	this->Mixes.erase(n);

	// Reactions
	this->Reactions.erase(n);

	// Temperature
	this->Temperatures.erase(n);

	// Pressure
	this->Pressures.erase(n);
}
void
cxxStorageBin::Clear(void) 
{
	// Delete all data

	// Solutions
	this->Solutions.clear();

	// Exchange
	this->Exchangers.clear();

	// Gas Phases
	this->GasPhases.clear();

	// Kinetics
	this->Kinetics.clear();

	// PPassemblage
	this->PPassemblages.clear();

	// SSassemblage
	this->SSassemblages.clear();

	// Surface
	this->Surfaces.clear();

	// Mix
	this->Mixes.clear();

	// Reactions
	this->Reactions.clear();

	// Temperature
	this->Temperatures.clear();

	// Pressure
	this->Pressures.clear();
}

cxxSystem &
cxxStorageBin::Get_System(void)
{
	return this->system;
}

void
cxxStorageBin::Set_System(cxxUse *use_ptr)
{
	// Initialize
	this->system.Initialize();
	// Solution
	if (use_ptr->Get_solution_ptr() != NULL)
	{
		std::map < int, cxxSolution >::iterator it =
			this->Solutions.find(use_ptr->Get_n_solution_user());
		if (it != this->Solutions.end())
		{
			this->system.Set_Solution(&(it->second));
		}
	}
	// Exchange
	if (use_ptr->Get_exchange_ptr() != NULL)
	{
		std::map < int, cxxExchange >::iterator it =
			this->Exchangers.find(use_ptr->Get_n_exchange_user());
		if (it != this->Exchangers.end())
		{
			this->system.Set_Exchange(&(it->second));
		}
	}
	// gas_phase
	if (use_ptr->Get_gas_phase_ptr() != NULL)
	{
		std::map < int, cxxGasPhase >::iterator it =
			this->GasPhases.find(use_ptr->Get_n_gas_phase_user());
		if (it != this->GasPhases.end())
		{
			this->system.Set_GasPhase(&(it->second));
		}
	}
	// kinetics
	if (use_ptr->Get_kinetics_ptr() != NULL)
	{
		std::map < int, cxxKinetics >::iterator it =
			this->Kinetics.find(use_ptr->Get_n_kinetics_user());
		if (it != this->Kinetics.end())
		{
			this->system.Set_Kinetics(&(it->second));
		}
	}
	// pp_assemblage
	if (use_ptr->Get_pp_assemblage_ptr() != NULL)
	{
		std::map < int, cxxPPassemblage >::iterator it =
			this->PPassemblages.find(use_ptr->Get_n_pp_assemblage_user());
		if (it != this->PPassemblages.end())
		{
			this->system.Set_PPassemblage(&(it->second));
		}
	}
	// ss_assemblage
	if (use_ptr->Get_ss_assemblage_ptr() != NULL)
	{
		std::map < int, cxxSSassemblage >::iterator it =
			this->SSassemblages.find(use_ptr->Get_n_ss_assemblage_user());
		if (it != this->SSassemblages.end())
		{
			this->system.Set_SSassemblage(&(it->second));
		}
	}
	// surface
	if (use_ptr->Get_surface_ptr() != NULL)
	{
		std::map < int, cxxSurface >::iterator it =
			this->Surfaces.find(use_ptr->Get_n_surface_user());
		if (it != this->Surfaces.end())
		{
			this->system.Set_Surface(&(it->second));
		}
	}
	// mix
	if (use_ptr->Get_mix_ptr() != NULL)
	{
		std::map < int, cxxMix >::iterator it =
			this->Mixes.find(use_ptr->Get_n_mix_user());
		if (it != this->Mixes.end())
		{
			this->system.Set_Mix(&(it->second));
		}
	}
	// reaction
	if (use_ptr->Get_reaction_ptr() != NULL)
	{
		std::map < int, cxxReaction >::iterator it =
			this->Reactions.find(use_ptr->Get_n_reaction_user());
		if (it != this->Reactions.end())
		{
			this->system.Set_Reaction(&(it->second));
		}
	}
	// reaction temperature
	if (use_ptr->Get_temperature_ptr() != NULL)
	{
		std::map < int, cxxTemperature >::iterator it =
			this->Temperatures.find(use_ptr->Get_n_temperature_user());
		if (it != this->Temperatures.end())
		{
			this->system.Set_Temperature(&(it->second));
		}
	}
	// reaction pressure
	if (use_ptr->Get_pressure_ptr() != NULL)
	{
		cxxPressure * p = Utilities::Rxn_find(this->Pressures, use_ptr->Get_n_pressure_user());
		if (p != NULL)
		{
			this->system.Set_Pressure(p);
		}
	}
}
void
cxxStorageBin::Set_System(int i)
{
	// Initialize
	this->system.Initialize();
	// Solution
	{
		std::map < int, cxxSolution >::iterator it = this->Solutions.find(i);
		if (it != this->Solutions.end())
		{
			this->system.Set_Solution(&(it->second));
		}
	}

	// Exchange
	{
		std::map < int, cxxExchange >::iterator it = this->Exchangers.find(i);
		if (it != this->Exchangers.end())
		{
			this->system.Set_Exchange(&(it->second));
		}
	}

	// gas_phase
	{
		std::map < int, cxxGasPhase >::iterator it = this->GasPhases.find(i);
		if (it != this->GasPhases.end())
		{
			this->system.Set_GasPhase(&(it->second));
		}
	}
	// kinetics
	{
		std::map < int, cxxKinetics >::iterator it = this->Kinetics.find(i);
		if (it != this->Kinetics.end())
		{
			this->system.Set_Kinetics(&(it->second));
		}
	}
	// pp_assemblage
	{
		std::map < int, cxxPPassemblage >::iterator it = this->PPassemblages.find(i);
		if (it != this->PPassemblages.end())
		{
			this->system.Set_PPassemblage(&(it->second));
		}
	}
	// ss_assemblage
	{
		std::map < int, cxxSSassemblage >::iterator it = this->SSassemblages.find(i);
		if (it != this->SSassemblages.end())
		{
			this->system.Set_SSassemblage(&(it->second));
		}
	}
	// surface
	{
		std::map < int, cxxSurface >::iterator it = this->Surfaces.find(i);
		if (it != this->Surfaces.end())
		{
			this->system.Set_Surface(&(it->second));
		}
	}
	// mix
	{
		std::map < int, cxxMix >::iterator it =	this->Mixes.find(i);
		if (it != this->Mixes.end())
		{
			this->system.Set_Mix(&(it->second));
		}
	}
	// reaction
	{
		std::map < int, cxxReaction >::iterator it = this->Reactions.find(i);
		if (it != this->Reactions.end())
		{
			this->system.Set_Reaction(&(it->second));
		}
	}
	// reaction temperature
	{
		std::map < int, cxxTemperature >::iterator it = this->Temperatures.find(i);
		if (it != this->Temperatures.end())
		{
			this->system.Set_Temperature(&(it->second));
		}
	}

	// reaction pressure
	{
		this->system.Set_Pressure(Utilities::Rxn_find(this->Pressures, i));
	}
}
