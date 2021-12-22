#include "Utils.h"
#include "Phreeqc.h"
#include "phqalloc.h"
#include <vector>
#include <assert.h>
#include "Exchange.h"
#include "GasPhase.h"
#include "PPassemblage.h"
#include "SSassemblage.h"
#include "SS.h"
#include "Solution.h"
#include "cxxKinetics.h"

#if defined(PHREEQCI_GUI)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/* ---------------------------------------------------------------------- */
int Phreeqc::
prep(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Input is model defined by the structure use.
 *   Most of routine is skipped if model, as defined by master.total
 *      plus use.pure_phases, is same as previous calculation.
 *   Routine sets up class unknown for each unknown.
 *   Determines elements, species, and phases that are in the model.
 *   Calculates mass-action equations for each species and phase.
 *   Routine builds a set of lists for calculating mass balance and
 *      for building jacobian.
 */
	cxxSolution *solution_ptr;

	if (state >= REACTION)
	{
		same_model = check_same_model();
	}
	else
	{
		same_model = FALSE;
		last_model.force_prep = true;
	}
	/*same_model = FALSE; */
/*
 *   Initialize s, master, and unknown pointers
 */
	solution_ptr = use.Get_solution_ptr();
	if (solution_ptr == NULL)
	{
		error_msg("Solution needed for calculation not found, stopping.",
				  STOP);
		return ERROR;
	}
	description_x = solution_ptr->Get_description();
/*
 *   Allocate space for unknowns
 *   Must allocate all necessary space before pointers to
 *   X are set.
 */
	if (same_model == FALSE || my_array.size() == 0)
	{
		clear();
		setup_unknowns();
/*
 *   Set unknown pointers, unknown types, validity checks
 */
		if (state == INITIAL_SOLUTION)
			convert_units(solution_ptr);
		setup_solution();
		setup_exchange();
		setup_surface();
		setup_pure_phases();
		setup_gas_phase();
		setup_ss_assemblage();
		setup_related_surface();
		tidy_redox();
		if (get_input_errors() > 0)
		{
			error_msg("Program terminating due to input errors.", STOP);
		}
/*
 *   Allocate space for array
 */
		my_array.resize((max_unknowns + 1) * max_unknowns);
		delta.resize(max_unknowns);
		residual.resize(max_unknowns);
		for (int j = 0; j < max_unknowns; j++)
		{
		  residual[j] = 0;
		}

/*
 *   Build lists to fill Jacobian array and species list
 */
		build_model();
		adjust_setup_pure_phases();
		adjust_setup_solution();
	}
	else
	{
/*
 *   If model is same, just update masses, don`t rebuild unknowns and lists
 */
		quick_setup();
	}
	if (debug_mass_balance)
	{
		output_msg(sformatf("\nTotals for the equation solver.\n"));
		output_msg(sformatf("\n\tRow\tName           Type       Total moles\n"));
		for (int i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type == PITZER_GAMMA)
				continue;
			output_msg(sformatf("\t%3d\t%-17s%2d    %15.6e\n",
				x[i]->number, x[i]->description, (int)x[i]->type, (double)x[i]->moles));
		}
		output_msg(sformatf("\n\n"));
	}
	if (get_input_errors() > 0)
	{
		error_msg("Program stopping due to input errors.", STOP);
	}
	if (sit_model) sit_make_lists();
	if (pitzer_model) 
		pitzer_make_lists();
	return (OK);
}

/* ---------------------------------------------------------------------- */
 int Phreeqc::
quick_setup(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Routine is used if model is the same as previous model
 *   Assumes moles of elements, exchangers, surfaces, gases, and solid solutions have
 *       been accumulated in array master, usually by subroutine step.
 *   Updates essential information for the model.
 */
	int i;
	for (i = 0; i < (int)master.size(); i++)
	{
		if (master[i]->s->type == SURF_PSI)
			continue;
		if (master[i]->s == s_eminus ||
			master[i]->s == s_hplus ||
			master[i]->s == s_h2o || master[i]->s == s_h2
			|| master[i]->s == s_o2)
			continue;
		if (master[i]->total > 0)
		{
			if (master[i]->s->secondary != NULL)
			{
				master[i]->s->secondary->unknown->moles = master[i]->total;
			}
			else
			{
				master[i]->unknown->moles = master[i]->total;
			}
		}
	}
/*
 *   Reaction: pH for charge balance
 */
	ph_unknown->moles = use.Get_solution_ptr()->Get_cb();
/*
 *   Reaction: pe for total hydrogen
 */
	if (mass_hydrogen_unknown != NULL)
	{
/* Use H - 2O linear combination in place of H */
#define COMBINE
		/*#define COMBINE_CHARGE */
#ifdef COMBINE
		mass_hydrogen_unknown->moles =
			use.Get_solution_ptr()->Get_total_h() - 2 * use.Get_solution_ptr()->Get_total_o();
#else
		mass_hydrogen_unknown->moles = use.Get_solution_ptr()->total_h;
#endif
	}
/*
 *   Reaction H2O for total oxygen
 */
	if (mass_oxygen_unknown != NULL)
	{
		mass_oxygen_unknown->moles = use.Get_solution_ptr()->Get_total_o();
	}

/*
 *   pp_assemblage
 */
	for (i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type == PP)
		{
			cxxPPassemblage * pp_assemblage_ptr = use.Get_pp_assemblage_ptr();
			std::map<std::string, cxxPPassemblageComp>::iterator it;
			//it =  pp_assemblage_ptr->Get_pp_assemblage_comps().find(x[i]->pp_assemblage_comp_name);
			cxxPPassemblageComp * comp_ptr = pp_assemblage_ptr->Find(x[i]->pp_assemblage_comp_name);
			assert(comp_ptr != NULL);
			//assert(it != pp_assemblage_ptr->Get_pp_assemblage_comps().end());
			//cxxPPassemblageComp * comp_ptr = &(it->second);
			x[i]->pp_assemblage_comp_ptr = comp_ptr;
			x[i]->moles = comp_ptr->Get_moles();
			/* A. Crapsi */
			x[i]->si    = comp_ptr->Get_si();
			x[i]->delta = comp_ptr->Get_delta();
			/* End A. Crapsi */
			x[i]->dissolve_only = comp_ptr->Get_dissolve_only() ? TRUE : FALSE;
			comp_ptr->Set_delta(0.0);
		}
	}
	// Need to update SIs for gases
	adjust_setup_pure_phases();

/*
 *   gas phase
 */
	if (gas_unknown != NULL)
	{
		cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
		if ((gas_phase_ptr->Get_type() == cxxGasPhase::GP_VOLUME) && 
			numerical_fixed_volume && 
			(gas_phase_ptr->Get_pr_in() || force_numerical_fixed_volume))
		{
			for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
			{	
				cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
				gas_unknowns[i]->moles = gc_ptr->Get_moles();
				if (gas_unknowns[i]->moles <= 0)
					gas_unknowns[i]->moles = MIN_TOTAL;
				gas_unknowns[i]->phase->pr_in = false;
				gas_unknowns[i]->phase->pr_phi = 1.0;
				gas_unknowns[i]->phase->pr_p = 0;
			}
		}
		else
		{
			gas_unknown->moles = 0.0;
			for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
			{	
				cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
				gas_unknown->moles += gc_ptr->Get_moles();
			}
			if (gas_unknown->moles <= 0)
				gas_unknown->moles = MIN_TOTAL;
			gas_unknown->ln_moles = log(gas_unknown->moles);
		}
	}

/*
 *   ss_assemblage
 */
	if (ss_unknown != NULL)
	{
		for (i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type == SS_MOLES)
				break;
		}

		std::vector<cxxSS *> ss_ptrs = use.Get_ss_assemblage_ptr()->Vectorize();
		for (size_t j = 0; j < ss_ptrs.size(); j++)
		{
			for (size_t k = 0; k < ss_ptrs[j]->Get_ss_comps().size(); k++)
			{
				x[i]->ss_ptr = ss_ptrs[j];
				cxxSScomp *comp_ptr = &(ss_ptrs[j]->Get_ss_comps()[k]);
				x[i]->ss_comp_ptr = comp_ptr;
				x[i]->moles = comp_ptr->Get_moles();
				if (x[i]->moles <= 0)
				{
					x[i]->moles = MIN_TOTAL_SS;
					comp_ptr->Set_moles(MIN_TOTAL_SS);
				}
				comp_ptr->Set_initial_moles(x[i]->moles);
				x[i]->ln_moles = log(x[i]->moles);

				x[i]->phase->dn = comp_ptr->Get_dn();
				x[i]->phase->dnb = comp_ptr->Get_dnb();
				x[i]->phase->dnc = comp_ptr->Get_dnc();
				x[i]->phase->log10_fraction_x =	comp_ptr->Get_log10_fraction_x();
				x[i]->phase->log10_lambda = comp_ptr->Get_log10_lambda();
				i++;
			}
		}
	}
/*
 *   exchange
 */
	// number of moles is set from master->moles above
/*
 *   surface
 */
	if (use.Get_surface_ptr() != NULL)
	{
		for (i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type == SURFACE)
			{
				break;
			}
		}
		for (; i < count_unknowns; i++)
		{
			if (x[i]->type == SURFACE_CB)
			{
				cxxSurfaceCharge *charge_ptr = use.Get_surface_ptr()->Find_charge(x[i]->surface_charge);
				x[i]->related_moles = charge_ptr->Get_grams();
				x[i]->mass_water = charge_ptr->Get_mass_water();
				/* moles picked up from master->total */
			}
			else if (x[i]->type == SURFACE_CB1 || x[i]->type == SURFACE_CB2)
			{				
				cxxSurfaceCharge *charge_ptr = use.Get_surface_ptr()->Find_charge(x[i]->surface_charge);
				x[i]->related_moles = charge_ptr->Get_grams();
				x[i]->mass_water = charge_ptr->Get_mass_water();
			}
			else if (x[i]->type == SURFACE)
			{
				/* moles picked up from master->total
				   except for surfaces related to kinetic minerals ... */
				cxxSurfaceComp *comp_ptr = use.Get_surface_ptr()->Find_comp(x[i]->surface_comp);
				if (comp_ptr->Get_rate_name().size() > 0)
				{
					cxxNameDouble::iterator lit;
					for (lit = comp_ptr->Get_totals().begin(); lit != comp_ptr->Get_totals().end(); lit++)
					{
						class element *elt_ptr = element_store(lit->first.c_str());
						class master *master_ptr = elt_ptr->master;
						if (master_ptr->type != SURF)
							continue;
						if (strcmp_nocase(x[i]->description, lit->first.c_str()) == 0)
						{
							x[i]->moles = lit->second;
						}
					}
				}
			}
			else
			{
				break;
			}

		}
	}
	save_model();
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_gas_phase(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Put coefficients into lists to sum iaps to test for equilibrium
 *   Put coefficients into lists to build jacobian for 
 *      sum of partial pressures equation and
 *      mass balance equations for elements contained in gases
 */
	size_t row, col;
	class master *master_ptr;
	class rxn_token *rxn_ptr;
	class unknown *unknown_ptr;
	LDBLE coef, coef_elt;

	if (gas_unknown == NULL)
		return (OK);
	cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
	if (gas_phase_ptr->Get_type() == cxxGasPhase::GP_VOLUME && 
		(gas_phase_ptr->Get_pr_in() || force_numerical_fixed_volume) && 
		numerical_fixed_volume)
	{
		return build_fixed_volume_gas();
	}
	for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
	{	
		cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
		int k;
		class phase *phase_ptr = phase_bsearch(gc_ptr->Get_phase_name().c_str() , &k, FALSE);
		assert(phase_ptr);
/*
 *   Determine elements in gas component
 */
		count_elts = 0;
		paren_count = 0;
		if (phase_ptr->rxn_x.token.size() == 0)
			continue;
		add_elt_list(phase_ptr->next_elt, 1.0);
#ifdef COMBINE
		change_hydrogen_in_elt_list(0);
#endif
/*
 *   Build mass balance sums for each element in gas
 */
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\n\tMass balance summations. %s.\n",
					   phase_ptr->name));
		}

		/* All elements in gas */
		for (int j = 0; j < count_elts; j++)
		{
			unknown_ptr = NULL;
			if (strcmp(elt_list[j].elt->name, "H") == 0)
			{
				unknown_ptr = mass_hydrogen_unknown;
			}
			else if (strcmp(elt_list[j].elt->name, "O") == 0)
			{
				unknown_ptr = mass_oxygen_unknown;
			}
			else
			{
				if (elt_list[j].elt->primary->in == TRUE)
				{
					unknown_ptr = elt_list[j].elt->primary->unknown;
				}
				else if (elt_list[j].elt->primary->s->secondary != NULL)
				{
					unknown_ptr =
						elt_list[j].elt->primary->s->secondary->unknown;
				}
			}
			if (unknown_ptr != NULL)
			{
				coef = elt_list[j].coef;
				store_mb(&(phase_ptr->moles_x), &(unknown_ptr->f), coef);
				if (debug_prep == TRUE)
				{
					output_msg(sformatf( "\t\t%-24s%10.3f\n",
							   unknown_ptr->description, (double) coef));
				}
			}
		}
		if (gas_phase_ptr->Get_type() == cxxGasPhase::GP_PRESSURE)
		{
			/* Total pressure of gases */
			store_mb(&(phase_ptr->p_soln_x), &(gas_unknown->f), 1.0);
		}
/*
 *   Build jacobian sums for mass balance equations
 */
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\n\tJacobian summations %s.\n\n",
					   phase_ptr->name));
		}
		for (int j = 0; j < count_elts; j++)
		{
			unknown_ptr = NULL;
			if (strcmp(elt_list[j].elt->name, "H") == 0)
			{
				unknown_ptr = mass_hydrogen_unknown;
			}
			else if (strcmp(elt_list[j].elt->name, "O") == 0)
			{
				unknown_ptr = mass_oxygen_unknown;
			}
			else
			{
				if (elt_list[j].elt->primary->in == TRUE)
				{
					unknown_ptr = elt_list[j].elt->primary->unknown;
				}
				else if (elt_list[j].elt->primary->s->secondary != NULL)
				{
					unknown_ptr =
						elt_list[j].elt->primary->s->secondary->unknown;
				}
			}
			if (unknown_ptr == NULL)
			{
				continue;
			}
			if (debug_prep == TRUE)
			{
				output_msg(sformatf( "\n\t%s.\n",
						   unknown_ptr->description));
			}
			row = unknown_ptr->number * (count_unknowns + 1);
			coef_elt = elt_list[j].coef;
			for (rxn_ptr = &phase_ptr->rxn_x.token[0] + 1;
				 rxn_ptr->s != NULL; rxn_ptr++)
			{

				if (rxn_ptr->s->secondary != NULL
					&& rxn_ptr->s->secondary->in == TRUE)
				{
					master_ptr = rxn_ptr->s->secondary;
				}
				else if (rxn_ptr->s->primary != NULL && rxn_ptr->s->primary->in == TRUE)
				{
					master_ptr = rxn_ptr->s->primary;
				}
				else
				{
					master_ptr = master_bsearch_primary(rxn_ptr->s->name);
					master_ptr->s->la = -999.0;
				}
				if (debug_prep == TRUE)
				{
					output_msg(sformatf( "\t\t%s\n",
							   master_ptr->s->name));
				}
				if (master_ptr->unknown == NULL)
				{
					continue;
				}
				if (master_ptr->in == FALSE)
				{
					error_string = sformatf(
							"Element, %s, in phase, %s, is not in model.",
							master_ptr->elt->name, phase_ptr->name);
					error_msg(error_string, CONTINUE);
					input_error++;
				}
				col = master_ptr->unknown->number;
				coef = coef_elt * rxn_ptr->coef;
				if (debug_prep == TRUE)
				{
					output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
							   master_ptr->s->name, (double) coef,
							   row / (count_unknowns + 1), col));
				}
				store_jacob(&(phase_ptr->moles_x),
					&(my_array[(size_t)row + (size_t)col]), coef);
			}
			if (gas_phase_ptr->Get_type() == cxxGasPhase::GP_PRESSURE)
			{
				/* derivative wrt total moles of gas */
				if (debug_prep == TRUE)
				{
					output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
							   "gas moles", (double) elt_list[j].coef,
							   row / (count_unknowns + 1),
							   gas_unknown->number));
				}
				store_jacob(&(phase_ptr->fraction_x),
					&(my_array[(size_t)row + (size_t)gas_unknown->number]), coef_elt);
			}
		}
/*
 *   Build jacobian sums for sum of partial pressures equation
 */
		if (gas_phase_ptr->Get_type() != cxxGasPhase::GP_PRESSURE)
			continue;
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\n\tPartial pressure eqn %s.\n\n",
					   phase_ptr->name));
		}
		unknown_ptr = gas_unknown;
		row = unknown_ptr->number * (count_unknowns + 1);
		for (rxn_ptr = &phase_ptr->rxn_x.token[0] + 1; rxn_ptr->s != NULL; rxn_ptr++)
		{
			if (rxn_ptr->s != s_eminus && rxn_ptr->s->in == FALSE)
			{
				error_string = sformatf(
					"Element in species, %s, in phase, %s, is not in model.",
					rxn_ptr->s->name, phase_ptr->name);
				warning_msg(error_string);
			}
			else
			{
				if (rxn_ptr->s->secondary != NULL
					&& rxn_ptr->s->secondary->in == TRUE)
				{
					master_ptr = rxn_ptr->s->secondary;
				}
				else if (rxn_ptr->s->primary != NULL && rxn_ptr->s->primary->in == TRUE)
				{
					master_ptr = rxn_ptr->s->primary;
				}
				else
				{
					master_ptr = master_bsearch_primary(rxn_ptr->s->name);
					if (master_ptr && master_ptr->s)
					{
						master_ptr->s->la = -999.0;
					}
				}
				if (master_ptr == NULL)
				{
					error_string = sformatf(
						"Master species for %s, in phase, %s, is not in model.",
						rxn_ptr->s->name, phase_ptr->name);
					error_msg(error_string, CONTINUE);
					input_error++;
				}
				else
				{
					if (debug_prep == TRUE)
					{
						output_msg(sformatf( "\t\t%s\n", master_ptr->s->name));
					}
					if (master_ptr->unknown == NULL)
					{
						assert(false);
						continue;
					}
					if (master_ptr->in == FALSE)
					{
						error_string = sformatf(
							"Element, %s, in phase, %s, is not in model.",
							master_ptr->elt->name, phase_ptr->name);
						warning_msg(error_string);
					}
					col = master_ptr->unknown->number;
					coef = rxn_ptr->coef;
					if (debug_prep == TRUE)
					{
						output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
							master_ptr->s->name, (double) coef,
							row / (count_unknowns + 1), col));
					}
					store_jacob(&(phase_ptr->p_soln_x), &(my_array[(size_t)row + (size_t)col]), coef);
				}
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_ss_assemblage(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Put coefficients into lists to sum iaps to test for equilibrium
 *   Put coefficients into lists to build jacobian for 
 *      mass action equation for component
 *      mass balance equations for elements contained in solid solutions
 */
	bool stop;
	size_t row, col;
	class master *master_ptr;
	class rxn_token *rxn_ptr;
	const char* cptr;

	if (ss_unknown == NULL)
		return (OK);
	cxxSS * ss_ptr_old = NULL;
	col = 0;
	for (int i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type != SS_MOLES)
			continue;
		//cxxSS *ss_ptr = use.Get_ss_assemblage_ptr()->Find(x[i]->ss_name);
		cxxSS *ss_ptr = (cxxSS *) x[i]->ss_ptr;
		assert(ss_ptr);
		if (ss_ptr != ss_ptr_old)
		{
			col = x[i]->number;
			ss_ptr_old = ss_ptr;
		}
/*
 *   Calculate function value (inverse saturation index)
 */
		if (x[i]->phase->rxn_x.token.size() == 0)
			continue;
		store_mb(&(x[i]->phase->lk), &(x[i]->f), 1.0);
		for (rxn_ptr = &x[i]->phase->rxn_x.token[0] + 1; rxn_ptr->s != NULL;
			 rxn_ptr++)
		{
			store_mb(&(rxn_ptr->s->la), &(x[i]->f), -rxn_ptr->coef);
		}
		/* include mole fraction */
		store_mb(&(x[i]->phase->log10_fraction_x), &(x[i]->f), 1.0);

		/* include activity coeficient */
		store_mb(&(x[i]->phase->log10_lambda), &(x[i]->f), 1.0);
/*
 *   Put coefficients into mass action equations
 */
		/* first IAP terms */
		for (rxn_ptr = &x[i]->phase->rxn_x.token[0] + 1; rxn_ptr->s != NULL;
			 rxn_ptr++)
		{
			if (rxn_ptr->s->secondary != NULL
				&& rxn_ptr->s->secondary->in == TRUE)
			{
				master_ptr = rxn_ptr->s->secondary;
			}
			else
			{
				master_ptr = rxn_ptr->s->primary;
			}
			if (master_ptr == NULL || master_ptr->unknown == NULL)
				continue;
			store_jacob0((int)x[i]->number, (int)master_ptr->unknown->number,
				rxn_ptr->coef);
		}

		if (ss_ptr->Get_a0() != 0.0 || ss_ptr->Get_a1() != 0.0)
		{
/*
 *   For binary solid solution
 */
			/* next dnc terms */
			row = x[i]->number * (count_unknowns + 1);
			if (x[i]->ss_comp_number == 0)
			{
				col = x[i]->number;
			}
			else
			{
				col = x[i]->number - 1;
			}
			store_jacob(&(x[i]->phase->dnc), &(my_array[(size_t)row + (size_t)col]), -1);

			/* next dnb terms */
			col++;
			store_jacob(&(x[i]->phase->dnb), &(my_array[(size_t)row + (size_t)col]), -1);
		}
		else
		{
/*
 *   For ideal solid solution
 */
			row = x[i]->number * (count_unknowns + 1);
			for (size_t j = 0; j < ss_ptr->Get_ss_comps().size(); j++)
			{
				if ((int) j != x[i]->ss_comp_number)
				{
/*					store_jacob (&(s_s_ptr->dn), &(array[row + col + j]), -1.0); */
					store_jacob(&(x[i]->phase->dn), &(my_array[(size_t)row + (size_t)col + (size_t)j]),
								-1.0);
				}
				else
				{
					store_jacob(&(x[i]->phase->dnb), &(my_array[(size_t)row + (size_t)col + (size_t)j]),
								-1.0);
				}
			}
		}
/*
 *   Put coefficients into mass balance equations
 */
		count_elts = 0;
		paren_count = 0;
		cptr = x[i]->phase->formula;
		get_elts_in_species(&cptr, 1.0);
/*
 *   Go through elements in phase
 */
#ifdef COMBINE
		change_hydrogen_in_elt_list(0);
#endif
		for (int j = 0; j < count_elts; j++)
		{

			if (strcmp(elt_list[j].elt->name, "H") == 0
				&& mass_hydrogen_unknown != NULL)
			{
				store_jacob0((int)mass_hydrogen_unknown->number, (int)x[i]->number,
					-elt_list[j].coef);
				store_sum_deltas(&(delta[i]), &mass_hydrogen_unknown->delta,
					elt_list[j].coef);

			}
			else if (strcmp(elt_list[j].elt->name, "O") == 0
					 && mass_oxygen_unknown != NULL)
			{
				store_jacob0((int)mass_oxygen_unknown->number, (int)x[i]->number,
					-elt_list[j].coef);
				store_sum_deltas(&(delta[i]), &mass_oxygen_unknown->delta,
					elt_list[j].coef);

			}
			else
			{
				master_ptr = elt_list[j].elt->primary;
				if (master_ptr->in == FALSE)
				{
					master_ptr = master_ptr->s->secondary;
				}
				if (master_ptr == NULL || master_ptr->in == FALSE)
				{
					if (state != ADVECTION && state != TRANSPORT
						&& state != PHAST)
					{
						error_string = sformatf(
								"Element in phase, %s, is not in model.",
								x[i]->phase->name);
						warning_msg(error_string);
					}
					if (master_ptr != NULL)
					{
						master_ptr->s->la = -999.9;
					}
/*
 *   Master species is in model
 */
				}
				else if (master_ptr->in == TRUE)
				{
					store_jacob0((int)master_ptr->unknown->number, (int)x[i]->number,
						-elt_list[j].coef);
					store_sum_deltas(&delta[i], &master_ptr->unknown->delta,
						elt_list[j].coef);
/*
 *   Master species in equation needs to be rewritten
 */
				}
				else if (master_ptr->in == REWRITE)
				{
					stop = FALSE;
					for (int k = 0; k < count_unknowns; k++)
					{
						if (x[k]->type != MB)
							continue;
						for (size_t l = 0; l < x[k]->master.size(); l++)
						{
							if (x[k]->master[l] == master_ptr)
							{
								store_jacob0((int)x[k]->master[0]->unknown->number,
									(int)x[i]->number, -elt_list[j].coef);
								store_sum_deltas(&delta[i],
									&x[k]->master[0]->unknown->
									delta, elt_list[j].coef);
								stop = TRUE;
								break;
							}
						}
						if (stop == TRUE)
							break;
					}
				}
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_jacobian_sums(int k)
/* ---------------------------------------------------------------------- */
{
/*
 *   Function builds lists sum_jacob1 and sum_jacob2 that describe how to sum molalities
 *   to form jacobian.
 */
	int i, j, kk;
	int count_g;
	LDBLE coef;
	LDBLE *source, *target;

	if (debug_prep == TRUE)
		output_msg(sformatf( "\n\tJacobian summations.\n"));
/*
 *   Calculate jacobian coefficients for each mass balance equation
 */
	for (i = 0; i < (int)mb_unknowns.size(); i++)
	{
/*
 *   Store d(moles) for a mass balance equation
 */
		/* initial solution only */
		if (mb_unknowns[i].unknown->type == SOLUTION_PHASE_BOUNDARY)
		{
			continue;
		}
		coef = mb_unknowns[i].coef;
		if (debug_prep == TRUE)
			output_msg(sformatf("\n\tMass balance eq:  %-13s\t%f\trow\tcol\n",
				mb_unknowns[i].unknown->description, (double)coef));
		store_dn(k, mb_unknowns[i].source, (int)mb_unknowns[i].unknown->number,
			coef, mb_unknowns[i].gamma_source);
/*
 *   Add extra terms for change in dg/dx in diffuse layer model
 */
		if (s[k]->type >= H2O || dl_type_x == cxxSurface::NO_DL)
		{
			continue;
		}
		else if ((mb_unknowns[i].unknown->type == MB ||
				  mb_unknowns[i].unknown->type == MH ||
				  mb_unknowns[i].unknown->type == MH2O) && state >= REACTION)
		{
			if (mass_oxygen_unknown != NULL)
			{
				/* term for water, sum of all surfaces */
				source = &s[k]->tot_dh2o_moles;
				target = &(my_array[(size_t)mb_unknowns[i].unknown->number * 
					(count_unknowns + 1) + (size_t)mass_oxygen_unknown->number]);
				if (debug_prep == TRUE)
				{
					output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
							   "sum[dn(i,s)/dlnwater]", (double) coef,
							   mb_unknowns[i].unknown->number,
							   mass_oxygen_unknown->number));
				}
				store_jacob(source, target, coef);
			}

			/* terms for psi, one for each surface */
			count_g = 0;
			for (j = 0; j < count_unknowns; j++)
			{
				if (x[j]->type != SURFACE_CB)
					continue;
				cxxSurfaceCharge *charge_ptr = use.Get_surface_ptr()->Find_charge(x[j]->surface_charge);
				source = s_diff_layer[k][charge_ptr->Get_name()].Get_dx_moles_address();
				target = &(my_array[(size_t)mb_unknowns[i].unknown->number *
								 (count_unknowns + 1) + (size_t)x[j]->number]);
				if (debug_prep == TRUE)
				{
					output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
							   "dg/dlny", (double) coef,
							   mb_unknowns[i].unknown->number, x[j]->number));
				}
				store_jacob(source, target, coef);
				count_g++;
				if (count_g >= (int) use.Get_surface_ptr()->Get_surface_charges().size())
					break;
			}

			/* terms for related phases */
			count_g = 0;
			for (j = 0; j < count_unknowns; j++)
			{
				if (x[j]->type != SURFACE_CB)
					continue;
				cxxSurfaceCharge *charge_ptr = use.Get_surface_ptr()->Find_charge(x[j]->surface_charge);
				/* has related phase */
				cxxSurfaceComp *comp_ptr = use.Get_surface_ptr()->Find_comp(x[(size_t)j - 1]->surface_comp);
				if (comp_ptr->Get_phase_name().size() == 0)
					continue;

				/* now find the related phase */
				for (kk = (int)count_unknowns - 1; kk >= 0; kk--)
				{
					if (x[kk]->type != PP)
						continue;
					//if (x[kk]->phase->name == string_hsave(comp_ptr->Get_phase_name().c_str()))
					if (strcmp_nocase(x[kk]->phase->name, comp_ptr->Get_phase_name().c_str()) == 0)
						break;
				}

				if (kk >= 0)
				{
					source = s_diff_layer[k][charge_ptr->Get_name()].Get_drelated_moles_address();
					target = &(my_array[(size_t)mb_unknowns[i].unknown->number *
									 (count_unknowns + 1) + (size_t)x[kk]->number]);
					if (debug_prep == TRUE)
					{
						output_msg(sformatf(
								   "\t\t%-24s%10.3f\t%d\t%d", "dphase",
								   (double) coef,
								   mb_unknowns[i].unknown->number,
								   x[kk]->number));
					}
					store_jacob(source, target, coef);
				}
				count_g++;
				if (count_g >= (int) use.Get_surface_ptr()->Get_surface_charges().size())
					break;
			}

		}
		else if (mb_unknowns[i].unknown->type == SURFACE_CB)
		{
			count_g = 0;
			for (j = 0; j < count_unknowns; j++)
			{
				if (x[j]->type != SURFACE_CB)
					continue;
				cxxSurfaceCharge *charge_ptr = use.Get_surface_ptr()->Find_charge(x[j]->surface_charge);
				if (mb_unknowns[i].unknown->number == x[j]->number)
				{
					source = s_diff_layer[k][charge_ptr->Get_name()].Get_dx_moles_address();
					target = &(my_array[(size_t)mb_unknowns[i].unknown->number *
						(count_unknowns + 1) + (size_t)x[j]->number]);
					if (debug_prep == TRUE)
					{
						output_msg(sformatf("\t\t%-24s%10.3f\t%d\t%d", "dg/dlny",
							(double)coef,
							mb_unknowns[i].unknown->number,
							x[j]->number));
					}
					store_jacob(source, target, coef);

					/* term for related phase */
					/* has related phase */
					cxxSurfaceComp *comp_ptr = use.Get_surface_ptr()->Find_comp(x[(size_t)j - 1]->surface_comp);
					if (comp_ptr->Get_phase_name().size() > 0)
					{
						/* now find the related phase */
						for (kk = (int)count_unknowns - 1; kk >= 0; kk--)
						{
							if (x[kk]->type != PP)
								continue;
							//if (x[kk]->phase->name == string_hsave(comp_ptr->Get_phase_name().c_str()))
							if (strcmp_nocase(x[kk]->phase->name, comp_ptr->Get_phase_name().c_str()) == 0)
								break;
						}
						if (kk >= 0)
						{
							source = s_diff_layer[k][charge_ptr->Get_name()].Get_drelated_moles_address();
							target = &(my_array[(size_t)(size_t)mb_unknowns[i].unknown->number *
								   (count_unknowns + 1) + (size_t)x[kk]->number]);
							if (debug_prep == TRUE)
							{
								output_msg(sformatf(
										   "\t\t%-24s%10.3f\t%d\t%d",
										   "dphase", (double) coef,
										   mb_unknowns[i].unknown->number,
										   x[kk]->number));
							}
							store_jacob(source, target, coef);
						}
					}

					if (mass_oxygen_unknown != NULL)
					{
						/* term for water, for same surfaces */
						source = s_diff_layer[k][charge_ptr->Get_name()].Get_dh2o_moles_address();
						target = &(my_array[(size_t)mb_unknowns[i].unknown->number *
							(count_unknowns + 1) +
							(size_t)mass_oxygen_unknown->number]);
						if (debug_prep == TRUE)
						{
							output_msg(sformatf(
									   "\t\t%-24s%10.3f\t%d\t%d",
									   "dn(i,s)/dlnwater", (double) coef,
									   mb_unknowns[i].unknown->number,
									   mass_oxygen_unknown->number));
						}
						store_jacob(source, target, coef);
					}
					break;
				}
				count_g++;
				if (count_g >= (int) use.Get_surface_ptr()->Get_surface_charges().size())
					break;
			}
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
build_mb_sums(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Function builds lists sum_mb1 and sum_mb2  that describe how to sum molalities
 *   to calculate mass balance sums, including activity of water, ionic strength,
 *   charge balance, and alkalinity.
 */
	int i;
	LDBLE *target;
/*
 *   Make space for lists
 */
	if (debug_prep == TRUE)
	{
		output_msg(sformatf( "\n\tMass balance summations.\n"));
	}
	for (i = 0; i < (int)mb_unknowns.size(); i++)
	{
		target = &(mb_unknowns[i].unknown->f);
		store_mb(mb_unknowns[i].source, target, mb_unknowns[i].coef);
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\t\t%-24s%10.3f\n",
					   mb_unknowns[i].unknown->description,
					   (double) mb_unknowns[i].coef));
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
build_model(void)
/* ---------------------------------------------------------------------- */
{
/*
 *    Guts of prep. Determines species in model, rewrites equations,
 *    builds lists for mass balance and jacobian sums.
 */
	int i, j;
	LDBLE coef_e;

	if (s_hplus == NULL || s_eminus == NULL || s_h2o == NULL)
	{
		error_msg("Data base is missing H+, H2O, or e- species.", CONTINUE);
		input_error++;
	}
/*
 *   Make space for lists of pointers to species in the model
 */
	// clear sum_species_map, which is built from s_x
	sum_species_map_db.clear();
	sum_species_map.clear();
	s_x.clear();
	sum_mb1.clear();
	sum_mb2.clear();
	sum_jacob0.clear();
	sum_jacob1.clear();
	sum_jacob2.clear();
	sum_delta.clear();
	species_list.clear();
/*
 *   Pick species in the model, determine reaction for model, build jacobian
 */
	s_x.clear();
	compute_gfw("H2O", &gfw_water);
	gfw_water *= 0.001;
	for (i = 0; i < (int)s.size(); i++)
	{
		if (s[i]->type > H2O && s[i]->type != EX && s[i]->type != SURF)
			continue;
		s[i]->in = FALSE;
		count_trxn = 0;
		trxn_add(s[i]->rxn_s, 1.0, false);	/* rxn_s is set in tidy_model */
/*
 *   Check if species is in model
 */
		s[i]->in = inout();
		if (s[i]->in == TRUE)
		{
			/* for isotopes, activity of water is for 1H and 16O */
			if (s[i]->gflag == 9)
			{
				gfw_water = 18.0 / 1000.0;
			}
			if (pitzer_model == FALSE && sit_model == FALSE)
				s[i]->lg = 0.0;
			compute_gfw(s[i]->name, &s[i]->gfw);
			size_t count_s_x = s_x.size();
			s_x.resize(count_s_x + 1);
			s_x[count_s_x] = s[i];
			
/*
 *   Write mass action equation for current model
 */
			//if (write_mass_action_eqn_x(STOP) == ERROR) continue;
			write_mass_action_eqn_x(STOP);
			if (s[i]->type == SURF)
			{
				add_potential_factor();
				add_cd_music_factors(i);
			}
			trxn_copy(s[i]->rxn_x);
			for (j = 0; j < 3; j++)
			{
				s[i]->dz[j] = s[i]->rxn_x.dz[j];
			}
			if (debug_mass_action == TRUE)
			{
				output_msg(sformatf( "\n%s\n\tMass-action equation\n",
						   s[i]->name));
				trxn_print();
			}
/*
 *   Determine mass balance equations, build sums for mass balance, build sums for jacobian
 */
			count_trxn = 0;
			trxn_add(s[i]->rxn_s, 1.0, false);
			if (s[i]->next_secondary.size() == 0)
			{
				write_mb_eqn_x();
			}
			else
			{
				count_elts = 0;
				add_elt_list(s[i]->next_secondary, 1.0);
			}
			if (s[i]->type == SURF)
			{
				add_potential_factor();
				add_cd_music_factors(i);
				add_surface_charge_balance();
				add_cd_music_charge_balances(i);
			}
			if (debug_prep == TRUE)
			{
				output_msg(sformatf( "\n%s, Element composition:\n",
						   trxn.token[0].s->name));
				for (j = 0; j < count_elts; j++)
				{
					output_msg(sformatf( "\t\t%-20s\t%10.2f\n",
							   elt_list[j].elt->name,
							   (double) elt_list[j].coef));
				}
			}
			//if (debug_prep == TRUE)
			//{
			//	output_msg(sformatf( "\n\tMass balance equation\n",
			//			   s[i]->name));
			//	trxn_print();
			//}
			if (s[i]->type < EMINUS)
			{
				mb_for_species_aq(i);
			}
			else if (s[i]->type == EX)
			{
				mb_for_species_ex(i);
			}
			else if (s[i]->type == SURF)
			{
				mb_for_species_surf(i);
			}
#ifdef COMBINE
			build_mb_sums();
#else
			if (s[i] != s_h2o)
			{
				build_mb_sums();
			}
#endif

			if (!pitzer_model && !sit_model)
				build_jacobian_sums(i);
/*
 *    Build list of species for summing and printing
 */
			if (s[i]->next_secondary.size() == 0)
			{
				write_mb_for_species_list(i);
			}
			else
			{
				count_elts = 0;
				add_elt_list(s[i]->next_secondary, 1.0);
			}
			build_species_list(i);
		}
	}
	if (dl_type_x != cxxSurface::NO_DL && (/*pitzer_model == TRUE || */sit_model == TRUE)) //DL_pitz
	{
		warning_msg("-diffuse_layer option not tested for SIT model");
	}
/*
 *   Sum diffuse layer water into hydrogen and oxygen mass balances
 */
	if (dl_type_x != cxxSurface::NO_DL && state >= REACTION)
	{
		for (i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type == SURFACE_CB)
			{
#ifndef COMBINE
				store_mb(&(x[i]->mass_water),
						 &(mass_hydrogen_unknwon->f), 2 / gfw_water);
#endif
				if (mass_oxygen_unknown != NULL)
				{
					store_mb(&(x[i]->mass_water),
							 &(mass_oxygen_unknown->f), 1 / gfw_water);
				}
			}
		}
	}
/*
 *   For Pitzer model add lg unknown for each aqueous species
 */
	if (pitzer_model == TRUE || sit_model == TRUE)
	{
		size_t j0 = count_unknowns;
		size_t j = count_unknowns + this->s_x.size();
		size_t k = j0;
		for (size_t i = j0; i < j; i++)
		{
			if (s_x[i - j0]->type == EX)
				continue;
			if (s_x[i - j0]->type == SURF)
				continue;
			x[k]->number = k;
			x[k]->type = PITZER_GAMMA;
			x[k]->s = s_x[i - j0];
			x[k]->description = s_x[i - j0]->name;
			k++;
			count_unknowns++;
		}
		sit_aqueous_unknowns = count_unknowns - j0;
	}
	/*
 *   Rewrite phases to current master species
 */
	for (i = 0; i < (int)phases.size(); i++)
	{
		count_trxn = 0;
		trxn_add_phase(phases[i]->rxn_s, 1.0, false);
		trxn_reverse_k();
		phases[i]->in = inout();
		if (phases[i]->in == TRUE)
		{
/*
 *   Replace e- in original equation with default redox reaction
 */
			coef_e = trxn_find_coef("e-", 1);
			if (equal(coef_e, 0.0, TOL) == FALSE)
			{
				trxn_add(pe_x[default_pe_x.c_str()], coef_e, TRUE);
			}
/*
 *   Rewrite reaction to current master species
 */
			write_mass_action_eqn_x(STOP);
			trxn_reverse_k();
			trxn_copy(phases[i]->rxn_x);
			write_phase_sys_total(i);
		}
	}
	build_solution_phase_boundaries();
	build_pure_phases();
	build_min_exch();
	build_min_surface();
	build_gas_phase();
	build_ss_assemblage();
/*
 *   Sort species list, by master only
 */
	if (species_list.size() > 1) qsort(&species_list[0], species_list.size(),
		  sizeof(class species_list), species_list_compare_master);
/*
 *   Save model description
 */
	save_model();

	if (input_error > 0)
	{
		error_msg("Stopping due to input errors.", STOP);
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_pure_phases(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Includes calculation of inverse saturation index in sum_mb.
 *   Puts coefficients in iap and mass balance equations for each phase.
 */
	bool stop;
	std::string token;
	const char* cptr;
	class master *master_ptr;
	class rxn_token *rxn_ptr;
/*
 *   Build into sums the logic to calculate inverse saturation indices for
 *   pure phases
 */
	if (pure_phase_unknown == NULL)
		return (OK);

/*
 *   Calculate inverse saturation index
 */
	for (int i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type != PP || x[i]->phase->rxn_x.token.size() == 0)
			continue;
		if (pure_phase_unknown == NULL)
			pure_phase_unknown = x[i];

		store_mb(&(x[i]->phase->lk), &(x[i]->f), 1.0);
		store_mb(&(x[i]->si), &(x[i]->f), 1.0);

		for (rxn_ptr = &x[i]->phase->rxn_x.token[0] + 1; rxn_ptr->s != NULL;
			 rxn_ptr++)
		{
			store_mb(&(rxn_ptr->s->la), &(x[i]->f), -rxn_ptr->coef);
		}
	}
	for (int i = 0; i < count_unknowns; i++)
	{
/*
 *  rxn_x is null if an element in phase is not in solution
 */
		if (x[i]->type != PP || x[i]->phase->rxn_x.token.size() == 0)
			continue;
/*
 *   Put coefficients into IAP equations
 */
		for (rxn_ptr = &x[i]->phase->rxn_x.token[0] + 1; rxn_ptr->s != NULL;
			 rxn_ptr++)
		{
			if (rxn_ptr->s->secondary != NULL
				&& rxn_ptr->s->secondary->in == TRUE)
			{
				master_ptr = rxn_ptr->s->secondary;
			}
			else
			{
				master_ptr = rxn_ptr->s->primary;
			}
			if (master_ptr == NULL || master_ptr->unknown == NULL)
				continue;
			store_jacob0((int)x[i]->number, (int)master_ptr->unknown->number,
				rxn_ptr->coef);
		}
/*
 *   Put coefficients into mass balance equations
 */
		count_elts = 0;
		paren_count = 0;
		//cxxPPassemblageComp * comp_ptr = pp_assemblage_ptr->Find(x[i]->pp_assemblage_comp_name);
		cxxPPassemblageComp * comp_ptr = (cxxPPassemblageComp *) x[i]->pp_assemblage_comp_ptr;
		if (comp_ptr->Get_add_formula().size() > 0)
		{
			cptr = comp_ptr->Get_add_formula().c_str();
			get_elts_in_species(&cptr, 1.0);
		}
		else
		{
			cptr = x[i]->phase->formula;
			get_elts_in_species(&cptr, 1.0);
		}
/*
 *   Go through elements in phase
 */

#ifdef COMBINE
		change_hydrogen_in_elt_list(0);
#endif
		for (int j = 0; j < count_elts; j++)
		{

			if (strcmp(elt_list[j].elt->name, "H") == 0
				&& mass_hydrogen_unknown != NULL)
			{
				store_jacob0((int)mass_hydrogen_unknown->number, (int)x[i]->number,
					-elt_list[j].coef);
				store_sum_deltas(&(delta[i]), &mass_hydrogen_unknown->delta,
					elt_list[j].coef);

			}
			else if (strcmp(elt_list[j].elt->name, "O") == 0
					 && mass_oxygen_unknown != NULL)
			{
				store_jacob0((int)mass_oxygen_unknown->number, (int)x[i]->number,
					-elt_list[j].coef);
				store_sum_deltas(&(delta[i]), &mass_oxygen_unknown->delta,
					elt_list[j].coef);

			}
			else
			{
				master_ptr = elt_list[j].elt->primary;
				if (master_ptr == NULL)
				{
						error_string = sformatf(
								"Element undefined, %s.",
								elt_list[j].elt->name);
						error_msg(error_string, STOP);					
				}
				if (master_ptr->in == FALSE)
				{
					master_ptr = master_ptr->s->secondary;
				}
				if (master_ptr == NULL || master_ptr->in == FALSE)
				{
					if (state != ADVECTION && state != TRANSPORT
						&& state != PHAST)
					{
						error_string = sformatf(
								"Element in phase, %s, is not in model.",
								x[i]->phase->name);
						warning_msg(error_string);
					}
					if (master_ptr != NULL)
					{
						master_ptr->s->la = -999.9;
					}
/*
 *   Master species is in model
 */
				}
				else if (master_ptr->in == TRUE)
				{
					store_jacob0((int)master_ptr->unknown->number, (int)x[i]->number,
						-elt_list[j].coef);
					store_sum_deltas(&delta[i], &master_ptr->unknown->delta,
						elt_list[j].coef);
/*
 *   Master species in equation needs to be rewritten
 */
				}
				else if (master_ptr->in == REWRITE)
				{
					stop = false;
					for (int k = 0; k < count_unknowns; k++)
					{
						if (x[k]->type != MB)
							continue;
						for (size_t l = 0; l < x[k]->master.size(); l++)
						{
							if (x[k]->master[l] == master_ptr)
							{
								store_jacob0((int)x[k]->master[0]->unknown->number,
									(int)x[i]->number, -elt_list[j].coef);
								store_sum_deltas(&delta[i],
									&x[k]->master[0]->unknown->
									delta, elt_list[j].coef);
								stop = TRUE;
								break;
							}
						}
						if (stop == TRUE)
							break;
					}
				}
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_solution_phase_boundaries(void)
/* ---------------------------------------------------------------------- */
{
	int i;
	class master *master_ptr;
	class rxn_token *rxn_ptr;
/*
 *   Build into sums the logic to calculate inverse saturation indices for
 *   solution phase boundaries
 */
	if (solution_phase_boundary_unknown == NULL)
		return (OK);
/*
 *   Calculate inverse saturation index
 */
	for (i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type != SOLUTION_PHASE_BOUNDARY)
			continue;

		store_mb(&(x[i]->phase->lk), &(x[i]->f), 1.0);
		store_mb(&(x[i]->si), &(x[i]->f), 1.0);
		if (x[i]->phase->in != TRUE)
		{
			error_string = sformatf(
					"Solution does not contain all elements for phase-boundary mineral, %s.",
					x[i]->phase->name);
			error_msg(error_string, CONTINUE);
			input_error++;
			break;
		}
		for (rxn_ptr = &x[i]->phase->rxn_x.token[0] + 1; rxn_ptr->s != NULL;
			 rxn_ptr++)
		{
			store_mb(&(rxn_ptr->s->la), &(x[i]->f), -rxn_ptr->coef);
		}
	}
	if (get_input_errors() > 0)
		return (ERROR);
/*
 *   Put coefficients into array
 */
	for (i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type != SOLUTION_PHASE_BOUNDARY)
			continue;
		for (rxn_ptr = &x[i]->phase->rxn_x.token[0] + 1; rxn_ptr->s != NULL;
			 rxn_ptr++)
		{
			if (rxn_ptr->s->secondary != NULL
				&& rxn_ptr->s->secondary->in == TRUE)
			{
				master_ptr = rxn_ptr->s->secondary;
			}
			else
			{
				master_ptr = rxn_ptr->s->primary;
			}
			if (master_ptr->unknown == NULL)
				continue;
			store_jacob0((int)x[i]->number, (int)master_ptr->unknown->number,
						 rxn_ptr->coef);
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
build_species_list(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Builds a list that includes an entry for each master species in each
 *   secondary reaction. Used for summing species of each element and 
 *   printing results.
 */
	int j;
	class master *master_ptr;
/*
 *   Treat species made only with H+, e-, and H2O specially
 */
	if (is_special(s[n]) == TRUE)
	{
		size_t count_species_list = species_list.size();
		species_list.resize(count_species_list + 1);
		species_list[count_species_list].master_s = s_hplus;
		species_list[count_species_list].s = s[n];
		species_list[count_species_list].coef = 0.0;
		return (OK);
	}
/*
 *   Treat exchange species specially
 */
	if (s[n]->type == EX)
	{
		if (s[n]->primary != NULL)
			return (OK);		/* master species has zero molality */
		for (j = 0; j < count_elts; j++)
		{
			if (elt_list[j].elt->master->s->type != EX)
				continue;
			master_ptr = elt_list[j].elt->master;
			size_t count_species_list = species_list.size();
			species_list.resize(count_species_list + 1);
			species_list[count_species_list].master_s =
				elt_list[j].elt->master->s;
			species_list[count_species_list].s = s[n];
			species_list[count_species_list].coef = master_ptr->coef *
				elt_list[j].coef;
		}
		return (OK);
	}
/*
 *   Treat surface species specially
 */
	if (s[n]->type == SURF_PSI)
		return (OK);
	if (s[n]->type == SURF)
	{
		for (j = 0; j < count_elts; j++)
		{
			if (elt_list[j].elt->master->s->type != SURF)
				continue;
			master_ptr = elt_list[j].elt->master;
			size_t count_species_list = species_list.size();
			species_list.resize(count_species_list + 1);
			species_list[count_species_list].master_s =
				elt_list[j].elt->master->s;
			species_list[count_species_list].s = s[n];
			species_list[count_species_list].coef = master_ptr->coef *
				elt_list[j].coef;
		}
		return (OK);
	}
/*
 *   Other aqueous species
 */
	for (j = 0; j < count_elts; j++)
	{
		if (is_special(elt_list[j].elt->master->s) == TRUE)
			continue;
		if (elt_list[j].elt->master->s->secondary != NULL)
		{
			master_ptr = elt_list[j].elt->master->s->secondary;
		}
		else
		{
			master_ptr = elt_list[j].elt->master->s->primary;
		}
		size_t count_species_list = species_list.size();
		species_list.resize(count_species_list + 1);
		species_list[count_species_list].master_s = master_ptr->s;
		species_list[count_species_list].s = s[n];
/*
 *    Find coefficient for element represented by master species
 */
		species_list[count_species_list].coef = master_ptr->coef *
			elt_list[j].coef;
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
clear(void)
/* ---------------------------------------------------------------------- */
{
	int i;
/*
 *   Resets information for setting up a new model
 */
	cxxSolution *solution_ptr;
/*
 *   Clear species solution-dependent data
 */
	solution_ptr = use.Get_solution_ptr();

	for (i = 0; i < (int)s.size(); i++)
	{
		s[i]->in = FALSE;
	}
/*
 *   Set pe structure
 */
	pe_x.clear();
	default_pe_x.clear();
	if (solution_ptr->Get_initial_data())
	{
		pe_x = solution_ptr->Get_initial_data()->Get_pe_reactions();
		default_pe_x = solution_ptr->Get_initial_data()->Get_default_pe();
	}
	else
	{
		default_pe_x = "pe";
		CReaction chem_rxn;
		pe_x[default_pe_x] = chem_rxn;
	}

/*
 *   Clear master species solution-dependent data
 */
	const char * pe_str = string_hsave("pe");
	for (i = 0; i < (int)master.size(); i++)
	{
		master[i]->in = FALSE;
		master[i]->unknown = NULL;
		if (solution_ptr->Get_initial_data())
		{
			master[i]->pe_rxn = solution_ptr->Get_initial_data()->Get_default_pe();
		}
		else
		{
			master[i]->pe_rxn = pe_str;
		}
/*
 *   copy primary reaction to secondary reaction
 */
		master[i]->rxn_secondary = master[i]->rxn_primary;
	}

	if (state == INITIAL_SOLUTION)
	{
		s_h2o->secondary->in = TRUE;
		s_hplus->secondary->in = TRUE;
	}
	else
	{
		s_h2o->primary->in = TRUE;
		s_hplus->primary->in = TRUE;
	}
	s_eminus->primary->in = TRUE;
/*
 *   Set all unknown pointers to NULL
 */
	mb_unknown = NULL;
	ah2o_unknown = NULL;
	mass_hydrogen_unknown = NULL;
	mass_oxygen_unknown = NULL;
	mu_unknown = NULL;
	alkalinity_unknown = NULL;
	carbon_unknown = NULL;
	ph_unknown = NULL;
	pe_unknown = NULL;
	charge_balance_unknown = NULL;
	solution_phase_boundary_unknown = NULL;
	pure_phase_unknown = NULL;
	exchange_unknown = NULL;
	surface_unknown = NULL;
	gas_unknown = NULL;
	ss_unknown = NULL;
/*
 *   Free arrays used in model   
 */
	free_model_allocs();
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
convert_units(cxxSolution *solution_ptr)
/* ---------------------------------------------------------------------- */
{
/*
 *   Converts solution concentrations to moles/kg water
 *   Uses totals.input conc to calculate totals.moles.
 */
	LDBLE sum_solutes;
	class master *master_ptr;
	std::string token;
	if (!solution_ptr->Get_new_def() || !solution_ptr->Get_initial_data())
   {
      input_error++;
      error_msg("Missing data for convert_units", 1);
   }
/*
 *   Convert units
 */
#ifdef ORIGINAL
	sum_solutes = exp(-solution_ptr->Get_ph() * LOG_10);
#else
	double g_h, g_oh;
	compute_gfw("H", &g_h);
	compute_gfw("OH", &g_oh);
	if (density_iterations == 0)
	{
		sum_solutes = exp(-solution_ptr->Get_ph() * LOG_10) * g_h;
		sum_solutes += exp((-14 + solution_ptr->Get_ph()) * LOG_10) * g_oh;
	}
	else
	{
		double soln_vol = calc_solution_volume();
		sum_solutes = s_hplus->moles / soln_vol * g_h;
		species* s_oh = s_search("OH-");
		sum_solutes += s_oh->moles / soln_vol * g_oh;
	}
#endif
	cxxISolution *initial_data_ptr = solution_ptr->Get_initial_data();
	std::map<std::string, cxxISolutionComp >::iterator jit = initial_data_ptr->Get_comps().begin();
	for ( ; jit != initial_data_ptr->Get_comps().end(); jit++)
	{
		cxxISolutionComp &comp_ref = jit->second;
		LDBLE moles;
		master_ptr = master_bsearch(comp_ref.Get_description().c_str());
		if (master_ptr != NULL)
		{
			if (master_ptr->minor_isotope == TRUE)
				continue;
		}

		// initially store 0.0 for totals
		solution_ptr->Get_totals()[comp_ref.Get_description()] = 0.0;

		if (strcmp(comp_ref.Get_description().c_str(), "H(1)") == 0 ||
			strcmp(comp_ref.Get_description().c_str(), "E") == 0)
		{
			continue;
		}
		if (comp_ref.Get_input_conc() <= 0)
			continue;
/*
 *   Get gfw
 */
		/* use given gfw if gfw > 0.0 */
		/* use formula give with "as" */
		if (comp_ref.Get_gfw() <= 0.0)
		{
			if (comp_ref.Get_as().size() > 0)
			{
				/* use given chemical formula to calculate gfw */
				if (compute_gfw(comp_ref.Get_as().c_str(), &dummy) == ERROR)
				{
					error_string = sformatf( "Could not compute gfw, %s.",
							comp_ref.Get_as().c_str());
					error_msg(error_string, CONTINUE);
					input_error++;
				}
				else
				{
					comp_ref.Set_gfw(dummy);
				}
				if (strcmp(comp_ref.Get_description().c_str(), "Alkalinity") == 0 &&
					strcmp(comp_ref.Get_as().c_str(), "CaCO3") == 0)
				{
					comp_ref.Set_gfw(comp_ref.Get_gfw() / 2.0);
					error_string = sformatf(
							"Equivalent wt for alkalinity should be Ca.5(CO3).5. Using %g g/eq.",
							(double) comp_ref.Get_gfw());
					warning_msg(error_string);
				}
				/* use gfw of master species */
			}
			else
			{
				const char* cptr = comp_ref.Get_description().c_str();
				copy_token(token, &cptr);
				master_ptr = master_bsearch(token.c_str());
				if (master_ptr != NULL)
				{
					/* use gfw for element redox state */
					comp_ref.Set_gfw(master_ptr->gfw);
				}
				else
				{
					error_string = sformatf( "Could not find gfw, %s.",
							comp_ref.Get_description().c_str());
					error_msg(error_string, CONTINUE);
					input_error++;
					continue;
				}
			}
		}
/*
 *   Convert liters to kg solution
 */
		moles = comp_ref.Get_input_conc();
		if (strstr(initial_data_ptr->Get_units().c_str(), "/l") != NULL)
		{
			moles *= 1.0 / (solution_ptr->Get_density());
		}
/*
 *   Convert milli or micro
 */
		char c = comp_ref.Get_units()[0];
		if (c == 'm')
		{
			moles *= 1e-3;
		}
		else if (c == 'u')
		{
			moles *= 1e-6;
		}
/*
 *   Sum grams of solute, convert from moles necessary
 */
		if (strstr(comp_ref.Get_units().c_str(), "g/kgs") != NULL ||
			strstr(comp_ref.Get_units().c_str(), "g/l") != NULL)
		{
			sum_solutes += moles;
		}
		else if (strstr(comp_ref.Get_units().c_str(), "Mol/kgs") != NULL ||
				 strstr(comp_ref.Get_units().c_str(), "Mol/l") != NULL ||
				 strstr(comp_ref.Get_units().c_str(), "eq/l") != NULL)
		{
			sum_solutes += moles * comp_ref.Get_gfw();
		}
/*
 *   Convert grams to moles, if necessary
 */
		if (strstr(comp_ref.Get_units().c_str(), "g/") != NULL && comp_ref.Get_gfw() != 0.0)
		{
			moles /= comp_ref.Get_gfw();
		}
		solution_ptr->Get_totals()[comp_ref.Get_description()] = moles;
	}
/*
 *   Convert /kgs to /kgw
 */
	if (strstr(initial_data_ptr->Get_units().c_str(), "kgs") != NULL ||
		strstr(initial_data_ptr->Get_units().c_str(), "/l") != NULL)
	{
		mass_water_aq_x = 1.0 - 1e-3 * sum_solutes;
		if (mass_water_aq_x <= 0)
		{
			error_string = sformatf( "Solute mass exceeds solution mass in conversion from /kgs to /kgw.\n"
				"Mass of water is negative.");
			error_msg(error_string, CONTINUE);
			input_error++;
		}
		cxxNameDouble::iterator it;
		for (it = solution_ptr->Get_totals().begin(); it != solution_ptr->Get_totals().end(); it++)
		{
			it->second = it->second / mass_water_aq_x;
		}
	}
/*
 *   Scale by mass of water in solution
 */
	mass_water_aq_x = solution_ptr->Get_mass_water();
	cxxNameDouble::iterator it;
	for (it = solution_ptr->Get_totals().begin(); it != solution_ptr->Get_totals().end(); it++)
	{
		it->second = it->second * mass_water_aq_x;
	}

	initial_data_ptr->Set_units(moles_per_kilogram_string);

	return (OK);
}

/* ---------------------------------------------------------------------- */
std::vector<class master *> Phreeqc::
get_list_master_ptrs(const char* cptr, class master *master_ptr)
/* ---------------------------------------------------------------------- */
{
/*
 *   Input: cptr contains a list of one or more master species names
 *   Output: space is allocated and a list of master species pointers is
 *           returned.
 */
	int j, l, count_list;
	char token[MAX_LENGTH];
	std::vector<class master*> master_ptr_list;
	class master *master_ptr0;
/*
 *   Make list of master species pointers
 */
	count_list = 0;
	//master_ptr_list = unknown_alloc_master();
	master_ptr0 = master_ptr;
	if (master_ptr0 == master_ptr->s->primary)
	{
/*
 *   First in list is primary species
 */
		for (j = 0; j < (int)master.size(); j++)
		{
			if (master[j] == master_ptr0)
				break;
		}
		j++;
/*
 *   Element has only one valence
 */
		if (j >= (int)master.size() || master[j]->elt->primary != master_ptr0)
		{
			master_ptr_list.push_back(master_ptr0);
/*
 *   Element has multiple valences
 */
		}
		else
		{
			if (master_ptr0->s->secondary == NULL)
			{
				error_string = sformatf(
						"Master species for valence states of element %s are not correct.\n\tPossibly related to master species for %s.",
						master_ptr0->elt->name, master[j]->elt->name);
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			master_ptr_list.push_back(master_ptr0->s->secondary);
			while (j < (int)master.size() && master[j]->elt->primary == master_ptr0)
			{
				if (master[j]->s->primary == NULL)
				{
					master_ptr_list.push_back(master[j]);
				}
				j++;
			}
		}
	}
	else
	{
/*
 *   First in list is secondary species, Include all valences from input
 */
		master_ptr_list.push_back(master_ptr0);
		while (copy_token(token, &cptr, &l) != EMPTY)
		{
			master_ptr = master_bsearch(token);
			if (master_ptr != NULL)
			{
				master_ptr_list.push_back(master_ptr);
			}
		}
	}
	return (master_ptr_list);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
inout(void)
/* ---------------------------------------------------------------------- */
{
	int i;
	class rxn_token_temp *token_ptr;
/*
 *   Routine goes through trxn to determine if each master species is
 *   in this model.
 *   Assumes equation is written in terms of primary and secondary species
 *   Checks to see if in is TRUE or REWRITE for each species
 *   Returns TRUE if in model
 *           FALSE if not
 */
	for (i = 1; i < count_trxn; i++)
	{
		token_ptr = &(trxn.token[i]);
		/*   Check primary master species in */
		if (token_ptr->s->primary != NULL
			&& (token_ptr->s->primary->in == TRUE))
			continue;
		/*   Check secondary master species */
		if ((token_ptr->s->secondary != NULL)
			&& (token_ptr->s->secondary->in != FALSE))
		{
			continue;
		}
		/*   Must be primary master species that is out */
		return (FALSE);
	}
	return (TRUE);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
is_special(class species *l_spec)
/* ---------------------------------------------------------------------- */
{
/*
 *   Checks to see if a species is composed of only H, O, and e-
 *   Returns TRUE if true
 *           FALSE if not
 */
	int special;
	class rxn_token *token_ptr;

	special = TRUE;
	for (token_ptr = &l_spec->rxn_s.token[0] + 1; token_ptr->s != NULL;
		 token_ptr++)
	{
		if (token_ptr->s != s_hplus &&
			token_ptr->s != s_h2o && token_ptr->s != s_eminus)
		{
			special = FALSE;
			break;
		}
	}
	return (special);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
store_mb_unknowns(class unknown *unknown_ptr, LDBLE * LDBLE_ptr, LDBLE coef,
				  LDBLE * gamma_ptr)
/* ---------------------------------------------------------------------- */
/*
 *   Takes an unknown pointer and a coefficient and puts in
 *   list of mb_unknowns
 */
{
	if (equal(coef, 0.0, TOL) == TRUE)
		return (OK);
	size_t count_mb_unknowns = mb_unknowns.size();
	mb_unknowns.resize(count_mb_unknowns + 1);
	mb_unknowns[count_mb_unknowns].unknown = unknown_ptr;
	mb_unknowns[count_mb_unknowns].source = LDBLE_ptr;
	mb_unknowns[count_mb_unknowns].gamma_source = gamma_ptr;
	mb_unknowns[count_mb_unknowns].coef = coef;
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
mb_for_species_aq(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Make list of mass balance and charge balance equations in which
 *   to insert species n. 
 *
 *        count_mb_unknowns - number of equations and summation relations
 *        mb_unknowns.unknown - pointer to unknown which contains row number
 *        mb_unknowns.source - pointer to the LDBLE number to be multiplied
 *                             by coef, usually moles.
 *        mb_unknowns.coef - coefficient of s[n] in equation or relation
 */
	int i, j;
	class master *master_ptr;
	class unknown *unknown_ptr;

	mb_unknowns.clear();
/*
 *   e- does not appear in any mass balances
 */
	if (s[n]->type == EMINUS)
		return (OK);
/* 
 *   Do not include diffuse layer in cb, alk, ah2o, mu
 */
	if (charge_balance_unknown != NULL && s[n]->type < H2O)
	{
		store_mb_unknowns(charge_balance_unknown, &s[n]->moles, s[n]->z,
						  &s[n]->dg);
	}
	if (alkalinity_unknown != NULL && s[n]->type < H2O)
	{
		store_mb_unknowns(alkalinity_unknown, &s[n]->moles, s[n]->alk,
						  &s[n]->dg);
	}
	if (ah2o_unknown != NULL && s[n]->type < H2O)
	{
		store_mb_unknowns(ah2o_unknown, &s[n]->moles, 1.0, &s[n]->dg);
	}
	if (mu_unknown != NULL && s[n]->type < H2O)
	{
		store_mb_unknowns(mu_unknown, &s[n]->moles, s[n]->z * s[n]->z,
						  &s[n]->dg);
	}
/* 
 *   Include diffuse layer in hydrogen and oxygen mass balance
 */
	if (mass_hydrogen_unknown != NULL)
	{
		if (dl_type_x != cxxSurface::NO_DL && state >= REACTION)
		{
#ifdef COMBINE
			store_mb_unknowns(mass_hydrogen_unknown, &s[n]->tot_g_moles,
							  s[n]->h - 2 * s[n]->o, &s[n]->dg_total_g);
#else
			store_mb_unknowns(mass_hydrogen_unknown, &s[n]->tot_g_moles,
							  s[n]->h, &s[n]->dg_total_g);
#endif
		}
		else
		{
#ifdef COMBINE
			store_mb_unknowns(mass_hydrogen_unknown, &s[n]->moles,
							  s[n]->h - 2 * s[n]->o, &s[n]->dg);
#else
			store_mb_unknowns(mass_hydrogen_unknown, &s[n]->moles, s[n]->h,
							  &s[n]->dg);
#endif
		}
	}
	if (mass_oxygen_unknown != NULL)
	{
		if (dl_type_x != cxxSurface::NO_DL && state >= REACTION)
		{
			store_mb_unknowns(mass_oxygen_unknown, &s[n]->tot_g_moles,
							  s[n]->o, &s[n]->dg_total_g);
		}
		else
		{
			store_mb_unknowns(mass_oxygen_unknown, &s[n]->moles, s[n]->o,
							  &s[n]->dg);
		}
	}
/* 
 *   Sum diffuse layer charge into (surface + DL) charge balance
 */
	if (use.Get_surface_ptr() != NULL && s[n]->type < H2O && dl_type_x != cxxSurface::NO_DL)
	{
		j = 0;
		for (i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type == SURFACE_CB)
			{
				cxxSurfaceCharge *charge_ptr = use.Get_surface_ptr()->Find_charge(x[i]->surface_charge);
				unknown_ptr = x[i];
				if (use.Get_surface_ptr()->Get_type() == cxxSurface::CD_MUSIC)
					unknown_ptr = x[(size_t)i + 2];

				store_mb_unknowns(unknown_ptr, s_diff_layer[n][charge_ptr->Get_name()].Get_g_moles_address(),
								  s[n]->z, s_diff_layer[n][charge_ptr->Get_name()].Get_dg_g_moles_address());
				j++;
			}
		}
	}
/*
 *   Other mass balances
 */
	for (i = 0; i < count_elts; i++)
	{
		if (elt_list[i].elt->master->s->type > AQ &&
			elt_list[i].elt->master->s->type < SOLID)
			continue;
		master_ptr = elt_list[i].elt->master;
		if (master_ptr->primary == TRUE)
		{
			if (master_ptr->s->secondary != NULL)
			{
				master_ptr = master_ptr->s->secondary;
			}
		}
		if (master_ptr->unknown == ph_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == pe_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == charge_balance_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == alkalinity_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == NULL)
		{
			//std::cerr << "NULL: " << master_ptr->s->name << std::endl;
			continue;
		}
		else if (master_ptr->unknown->type == SOLUTION_PHASE_BOUNDARY)
		{
			continue;
		}
		if (dl_type_x != cxxSurface::NO_DL && state >= REACTION)
		{
			store_mb_unknowns(master_ptr->unknown,
							  &s[n]->tot_g_moles,
							  elt_list[i].coef * master_ptr->coef,
							  &s[n]->dg_total_g);
		}
		else
		{
			store_mb_unknowns(master_ptr->unknown,
							  &s[n]->moles,
							  elt_list[i].coef * master_ptr->coef, &s[n]->dg);
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
mb_for_species_ex(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Make list of mass balance and charge balance equations in which
 *   to insert exchange species n. 
 *
 *        count_mb_unknowns - number of equations and summation relations
 *        mb_unknowns.source - pointer to the LDBLE number to be multiplied
 *                             by coef, usually moles.
 *        mb_unknowns.unknown - pointer to unknown which contains row number
 *        mb_unknowns.coef - coefficient of s[n] in equation or relation
 */
	int i;
	class master *master_ptr;

	mb_unknowns.clear();
/*
 *   Master species for exchange do not appear in any mass balances
 */
	if (s[n]->type == EX && s[n]->primary != NULL)
		return (OK);
/* 
 *   Include diffuse layer in hydrogen and oxygen mass balance
 */
	if (charge_balance_unknown != NULL)
	{
		store_mb_unknowns(charge_balance_unknown, &s[n]->moles, s[n]->z,
						  &s[n]->dg);
	}
	if (mass_hydrogen_unknown != NULL)
	{
#ifdef COMBINE
		store_mb_unknowns(mass_hydrogen_unknown, &s[n]->moles,
						  s[n]->h - 2 * s[n]->o, &s[n]->dg);
#else
		store_mb_unknowns(mass_hydrogen_unknown, &s[n]->moles, s[n]->h,
						  &s[n]->dg);
#endif
	}
	if (mass_oxygen_unknown != NULL)
	{
		store_mb_unknowns(mass_oxygen_unknown, &s[n]->moles, s[n]->o,
						  &s[n]->dg);
	}
/*
 *   Other mass balances
 */
	for (i = 0; i < count_elts; i++)
	{
		if (elt_list[i].elt->master->s->type > AQ &&
			elt_list[i].elt->master->s->type < SOLID)
			continue;
		master_ptr = elt_list[i].elt->master;
		if (master_ptr->primary == TRUE)
		{
			if (master_ptr->s->secondary != NULL)
			{
				master_ptr = master_ptr->s->secondary;
			}
		}
/*
 *   Special for ph_unknown, pe_unknown, and alkalinity_unknown
 */
		if (master_ptr->unknown == ph_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == pe_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == alkalinity_unknown)
		{
			continue;
		}
/*
 *   EX, sum exchange species only into EXCH mass balance in initial calculation
 *   into all mass balances in reaction calculation
 */
		if (state >= REACTION || master_ptr->s->type == EX)
		{
			store_mb_unknowns(master_ptr->unknown, &s[n]->moles,
							  elt_list[i].coef * master_ptr->coef, &s[n]->dg);
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
mb_for_species_surf(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Make list of mass balance and charge balance equations in which
 *   to insert species n. 
 *
 *        count_mb_unknowns - number of equations and summation relations
 *        mb_unknowns.source - pointer to the LDBLE number to be multiplied
 *                             by coef, usually moles.
 *        mb_unknowns.unknown - pointer to unknown which contains row number
 *        mb_unknowns.coef - coefficient of s[n] in equation or relation
 */
	int i;
	class master *master_ptr;

	mb_unknowns.clear();
/*
 *   Include in charge balance, if diffuse_layer_x == FALSE
 */
	if (charge_balance_unknown != NULL && dl_type_x == cxxSurface::NO_DL)
	{
		store_mb_unknowns(charge_balance_unknown, &s[n]->moles, s[n]->z,
						  &s[n]->dg);
	}
/* 
 *   Include diffuse layer in hydrogen and oxygen mass balance
 */
	if (mass_hydrogen_unknown != NULL)
	{
#ifdef COMBINE
		store_mb_unknowns(mass_hydrogen_unknown, &s[n]->moles,
						  s[n]->h - 2 * s[n]->o, &s[n]->dg);
#else
		store_mb_unknowns(mass_hydrogen_unknown, &s[n]->moles, s[n]->h,
						  &s[n]->dg);
#endif
	}
	if (mass_oxygen_unknown != NULL)
	{
		store_mb_unknowns(mass_oxygen_unknown, &s[n]->moles, s[n]->o,
						  &s[n]->dg);
	}
/*
 *   Other mass balances
 */
/*
 *   Other mass balances
 */
	for (i = 0; i < count_elts; i++)
	{
/*   Skip H+, e-, and H2O */
		if (elt_list[i].elt->master->s->type > AQ &&
			elt_list[i].elt->master->s->type < SOLID)
			continue;
		master_ptr = elt_list[i].elt->master;
		if (master_ptr->primary == TRUE)
		{
			if (master_ptr->s->secondary != NULL)
			{
				master_ptr = master_ptr->s->secondary;
			}
		}
/*
 *   SURF_PSI, sum surface species in (surface + DL) charge balance
 */
		if (master_ptr->s->type == SURF_PSI
			&& use.Get_surface_ptr()->Get_type() != cxxSurface::CD_MUSIC)
		{
			store_mb_unknowns(master_ptr->unknown, &s[n]->moles, s[n]->z,
							  &s[n]->dg);
			continue;
		}
		if (master_ptr->s->type == SURF_PSI
			&& use.Get_surface_ptr()->Get_type() == cxxSurface::CD_MUSIC)
		{
			store_mb_unknowns(master_ptr->unknown, &s[n]->moles, s[n]->dz[0],
							  &s[n]->dg);
			continue;
		}
		if (master_ptr->s->type == SURF_PSI1)
		{
			store_mb_unknowns(master_ptr->unknown, &s[n]->moles, s[n]->dz[1],
							  &s[n]->dg);
			continue;
		}
		if (master_ptr->s->type == SURF_PSI2)
		{
			store_mb_unknowns(master_ptr->unknown, &s[n]->moles, s[n]->dz[2],
							  &s[n]->dg);
			/*
			   if (diffuse_layer_x == TRUE) {
			   store_mb_unknowns(master_ptr->unknown, &s[n]->moles, s[n]->z, &s[n]->dg ); 
			   } else {
			   store_mb_unknowns(master_ptr->unknown, &s[n]->moles, s[n]->dz[2], &s[n]->dg ); 
			   }
			 */
			continue;
		}
/*
 *   Special for ph_unknown, pe_unknown, and alkalinity_unknown
 */
		if (master_ptr->unknown == ph_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == pe_unknown)
		{
			continue;
		}
		else if (master_ptr->unknown == alkalinity_unknown)
		{
			continue;
		}
/*
 *   SURF, sum surface species only into SURFACE mass balance in initial calculation
 *   into all mass balances in reaction calculation
 */
		if (state >= REACTION || master_ptr->s->type == SURF)
		{
			store_mb_unknowns(master_ptr->unknown, &s[n]->moles,
							  elt_list[i].coef * master_ptr->coef, &s[n]->dg);
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
reprep(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   If a basis species has been switched, makes new model.
 *   Unknowns are not changed, but mass-action equations are
 *   rewritten and lists for mass balance and jacobian are regenerated
 */
	int i;
/*
 *   Initialize s, master, and unknown pointers
 */
	for (i = 0; i < (int)master.size(); i++)
	{
		if (master[i]->in == FALSE)
			continue;
		master[i]->rxn_secondary = master[i]->rxn_primary;
	}
	resetup_master();
/*
 *   Set unknown pointers, unknown types, validity checks
 */
	tidy_redox();
	if (get_input_errors() > 0)
	{
		error_msg("Program terminating due to input errors.", STOP);
	}
/*
 *   Free arrays built in build_model
 */
	s_x.clear();
	sum_mb1.clear();
	sum_mb2.clear();
	sum_jacob0.clear();
	sum_jacob1.clear();
	sum_jacob2.clear();
	sum_delta.clear(); 
/*
 *   Build model again
 */
	build_model();
	k_temp(tc_x, patm_x);

	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
resetup_master(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   For basis switch, rewrite equations for master species
 *   Set master_ptr->rxn_secondary,
 *       master_ptr->pe_rxn,
 *       and special cases for alkalinity, carbon, and pH.
 */
	int i, j;
	class master *master_ptr, *master_ptr0;

	for (i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type != MB)
			continue;
		master_ptr0 = x[i]->master[0];
		for (j = 0; j < x[i]->master.size(); j++)
		{
			master_ptr = x[i]->master[j];
/*
 *   Set flags
 */
			if (j == 0)
			{
				if (master_ptr->s->primary == NULL)
				{
					master_ptr->rxn_secondary = master_ptr->s->rxn_s;
				}
			}
			else
			{
				if (master_ptr0->s->primary == NULL)
				{
					rewrite_master_to_secondary(master_ptr, master_ptr0);
					trxn_copy(master_ptr->rxn_secondary);
				}
			}
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
write_mass_action_eqn_x(int stop)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reduce mass-action equation to the master species that are in the model
 */
	LDBLE coef_e;
	int count, repeat;
	int i;
	size_t count_rxn_orig;
/*
 *   Rewrite any secondary master species flagged REWRITE
 *   Replace pe if necessary
 */
	count = 0;
	repeat = TRUE;
	while (repeat == TRUE)
	{
		count++;
		if (count > MAX_ADD_EQUATIONS)
		{
			std::string name;
			name = "Unknown";
			if (trxn.token[0].s != NULL)
			{
				name = trxn.token[0].s->name;
			}
			
			input_error++;
			error_string = sformatf( "Could not reduce equation "
					"to primary and secondary species that are "
					"in the model.  Species: %s.", name.c_str());
			if (stop == STOP)
			{
				error_msg(error_string, CONTINUE);
			}
			else
			{
				warning_msg(error_string);
			}
			return (ERROR);
		}
		repeat = FALSE;
		count_rxn_orig = count_trxn;
		for (i = 1; i < count_rxn_orig; i++)
		{
			if (trxn.token[i].s->secondary == NULL)
				continue;
			if (trxn.token[i].s->secondary->in == REWRITE)
			{
				repeat = TRUE;
				coef_e =
					rxn_find_coef(trxn.token[i].s->secondary->rxn_secondary,
								  "e-");
				trxn_add(trxn.token[i].s->secondary->rxn_secondary,
						 trxn.token[i].coef, false);
				if (equal(coef_e, 0.0, TOL) == FALSE)
				{
					std::map < std::string, CReaction >::iterator chemRxnIt = pe_x.find(trxn.token[i].s->secondary->pe_rxn);
					if ( chemRxnIt == pe_x.end() )
					{
						CReaction& rxn_ref = pe_x[trxn.token[i].s->secondary->pe_rxn];
						trxn_add(rxn_ref, trxn.token[i].coef * coef_e, FALSE);
						// Create temporary rxn object and add reactions together
						CReaction rxn;
						trxn_add(rxn, trxn.token[i].coef * coef_e, FALSE);
					}
					else
					{
						// Get reaction referred to by iterator and add reactions together
						trxn_add(chemRxnIt->second, trxn.token[i].coef * coef_e, FALSE);
					}
				}
			}
		}
		trxn_combine();
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
add_potential_factor(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Add the potential factor to surface mass-action equations.
 *   Factor is essentially the activity coefficient, representing
 *   the work required to bring charged ions to the surface
 */
	int i;
	std::string token;
	LDBLE sum_z;
	class master *master_ptr;
	class unknown *unknown_ptr;

	if (use.Get_surface_ptr() == NULL)
	{
		input_error++;
		error_string = sformatf(
				"SURFACE not defined for surface species %s",
				trxn.token[0].name);
		error_msg(error_string, CONTINUE);
		return(OK);
	}
	if (use.Get_surface_ptr()->Get_type() != cxxSurface::DDL && use.Get_surface_ptr()->Get_type() != cxxSurface::CCM)
		return (OK);
	sum_z = 0.0;
	master_ptr = NULL;
/*
 *   Find sum of charge of aqueous species and surface master species
 */
	for (i = 1; i < count_trxn; i++)
	{
		if (trxn.token[i].s->type == AQ || trxn.token[i].s == s_hplus ||
			trxn.token[i].s == s_eminus)
		{
			sum_z += trxn.token[i].s->z * trxn.token[i].coef;
		}
		if (trxn.token[i].s->type == SURF)
		{
			master_ptr = trxn.token[i].s->primary;
		}
	}
/*
 *  Find potential unknown for surface species
 */
	if (master_ptr == NULL)
	{
		error_string = sformatf(
				"Did not find a surface species in equation defining %s",
				trxn.token[0].name);
		error_msg(error_string, CONTINUE);
		error_string = sformatf(
				"One of the following must be defined with SURFACE_SPECIES:");
		error_msg(error_string, CONTINUE);
		for (i = 1; i < count_trxn; i++)
		{
			error_string = sformatf( "     %s", trxn.token[i].name);
			error_msg(error_string, CONTINUE);
		}
		input_error++;
		return (ERROR);
	}
	token =  master_ptr->elt->name;
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI);
	if (unknown_ptr == NULL)
	{
		error_string = sformatf(
		  "No potential unknown found for surface species %s.", token.c_str());
		error_msg(error_string, STOP);
	}
	else
	{
		master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	}
/*
 *   Make sure there is space
 */
	if (count_trxn + 1 > trxn.token.size())
		trxn.token.resize(count_trxn + 1);
/*
 *   Include psi in mass action equation
 */
	if (master_ptr != NULL)
	{
		trxn.token[count_trxn].name = master_ptr->s->name;
		trxn.token[count_trxn].s = master_ptr->s;
		trxn.token[count_trxn].coef = -2.0 * sum_z;
		count_trxn++;
	}
	else
	{
		output_msg(sformatf(
				   "How did this happen in add potential factor?\n"));
	}

	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
add_cd_music_factors(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Add the potential factors for cd_music to surface mass-action equations.
 *   Factors are essentially the activity coefficient, representing
 *   the work required to bring charged ions to the three charge layers 
 *   of the cd_music model
 */
	int i;
	std::string token;
	class master *master_ptr;
	class unknown *unknown_ptr;
	if (use.Get_surface_ptr() == NULL)
	{
		input_error++;
		error_string = sformatf(
				"SURFACE not defined for surface species %s",
				trxn.token[0].name);
		error_msg(error_string, CONTINUE);
		return(OK);
	}
	if (use.Get_surface_ptr()->Get_type() != cxxSurface::CD_MUSIC)
		return (OK);
	master_ptr = NULL;
/*
 *   Find sum of charge of aqueous species and surface master species
 */
	for (i = 1; i < count_trxn; i++)
	{
		if (trxn.token[i].s->type == SURF)
		{
			master_ptr = trxn.token[i].s->primary;
		}
	}
/*
 *  Find potential unknown for surface species
 */
	if (master_ptr == NULL)
	{
		error_string = sformatf(
				"Did not find a surface species in equation defining %s",
				trxn.token[0].name);
		error_msg(error_string, CONTINUE);
		error_string = sformatf(
				"One of the following must be defined with SURFACE_SPECIES:");
		error_msg(error_string, CONTINUE);
		for (i = 1; i < count_trxn; i++)
		{
			error_string = sformatf( "     %s", trxn.token[i].name);
			error_msg(error_string, CONTINUE);
		}
		input_error++;
		return (ERROR);
	}
	token = master_ptr->elt->name;
	/*
	 *  Plane 0
	 */
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI);
	if (unknown_ptr == NULL)
	{
		error_string = sformatf(
		  "No potential unknown found for surface species %s.", token.c_str());
		error_msg(error_string, STOP);
		return (ERROR);
	}
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	/*
	 *   Make sure there is space
	 */
	if (count_trxn + 3 > trxn.token.size())
		trxn.token.resize(count_trxn + 3);
	/*
	 *   Include psi in mass action equation
	 */
	trxn.token[count_trxn].name = master_ptr->s->name;
	trxn.token[count_trxn].s = master_ptr->s;
	/*trxn.token[count_trxn].coef = s[n]->dz[0];*/
	trxn.token[count_trxn].coef = trxn.dz[0];

	count_trxn++;

	/*
	 *  Plane 1
	 */
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI1);
	if (unknown_ptr == NULL)
	{
		error_string = sformatf(
		  "No potential unknown found for surface species %s.", token.c_str());
		error_msg(error_string, STOP);
		return (ERROR);
	}
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	/*
	 *   Include psi in mass action equation
	 */
	trxn.token[count_trxn].name = master_ptr->s->name;
	trxn.token[count_trxn].s = master_ptr->s;
	/*trxn.token[count_trxn].coef = s[n]->dz[1];*/
	trxn.token[count_trxn].coef = trxn.dz[1];
	count_trxn++;
	/*
	 *  Plane 2
	 */
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI2);
	if (unknown_ptr == NULL)
	{
		error_string = sformatf(
		  "No potential unknown found for surface species %s.", token.c_str());
		error_msg(error_string, STOP);
		return (ERROR);
	}
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	/*
	 *   Include psi in mass action equation
	 */
	trxn.token[count_trxn].name = master_ptr->s->name;
	trxn.token[count_trxn].s = master_ptr->s;
	/*trxn.token[count_trxn].coef = s[n]->dz[2];*/
	trxn.token[count_trxn].coef = trxn.dz[2];
	count_trxn++;

	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
add_surface_charge_balance(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Include charge balance in list for mass-balance equations
 */
	int i;
	const char* cptr;
	std::string token;

	class master *master_ptr;
	class unknown *unknown_ptr;
	if (use.Get_surface_ptr() == NULL)
	{
		input_error++;
		error_string = sformatf(
				"SURFACE not defined for surface species %s",
				trxn.token[0].name);
		error_msg(error_string, CONTINUE);
		return(OK);
	}
	if (use.Get_surface_ptr()->Get_type() != cxxSurface::DDL && use.Get_surface_ptr()->Get_type() != cxxSurface::CCM)
		return (OK);
	master_ptr = NULL;
/*
 *   Find master species
 */
	for (i = 0; i < count_elts; i++)
	{
		if (elt_list[i].elt->primary->s->type == SURF)
		{
			master_ptr = elt_list[i].elt->primary;
			break;
		}
	}
	if (i >= count_elts)
	{
		error_string = sformatf(
				"No surface master species found for surface species.");
		error_msg(error_string, STOP);
		return(OK);
	}
/*
 *  Find potential unknown for surface species
 */
	token = master_ptr->elt->name;
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI);
	if (unknown_ptr == NULL)
	{
		error_string = sformatf(
		  "No potential unknown found for surface species %s.", token.c_str());
		error_msg(error_string, STOP);
		return(OK);
	}
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
/*
 *   Include charge balance in list for mass-balance equations
 */
	cptr = master_ptr->elt->name;
	get_secondary_in_species(&cptr, 1.0);

	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
add_cd_music_charge_balances(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Add the potential factor to surface mass-action equations.
 *   Factor is essentially the activity coefficient, representing
 *   the work required to bring charged ions to the surface
 */
	int i;
	std::string token;

	class master *master_ptr;
	class unknown *unknown_ptr;
	if (use.Get_surface_ptr() == NULL)
	{
		input_error++;
		error_string = sformatf(
				"SURFACE not defined for surface species %s",
				trxn.token[0].name);
		error_msg(error_string, CONTINUE);
		return(OK);
	}
	if (use.Get_surface_ptr()->Get_type() != cxxSurface::CD_MUSIC)
		return (OK);
	master_ptr = NULL;
/*
 *   Find master species
 */
	for (i = 0; i < count_elts; i++)
	{
		if (elt_list[i].elt->primary->s->type == SURF)
		{
			master_ptr = elt_list[i].elt->primary;
			break;
		}
	}
	if (i >= count_elts || master_ptr == NULL)
	{
		error_string = sformatf(
				"No surface master species found for surface species.");
		error_msg(error_string, STOP);
		return ERROR;
	}
	/*
	 *  Find potential unknown for plane 0
	 */
	token = master_ptr->elt->name;
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI);
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	/*
	 *   Include charge balance in list for mass-balance equations
	 */
	{
		const char* cptr = master_ptr->elt->name;
		get_secondary_in_species(&cptr, s[n]->dz[0]);
	}
	/*
	 *  Find potential unknown for plane 1
	 */
	token = master_ptr->elt->name;
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI1);
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	/*
	 *   Include charge balance in list for mass-balance equations
	 */
	{
		const char* cptr = master_ptr->elt->name;
		get_secondary_in_species(&cptr, s[n]->dz[1]);
	}
	/*
	 *  Find potential unknown for plane 2
	 */
	token = master_ptr->elt->name;
	unknown_ptr = find_surface_charge_unknown(token, SURF_PSI2);
	master_ptr = unknown_ptr->master[0];	/* potential for surface component */
	/*
	 *   Include charge balance in list for mass-balance equations
	 */
	{
		const char* cptr = master_ptr->elt->name;
		get_secondary_in_species(&cptr, s[n]->dz[2]);
	}

	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
rewrite_master_to_secondary(class master *master_ptr1,
							class master *master_ptr2)
/* ---------------------------------------------------------------------- */
{
/*
 *   Write equation for secondary master species in terms of another secondary master species
 *   Store result in rxn_secondary of master_ptr.
 */
	LDBLE coef1, coef2;
	class master *master_ptr_p1, *master_ptr_p2;
/*
 *   Check that the two master species have the same primary master species
 */
	master_ptr_p1 = master_ptr1->elt->primary;
	master_ptr_p2 = master_ptr2->elt->primary;
	if (master_ptr_p1 != master_ptr_p2 || master_ptr_p1 == NULL)
	{
		error_string = sformatf(
				"All redox states must be for the same element. %s\t%s.",
				master_ptr1->elt->name, master_ptr2->elt->name);
		error_msg(error_string, CONTINUE);
		input_error++;
		return (ERROR);
	}
/*
 *   Find coefficient of primary master in reaction
 */
	coef1 = rxn_find_coef(master_ptr1->rxn_primary, master_ptr_p1->s->name);
	coef2 = rxn_find_coef(master_ptr2->rxn_primary, master_ptr_p1->s->name);
	if (equal(coef1, 0.0, TOL) == TRUE || equal(coef2, 0.0, TOL) == TRUE)
	{
		error_string = sformatf(
				"One of these equations does not contain master species for element, %s or %s.",
				master_ptr1->s->name, master_ptr2->s->name);
		error_msg(error_string, CONTINUE);
		input_error++;
		return (ERROR);
	}
/*
 *   Rewrite equation to secondary master species
 */
	count_trxn = 0;
	trxn_add(master_ptr1->rxn_primary, 1.0, false);
	trxn_add(master_ptr2->rxn_primary, -coef1 / coef2, true);
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_exchange(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Fill in data for exchanger in unknowns structures
 */
	class master *master_ptr;
	std::vector<class master*> master_ptr_list;

	if (use.Get_exchange_ptr() == NULL)
		return (OK);
	for (size_t j = 0; j < use.Get_exchange_ptr()->Get_exchange_comps().size(); j++)
	{
		cxxExchComp & comp_ref = use.Get_exchange_ptr()->Get_exchange_comps()[j];
		//{
		//	element * elt_ptr = element_store(comp_ref.Get_formula().c_str());
		//	if (elt_ptr == NULL || elt_ptr->master == NULL)
		//	{
		//		error_string = sformatf( "Component not in database, %s", comp_ref.Get_formula().c_str());
		//		input_error++;
		//		error_msg(error_string, CONTINUE);
		//		continue;
		//	}
		//}

		cxxNameDouble nd(comp_ref.Get_totals());
		cxxNameDouble::iterator it = nd.begin();
		for ( ; it != nd.end(); it++)
		{
/*
 *   Find master species
 */
			element * elt_ptr = element_store(it->first.c_str());
			if (elt_ptr == NULL || elt_ptr->master == NULL)
			{
				error_string = sformatf( "Master species not in database "
						"for %s, skipping element.",
						it->first.c_str());
				input_error++;
				error_msg(error_string, CONTINUE);
				continue;
			}
			master_ptr = elt_ptr->master;
			if (master_ptr->type != EX)
				continue;
/*
 *   Check for data already given
 */
			if (master_ptr->in != FALSE)
			{
				x[master_ptr->unknown->number]->moles +=
					it->second;
			}
			else
			{
/*
 *   Set flags
 */
				master_ptr_list.clear();
				master_ptr_list.push_back(master_ptr);
				master_ptr->in = TRUE;
/*
 *   Set unknown data
 */
				x[count_unknowns]->type = EXCH;
				x[count_unknowns]->exch_comp = string_hsave(it->first.c_str());
				x[count_unknowns]->description = elt_ptr->name;
				x[count_unknowns]->moles = it->second;
				x[count_unknowns]->master = master_ptr_list;
				x[count_unknowns]->master[0]->unknown = x[count_unknowns];
				count_unknowns++;
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_gas_phase(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Fill in data for gas phase unknown (sum of partial pressures)
 *   in unknown structure
 */
	if (use.Get_gas_phase_ptr() == NULL)
		return (OK);
	cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
	if (gas_phase_ptr->Get_type() == cxxGasPhase::GP_VOLUME && (
		gas_phase_ptr->Get_pr_in() || force_numerical_fixed_volume) && numerical_fixed_volume)
	{
		return setup_fixed_volume_gas();
	}

/*
 *   One for total moles in gas
 */
	x[count_unknowns]->type = GAS_MOLES;
	x[count_unknowns]->description = string_hsave("gas moles");
	x[count_unknowns]->moles = 0.0;
	for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
	{	
		cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
		x[count_unknowns]->moles += gc_ptr->Get_moles();
	}
	if (x[count_unknowns]->moles <= 0)
		x[count_unknowns]->moles = MIN_TOTAL;
	x[count_unknowns]->ln_moles = log(x[count_unknowns]->moles);
	gas_unknown = x[count_unknowns];
	count_unknowns++;
	return (OK);
}


/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_ss_assemblage(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Fill in data for solid solution unknowns (sum of partial pressures)
 *   in unknown structure
 */
	if (use.Get_ss_assemblage_ptr() == NULL)
		return (OK);
/*
 *   One for each component in each solid solution
 */
	ss_unknown = NULL;
	std::vector<cxxSS *> ss_ptrs = use.Get_ss_assemblage_ptr()->Vectorize();
	for (size_t j = 0; j < ss_ptrs.size(); j++)
	{
		for (size_t i = 0; i < ss_ptrs[j]->Get_ss_comps().size(); i++)
		{
			cxxSScomp *comp_ptr = &(ss_ptrs[j]->Get_ss_comps()[i]);
			int l;
			class phase* phase_ptr = phase_bsearch(comp_ptr->Get_name().c_str(), &l, FALSE);
			x[count_unknowns]->type = SS_MOLES;
			x[count_unknowns]->description = string_hsave(comp_ptr->Get_name().c_str());
			x[count_unknowns]->moles = 0.0;
			if (comp_ptr->Get_moles() <= 0)
			{
				comp_ptr->Set_moles(MIN_TOTAL_SS);
			}
			x[count_unknowns]->moles = comp_ptr->Get_moles();
			comp_ptr->Set_initial_moles(x[count_unknowns]->moles);
			x[count_unknowns]->ln_moles = log(x[count_unknowns]->moles);
			x[count_unknowns]->ss_name = string_hsave(ss_ptrs[j]->Get_name().c_str());
			x[count_unknowns]->ss_ptr =  ss_ptrs[j];
			x[count_unknowns]->ss_comp_name = string_hsave(comp_ptr->Get_name().c_str());
			x[count_unknowns]->ss_comp_ptr = comp_ptr;
			x[count_unknowns]->ss_comp_number = (int) i;
			x[count_unknowns]->phase = phase_ptr;
			x[count_unknowns]->number = count_unknowns;
			x[count_unknowns]->phase->dn = comp_ptr->Get_dn();
			x[count_unknowns]->phase->dnb =	comp_ptr->Get_dnb();
			x[count_unknowns]->phase->dnc = comp_ptr->Get_dnc();
			x[count_unknowns]->phase->log10_fraction_x = comp_ptr->Get_log10_fraction_x();
			x[count_unknowns]->phase->log10_lambda =comp_ptr->Get_log10_lambda();
			if (ss_unknown == NULL)
				ss_unknown = x[count_unknowns];
			count_unknowns++;
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_surface(void)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Fill in data for surface assemblage in unknown structure
	 */
	std::vector<class master*> master_ptr_list;
	size_t mb_unknown_number;

	if (use.Get_surface_ptr() == NULL)
		return (OK);

	for (size_t i = 0; i < use.Get_surface_ptr()->Get_surface_comps().size(); i++)
	{
		cxxSurfaceComp *comp_ptr = &(use.Get_surface_ptr()->Get_surface_comps()[i]);
		/*
		 *   Find master species for each surface, setup unknown structure
		 */
		cxxNameDouble::iterator jit;
		for (jit = comp_ptr->Get_totals().begin(); jit != comp_ptr->Get_totals().end(); jit++)
		{
			class element *elt_ptr = element_store(jit->first.c_str());
			class master *master_ptr = elt_ptr->master;
			if (master_ptr == NULL)
			{
				error_string = sformatf(
						"Master species not in database for %s, skipping element.",
						elt_ptr->name);
				warning_msg(error_string);
				continue;
			}
			if (master_ptr->type != SURF)
				continue;
			/*
			 *   Check that data not already given
			 */ 
			if (master_ptr->in != FALSE)
			{
				error_string = sformatf(
						"Analytical data entered twice for %s.",
						master_ptr->s->name);
				error_msg(error_string, CONTINUE);
				input_error++;
				continue;
			}
			/*
			 *   Set flags
			 */
			master_ptr_list.clear();
			master_ptr_list.push_back(master_ptr);
			master_ptr->in = TRUE;
			/*
			 *   Setup mass balance unknown
			 */
			x[count_unknowns]->type = SURFACE;
			x[count_unknowns]->description = string_hsave(jit->first.c_str());
			x[count_unknowns]->number = count_unknowns;
			x[count_unknowns]->surface_comp = string_hsave(comp_ptr->Get_formula().c_str());
			x[count_unknowns]->master = master_ptr_list;
			x[count_unknowns]->master[0]->unknown = x[count_unknowns];
			x[count_unknowns]->moles = jit->second;
			if (surface_unknown == NULL)
				surface_unknown = x[count_unknowns];
			x[count_unknowns]->potential_unknown = NULL;
			count_unknowns++;
			/*if (use.Get_surface_ptr()->edl == FALSE) continue; */
			if (use.Get_surface_ptr()->Get_type() == cxxSurface::DDL || use.Get_surface_ptr()->Get_type() == cxxSurface::CCM)
			{
				/*
				 *   Setup surface-potential unknown
				 */
				std::string token = master_ptr->elt->name;
				class unknown *unknown_ptr = find_surface_charge_unknown(token, SURF_PSI);
				if (unknown_ptr != NULL)
				{
					x[count_unknowns - 1]->potential_unknown = unknown_ptr;
				}
				else
				{
					/*
					 *   Find master species
					 */
					replace("_CB", "_psi", token);
					master_ptr = master_bsearch(token.c_str());
					master_ptr_list.clear();
					master_ptr_list.push_back(master_ptr);
					master_ptr->in = TRUE;
					/*
					 *   Find surface charge structure
					 */
					cxxSurfaceCharge *charge_ptr =  use.Get_surface_ptr()->
						Find_charge(comp_ptr->Get_charge_name());
					if (charge_ptr == NULL)
					{
						input_error++;
						error_msg(sformatf("Charge structure not defined for surface, %s", use.Get_surface_ptr()->Get_description().c_str()), CONTINUE);
						continue;
					}
					x[count_unknowns]->type = SURFACE_CB;
					x[count_unknowns]->surface_charge = string_hsave(charge_ptr->Get_name().c_str());
					x[count_unknowns]->related_moles = charge_ptr->Get_grams();
					x[count_unknowns]->mass_water = charge_ptr->Get_mass_water();
					replace("_psi", "_CB", token);
					x[count_unknowns]->description = string_hsave(token.c_str());
					x[count_unknowns]->master = master_ptr_list;
					x[count_unknowns]->master[0]->unknown = x[count_unknowns];
					x[count_unknowns]->moles = 0.0;
					x[count_unknowns - 1]->potential_unknown = x[count_unknowns];
					x[count_unknowns]->surface_comp = x[count_unknowns - 1]->surface_comp;
					count_unknowns++;
				}
			}
			else if (use.Get_surface_ptr()->Get_type() == cxxSurface::CD_MUSIC)
			{
				/*
				 *   Setup 3 surface-potential unknowns
				 */
				mb_unknown_number = count_unknowns - 1;
				std::string token(master_ptr->elt->name);
				std::string mass_balance_name(token);
				int plane;
				for (plane = SURF_PSI; plane <= SURF_PSI2; plane++)
				{
					std::string cb_suffix("_CB");
					std::string psi_suffix("_psi");
					class unknown **unknown_target;
					unknown_target = NULL;
					int type = SURFACE_CB;
					switch (plane)
					{
					case SURF_PSI:
						type = SURFACE_CB;
						unknown_target =
							&(x[mb_unknown_number]->potential_unknown);
						break;
					case SURF_PSI1:
						cb_suffix.append("b");
						psi_suffix.append("b");
						type = SURFACE_CB1;
						unknown_target = &(x[mb_unknown_number]->potential_unknown1);
						break;
					case SURF_PSI2:
						cb_suffix.append("d");
						psi_suffix.append("d");
						type = SURFACE_CB2;
						unknown_target = &(x[mb_unknown_number]->potential_unknown2);
						break;
					}
					class unknown *unknown_ptr = find_surface_charge_unknown(token, plane);
					if (unknown_ptr != NULL)
					{
						*unknown_target = unknown_ptr;
					}
					else
					{
						/*
						 *   Find master species
						 */
						replace(cb_suffix.c_str(), psi_suffix.c_str(), token);
						master_ptr = master_bsearch(token.c_str());
						master_ptr_list.clear();
						master_ptr_list.push_back(master_ptr);
						master_ptr->in = TRUE;
						/*
						 *   Find surface charge structure
						 */
						cxxSurfaceCharge *charge_ptr =  use.Get_surface_ptr()->
							Find_charge(comp_ptr->Get_charge_name());
						x[count_unknowns]->type = type;
						x[count_unknowns]->surface_charge = string_hsave(charge_ptr->Get_name().c_str());
						x[count_unknowns]->related_moles = charge_ptr->Get_grams();
						x[count_unknowns]->mass_water = charge_ptr->Get_mass_water();
						replace(psi_suffix.c_str(), cb_suffix.c_str(), token);
						x[count_unknowns]->description = string_hsave(token.c_str());
						x[count_unknowns]->master = master_ptr_list;
						/*
						 *   Find surface charge structure
						 */
						if (plane == SURF_PSI)
						{
							/*use.Get_surface_ptr()->charge[k].psi_master = x[count_unknowns]->master[0]; */
							x[mb_unknown_number]->potential_unknown =
								x[count_unknowns];
						}
						else if (plane == SURF_PSI1)
						{
							/*use.Get_surface_ptr()->charge[k].psi_master1 = x[count_unknowns]->master[0]; */
							x[mb_unknown_number]->potential_unknown1 =
								x[count_unknowns];
						}
						else if (plane == SURF_PSI2)
						{
							/*use.Get_surface_ptr()->charge[k].psi_master2 = x[count_unknowns]->master[0]; */
							x[mb_unknown_number]->potential_unknown2 =
								x[count_unknowns];
						}
						x[count_unknowns]->master[0]->unknown =
							x[count_unknowns];
						x[count_unknowns]->moles = 0.0;
						x[count_unknowns]->surface_comp =
							x[mb_unknown_number]->surface_comp;
						count_unknowns++;
					}
				}
				/* Add SURFACE unknown to a list for SURF_PSI */
				class unknown *unknown_ptr = find_surface_charge_unknown(token, SURF_PSI);
				unknown_ptr->comp_unknowns.push_back(x[mb_unknown_number]);

			}
		}
	}
	/*
	 *   check related phases
	 */
	if (use.Get_surface_ptr()->Get_related_phases())
	{
		cxxPPassemblage *pp_ptr = Utilities::Rxn_find(Rxn_pp_assemblage_map, use.Get_n_surface_user());
		for (size_t i = 0; i < use.Get_surface_ptr()->Get_surface_comps().size(); i++)
		{
			if (use.Get_surface_ptr()->Get_surface_comps()[i].Get_phase_name().size() > 0)
			{
				if (pp_ptr == NULL || 
					(pp_ptr->Get_pp_assemblage_comps().find(use.Get_surface_ptr()->Get_surface_comps()[i].Get_phase_name()) == 
					pp_ptr->Get_pp_assemblage_comps().end()))
				{
					Rxn_new_surface.insert(use.Get_n_surface_user());
					cxxSurface *surf_ptr = Utilities::Rxn_find(Rxn_surface_map, use.Get_n_surface_user());
					surf_ptr->Set_new_def(true);
					this->tidy_min_surface();
					return (FALSE);
				}
			}
		}
		for (int i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type != SURFACE_CB)
				continue;
			cxxSurfaceComp *comp_i_ptr = use.Get_surface_ptr()->Find_comp(x[i]->surface_comp);
			for (int j = 0; j < count_unknowns; j++)
			{
				if (x[j]->type != SURFACE)
					continue;
				if (x[j]->potential_unknown != x[i])
					continue;
				cxxSurfaceComp *comp_j_ptr = use.Get_surface_ptr()->Find_comp(x[j]->surface_comp);
				std::string name1, name2;
				if (comp_j_ptr->Get_phase_name() !=
					comp_i_ptr->Get_phase_name())
				{
					if (comp_i_ptr->Get_phase_name().size() == 0)
					{
						name1 = "None";
					}
					else
					{
						name1 = comp_i_ptr->Get_phase_name();
					}
					if (comp_j_ptr->Get_phase_name().size() == 0)
					{
						name2 = "None";
					}
					else
					{
						name2 = comp_j_ptr->Get_phase_name();
					}
					input_error++;

					error_string = sformatf(
							"All surface sites for a single component must be related to the same phase.\n\tSite: %s is related to %s, Site: %s is related to %s",
							comp_i_ptr->Get_master_element().c_str(), name1.c_str(),
							comp_j_ptr->Get_master_element().c_str(), name2.c_str());
					error_msg(error_string, CONTINUE);
				}
			}
		}
	}
	/*
	 *   check related kinetics
	 */
	if (use.Get_surface_ptr()->Get_related_rate())
	{
		cxxKinetics *kinetics_ptr = Utilities::Rxn_find(Rxn_kinetics_map, use.Get_n_surface_user());
		for (size_t i = 0; i < use.Get_surface_ptr()->Get_surface_comps().size(); i++)
		{
			if (use.Get_surface_ptr()->Get_surface_comps()[i].Get_rate_name().size() > 0)
			{
				if (kinetics_ptr == NULL || 
					(kinetics_ptr->Find(use.Get_surface_ptr()->Get_surface_comps()[i].Get_rate_name()) == NULL))
				{
					Rxn_new_surface.insert(use.Get_n_surface_user());
					this->tidy_kin_surface();
					return (FALSE);
				}
			}
		}
		for (int i = 0; i < count_unknowns; i++)
		{
			if (x[i]->type != SURFACE_CB)
				continue;
			cxxSurfaceComp *comp_i_ptr = use.Get_surface_ptr()->Find_comp(x[i]->surface_comp);
			for (int j = 0; j < count_unknowns; j++)
			{
				if (x[j]->type != SURFACE)
					continue;
				if (x[j]->potential_unknown != x[i])
					continue;
				cxxSurfaceComp *comp_j_ptr = use.Get_surface_ptr()->Find_comp(x[j]->surface_comp);
				if (comp_j_ptr->Get_rate_name() !=
					comp_i_ptr->Get_rate_name())
				{
					std::string name1, name2;
					if (comp_i_ptr->Get_rate_name().size() == 0)
					{
						name1 = "None";
					}
					else
					{
						name1 = comp_i_ptr->Get_rate_name();
					}
					if (comp_j_ptr->Get_rate_name().size() == 0)
					{
						name2 = "None";
					}
					else
					{
						name2 = comp_j_ptr->Get_rate_name();
					}
					input_error++;
					error_string = sformatf(
							"All surface sites for a single component must be related to the same kinetic reaction.\n\tSite: %s is related to %s, Site: %s is related to %s",
							comp_i_ptr->Get_master_element().c_str(), name1.c_str(),
							comp_j_ptr->Get_master_element().c_str(), name2.c_str());
					error_msg(error_string, CONTINUE);
				}
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
class unknown * Phreeqc::
find_surface_charge_unknown(std::string &str, int plane)
/* ---------------------------------------------------------------------- */
{
/*
 *    Makes name for the potential unknown and returns in str_ptr
 *    Returns NULL if this unknown not in unknown list else
 *    returns a pointer to the potential unknown
 */
	std::string token;
	Utilities::replace("_", " ", str);
	std::string::iterator b = str.begin();
	std::string::iterator e = str.end();
	CParser::copy_token(token, b, e);
	if (plane == SURF_PSI)
	{
		token.append("_CB");
	}
	else if (plane == SURF_PSI1)
	{
		token.append("_CBb");
	}
	else if (plane == SURF_PSI2)
	{
		token.append("_CBd");
	}
	str = token;
	for (int i = 0; i < count_unknowns; i++)
	{
		if (strcmp(str.c_str(), x[i]->description) == 0)
		{
			return (x[i]);
		}
	}
	return (NULL);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_master_rxn(const std::vector<class master *> &master_ptr_list, const std::string &pe_rxn)
/* ---------------------------------------------------------------------- */
{
/*
 *   Rewrites rxn_secondary for all redox states in list
 *   First, in = TRUE; others, in = REWRITE 
 */
	class master *master_ptr, *master_ptr0;
/*
 *   Set master_ptr->in, master_ptr->rxn
 */
	master_ptr0 = master_ptr_list[0];
	for (size_t j = 0; j < master_ptr_list.size(); j++)
	{
		master_ptr = master_ptr_list[j];
/*
 *   Check that data not already given
 */
		if (master_ptr->s == s_h2o)
		{
			error_string = sformatf(
					"Cannot enter concentration data for O(-2),\n\tdissolved oxygen is O(0),\n\tfor mass of water, use -water identifier.");
			error_msg(error_string, CONTINUE);
			input_error++;
			continue;
		}

		if (master_ptr->in != FALSE)
		{
			if (master_ptr->s != s_eminus && master_ptr->s != s_hplus)
			{
				error_string = sformatf(
						"Analytical data entered twice for %s.",
						master_ptr->s->name);
				error_msg(error_string, CONTINUE);
				input_error++;
				continue;
			}
		}
/*
 *   Set flags
 */
		if (j == 0)
		{
			master_ptr->in = TRUE;
			if (master_ptr->s->primary == NULL)
			{
				master_ptr->rxn_secondary = master_ptr->s->rxn_s;
			}
		}
		else
		{
			master_ptr->in = REWRITE;
			if (master_ptr0->s->primary == NULL)
			{
				rewrite_master_to_secondary(master_ptr, master_ptr0);
				trxn_copy(master_ptr->rxn_secondary);
			}
		}
		master_ptr->pe_rxn = string_hsave(pe_rxn.c_str());
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
LDBLE Phreeqc::
calc_PR(std::vector<class phase *> phase_ptrs, LDBLE P, LDBLE TK, LDBLE V_m)
/* ---------------------------------------------------------------------- */
/*  Calculate fugacity and fugacity coefficient for gas pressures if critical T and P
    are defined.
  1) Solve molar volume V_m or total pressure P from Peng-Robinson's EOS:
  P = R * T / (V_m - b) - a * aa / (V_m^2 + 2 * b * V_m - b^2)
     a = 0.457235 * (R * T_c)^2 / P_c
     b = 0.077796 * R * T_c / P_c
     aa = (1 + kk * (1 - T_r^0.5))^2
     kk = 0.37464 + 1.54226 * omega - 0.26992 * omega^2
     T_r = T / T_c
  multicomponent gas phase:
     use: b_sum = Sum(x_i * b), x_i is mole-fraction
          a_aa_sum = Sum_i( Sum_j(x_i * x_j * (a_i * aa_i * a_j * aa_j)^0.5) )
  2) Find the fugacity coefficient phi for gas i:
  log(phi_i) = B_ratio * (z - 1) - log(z - B) + A / (2.8284 * B) * (B_ratio - 2 / a_aa_sum * a_aa_sum2) *\
           log((z + 2.4142 * B) / (z - 0.4142 * B))
     B_ratio = b_i / b_sum
     A = a_aa_sum * P / R_TK^2
     B = b_sum * P / R_TK
     a_aa_sum2 = Sum_j(x_j * (a_aa_i * a_aa_j)^0.5
  3) correct the solubility of gas i with:
  pr_si_f = log10(phi_i) -  Delta_V_i * (P - 1) / (2.303 * R * TK);
*/
{
	int i, i1, n_g = (int) phase_ptrs.size();
	LDBLE T_c, P_c;
	LDBLE A, B, B_r, /*b2,*/ kk, oo, a_aa, T_r;
	LDBLE m_sum, /*b_sum, a_aa_sum,*/ a_aa_sum2;
	LDBLE phi;
	LDBLE /*R_TK,*/ R = R_LITER_ATM; /* L atm / (K mol) */
	LDBLE r3[4], r3_12, rp, rp3, rq, rz, ri, ri1, one_3 = 0.33333333333333333;
	LDBLE disct, vinit, v1, ddp, dp_dv, dp_dv2;
	int it;
	class phase *phase_ptr, *phase_ptr1;
	cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
	bool halved;
	R_TK = R * TK;
	m_sum = b_sum = a_aa_sum = 0.0;
	for (i = 0; i < n_g; i++)
	{
		phase_ptr = phase_ptrs[i];
		if (n_g > 1)
		{
			if (phase_ptr->moles_x == 0)
				continue;
			m_sum += phase_ptr->moles_x;
		}
		if (phase_ptr->t_c == 0.0 || phase_ptr->p_c == 0.0)
			error_msg("Cannot calculate a mixture of ideal and Peng_Robinson gases,\n       please define Tc and Pc for the active gases in PHASES.", STOP);
			//continue;
		if (!phase_ptr->pr_a)
		{
			T_c = phase_ptr->t_c;
			P_c = phase_ptr->p_c;
			phase_ptr->pr_a = 0.457235 * R * R * T_c * T_c / P_c;
			phase_ptr->pr_b = 0.077796 * R * T_c / P_c;
			T_r = TK / T_c;
			oo = phase_ptr->omega;
			kk = 0.37464 + oo * (1.54226 - 0.26992 * oo);
			phase_ptr->pr_alpha = pow(1 + kk * (1 - sqrt(T_r)), 2);
			phase_ptr->pr_tk = TK;
//			phase_ptr->pr_in = true;
		}
		if (phase_ptr->pr_tk != TK)
		{
			T_r = TK / phase_ptr->t_c;
			oo = phase_ptr->omega;
			kk = 0.37464 + oo * (1.54226 - 0.26992 * oo);
			phase_ptr->pr_alpha = pow(1 + kk * (1 - sqrt(T_r)), 2);
			phase_ptr->pr_tk = TK;
//			phase_ptr->pr_in = true;
		}
	}
	for (i = 0; i < n_g; i++)
	{
		phase_ptr = phase_ptrs[i];
		if (n_g == 1)
		{
			phase_ptr->fraction_x = 1.0;
			break;
		}
		if (m_sum == 0)
			return (OK);
		phase_ptr->fraction_x = phase_ptr->moles_x / m_sum;
	}
	 
	for (i = 0; i < n_g; i++)
	{
		a_aa_sum2 = 0.0;
		phase_ptr = phase_ptrs[i];
		//if (phase_ptr->t_c == 0.0 || phase_ptr->p_c == 0.0)
		//	continue;
		b_sum += phase_ptr->fraction_x * phase_ptr->pr_b;
		for (i1 = 0; i1 < n_g; i1++)
		{
			phase_ptr1 = phase_ptrs[i1];
			//if (phase_ptr1->t_c == 0.0 || phase_ptr1->p_c == 0.0)
			//	continue;
			if (phase_ptr1->fraction_x == 0)
				continue;
			a_aa = sqrt(phase_ptr->pr_a * phase_ptr->pr_alpha *
				        phase_ptr1->pr_a * phase_ptr1->pr_alpha);
			if (!strcmp(phase_ptr->name, "H2O(g)"))
			{
				if (!strcmp(phase_ptr1->name, "CO2(g)"))
					a_aa *= 0.81; // Soreide and Whitson, 1992, FPE 77, 217
				else if (!strcmp(phase_ptr1->name, "H2S(g)") || !strcmp(phase_ptr1->name, "H2Sg(g)"))
					a_aa *= 0.81;
				else if (!strcmp(phase_ptr1->name, "CH4(g)") || !strcmp(phase_ptr1->name, "Mtg(g)") || !strcmp(phase_ptr1->name, "Methane(g)"))
					a_aa *= 0.51;
				else if (!strcmp(phase_ptr1->name, "N2(g)") || !strcmp(phase_ptr1->name, "Ntg(g)"))
					a_aa *= 0.51;
				else if (!strcmp(phase_ptr1->name, "Ethane(g)"))
					a_aa *= 0.51;
				else if (!strcmp(phase_ptr1->name, "Propane(g)"))
					a_aa *= 0.45;
			}
			if (!strcmp(phase_ptr1->name, "H2O(g)"))
			{
				if (!strcmp(phase_ptr->name, "CO2(g)"))
					a_aa *= 0.81;
				else if (!strcmp(phase_ptr->name, "H2S(g)") || !strcmp(phase_ptr->name, "H2Sg(g)"))
					a_aa *= 0.81;
				else if (!strcmp(phase_ptr->name, "CH4(g)") || !strcmp(phase_ptr->name, "Mtg(g)") || !strcmp(phase_ptr->name, "Methane(g)"))
					a_aa *= 0.51;
				else if (!strcmp(phase_ptr->name, "N2(g)") || !strcmp(phase_ptr->name, "Ntg(g)"))
					a_aa *= 0.51;
				else if (!strcmp(phase_ptr->name, "Ethane(g)"))
					a_aa *= 0.51;
				else if (!strcmp(phase_ptr->name, "Propane(g)"))
					a_aa *= 0.45;
			}
			a_aa_sum += phase_ptr->fraction_x * phase_ptr1->fraction_x * a_aa;
			a_aa_sum2 += phase_ptr1->fraction_x * a_aa;
		}
		phase_ptr->pr_aa_sum2 = a_aa_sum2;
	}
	b2 = b_sum * b_sum;

	if (V_m)
	{
		P = R_TK / (V_m - b_sum) - a_aa_sum / (V_m * (V_m + 2 * b_sum) - b2);
		if (iterations > 0 && P < 150 && V_m < 1.01)
		{
			// check for 3-roots...
			r3[1] = b_sum - R_TK / P;
			r3[2] = -3.0 * b2 + (a_aa_sum - R_TK * 2.0 * b_sum) / P;
			r3[3] = b2 * b_sum + (R_TK * b2 - b_sum * a_aa_sum) / P;
			// the discriminant of the cubic eqn...
			disct = 18. * r3[1] * r3[2] * r3[3] -
				4. * pow(r3[1], 3) * r3[3] + 
				r3[1] * r3[1] * r3[2] * r3[2] -
				4. * pow(r3[2], 3) - 
				27. * r3[3] * r3[3];
			//if (iterations > 50)
			//	it = 0;	// debug
			if (disct > 0)
			{
				// 3-roots, find the largest P...
				it = 0;
				halved = false;
				ddp = 1e-9;
				v1 = vinit = 0.729;
				dp_dv = f_Vm(v1, this);
				while (fabs(dp_dv) > 1e-11 && it < 40)
				{
					it +=1;
					dp_dv2 = f_Vm(v1 - ddp, this);
					v1 -= (dp_dv * ddp / (dp_dv - dp_dv2));
					if (!halved && (v1 > vinit || v1 < 0.03))
					{
						if (vinit > 0.329)
							vinit -= 0.1;
						else
							vinit -=0.05;
						if (vinit < 0.03)
						{
							vinit = halve(f_Vm, 0.03, 1.0, 1e-3);
							if (f_Vm(vinit - 2e-3, this) < 0)
								vinit = halve(f_Vm, vinit + 2e-3, 1.0, 1e-3);
							halved = true;
						}
						v1 = vinit;
					}
					dp_dv = f_Vm(v1, this);
					if (fabs(dp_dv) < 1e-11)
					{
						if (f_Vm(v1 - 1e-4, this) < 0)
						{
							v1 = halve(f_Vm, v1 + 1e-4, 1.0, 1e-3);
							dp_dv = f_Vm(v1, this);
						}
					}
				}
				if (it == 40)
				{
// accept a (possible) whobble in the curve...
//					error_msg("No convergence when calculating P in Peng-Robinson.", STOP);
				}
				if (V_m < v1 && it < 40)
					P = R_TK / (v1 - b_sum) - a_aa_sum / (v1 * (v1 + 2 * b_sum) - b2);
			}
		}
		if (P <= 0) // iterations = -1
			P = 1;
	} else
	{
		if (P < 1e-10)
			P = 1e-10;
		r3[1] = b_sum - R_TK / P;
		r3_12 = r3[1] * r3[1];
		r3[2] = -3.0 * b2 + (a_aa_sum - R_TK * 2.0 * b_sum) / P;
		r3[3] = b2 * b_sum + (R_TK * b2 - b_sum * a_aa_sum) / P;
		// solve t^3 + rp*t + rq = 0.
		// molar volume V_m = t - r3[1] / 3... 
		rp = r3[2] - r3_12 / 3;
		rp3 = rp * rp * rp;
		rq = (2.0 * r3_12 * r3[1] - 9.0 * r3[1] * r3[2]) / 27 + r3[3];
		rz = rq * rq / 4 + rp3 / 27;
		if (rz >= 0) // Cardono's method...
		{
			ri = sqrt(rz);
			if (ri + rq / 2 <= 0)
			{
				V_m = pow(ri - rq / 2, one_3) + pow(- ri - rq / 2, one_3) - r3[1] / 3;
			}
			else
			{
				ri = - pow(ri + rq / 2, one_3);
				V_m = ri - rp / (3.0 * ri) - r3[1] / 3;
			}
		}
		else // use complex plane...
		{
			ri = sqrt(- rp3 / 27); // rp < 0
			ri1 = acos(- rq / 2 / ri);
			V_m = 2.0 * pow(ri, one_3) * cos(ri1 / 3) - r3[1] / 3;
		}
	}
 // calculate the fugacity coefficients...
	for (i = 0; i < n_g; i++)
	{
		phase_ptr = phase_ptrs[i];
		if (phase_ptr->fraction_x == 0.0)
		{
			phase_ptr->pr_p = 0;
			phase_ptr->pr_phi = 1;
			phase_ptr->pr_si_f = 0.0;
			continue;
		}
		phase_ptr->pr_p = phase_ptr->fraction_x * P;
		rz = P * V_m / R_TK;
		A = a_aa_sum * P / (R_TK * R_TK);
		B = b_sum * P / R_TK;
		B_r = phase_ptr->pr_b / b_sum;
		if (rz > B)
		{
			phi = B_r * (rz - 1) - log(rz - B) + A / (2.828427 * B) * (B_r - 2.0 * phase_ptr->pr_aa_sum2 / a_aa_sum) *
				log((rz + 2.41421356 * B) / (rz - 0.41421356 * B));
			//phi = (phi > 4.44 ? 4.44 : (phi < -3 ? -3 : phi));
			//if (phi > 4.44)
			//	phi = 4.44;
		}
		else
			phi = -3.0; // fugacity coefficient = 0.05
		//if (/*!strcmp(phase_ptr->name, "H2O(g)") && */phi < -3)
		//{
		////	 avoid such phi...
		//	phi = -3;
		//}
		phase_ptr->pr_phi = exp(phi);
		phase_ptr->pr_si_f = phi / LOG_10;
		// for initial equilibrations, adapt log_k of the gas phase...
		if (state < REACTION)
		{
			rho_0 = calc_rho_0(TK - 273.15, P);
			calc_dielectrics(TK - 273.15, P);
			phase_ptr->lk = calc_lk_phase(phase_ptr, TK, P);
		}
		phase_ptr->pr_in = true;
	}
	if (gas_phase_ptr && iterations > 2)
	{
		if (gas_phase_ptr->Get_type() == cxxGasPhase::GP_VOLUME)
		{
			gas_phase_ptr->Set_total_p(P);
		}
		gas_phase_ptr->Set_v_m(V_m);
		return (OK);
	}
	return (V_m);
}

LDBLE Phreeqc::
f_Vm(LDBLE v1, void *cookie)
/* ---------------------------------------------------------------------- */
{
	LDBLE ff;
	Phreeqc * pThis;
	pThis = (Phreeqc *) cookie;

	ff = v1 * (v1 + 2 * pThis->b_sum) - pThis->b2;
	LDBLE dp_dv = -pThis->R_TK / pow(v1 - pThis->b_sum, 2) +
					pThis->a_aa_sum * 2 * (v1 + pThis->b_sum) / (ff * ff);
	return dp_dv;
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_pure_phases(void)
/* ---------------------------------------------------------------------- */
{
	//LDBLE si_org;
/*
 *   Fills in data for pure_phase assemglage in unknown structure
 */

	if (use.Get_pp_assemblage_ptr() == NULL)
		return (OK);
	cxxPPassemblage * pp_assemblage_ptr = use.Get_pp_assemblage_ptr();
/*
 *   Setup unknowns
 */
	std::map<std::string, cxxPPassemblageComp>::iterator it;
	it =  pp_assemblage_ptr->Get_pp_assemblage_comps().begin();
	for ( ; it != pp_assemblage_ptr->Get_pp_assemblage_comps().end(); it++)
	{
		cxxPPassemblageComp * comp_ptr = &(it->second);
		int j;
		class phase * phase_ptr = phase_bsearch(it->first.c_str(), &j, FALSE);
		assert(phase_ptr);
		x[count_unknowns]->type = PP;
		x[count_unknowns]->description = string_hsave(comp_ptr->Get_name().c_str());
		x[count_unknowns]->pp_assemblage_comp_name = x[count_unknowns]->description;
		x[count_unknowns]->pp_assemblage_comp_ptr = comp_ptr;
		x[count_unknowns]->moles = comp_ptr->Get_moles();
		x[count_unknowns]->phase = phase_ptr;
		x[count_unknowns]->si = comp_ptr->Get_si();
		//si_org = comp_ptr->Get_si_org();
		/* si_org is used for Peng-Robinson gas, with the fugacity
		   coefficient added later in adjust_pure_phases,
		   when rxn_x has been defined for each phase in the model */
		x[count_unknowns]->delta = comp_ptr->Get_delta();	
		x[count_unknowns]->dissolve_only = comp_ptr->Get_dissolve_only();
		if (pure_phase_unknown == NULL)
			pure_phase_unknown = x[count_unknowns];
		count_unknowns++;
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
adjust_setup_pure_phases(void)
/* ---------------------------------------------------------------------- */
{
	int i;
	class phase *phase_ptr;
	LDBLE si_org, p, t;
/*
 *   Fills in data for pure_phase assemglage in unknown structure
 */

	if (use.Get_pp_assemblage_ptr() == NULL)
		return (OK);
/*
 *   Adjust si for gases
 */
	for (i = 0; i < count_unknowns; i++)
	{
		std::vector<class phase *> phase_ptrs;
		if (x[i]->type == PP)
		{
			phase_ptr = x[i]->phase;
			phase_ptrs.push_back(phase_ptr);
			//cxxPPassemblageComp * comp_ptr = pp_assemblage_ptr->Find(x[i]->pp_assemblage_comp_name);
			cxxPPassemblageComp * comp_ptr = (cxxPPassemblageComp * ) x[i]->pp_assemblage_comp_ptr;
			si_org = comp_ptr->Get_si_org();
			if (phase_ptr->p_c > 0 && phase_ptr->t_c > 0)
			{
				if (si_org > 3.5)
					si_org = 3.5;
				p = exp(si_org * LOG_10);
				patm_x = p;
				t = use.Get_solution_ptr()->Get_tc() + 273.15;
				if (!phase_ptr->pr_in || p != phase_ptr->pr_p || t != phase_ptr->pr_tk)
				{
					calc_PR(phase_ptrs, p, t, 0);
				}
				x[i]->si = si_org + phase_ptr->pr_si_f;
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_solution(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Fills in data in unknown structure for the solution
 */
	class master *master_ptr;
	cxxSolution *solution_ptr;
	const char* cptr;
	std::string token;
	class master_isotope *master_isotope_ptr;
	class phase *phase_ptr;

	solution_ptr = use.Get_solution_ptr();
	count_unknowns = 0;

	/*
	 * Treat minor isotopes as special in initial solution calculation
	 */
	if (solution_ptr->Get_initial_data())
	{
		std::map<std::string, cxxISolutionComp >::iterator comp_it = solution_ptr->Get_initial_data()->Get_comps().begin();
		for ( ; comp_it != solution_ptr->Get_initial_data()->Get_comps().end(); comp_it++)
		{
			master_ptr = master_bsearch(comp_it->first.c_str());
			if ((master_ptr != NULL)
				&& (master_ptr->minor_isotope == TRUE)
				&& (initial_solution_isotopes == FALSE))
			{
				master_isotope_ptr = master_isotope_search(comp_it->first.c_str());
				if (master_isotope_ptr != NULL)
				{
					master_isotope_ptr->ratio = comp_it->second.Get_input_conc();
				}
			}
		}
	}
	cxxNameDouble::iterator it = solution_ptr->Get_totals().begin();
	for ( ; it != solution_ptr->Get_totals().end(); it++)
	{
		cxxISolutionComp *comp_ptr = NULL;
		if (solution_ptr->Get_initial_data())
		{
			std::map<std::string, cxxISolutionComp >::iterator comp_it;
			comp_it = solution_ptr->Get_initial_data()->Get_comps().find(it->first.c_str());
			comp_ptr = &(comp_it->second);
		}
		cptr = it->first.c_str();
		copy_token(token, &cptr);
		master_ptr = master_bsearch(token.c_str());
/*
 *   Check that total not <= zero
 */
		if (it->second <= 0.0)
		{
			if (strcmp(token.c_str(), "H(1)") != 0 && strcmp(token.c_str(), "E") != 0)
			{
				continue;
			}
		}
/*
 *   Find master species
 */
		master_ptr = master_bsearch(token.c_str());
		if (master_ptr == NULL)
		{
			error_string = sformatf(
					"Master species not in database for %s, skipping element.",
					it->first.c_str());
			warning_msg(error_string);
			continue;
		}
		if (master_ptr->type != AQ)
		{
			/*                      solution_ptr->totals[i].skip = TRUE; */
			error_string = sformatf(
					"Only aqueous concentrations are allowed in solution data, ignoring %s.",
					it->first.c_str());
			warning_msg(error_string);
			continue;
		}
/*
 *   Store list of master species pointers, set master[i].in and master[i].rxn for list
 */
		x[count_unknowns]->master = get_list_master_ptrs(cptr, master_ptr);
		if (comp_ptr)
		{
			setup_master_rxn(x[count_unknowns]->master, comp_ptr->Get_pe_reaction());
		}
		else
		{
			setup_master_rxn(x[count_unknowns]->master, "pe");
		}
		
/*
 *   Set default unknown data
 */
		x[count_unknowns]->type = MB;
		x[count_unknowns]->description = string_hsave(it->first.c_str());
		for (size_t j = 0; j < x[count_unknowns]->master.size(); j++)
		{
			x[count_unknowns]->master[j]->unknown = x[count_unknowns];
		}
		x[count_unknowns]->moles = it->second;
/*
 *   Set pointers
 */
		cptr = it->first.c_str();
		copy_token(token, &cptr);
		Utilities::str_tolower(token);
		if (strstr(token.c_str(), "alk") != NULL)
		{
			if (alkalinity_unknown == NULL)
			{
				x[count_unknowns]->type = ALK;
				alkalinity_unknown = x[count_unknowns];
			}
			else
			{
				error_msg("Alkalinity entered more than once.", CONTINUE);
				input_error++;
			}
		}
		else if (strcmp(token.c_str(), "c") == 0 || strcmp(token.c_str(), "c(4)") == 0)
		{
			if (carbon_unknown == NULL)
			{
				carbon_unknown = x[count_unknowns];
			}
			else
			{
				error_msg("Carbon entered more than once.", CONTINUE);
				input_error++;
			}
		}
		else if (strcmp(token.c_str(), "h(1)") == 0)
		{
			if (ph_unknown == NULL)
			{
				ph_unknown = x[count_unknowns];
			}
			else
			{
				error_msg("pH entered more than once.", CONTINUE);
				input_error++;
			}
		}
		else if (strcmp(token.c_str(), "e") == 0)
		{
			if (pe_unknown == NULL)
			{
				pe_unknown = x[count_unknowns];
			}
			else
			{
				error_msg("pe entered more than once.", CONTINUE);
				input_error++;
			}
		}
/*
 *   Charge balance unknown
 */
		if (comp_ptr && comp_ptr->Get_equation_name().size() > 0)
		{
			cptr = comp_ptr->Get_equation_name().c_str();
			copy_token(token, &cptr);
			Utilities::str_tolower(token);
			if (strstr(token.c_str(), "charge") != NULL)
			{
				if (charge_balance_unknown == NULL)
				{
					charge_balance_unknown = x[count_unknowns];
					x[count_unknowns]->type = CB;
					if (charge_balance_unknown == ph_unknown)
					{
						x[count_unknowns]->moles = solution_ptr->Get_cb();
					}
				}
				else
				{
					error_msg("Charge balance specified for more"
							  " than one species.", CONTINUE);
					input_error++;
				}
			}
			else
			{
/*
 *   Solution phase boundaries
 */
				int l;
				phase_ptr = phase_bsearch(comp_ptr->Get_equation_name().c_str(), &l, FALSE);
				if (phase_ptr == NULL)
				{
					error_string = sformatf( "Expected a mineral name, %s.",
							comp_ptr->Get_equation_name().c_str());
					error_msg(error_string, CONTINUE);
					input_error++;
				}
				x[count_unknowns]->type = SOLUTION_PHASE_BOUNDARY;
				x[count_unknowns]->phase = phase_ptr;
				x[count_unknowns]->si = comp_ptr->Get_phase_si();
				/* For Peng-Robinson gas, the fugacity
				   coefficient is added later in adjust_setup_solution,
				   when rxn_x has been defined for each phase in the model */
				if (solution_phase_boundary_unknown == NULL)
				{
					solution_phase_boundary_unknown = x[count_unknowns];
				}
			}
		}
		count_unknowns++;
	}
/*
 *   Set mb_unknown
 */
	if (count_unknowns > 0)
		mb_unknown = x[0];
/*
 *   Special for alkalinity
 */
	if (alkalinity_unknown != NULL)
	{
		if (carbon_unknown != NULL)
		{
/*
 *   pH adjusted to obtain given alkalinity
 */
			if (ph_unknown == NULL)
			{
				output_msg(sformatf("\npH will be adjusted to obtain desired alkalinity.\n\n"));
				ph_unknown = alkalinity_unknown;
				master_ptr = master_bsearch("H(1)");
				alkalinity_unknown->master[0] = master_ptr;
				master_ptr->in = TRUE;
				master_ptr->unknown = ph_unknown;
				ph_unknown->master[0] = master_ptr;
				ph_unknown->description = string_hsave("H(1)");
			}
			else
			{
				error_msg("pH adjustment is needed for alkalinity but"
						  " charge balance or a phase boundary was also specified.",
						  CONTINUE);
				input_error++;
			}
/*
 *   Carbonate ion adjusted to obtain given alkalintiy
 */
		}
		else
		{
			if (alkalinity_unknown->master[0]->s->secondary != NULL)
			{
				alkalinity_unknown->master[0]->s->secondary->in = TRUE;
				alkalinity_unknown->master[0]->s->secondary->unknown =
					alkalinity_unknown;
			}
			else
			{
				error_msg
					("Error in definition of Alkalinity in SOLUTION_MASTER_SPECIES and SOLUTION_SPECIES.\n\tAlkalinity master species should be same as master species for C(4).",
					 CONTINUE);
				input_error++;
			}
		}
	}
	//if (pitzer_model == FALSE && sit_model == FALSE)
	{
		/*
		 *   Ionic strength
		 */
		mu_unknown = x[count_unknowns];
		x[count_unknowns]->description = string_hsave("Mu");
		x[count_unknowns]->type = MU;
		x[count_unknowns]->number = count_unknowns;
		x[count_unknowns]->moles = 0.0;
		count_unknowns++;
	}
	/*
	 *   Activity of water
	 */
	ah2o_unknown = x[count_unknowns];
	ah2o_unknown->description = string_hsave("A(H2O)");
	ah2o_unknown->type = AH2O;
	ah2o_unknown->number = count_unknowns;
	ah2o_unknown->master.push_back(master_bsearch("O"));
	ah2o_unknown->master[0]->unknown = ah2o_unknown;
	ah2o_unknown->moles = 0.0;
	count_unknowns++;

	if (state >= REACTION)
	{
/*

 *   Reaction: pH for charge balance
 */
		ph_unknown = x[count_unknowns];
		ph_unknown->description = string_hsave("pH");
		ph_unknown->type = CB;
		ph_unknown->moles = solution_ptr->Get_cb();
		ph_unknown->number = count_unknowns;
		ph_unknown->master.push_back(s_hplus->primary);
		ph_unknown->master[0]->unknown = ph_unknown;
		charge_balance_unknown = ph_unknown;
		count_unknowns++;
/*
 *   Reaction: pe for total hydrogen
 */
		pe_unknown = x[count_unknowns];
		mass_hydrogen_unknown = x[count_unknowns];
		mass_hydrogen_unknown->description = string_hsave("Hydrogen");
		mass_hydrogen_unknown->type = MH;
#ifdef COMBINE
		mass_hydrogen_unknown->moles =
			solution_ptr->Get_total_h() - 2 * solution_ptr->Get_total_o();
#else
		mass_hydrogen_unknown->moles = solution_ptr->total_h;
#endif
		mass_hydrogen_unknown->number = count_unknowns;
		mass_hydrogen_unknown->master.push_back(s_eminus->primary);
		mass_hydrogen_unknown->master[0]->unknown = mass_hydrogen_unknown;
		count_unknowns++;
/*
 *   Reaction H2O for total oxygen
 */
		mass_oxygen_unknown = x[count_unknowns];
		mass_oxygen_unknown->description = string_hsave("Oxygen");
		mass_oxygen_unknown->type = MH2O;
		mass_oxygen_unknown->moles = solution_ptr->Get_total_o();
		mass_oxygen_unknown->number = count_unknowns;
		mass_oxygen_unknown->master.push_back(s_h2o->primary);
		count_unknowns++;
	}
/*
 *   Validity tests
 */
	if ((ph_unknown != NULL) &&
		(ph_unknown == charge_balance_unknown)
		&& (alkalinity_unknown != NULL))
	{
		error_msg("pH adustment cannot attain charge balance"
				  " when alkalinity is fixed.", CONTINUE);
		input_error++;
	}
	if ((alkalinity_unknown != NULL) &&
		(alkalinity_unknown->type == CB ||
		 alkalinity_unknown->type == SOLUTION_PHASE_BOUNDARY))
	{
		error_msg("Alkalinity cannot be used with charge balance"
				  " or solution phase boundary constraints.", CONTINUE);
		input_error++;
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
adjust_setup_solution(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Fills in data in unknown structure for the solution
 */
	int i;
	class phase *phase_ptr;
	LDBLE p, t;

	for (i = 0; i < count_unknowns; i++)
	{
		std::vector<class phase *> phase_ptrs;
		if (x[i]->type == SOLUTION_PHASE_BOUNDARY)
		{
			x[count_unknowns]->type = SOLUTION_PHASE_BOUNDARY;
			phase_ptr = x[i]->phase;
			phase_ptrs.push_back(phase_ptr);
			if (phase_ptr->p_c > 0 && phase_ptr->t_c > 0)
			{
				if (x[i]->si > 3.5)
					x[i]->si = 3.5;
				p = exp(x[i]->si * LOG_10);
				patm_x = p;
				t = use.Get_solution_ptr()->Get_tc() + 273.15;
				if (!phase_ptr->pr_in || p != phase_ptr->pr_p || t != phase_ptr->pr_tk)
				{
					calc_PR(phase_ptrs, p, t, 0);
				}
				x[i]->si += phase_ptr->pr_si_f;
			}
		}
	}
	return (OK);

}

/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_unknowns(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Counts unknowns and allocates space for unknown structures
 */
	int i;
	cxxSolution *solution_ptr;

	solution_ptr = use.Get_solution_ptr();
/*
 *   Calculate maximum number of unknowns
 */
	max_unknowns = 0;
/*
 *   Count mass balance in solution
 */
	if (solution_ptr->Get_initial_data())
	{
		max_unknowns += (int) solution_ptr->Get_initial_data()->Get_comps().size();
	}
	else
	{
		max_unknowns += (int) solution_ptr->Get_totals().size();
	}
/*
 *   Add 5 for ionic strength, activity of water, charge balance, total H, total O
 */
	max_unknowns += 5;
/*
 *   Count pure phases
 */
	if (use.Get_pp_assemblage_ptr() != NULL)
	{
		cxxPPassemblage * pp_assemblage_ptr = use.Get_pp_assemblage_ptr();
		max_unknowns += (int) pp_assemblage_ptr->Get_pp_assemblage_comps().size();
	}
/*
 *   Count exchange
 */
	if (use.Get_exchange_ptr() != NULL)
	{
		cxxExchange *exchange_ptr = use.Get_exchange_ptr();
		for (size_t j = 0; j < exchange_ptr->Get_exchange_comps().size(); j++)
		{
			cxxExchComp & comp_ref = exchange_ptr->Get_exchange_comps()[j];
			cxxNameDouble nd(comp_ref.Get_totals());
			cxxNameDouble::iterator it = nd.begin();
			for ( ; it != nd.end(); it++)
			{
				element * elt_ptr = element_store(it->first.c_str());
				if (elt_ptr == NULL || elt_ptr->master == NULL)
				{
					error_string = sformatf(
							"Master species missing for element %s",
							it->first.c_str());
					error_msg(error_string, STOP);
				}
				if (elt_ptr->master->type == EX)
				{
					max_unknowns++;
				}
			}
		}
	}
/*
 *   Count surfaces
 */
	if (use.Get_surface_ptr() != NULL)
	{
		if (use.Get_surface_ptr()->Get_type() != cxxSurface::CD_MUSIC)
		{
			max_unknowns +=	(int) use.Get_surface_ptr()->Get_surface_comps().size() + 
				(int) use.Get_surface_ptr()->Get_surface_charges().size();
		}
		else
		{
			max_unknowns +=	(int)(use.Get_surface_ptr()->Get_surface_comps().size() +
				4 * use.Get_surface_ptr()->Get_surface_charges().size());
		}
	}
/*
 *   Count gas components
 */
	if (use.Get_gas_phase_ptr() != NULL)
	{
		cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
		if (gas_phase_ptr->Get_type() == cxxGasPhase::GP_VOLUME && 
			(gas_phase_ptr->Get_pr_in() || force_numerical_fixed_volume) && numerical_fixed_volume)
		{
			max_unknowns += (int) gas_phase_ptr->Get_gas_comps().size();
		}
		else
		{
			max_unknowns++;
		}
	}
/*
 *   Count solid solutions
 */
	if (use.Get_ss_assemblage_ptr() != NULL)
	{
		std::vector<cxxSS *> ss_ptrs = use.Get_ss_assemblage_ptr()->Vectorize();
		for (size_t i = 0; i < ss_ptrs.size(); i++)
		{
			max_unknowns += (int) ss_ptrs[i]->Get_ss_comps().size();
		}
	}
/*
 *   Pitzer/Sit
 */
	max_unknowns++;
	if (pitzer_model == TRUE || sit_model == TRUE)
	{
		max_unknowns += (int)s.size();
	}

/*
 *   Allocate space for pointer array and structures
 */
	x.resize(max_unknowns);
	for (i = 0; i < max_unknowns; i++)
	{
		x[i] = (class unknown *) unknown_alloc();
		x[i]->number = i;
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
store_dn(int k, LDBLE * source, int row, LDBLE coef_in, LDBLE * gamma_source)
/* ---------------------------------------------------------------------- */
{
/*
 *   Stores the terms for d moles of species k in solution into row, multiplied
 *   by coef_in
 */
	size_t col;
	LDBLE coef;
	class rxn_token *rxn_ptr;
	class master *master_ptr;

	if (equal(coef_in, 0.0, TOL) == TRUE)
	{
		return (OK);
	}
/*   Gamma term for d molality of species */
/*   Note dg includes molality as a factor */

	row = row * ((int)count_unknowns + 1);
	if (s[k]->type != SURF && s[k] != s_h2o)
	{
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
					   "Activity coefficient", (double) (-1.0 * coef_in),
					   row / (count_unknowns + 1), mu_unknown->number));
		}
		/* mu term */
		if (gamma_source != NULL)
		{
			store_jacob(gamma_source, &my_array[(size_t)row + (size_t)mu_unknown->number],
						-1.0 * coef_in);
		}
	}
/*
 *   Mass of water factor
 */
	if (mass_oxygen_unknown != NULL && s[k]->type != EX && s[k]->type != SURF)
	{
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
					   mass_oxygen_unknown->master[0]->s->name,
					   (double) coef_in, row / (count_unknowns + 1),
					   mass_oxygen_unknown->number));
		}
		store_jacob(source, &(my_array[(size_t)row + (size_t)mass_oxygen_unknown->number]),
					coef_in);
	}
	if (s[k] == s_h2o)
		return (OK);
	for (rxn_ptr = &s[k]->rxn_x.token[0] + 1; rxn_ptr->s != NULL; rxn_ptr++)
	{
		if (rxn_ptr->s->secondary != NULL
			&& rxn_ptr->s->secondary->in == TRUE)
		{
			master_ptr = rxn_ptr->s->secondary;
		}
		else
		{
			master_ptr = rxn_ptr->s->primary;
		}
		//if (debug_prep == TRUE)
		//{
		//	output_msg(sformatf( "\t\t%s\n", master_ptr->s->name));
		//}
		if (master_ptr == NULL ||master_ptr->unknown == NULL)
			continue;
		col = master_ptr->unknown->number;
		coef = coef_in * rxn_ptr->coef;
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\t\t%-24s%10.3f\t%d\t%d",
					   master_ptr->s->name, (double) coef,
					   row / (count_unknowns + 1), col));
		}
		store_jacob(source, &(my_array[(size_t)row + (size_t)col]), coef);
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
store_jacob(LDBLE * source, LDBLE * target, LDBLE coef)
/* ---------------------------------------------------------------------- */
{
/*
 *   Adds a new item to either sum_jacob1 or sum_jacob2
 *   If coef is 1.0, adds to sum_jacob1, which does not require a multiply
 *   Otherwise, adds to sum_jacob2, which allows multiply by coef
 */
	if (equal(coef, 1.0, TOL) == TRUE)
	{
		size_t count_sum_jacob1 = sum_jacob1.size();
		sum_jacob1.resize(count_sum_jacob1 + 1);
		if (debug_prep == TRUE)
		{
			output_msg(sformatf( "\tjacob1 %d\n", (int)count_sum_jacob1));
		}
		sum_jacob1[count_sum_jacob1].source = source;
		sum_jacob1[count_sum_jacob1].target = target;
	}
	else
	{
		size_t count_sum_jacob2 = sum_jacob2.size();
		sum_jacob2.resize(count_sum_jacob2 + 1);
		if (debug_prep == TRUE)
		{
			output_msg(sformatf("\tjacob2 %d\n", count_sum_jacob2));
		}
		sum_jacob2[count_sum_jacob2].source = source;
		sum_jacob2[count_sum_jacob2].target = target;
		sum_jacob2[count_sum_jacob2].coef = coef;
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
store_jacob0(int row, int column, LDBLE coef)
/* ---------------------------------------------------------------------- */
{
/*
 *   Stores in list a constant coef which will be added into jacobian array
 */
	size_t count_sum_jacob0 = sum_jacob0.size();
	sum_jacob0.resize(count_sum_jacob0 + 1);
	sum_jacob0[count_sum_jacob0].target =
		&(my_array[(size_t)row * (count_unknowns + 1) + (size_t)column]);
	sum_jacob0[count_sum_jacob0].coef = coef;
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
store_mb(LDBLE * source, LDBLE * target, LDBLE coef)
/* ---------------------------------------------------------------------- */
{
/*
 *   Adds item to list sum_mb1 or sum_mb2
 *   If coef is 1.0, adds to sum_mb1, which does not require a multiply
 *   else, adds to sum_mb2, which will multiply by coef
 */
	if (equal(coef, 1.0, TOL) == TRUE)
	{
		size_t count_sum_mb1 = sum_mb1.size();
		sum_mb1.resize(count_sum_mb1 + 1);
		sum_mb1[count_sum_mb1].source = source;
		sum_mb1[count_sum_mb1].target = target;
	}
	else
	{
		size_t count_sum_mb2 = sum_mb2.size();
		sum_mb2.resize(count_sum_mb2 + 1);
		sum_mb2[count_sum_mb2].source = source;
		sum_mb2[count_sum_mb2].coef = coef;
		sum_mb2[count_sum_mb2].target = target;
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
store_sum_deltas(LDBLE * source, LDBLE * target, LDBLE coef)
/* ---------------------------------------------------------------------- */
{
/*
 *   List sum_delta is summed to determine the change in the mass of 
 *   each element due to mass transfers of minerals, changes show up
 *   in x[i]->delta. These may be multiplied by a factor under some
 *   situations where the entire calculated step is not taken
 */
	size_t count_sum_delta = sum_delta.size();
	sum_delta.resize(count_sum_delta + 1);
	sum_delta[count_sum_delta].source = source;
	sum_delta[count_sum_delta].target = target;
	sum_delta[count_sum_delta].coef = coef;
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
switch_bases(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Check if activity of first master species is predominant among activities of
 *   secondary master species included in mass balance.
 */
	int i;
	int first;
	int return_value;
	LDBLE la, la1;
	class master *master_ptr;

	return_value = FALSE;
	for (i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type != MB)
			continue;
		if (x[i]->type == PITZER_GAMMA)
			break;
		first = 0;
		la = x[i]->master[0]->s->la;
		for (size_t j = 1; j < x[i]->master.size(); j++)
		{
			la1 = x[i]->master[j]->s->lm + x[i]->master[j]->s->lg;
			if (first == 0 && la1 > la + 10.)
			{
				la = la1;
				first = (int)j;
			}
			else if (first != 0 && la1 > la)
			{
				la = la1;
				first = (int)j;
			}
		}
		if (first != 0)
		{
			master_ptr = x[i]->master[0];
			x[i]->master[0] = x[i]->master[first];
			x[i]->master[0]->in = TRUE;
			x[i]->master[first] = master_ptr;
			x[i]->master[first]->in = REWRITE;
/*
			fprintf(stderr, "Switching bases to %s.\tIteration %d\n",
					   x[i]->master[0]->s->name, iterations, la, x[i]->master[0]->s->la);
 */
			x[i]->master[0]->s->la = la;
			x[i]->la = la;
			log_msg(sformatf( "Switching bases to %s.\tIteration %d\n",
					   x[i]->master[0]->s->name, iterations));
			return_value = TRUE;
		}
	}
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
tidy_redox(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Write pe redox reactions (rxn in struct pe_data) in terms of master species
 *   defined in analytical data
 *   
 */
	std::string token, tok1, tok2;
	class master *master_ptr1, *master_ptr2;
/*
 *   Keep valences of oxygen and hydrogen in model, if not already in
 */
	for (int i = 0; i < (int)master.size(); i++)
	{
		if (master[i]->primary == TRUE &&
			(master[i]->s == s_hplus || master[i]->s == s_h2o))
		{
			int j = i + 1;
			while (j < (int)master.size() && master[j]->elt->primary == master[i])
			{
				if (master[j]->in == FALSE && master[j]->s != master[i]->s)
				{
					master[j]->in = REWRITE;
					master[j]->pe_rxn = master[i]->pe_rxn;
				}
				j++;
			}
		}
	}
/*
 *   Writes equations for e- for each redox couple used in solution n
 */
	std::map < std::string, CReaction >::iterator it;
	for (it = pe_x.begin(); it != pe_x.end(); it++)
	{
		if (strcmp_nocase(it->first.c_str(), "pe") == 0)
		{
			CReaction temp_rxn(s_eminus->rxn);
			it->second = temp_rxn;
		}
		else
		{
			token = it->first;
			replace("/", " ", token);
			std::string::iterator b = token.begin();
			std::string::iterator e = token.end();

/*
 *   Get redox states and elements from redox couple
 */
			CParser::copy_token(tok1, b, e);
			CParser::copy_token(tok2, b, e);
/*
 *   Find master species
 */
			master_ptr1 = master_bsearch(tok1.c_str());
			master_ptr2 = master_bsearch(tok2.c_str());
			if (master_ptr1 != NULL && master_ptr2 != NULL)
			{
				rewrite_master_to_secondary(master_ptr1, master_ptr2);
/*
 *   Rewrite equation to e-
 */
				trxn_swap("e-");
			}
			else
			{
				error_string = sformatf(
						"Cannot find master species for redox couple, %s.",
						it->first.c_str());
				error_msg(error_string, STOP);
			}
			if (inout() == FALSE)
			{
				error_string = sformatf(
						"Analytical data missing for redox couple, %s\n\t Using pe instead.",
						it->first.c_str());
				warning_msg(error_string);
				CReaction temp_rxn(s_eminus->rxn);
				it->second = temp_rxn;
			}
			else
			{
				CReaction rxn(count_trxn + 1);
				trxn_copy(rxn);
				CReaction temp_rxn(rxn);
				it->second = temp_rxn;
			}
		}
	}
/*
 *   Rewrite equations to master species that are "in" the model
 */
	for (it = pe_x.begin(); it != pe_x.end(); it++)
	{
		count_trxn = 0;
		trxn_add(it->second, 1.0, FALSE);
		if (write_mass_action_eqn_x(CONTINUE) == FALSE)
		{
			error_string = sformatf( "Could not rewrite redox "
					"couple equation for %s\n\t Possibly missing data for one "
					"of the redox states.", it->first.c_str());
			warning_msg(error_string);
			error_string = sformatf( "Using pe instead of %s.",
					it->first.c_str());
			warning_msg(error_string);
			CReaction temp_rxn(s_eminus->rxn);
			it->second = temp_rxn;
		}
		else
		{
			CReaction rxn(count_trxn + 1);
			trxn_copy(rxn);
			CReaction temp_rxn(rxn);
			it->second = temp_rxn;
		}
	}

	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
write_mb_eqn_x(void)
/* ---------------------------------------------------------------------- */
{
	int count, repeat;
	int i;
	size_t count_rxn_orig;
	class master *master_ptr;
/*
 *   Rewrite any secondary master species flagged REWRITE
 *   Don`t add in any pe reactions
 */
	count = 0;
	repeat = TRUE;
	while (repeat == TRUE)
	{
		count++;
		if (count > MAX_ADD_EQUATIONS)
		{
			std::string name;
			name = "Unknown";
			if (trxn.token[0].s != NULL)
			{
				name = trxn.token[0].s->name;
			}
			error_string = sformatf( "Could not reduce equation "
					"to primary and secondary species that are "
					"in the model.  Species: %s.", name.c_str());
			error_msg(error_string, CONTINUE);
			return (ERROR);
		}
		repeat = FALSE;
		count_rxn_orig = count_trxn;
		for (i = 1; i < count_rxn_orig; i++)
		{
			if (trxn.token[i].s->secondary == NULL)
				continue;
			if (trxn.token[i].s->secondary->in == REWRITE)
			{
				repeat = TRUE;
				trxn_add(trxn.token[i].s->secondary->rxn_secondary,
						 trxn.token[i].coef, false);
			}
		}
		trxn_combine();
	}
/*
 *  
 */
	count_elts = 0;
	paren_count = 0;
	for (size_t i = 1; i < count_trxn; i++)
	{
		size_t j = count_elts;
		const char* cptr = trxn.token[i].s->name;
		get_elts_in_species(&cptr, trxn.token[i].coef);
		for (size_t k = j; k < count_elts; k++)
		{
			if (trxn.token[i].s->secondary != NULL)
			{
				master_ptr = trxn.token[i].s->secondary->elt->primary;
			}
			else
			{
				master_ptr = trxn.token[i].s->primary;
			}
			if (elt_list[k].elt == master_ptr->elt)
			{
				elt_list[k].coef = 0.0;
				break;
			}
		}
		if (trxn.token[i].s->secondary == NULL)
		{
			const char* cptr = trxn.token[i].s->primary->elt->name;
			get_secondary_in_species(&cptr, trxn.token[i].coef);
		}
		else
		{
			cptr = trxn.token[i].s->secondary->elt->name;
			get_secondary_in_species(&cptr, trxn.token[i].coef);
		}
	}
	elt_list_combine();
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
write_mb_for_species_list(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Sets up data to add to species_list
 *   Original secondary redox states are retained
 */
	int i;
/*
 *   Start with secondary reaction
 */
	count_trxn = 0;
	trxn_add(s[n]->rxn_s, 1.0, false);
/*
 *   Copy to elt_list
 */
	count_elts = 0;
	paren_count = 0;
	for (i = 1; i < count_trxn; i++)
	{
		if (trxn.token[i].s->secondary == NULL)
		{
			const char* cptr = trxn.token[i].s->primary->elt->name;
			get_secondary_in_species(&cptr, trxn.token[i].coef);
		}
		else
		{
			const char* cptr = trxn.token[i].s->secondary->elt->name;
			if (get_secondary_in_species(&cptr, trxn.token[i].coef) == ERROR)
			{
				input_error++;
				error_string = sformatf( "Error parsing %s.", trxn.token[i].s->secondary->elt->name);
				error_msg(error_string, CONTINUE);
			}
		}
	}
	for (i = 0; i < count_elts; i++)
	{
		if (strcmp(elt_list[i].elt->name, "O(-2)") == 0)
		{
			if (count_elts >= (int)elt_list.size())
			{
				elt_list.resize(count_elts + 1);
			}
			elt_list[count_elts].elt = element_h_one;
			elt_list[count_elts].coef = elt_list[i].coef * 2;
			count_elts++;
		}
	}
	elt_list_combine();
	s[n]->next_sys_total.clear();
	s[n]->next_sys_total = elt_list_vsave();
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
write_phase_sys_total(int n)
/* ---------------------------------------------------------------------- */
{
/*
 *   Sets up data to add to species_list
 *   Original secondary redox states are retained
 */
	int i;
/*
 *   Start with secondary reaction
 */
	count_trxn = 0;
	trxn_add_phase(phases[n]->rxn_s, 1.0, false);
/*
 *   Copy to elt_list
 */
	count_elts = 0;
	paren_count = 0;
	for (i = 1; i < count_trxn; i++)
	{
		if (trxn.token[i].s->secondary == NULL)
		{
			const char* cptr = trxn.token[i].s->primary->elt->name;
			get_secondary_in_species(&cptr, trxn.token[i].coef);
		}
		else
		{
			const char* cptr = trxn.token[i].s->secondary->elt->name;
			get_secondary_in_species(&cptr, trxn.token[i].coef);
		}
	}
	for (i = 0; i < count_elts; i++)
	{
		if (strcmp(elt_list[i].elt->name, "O(-2)") == 0)
		{
			if (count_elts >= (int)elt_list.size())
			{
				elt_list.resize(count_elts + 1);
			}
			elt_list[count_elts].elt = element_h_one;
			elt_list[count_elts].coef = elt_list[i].coef * 2;
			count_elts++;
		}
	}
	elt_list_combine();
	phases[n]->next_sys_total.clear();
	phases[n]->next_sys_total = elt_list_vsave();
	return (OK);
}
/* ---------------------------------------------------------------------- */
LDBLE Phreeqc::
calc_lk_phase(phase *p_ptr, LDBLE TK, LDBLE pa)
/* ---------------------------------------------------------------------- */
{
/* 
 * calculate log_k for a single phase, correct for pressure
 * see calc_vm (below) for details.
 */

	CReaction *r_ptr = (p_ptr->rxn_x.size() ? &p_ptr->rxn_x :\
		(p_ptr->rxn_s.size() ? &p_ptr->rxn_s : NULL));
	if (!r_ptr)
		return 0.0;
	if (!r_ptr->logk[vm0]) // in case Vm of the phase is 0...
		return k_calc(r_ptr->logk, TK, pa * PASCAL_PER_ATM);

	LDBLE tc = TK - 273.15;
	LDBLE pb_s = 2600. + pa * 1.01325, TK_s = tc + 45.15, sqrt_mu = sqrt(mu_x); 
	LDBLE d_v = 0.0;
	species * s_ptr;

	for (size_t i = 0; r_ptr->token[i].name; i++)
	{
		if (!r_ptr->token[i].s)
			continue;
		s_ptr = r_ptr->token[i].s;
		//if (!strcmp(s_ptr->name, "H+"))
		if (s_ptr == s_hplus)
			continue;
		//if (!strcmp(s_ptr->name, "e-"))
		if (s_ptr == s_eminus)
			continue;
		//if (!strcmp(s_ptr->name, "H2O"))
		if (s_ptr == s_h2o)
		{
			d_v += r_ptr->token[i].coef * 18.016 / calc_rho_0(tc, pa);
			continue;
		}
		else if (s_ptr->logk[vma1])
		{
		/* supcrt volume at I = 0... */
			d_v += r_ptr->token[i].coef *
				(s_ptr->logk[vma1] + s_ptr->logk[vma2] / pb_s +
				(s_ptr->logk[vma3] + s_ptr->logk[vma4] / pb_s) / TK_s -
				s_ptr->logk[wref] * QBrn);
			//if (dgdP && s_ptr->z)
			//{
			//	LDBLE re = s_ptr->z * s_ptr->z / (s_ptr->logk[wref] / 1.66027e5 + s_ptr->z / 3.082);
			//	LDBLE Z3 = fabs(pow(s_ptr->z, 3)) / re / re - s_ptr->z / 9.498724;
			//	d_v += r_ptr->token[i].coef * ZBrn * 1.66027e5 * Z3 * dgdP;
			//}
			if (s_ptr->z)
				{
			/* the ionic strength term * I^0.5... */
				if (s_ptr->logk[b_Av] < 1e-5)
					d_v += s_ptr->z * s_ptr->z * 0.5 * DH_Av * sqrt_mu;
				else
				{
					/* limit the Debye-Hueckel slope by b... */
					d_v += s_ptr->z * s_ptr->z * 0.5 * DH_Av *
						sqrt_mu / (1 + s_ptr->logk[b_Av] * DH_B * sqrt_mu);
				}
				/* plus the volume terms * I... */
				if (s_ptr->logk[vmi1] != 0.0 || s_ptr->logk[vmi2] != 0.0 || s_ptr->logk[vmi3] != 0.0)
				{
					LDBLE bi = s_ptr->logk[vmi1] + s_ptr->logk[vmi2] / TK_s + s_ptr->logk[vmi3] * TK_s;
					if (s_ptr->logk[vmi4] == 1.0)
						d_v += bi * mu_x;
					else
						d_v +=  bi * pow(mu_x, s_ptr->logk[vmi4]);
				}
			}
		}
		//else if (s_x[i]->millero[0])
		else if (s_ptr->millero[0])
		{
		/* Millero volume at I = 0... */
			d_v += s_ptr->millero[0] + tc * (s_ptr->millero[1] + tc * s_ptr->millero[2]);
			if (s_ptr->z)
			{
			/* the ionic strength terms... */
				d_v += s_ptr->z * s_ptr->z * 0.5 * DH_Av * sqrt_mu +
					(s_ptr->millero[3] + tc * (s_ptr->millero[4] + tc * s_ptr->millero[5])) * mu_x;
			}
		}
		else
			continue;
	}
	d_v -= p_ptr->logk[vm0];
	r_ptr->logk[delta_v] = d_v;
	if (r_ptr->token[0].name && !strcmp(r_ptr->token[0].name, "H2O(g)"))
		r_ptr->logk[delta_v] = 0.0;

	return k_calc(r_ptr->logk, TK, pa * PASCAL_PER_ATM);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
calc_vm(LDBLE tc, LDBLE pa)
/* ---------------------------------------------------------------------- */
{
/*
 *  Calculate molar volumes for aqueous species with a Redlich type eqn:
    Vm = Vm0(tc) + (Av / 2) * z^2 * I^0.5 + coef(tc) * I^(b4).
 *    Vm0(tc) is calc'd using supcrt parms, or from millero[0] + millero[1] * tc + millero[2] * tc^2
 *    for Av * z^2 * I^0.5, see Redlich and Meyer, Chem. Rev. 64, 221.
          Av is in (cm3/mol)(mol/kg)^-0.5, = DH_Av.
		  If b_Av != 0, the extended DH formula is used: I^0.5 /(1 + b_Av * DH_B * I^0.5).
		  DH_Av and DH_B are from calc_dielectrics(tc, pa).
 *	  coef(tc) = logk[vmi1] + logk[vmi2] / (TK - 228) + logk[vmi3] * (TK - 228).
 *    b4 = logk[vmi4], or
 *	  coef(tc) = millero[3] + millero[4] * tc + millero[5] * tc^2
 */
	if (llnl_temp.size() > 0) return OK;
	LDBLE pb_s = 2600. + pa * 1.01325, TK_s = tc + 45.15, sqrt_mu = sqrt(mu_x); 
	for (int i = 0; i < (int)this->s_x.size(); i++)
	{
		//if (!strcmp(s_x[i]->name, "H2O"))
		if (s_x[i] == s_h2o)
		{
			s_x[i]->logk[vm_tc] = 18.016 / rho_0;
			continue;
		}
		if (s_x[i]->logk[vma1])
		{
		/* supcrt volume at I = 0... */
			s_x[i]->rxn_x.logk[vm_tc] = s_x[i]->logk[vma1] + s_x[i]->logk[vma2] / pb_s +
				(s_x[i]->logk[vma3] + s_x[i]->logk[vma4] / pb_s) / TK_s -
				s_x[i]->logk[wref] * QBrn;
			/* A (small) correction by Shock et al., 1992, for 155 < tc < 255, P_sat < P < 1e3.
			   The vma1..a4 and wref numbers are refitted for major cations and anions on xpts,
			   probably invalidates the correction. */
			//if (dgdP && s_x[i]->z)
			//{
			//	LDBLE re = s_x[i]->z * s_x[i]->z / (s_x[i]->logk[wref] / 1.66027e5 + s_x[i]->z / 3.082);
			//	LDBLE Z3 = fabs(pow(s_x[i]->z, 3)) / re / re - s_x[i]->z / 9.498724;
			//	s_x[i]->rxn_x.logk[vm_tc] += ZBrn * 1.66027e5 * Z3 * dgdP;
			//}
			if (s_x[i]->z)
			{
			/* the ionic strength term * I^0.5... */
				if (s_x[i]->logk[b_Av] < 1e-5)
					s_x[i]->rxn_x.logk[vm_tc] += s_x[i]->z * s_x[i]->z * 0.5 * DH_Av * sqrt_mu;
				else
				{
					/* limit the Debye-Hueckel slope by b... */
					/* pitzer... */
					//s_x[i]->rxn_x.logk[vm_tc] += s_x[i]->z * s_x[i]->z * 0.5 * DH_Av *
					//	log(1 + s_x[i]->logk[b_Av] * sqrt(mu_x)) / s_x[i]->logk[b_Av];
					/* extended DH... */
					s_x[i]->rxn_x.logk[vm_tc] += s_x[i]->z * s_x[i]->z * 0.5 * DH_Av *
						sqrt_mu / (1 + s_x[i]->logk[b_Av] * DH_B * sqrt_mu);
				}
				/* plus the volume terms * I... */
				if (s_x[i]->logk[vmi1] != 0.0 || s_x[i]->logk[vmi2] != 0.0 || s_x[i]->logk[vmi3] != 0.0)
				{
					LDBLE bi = s_x[i]->logk[vmi1] + s_x[i]->logk[vmi2] / TK_s + s_x[i]->logk[vmi3] * TK_s;
					if (s_x[i]->logk[vmi4] == 1.0)
						s_x[i]->rxn_x.logk[vm_tc] += bi * mu_x;
					else
						s_x[i]->rxn_x.logk[vm_tc] += bi * pow(mu_x, s_x[i]->logk[vmi4]);
				}
			}
		}
		else if (s_x[i]->millero[0])
		{
		/* Millero volume at I = 0... */
			s_x[i]->rxn_x.logk[vm_tc] = s_x[i]->millero[0] + tc * (s_x[i]->millero[1] + tc * s_x[i]->millero[2]);
			if (s_x[i]->z)
			{
			/* the ionic strength terms... */
				s_x[i]->rxn_x.logk[vm_tc] += s_x[i]->z * s_x[i]->z * 0.5 * DH_Av * sqrt_mu +
					(s_x[i]->millero[3] + tc * (s_x[i]->millero[4] + tc * s_x[i]->millero[5])) * mu_x;
			}
		}
		else
			continue;

		/* for calculating delta_v of the reaction... */
		s_x[i]->logk[vm_tc] = s_x[i]->rxn_x.logk[vm_tc];
	}
	return OK;
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
k_temp(LDBLE tc, LDBLE pa) /* pa - pressure in atm */
/* ---------------------------------------------------------------------- */
{
/*
 *  Calculates log k's for all species and pure_phases
 */

	if (tc == current_tc && pa == current_pa && ((fabs(mu_x - current_mu) < 1e-3 * mu_x) || !mu_terms_in_logk))
		return OK;

	int i;
	LDBLE tempk = tc + 273.15;
/*
 *  Calculate log k for all aqueous species
 */
	/* calculate relative molar volumes for tc... */
	rho_0 = calc_rho_0(tc, pa);

	pa = patm_x;
	calc_dielectrics(tc, pa);

	calc_vm(tc, pa);

	mu_terms_in_logk = false;
	for (i = 0; i < (int)this->s_x.size(); i++)
	{
		//if (s_x[i]->rxn_x.logk[vm_tc])
		/* calculate delta_v for the reaction... */
			s_x[i]->rxn_x.logk[delta_v] = calc_delta_v(*&s_x[i]->rxn_x, false);
		if (tc == current_tc && s_x[i]->rxn_x.logk[delta_v] == 0)
			continue;
		mu_terms_in_logk = true;
		s_x[i]->lk = k_calc(s_x[i]->rxn_x.logk, tempk, pa * PASCAL_PER_ATM);
	}
/*
 *    Calculate log k for all pure phases
 */
	for (i = 0; i < (int)phases.size(); i++)
	{
		if (phases[i]->in == TRUE)  
		{

			phases[i]->rxn_x.logk[delta_v] = calc_delta_v(*&phases[i]->rxn_x, true) -
				phases[i]->logk[vm0];
			if (phases[i]->rxn_x.logk[delta_v])
				mu_terms_in_logk = true;
			phases[i]->lk = k_calc(phases[i]->rxn_x.logk, tempk, pa * PASCAL_PER_ATM);

		}
	}
/*
 *    Calculate miscibility gaps for solid solutions
 */
	if (use.Get_ss_assemblage_ptr() != NULL)
	{
		std::vector<cxxSS *> ss_ptrs = use.Get_ss_assemblage_ptr()->Vectorize();
		for (size_t i = 0; i < ss_ptrs.size(); i++)
		{
			if (fabs(tempk - ss_ptrs[i]->Get_tk()) > 0.01)
			{
				ss_prep(tempk, ss_ptrs[i], FALSE);
			}
		}
	}

	current_tc = tc;
	current_pa = pa;
	current_mu = mu_x;

	return (OK);
}

/* ---------------------------------------------------------------------- */
LDBLE Phreeqc::
k_calc(LDBLE * l_logk, LDBLE tempk, LDBLE presPa)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Calculates log k at specified temperature and pressure
	 *   Returns calculated log k.
	 *
	 *   delta_v is in cm3/mol.
	 */

	/* Molar energy */
	LDBLE me = tempk * R_KJ_DEG_MOL;

	/* Pressure difference */
	LDBLE delta_p = presPa - REF_PRES_PASCAL;

	/* Calculate new log k value for this temperature and pressure */
	LDBLE lk = l_logk[logK_T0] 
		- l_logk[delta_h] * (298.15 - tempk) / (LOG_10 * me * 298.15)
		+ l_logk[T_A1]
		+ l_logk[T_A2] * tempk
		+ l_logk[T_A3] / tempk
		+ l_logk[T_A4] * log10(tempk)
		+ l_logk[T_A5] / (tempk * tempk)
		+ l_logk[T_A6] * tempk * tempk;
	if (delta_p > 0)
		/* cm3 * J /mol = 1e-9 m3 * kJ /mol */
		lk -= l_logk[delta_v] * 1E-9 * delta_p / (LOG_10 * me);
	return lk;
}


/* ---------------------------------------------------------------------- */
 int Phreeqc::
save_model(void)
/* ---------------------------------------------------------------------- */
{
	int i;
/*
 *   mark master species 
 */
	for (i = 0; i < (int)master.size(); i++)
	{
		master[i]->last_model = FALSE;
		if (master[i]->total > 0)
		{
			if (master[i]->primary == TRUE)
			{
				master[i]->last_model = TRUE;
			}
			else
			{
				/* mark primary master */
				master[i]->s->secondary->elt->primary->last_model = TRUE;
			}
		}
	}
/*
 *   save list of phase pointers for gas phase
 */
	if (use.Get_gas_phase_ptr() != NULL)
	{
		cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
		last_model.gas_phase_type = gas_phase_ptr->Get_type();
		last_model.gas_phase.resize(gas_phase_ptr->Get_gas_comps().size());
		for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
		{	
			cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
			int k;
			class phase *phase_ptr = phase_bsearch(gc_ptr->Get_phase_name().c_str() , &k, FALSE);
			assert(phase_ptr);
			last_model.gas_phase[i] = phase_ptr;
		}
	}
	else
	{
		last_model.gas_phase_type = cxxGasPhase::GP_UNKNOWN;
		last_model.gas_phase.clear();
	}
/*
 *   save list of names of solid solutions
 */
	if (use.Get_ss_assemblage_ptr() != NULL)
	{
		last_model.ss_assemblage.resize(use.Get_ss_assemblage_ptr()->Get_SSs().size());
		std::vector<cxxSS *> ss_ptrs = use.Get_ss_assemblage_ptr()->Vectorize();
		for (size_t j = 0; j < ss_ptrs.size(); j++)
		{
			last_model.ss_assemblage[j] = string_hsave(ss_ptrs[j]->Get_name().c_str());
		}
	}
	else
	{
		last_model.ss_assemblage.clear();
	}
/*
 *   save list of phase pointers for pp_assemblage
 */
	if (use.Get_pp_assemblage_ptr() != NULL)
	{
		cxxPPassemblage * pp_assemblage_ptr = use.Get_pp_assemblage_ptr();
		last_model.pp_assemblage.resize(pp_assemblage_ptr->Get_pp_assemblage_comps().size());
		last_model.add_formula.resize(pp_assemblage_ptr->Get_pp_assemblage_comps().size());
		last_model.si.resize(pp_assemblage_ptr->Get_pp_assemblage_comps().size());
		std::map<std::string, cxxPPassemblageComp>::iterator it;
		it =  pp_assemblage_ptr->Get_pp_assemblage_comps().begin();
		i = 0;
		for ( ; it != pp_assemblage_ptr->Get_pp_assemblage_comps().end(); it++)
		{
			int j;
			class phase * phase_ptr = phase_bsearch(it->first.c_str(), &j, false);
			assert(phase_ptr);
			last_model.pp_assemblage[i] = phase_ptr;
			last_model.add_formula[i] = string_hsave(it->second.Get_add_formula().c_str());
			last_model.si[i] = it->second.Get_si();
			i++;
		}
	}
	else
	{
		last_model.pp_assemblage.clear();
		last_model.add_formula.clear();
		last_model.si.clear();
	}
/*
 *   save data for surface
 */
	if (use.Get_surface_ptr() != NULL)
	{
		/* comps */
		last_model.surface_comp.resize(use.Get_surface_ptr()->Get_surface_comps().size());
		for (i = 0; i < (int) use.Get_surface_ptr()->Get_surface_comps().size(); i++)
		{
			last_model.surface_comp[i] = string_hsave(use.Get_surface_ptr()->Get_surface_comps()[i].Get_formula().c_str());
		}
		/* charge */
		last_model.surface_charge.resize(use.Get_surface_ptr()->Get_surface_charges().size());
		for (i = 0; i < (int) use.Get_surface_ptr()->Get_surface_charges().size(); i++)
		{
			last_model.surface_charge[i] = string_hsave(use.Get_surface_ptr()->Get_surface_charges()[i].Get_name().c_str());
		}
		last_model.dl_type = use.Get_surface_ptr()->Get_dl_type();
		/*last_model.edl = use.Get_surface_ptr()->edl; */
		last_model.surface_type = use.Get_surface_ptr()->Get_type();
	}
	else
	{
		last_model.dl_type = cxxSurface::NO_DL;
		/*last_model.edl = -1; */
		last_model.surface_type = cxxSurface::UNKNOWN_DL;
		last_model.surface_comp.clear();
		last_model.surface_charge.clear();
	}

	current_tc = NAN;
	current_pa = NAN;
	current_mu = NAN;
	mu_terms_in_logk = true;

	last_model.numerical_fixed_volume = numerical_fixed_volume;

	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
check_same_model(void)
/* ---------------------------------------------------------------------- */
{
	int i;
/*
 *   Force new model to be built in prep
 */
	if (last_model.force_prep)
	{
		last_model.force_prep = false;
		return (FALSE);
	}
	if (state == TRANSPORT && cell_data[cell_no].same_model)
		return TRUE;
/*
 *   Check master species
 */
	for (i = 0; i < (int)master.size(); i++)
	{
/*
		output_msg(sformatf("%s\t%e\t%d\n", master[i]->elt->name,
			master[i]->total, master[i]->last_model);
 */
		if (master[i]->s == s_hplus || master[i]->s == s_h2o)
			continue;
		if (master[i]->total > MIN_TOTAL && master[i]->last_model == TRUE)
		{
			if (master[i]->s->secondary != NULL)
			{
				if (master[i]->s->secondary->unknown != NULL)
					continue;
			}
			else
			{
				if (master[i]->unknown != NULL)
					continue;
			}
		}
		if (master[i]->total <= MIN_TOTAL && master[i]->last_model == FALSE)
			continue;
		return (FALSE);
	}
/*
 *   Check gas_phase
 */
	if (use.Get_gas_phase_ptr() != NULL)
	{
		cxxGasPhase * gas_phase_ptr = use.Get_gas_phase_ptr();
		if (last_model.gas_phase.size() != (int)gas_phase_ptr->Get_gas_comps().size())
			return (FALSE);
		if (last_model.numerical_fixed_volume != numerical_fixed_volume)
			return (FALSE);
		if (last_model.gas_phase_type != gas_phase_ptr->Get_type())
			return (FALSE);
		for (i = 0; i < (int) gas_phase_ptr->Get_gas_comps().size(); i++)
		{
			cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
			int k;
			class phase *phase_ptr = phase_bsearch(gc_ptr->Get_phase_name().c_str() , &k, FALSE);
			assert(phase_ptr);
			if (last_model.gas_phase[i] != phase_ptr)
			{
				return (FALSE);
			}
		}
	}
	else
	{
		if (last_model.gas_phase.size() > 0)
			return (FALSE);
	}
/*
 *   Check solid solutions
 */
	if (use.Get_ss_assemblage_ptr() != NULL)
	{
		if (last_model.ss_assemblage.size() != (int) use.Get_ss_assemblage_ptr()->Get_SSs().size())
			return (FALSE);
		std::vector<cxxSS *> ss_ptrs = use.Get_ss_assemblage_ptr()->Vectorize();
		for (size_t i = 0; i < ss_ptrs.size(); i++)
		{
			if (last_model.ss_assemblage[i] != string_hsave(ss_ptrs[i]->Get_name().c_str()))
			{
				return (FALSE);
			}
		}
	}
	else
	{
		if (last_model.ss_assemblage.size() > 0)
			return (FALSE);
	}
/*
 *   Check pure_phases
 */
	if (use.Get_pp_assemblage_ptr() != NULL)
	{
		cxxPPassemblage * pp_assemblage_ptr = use.Get_pp_assemblage_ptr();
		if (last_model.pp_assemblage.size() != (int) pp_assemblage_ptr->Get_pp_assemblage_comps().size())
			return (FALSE);

		std::map<std::string, cxxPPassemblageComp>::iterator it;
		it =  pp_assemblage_ptr->Get_pp_assemblage_comps().begin();
		i = 0;
		for ( ; it != pp_assemblage_ptr->Get_pp_assemblage_comps().end(); it++)
		{
			int j;
			class phase * phase_ptr = phase_bsearch(it->first.c_str(), &j, FALSE);
			assert(phase_ptr); 
			if (last_model.pp_assemblage[i] != phase_ptr)
			{
				return (FALSE);
			}
			if (last_model.add_formula[i] !=
				string_hsave(it->second.Get_add_formula().c_str()))
			{
				return (FALSE);
			}
			i++;
			/* A. Crapsi
			if (last_model.si[i] != use.Get_pp_assemblage_ptr()->pure_phases[i].si)
			{
				return (FALSE);
			}
			*/
		}
	}
	else
	{
		if (last_model.pp_assemblage.size() > 0)
			return (FALSE);
	}
/*
 *   Check surface
 */
	if (use.Get_surface_ptr() != NULL)
	{
		if (last_model.surface_comp.size() != (int) use.Get_surface_ptr()->Get_surface_comps().size())
			return (FALSE);
		if (last_model.surface_charge.size() != (int) use.Get_surface_ptr()->Get_surface_charges().size())
			return (FALSE);
		if (last_model.dl_type != use.Get_surface_ptr()->Get_dl_type())
			return (FALSE);
		/*if (last_model.edl != use.Get_surface_ptr()->edl) return(FALSE); */
		if (last_model.surface_type != use.Get_surface_ptr()->Get_type())
			return (FALSE);
		/*
		   if (last_model.only_counter_ions != use.Get_surface_ptr()->only_counter_ions) return(FALSE);
		 */
		for (i = 0; i < (int) use.Get_surface_ptr()->Get_surface_comps().size(); i++)
		{
			if (last_model.surface_comp[i] !=
				string_hsave(use.Get_surface_ptr()->Get_surface_comps()[i].Get_formula().c_str()))
				return (FALSE);
			if (use.Get_surface_ptr()->Get_surface_comps()[i].Get_phase_name().size() > 0)
			{
				cxxPPassemblage *pp_ptr = Utilities::Rxn_find(Rxn_pp_assemblage_map, use.Get_n_surface_user());
				if (pp_ptr == NULL || (pp_ptr->Get_pp_assemblage_comps().find(use.Get_surface_ptr()->Get_surface_comps()[i].Get_phase_name()) == 
							pp_ptr->Get_pp_assemblage_comps().end()))
				{
					Rxn_new_surface.insert(use.Get_n_surface_user());
					cxxSurface *surf_ptr = Utilities::Rxn_find(Rxn_surface_map, use.Get_n_surface_user());
					surf_ptr->Set_new_def(true);
					this->tidy_min_surface();
					return (FALSE);
		}
			}
			if (use.Get_surface_ptr()->Get_surface_comps()[i].Get_rate_name().size() > 0)
			{
				cxxKinetics *kinetics_ptr = Utilities::Rxn_find(Rxn_kinetics_map, use.Get_n_surface_user());
				if (kinetics_ptr == NULL || 
						(kinetics_ptr->Find(use.Get_surface_ptr()->Get_surface_comps()[i].Get_rate_name()) == NULL))
				{
					Rxn_new_surface.insert(use.Get_n_surface_user());
					cxxSurface *surf_ptr = Utilities::Rxn_find(Rxn_surface_map, use.Get_n_surface_user());
					surf_ptr->Set_new_def(true);
					this->tidy_kin_surface();
					return (FALSE);
				}
			}
		}
		for (i = 0; i < (int) use.Get_surface_ptr()->Get_surface_charges().size(); i++)
		{
			if (last_model.surface_charge[i] !=
				string_hsave(use.Get_surface_ptr()->Get_surface_charges()[i].Get_name().c_str()))
				return (FALSE);
		}
	}
	else
	{
		if (last_model.surface_comp.size() > 0)
			return (FALSE);
	}
/*
 *   Model is the same
 */
	return (TRUE);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_min_exch(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Defines proportionality factor between mineral and exchanger to
 *   jacob0
 */
	int j, k, jj;
	size_t row;
	class master *master_ptr;
	class unknown *unknown_ptr;
	LDBLE coef;

	if (use.Get_exchange_ptr() == NULL)
		return (OK);
	cxxExchange *ex_ptr = use.Get_exchange_ptr();
	int n_user = ex_ptr->Get_n_user();
	cxxExchange * exchange_ptr = Utilities::Rxn_find(Rxn_exchange_map, n_user);
	if (exchange_ptr == NULL)
	{
		input_error++;
		error_string = sformatf( "Exchange %d not found.",
				use.Get_n_exchange_user());
		error_msg(error_string, CONTINUE);
		return ERROR;
	}
	n_user = exchange_ptr->Get_n_user();
	if (!exchange_ptr->Get_related_phases())
		return (OK);
	for (size_t i = 0; i < exchange_ptr->Get_exchange_comps().size(); i++)
	{
		cxxExchComp & comp_ref = exchange_ptr->Get_exchange_comps()[i];
		if (comp_ref.Get_phase_name().size() == 0)
			continue;
		// Find exchange master
		cxxNameDouble nd(comp_ref.Get_totals());
		cxxNameDouble::iterator it = nd.begin();
		class master *exchange_master = NULL;
		for ( ; it != nd.end(); it++)
		{
			element * elt_ptr = element_store(it->first.c_str());
			assert (elt_ptr);
			if (elt_ptr->master->type == EX)
			{
				exchange_master = elt_ptr->master;
			}
		}
		if (exchange_master == NULL)
		{
			input_error++;
			error_string = sformatf(
					"Did not find master exchange species for %s",
					comp_ref.Get_formula().c_str());
			error_msg(error_string, CONTINUE);
			continue;
		}
		/* find unknown number */
		for (j = (int)count_unknowns - 1; j >= 0; j--)
		{
			if (x[j]->type != EXCH)
				continue;
			if (x[j]->master[0] == exchange_master)
				break;
		}
		for (k = (int)count_unknowns - 1; k >= 0; k--)
		{
			if (x[k]->type != PP)
				continue;
			//if (x[k]->phase->name == string_hsave(comp_ref.Get_phase_name().c_str()))
			if (strcmp_nocase(x[k]->phase->name, comp_ref.Get_phase_name().c_str()) == 0)
				break;
		}
		if (j == -1)
		{
			input_error++;
			error_string = sformatf(
					"Did not find unknown for master exchange species %s",
					exchange_master->s->name);
			error_msg(error_string, CONTINUE);
		}
		if (j == -1 || k == -1)
			continue;
/*
 *   Build jacobian
 */

		/* charge balance */
		store_jacob0((int)charge_balance_unknown->number, (int)x[k]->number,
					 comp_ref.Get_formula_z() * comp_ref.Get_phase_proportion());
		store_sum_deltas(&delta[k], &charge_balance_unknown->delta,
						 -comp_ref.Get_formula_z() * comp_ref.Get_phase_proportion());


		/* mole balance balance */
		count_elts = 0;
		paren_count = 0;
		{
			const char* cptr = comp_ref.Get_formula().c_str();
			get_elts_in_species(&cptr, 1.0);
		}
#ifdef COMBINE
		change_hydrogen_in_elt_list(0);
#endif
		for (jj = 0; jj < count_elts; jj++)
		{
			master_ptr = elt_list[jj].elt->primary;
			if (master_ptr == NULL)
			{
				input_error++;
				error_string = sformatf(
						"Did not find unknown for %s, exchange related to mineral %s",
						elt_list[jj].elt->primary->elt->name, comp_ref.Get_phase_name().c_str());
				error_msg(error_string, STOP);
			}
			if (master_ptr->in == FALSE)
			{
				master_ptr = master_ptr->s->secondary;
			}
			if (master_ptr->s->type == EX)
			{
				if (equal
					(x[j]->moles,
					 x[k]->moles * elt_list[jj].coef *
					 comp_ref.Get_phase_proportion(),
					 5.0 * convergence_tolerance) == FALSE)
				{
					error_string = sformatf(
							"Resetting number of sites in exchanger %s (=%e) to be consistent with moles of phase %s (=%e).\n%s",
							master_ptr->s->name, (double) x[j]->moles,
							comp_ref.Get_phase_name().c_str(),
							(double) (x[k]->moles * elt_list[jj].coef *
									  comp_ref.Get_phase_proportion()),
							"\tHas equilibrium_phase assemblage been redefined?\n");
					warning_msg(error_string);
					x[j]->moles =
						x[k]->moles * elt_list[jj].coef *
						comp_ref.Get_phase_proportion();
				}
			}
			coef = elt_list[jj].coef;
			if (master_ptr->s == s_hplus)
			{
				row = mass_hydrogen_unknown->number;
				unknown_ptr = mass_hydrogen_unknown;
			}
			else if (master_ptr->s == s_h2o)
			{
				row = mass_oxygen_unknown->number;
				unknown_ptr = mass_oxygen_unknown;
			}
			else
			{
				row = master_ptr->unknown->number;
				unknown_ptr = master_ptr->unknown;
			}
			store_jacob0((int)row, (int)x[k]->number,
						 coef * comp_ref.Get_phase_proportion());
			store_sum_deltas(&delta[k], &unknown_ptr->delta,
							 -coef * comp_ref.Get_phase_proportion());
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
build_min_surface(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Defines proportionality factor between mineral and surface to
 *   jacob0
 */
	if (use.Get_surface_ptr() == NULL)
		return (OK);
	cxxSurface *surface_ptr = use.Get_surface_ptr();
	if (!surface_ptr->Get_related_phases())
		return (OK);
	for (size_t i = 0; i < surface_ptr->Get_surface_comps().size(); i++)
	{
		cxxSurfaceComp *comp_ptr = &(surface_ptr->Get_surface_comps()[i]);
		if (comp_ptr->Get_phase_name().size() == 0)
			continue;
		class element *elt_ptr = element_store(comp_ptr->Get_master_element().c_str());
		/* find unknown number */
		int j;
		for (j = (int)count_unknowns - 1; j >= 0; j--)
		{
			if (x[j]->type != SURFACE)
				continue;
			if (x[j]->master[0] == elt_ptr->master)
				break;
		}
		int k;
		for (k = (int)count_unknowns - 1; k >= 0; k--)
		{
			if (x[k]->type != PP)
				continue;
			//if (x[k]->phase->name == string_hsave(comp_ptr->Get_phase_name().c_str()))
			if (strcmp_nocase(x[k]->phase->name, comp_ptr->Get_phase_name().c_str()) == 0)
				break;
		}
		if (j == -1)
		{
			input_error++;
			error_string = sformatf(
					"Did not find unknown for master surface species %s",
					elt_ptr->master->s->name);
			error_msg(error_string, CONTINUE);
		}
		if (j == -1 || k == -1)
			continue;

		/* update grams == moles in this case */
		if (j < count_unknowns - 1 && x[(size_t)j + 1]->type == SURFACE_CB)
		{
			store_sum_deltas(&delta[k], &(x[(size_t)j + 1]->related_moles), -1.0);
		}

		/* charge balance */
		store_jacob0((int)charge_balance_unknown->number, (int)x[k]->number,
					 comp_ptr->Get_formula_z() * comp_ptr->Get_phase_proportion());
		store_sum_deltas(&delta[k], &charge_balance_unknown->delta,
						 -comp_ptr->Get_formula_z() * comp_ptr->Get_phase_proportion());
		count_elts = 0;
		paren_count = 0;
		{
			/* Add specified formula for all types of surfaces */
			const char* cptr1 = comp_ptr->Get_formula().c_str();
			get_elts_in_species(&cptr1, 1.0);
		}
#ifdef COMBINE
		change_hydrogen_in_elt_list(0);
#endif
		for (int jj = 0; jj < count_elts; jj++)
		{
			class master * master_ptr = elt_list[jj].elt->primary;
			if (master_ptr->in == FALSE)
			{
				master_ptr = master_ptr->s->secondary;
			}
			if (master_ptr == NULL)
			{
				input_error++;
				error_string = sformatf(
						"Did not find unknown for %s, surface related to mineral %s",
						elt_list[jj].elt->primary->elt->name, comp_ptr->Get_phase_name().c_str());
				error_msg(error_string, STOP);
			}
			if (master_ptr->s->type == SURF)
			{
				if (equal
					(x[j]->moles,
					 x[k]->moles * elt_list[jj].coef *
					 comp_ptr->Get_phase_proportion(),
					 5.0 * convergence_tolerance) == FALSE)
				{
					error_string = sformatf(
							"Resetting number of sites in surface %s (=%e) to be consistent with moles of phase %s (=%e).\n%s",
							master_ptr->s->name, (double) x[j]->moles,
							comp_ptr->Get_phase_name().c_str(),
							(double) (x[k]->moles * elt_list[jj].coef *
									  comp_ptr->Get_phase_proportion()),
							"\tHas equilibrium_phase assemblage been redefined?\n");
					warning_msg(error_string);
					x[j]->moles =
						x[k]->moles * elt_list[jj].coef *
						comp_ptr->Get_phase_proportion();
				}
			}
			LDBLE coef = elt_list[jj].coef;
			size_t row;
			class unknown *unknown_ptr;
			if (master_ptr->s == s_hplus)
			{
				row = mass_hydrogen_unknown->number;
				unknown_ptr = mass_hydrogen_unknown;
			}
			else if (master_ptr->s == s_h2o)
			{
				row = mass_oxygen_unknown->number;
				unknown_ptr = mass_oxygen_unknown;
			}
			else
			{
				row = master_ptr->unknown->number;
				unknown_ptr = master_ptr->unknown;
			}
			store_jacob0((int)row, (int)x[k]->number,
						 coef * comp_ptr->Get_phase_proportion());
			store_sum_deltas(&delta[k], &unknown_ptr->delta,
							 -coef * comp_ptr->Get_phase_proportion());
		}

	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
setup_related_surface(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Fill in data for surface assemblage in unknown structure
 */
	if (use.Get_surface_ptr() == NULL)
		return (OK);
	if (!use.Get_surface_ptr()->Get_related_phases())
		return (OK);

	for (int i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type == SURFACE)
		{
			cxxSurfaceComp *comp_ptr = use.Get_surface_ptr()->Find_comp(x[i]->surface_comp);
			if (comp_ptr->Get_phase_name().size() > 0)
			{
				int k;
				for (k = (int)count_unknowns - 1; k >= 0; k--)
				{
					if (x[k]->type != PP)
						continue;
					//if (x[k]->phase->name == string_hsave(comp_ptr->Get_phase_name().c_str()))
					if (strcmp_nocase(x[k]->phase->name, comp_ptr->Get_phase_name().c_str()) == 0)
						break;
				}
				if (k == -1)
					continue;

				x[i]->phase_unknown = x[k];
/* !!!!! */
				x[i]->moles = x[k]->moles * comp_ptr->Get_phase_proportion();

			}
		}
		else if (x[i]->type == SURFACE_CB)
		{
			cxxSurfaceComp *comp_ptr = use.Get_surface_ptr()->Find_comp(x[(size_t)i-1]->surface_comp);
			if (comp_ptr->Get_phase_name().size() > 0)
			{
				cxxSurfaceComp *comp_i_ptr = use.Get_surface_ptr()->Find_comp(x[i]->surface_comp);
				int k;
				for (k = (int)count_unknowns - 1; k >= 0; k--)
				{
					if (x[k]->type != PP)
						continue;
					//if (x[k]->phase->name == string_hsave(comp_i_ptr->Get_phase_name().c_str()))
					if (strcmp_nocase(x[k]->phase->name, comp_i_ptr->Get_phase_name().c_str()) == 0)
						break;
				}
				if (k == -1)
					continue;

				x[i]->phase_unknown = x[k];
				/* !!!! Added for security, not checked... */
				x[i]->related_moles = x[k]->moles * comp_i_ptr->Get_phase_proportion();
			}
		}
	}
	return (OK);
}
