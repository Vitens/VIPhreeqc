#include <time.h>
#include <assert.h>
#include "Utils.h"
#include "Phreeqc.h"
#include "phqalloc.h"
#include "PBasic.h"
#include "Temperature.h"
#include "Exchange.h"
#include "GasPhase.h"
#include "Reaction.h"
#include "PPassemblage.h"
#include "SSassemblage.h"
#include "cxxKinetics.h"
#include "Solution.h"

#if defined(WINDOWS) || defined(_WINDOWS)
#include <windows.h>
#endif

#if defined(PHREEQCI_GUI)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/* ---------------------------------------------------------------------- */
void Phreeqc::
initialize(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Initialize global variables
 */
	moles_per_kilogram_string = "Mol/kgw";
/*
 *   Allocate space
 */
	cell_data.resize((size_t)count_cells + 2); // initialized by global_structures.h

	count_inverse = 0;
	space((void **) ((void *) &line), INIT, &max_line, sizeof(char));

	space((void **) ((void *) &line_save), INIT, &max_line, sizeof(char));

	// one stag_data in phreeqc.h, initialized in global_structures

	// user_print
	user_print = new class rate;
	user_print->name = string_hsave("User_print");
	user_print->commands.clear();
	user_print->linebase = NULL;
	user_print->varbase = NULL;
	user_print->loopbase = NULL;
	/*
	   Initialize llnl aqueous model parameters
	 */
	a_llnl = b_llnl = 0.0;
    // new PBasic
	if (basic_interpreter != NULL)
	{
		basic_free();
	}
	basic_interpreter = new PBasic(this, phrq_io);
	// allocate one change_surf
	change_surf =
		(struct Change_Surf *)
		PHRQ_malloc((size_t) (2 * sizeof(struct Change_Surf)));
	if (change_surf == NULL)
		malloc_error();
	change_surf[0].cell_no = -99;
	change_surf[0].next = TRUE;
	change_surf[1].cell_no = -99;
	change_surf[1].next = FALSE;
	/*
	* define constant named log_k
	*/
	class logk* logk_ptr = logk_store("XconstantX", TRUE);
	read_log_k_only("1.0", &logk_ptr->log_k[0]);

#ifdef PHREEQCI_GUI
	g_spread_sheet.heading            = NULL;
	g_spread_sheet.units              = NULL;
	g_spread_sheet.defaults.units     = NULL;
	g_spread_sheet.defaults.redox     = NULL;
	assert(g_spread_sheet.rows.empty());
	assert(g_spread_sheet.defaults.iso.empty());
#endif

	// Initialize cvode
	cvode_init();

	// Allocate space for pitzer
	pitzer_init();

	// Allocate space for sit
	sit_init();

	use_kinetics_limiter = false;

	return;
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
set_use(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Structure "use" has list of solution, ex, surf, pp_assemblage,
 *   gas_phase and solid solution to use in current calculations,
 *   also mix, irrev, and temp.
 *   This routine searches for the user numbers in each list
 *   (solution, ex, ...) and sets a pointer in structure use
 */

/*
 *   Initial solution case
 */
	use.Set_pp_assemblage_ptr(NULL);
	use.Set_mix_ptr(NULL);
	use.Set_reaction_ptr(NULL);
	use.Set_exchange_ptr(NULL);
	use.Set_kinetics_ptr(NULL);
	use.Set_surface_ptr(NULL);
	use.Set_temperature_ptr(NULL);
	use.Set_pressure_ptr(NULL);
	use.Set_gas_phase_ptr(NULL);
	use.Set_ss_assemblage_ptr(NULL);

	if (state < REACTION)
	{
		return (OK);
	}
/*
 *   Reaction case
 */
	if (use.Get_pp_assemblage_in() == FALSE &&
		use.Get_reaction_in() == FALSE &&
		use.Get_mix_in() == FALSE &&
		use.Get_exchange_in() == FALSE &&
		use.Get_kinetics_in() == FALSE &&
		use.Get_surface_in() == FALSE &&
		use.Get_temperature_in() == FALSE &&
		use.Get_pressure_in() == FALSE &&
		use.Get_gas_phase_in() == FALSE && use.Get_ss_assemblage_in() == FALSE)
	{
		return (FALSE);
	}
	if (use.Get_solution_in() == FALSE && use.Get_mix_in() == FALSE)
		return (FALSE);
/*
 *   Find solution
 */
	if (use.Get_solution_in())
	{
		use.Set_solution_ptr(Utilities::Rxn_find(Rxn_solution_map, use.Get_n_solution_user()));
		if (use.Get_solution_ptr() == NULL)
		{
			error_string = sformatf( "Solution %d not found.",
					use.Get_n_solution_user());
			error_msg(error_string, STOP);
		}
	}
/*
 *   Find mixture
 */
	if (use.Get_mix_in() == TRUE)
	{
		use.Set_mix_ptr(Utilities::Rxn_find(Rxn_mix_map, use.Get_n_mix_user()));
		use.Set_n_mix_user_orig(use.Get_n_mix_user());
		if (use.Get_mix_ptr() == NULL)
		{
			error_string = sformatf( "Mix %d not found.",
					use.Get_n_mix_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_mix_ptr(NULL);
	}
/*
 *   Find pure phase assemblage
 */
	if (use.Get_pp_assemblage_in() == TRUE)
	{
		use.Set_pp_assemblage_ptr(Utilities::Rxn_find(Rxn_pp_assemblage_map, use.Get_n_pp_assemblage_user()));
		if (use.Get_pp_assemblage_ptr() == NULL)
		{
			error_string = sformatf( "Pure phase assemblage %d not found.",
					use.Get_n_pp_assemblage_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_pp_assemblage_ptr(NULL);
	}
/*
 *   Find irrev reaction
 */
	if (use.Get_reaction_in() == TRUE)
	{
		use.Set_reaction_ptr(Utilities::Rxn_find(Rxn_reaction_map, use.Get_n_reaction_user()));
		if (use.Get_reaction_ptr() == NULL)
		{
			error_string = sformatf( "Reaction %d not found.",
					use.Get_n_reaction_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_reaction_ptr(NULL);
	}
/*
 *   Find exchange
 */
	if (use.Get_exchange_in() == TRUE)
	{
		use.Set_exchange_ptr(Utilities::Rxn_find(Rxn_exchange_map, use.Get_n_exchange_user()));
		if (use.Get_exchange_ptr() == NULL)
		{
			error_string = sformatf( "Exchange %d not found.",
					use.Get_n_exchange_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_exchange_ptr(NULL);
	}
/*
 *   Find kinetics
 */
	if (use.Get_kinetics_in() == TRUE)
	{
		use.Set_kinetics_ptr(Utilities::Rxn_find(Rxn_kinetics_map, use.Get_n_kinetics_user()));
		if (use.Get_kinetics_ptr() == NULL)
		{
			error_string = sformatf( "Kinetics %d not found.",
					use.Get_n_kinetics_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_kinetics_ptr(NULL);
	}
/*
 *   Find surface
 */
	dl_type_x = cxxSurface::NO_DL;
	if (use.Get_surface_in() == TRUE)
	{
		use.Set_surface_ptr(Utilities::Rxn_find(Rxn_surface_map, use.Get_n_surface_user()));
		if (use.Get_surface_ptr() == NULL)
		{
			error_string = sformatf( "Surface %d not found.",
					use.Get_n_surface_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_surface_ptr(NULL);
	}
/*
 *   Find temperature
 */
	if (use.Get_temperature_in() == TRUE)
	{
		use.Set_temperature_ptr(Utilities::Rxn_find(Rxn_temperature_map, use.Get_n_temperature_user()));
		if (use.Get_temperature_ptr() == NULL)
		{
			error_string = sformatf( "Temperature %d not found.",
					use.Get_n_temperature_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_temperature_ptr(NULL);
	}
/*
 *   Find pressure
 */
	if (use.Get_pressure_in() == TRUE)
	{
		use.Set_pressure_ptr(Utilities::Rxn_find(Rxn_pressure_map, use.Get_n_pressure_user()));
		if (use.Get_pressure_ptr() == NULL)
		{
			error_string = sformatf( "Pressure %d not found.",	use.Get_n_pressure_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_pressure_ptr(NULL);
	}
/*
 *   Find gas
 */
	if (use.Get_gas_phase_in() == TRUE)
	{
		use.Set_gas_phase_ptr(Utilities::Rxn_find(Rxn_gas_phase_map, use.Get_n_gas_phase_user()));
		if (use.Get_gas_phase_ptr() == NULL)
		{
			error_string = sformatf( "Gas_phase %d not found.",
					use.Get_n_gas_phase_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_gas_phase_ptr(NULL);
	}
/*
 *   Find ss_assemblage
 */
	if (use.Get_ss_assemblage_in() == TRUE)
	{
		use.Set_ss_assemblage_ptr(Utilities::Rxn_find(Rxn_ss_assemblage_map, use.Get_n_ss_assemblage_user()));
		if (use.Get_ss_assemblage_ptr() == NULL)
		{
			error_string = sformatf( "ss_assemblage %d not found.",
					use.Get_n_ss_assemblage_user());
			error_msg(error_string, STOP);
		}
	}
	else
	{
		use.Set_ss_assemblage_ptr(NULL);
	}
	/*
	if (use.irrev_ptr != NULL && use.Get_kinetics_ptr() != NULL)
	{
		warning_msg("Should not use REACTION in same simulation with KINETICS.");
	}
	*/
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
initial_solutions(int print)
/* ---------------------------------------------------------------------- */
{
/*
 *   Go through list of solutions, make initial solution calculations
 *   for any marked "new".
 */
	int converge, converge1;
	int last, n_user, print1;
	char token[2 * MAX_LENGTH];

	state = INITIAL_SOLUTION;
	set_use();
	print1 = TRUE;
	dl_type_x = cxxSurface::NO_DL;
	//std::map<int, cxxSolution>::iterator it = Rxn_solution_map.begin();
	//for ( ; it != Rxn_solution_map.end(); it++)
	//{
	//for (size_t nn = 0; nn < Rxn_new_solution.size(); nn++)
	for (std::set<int>::const_iterator nit = Rxn_new_solution.begin(); nit != Rxn_new_solution.end(); nit++)
	{
		std::map<int, cxxSolution>::iterator it = Rxn_solution_map.find(*nit);
		if (it == Rxn_solution_map.end())
		{
			assert(false);
		}
		cxxSolution &solution_ref = it->second;
		initial_solution_isotopes = FALSE;
		if (solution_ref.Get_new_def())
		{
			if (print1 == TRUE && print == TRUE)
			{
				dup_print("Beginning of initial solution calculations.",
						  TRUE);
				print1 = FALSE;
			}
			if (print == TRUE)
			{
				sprintf(token, "Initial solution %d.\t%.350s",
						solution_ref.Get_n_user(), solution_ref.Get_description().c_str());
				dup_print(token, FALSE);
			}
			use.Set_solution_ptr(&solution_ref);
			LDBLE d0 = solution_ref.Get_density();
			//LDBLE d1 = 0;
			bool diag = (diagonal_scale == TRUE) ? true : false;
			int count_iterations = 0;
			std::string input_units = solution_ref.Get_initial_data()->Get_units();
			cxxISolution *initial_data_ptr = solution_ref.Get_initial_data();
			density_iterations = 0;
			for (;;)
			{
				prep();
				k_temp(solution_ref.Get_tc(), solution_ref.Get_patm());
				set(TRUE);
				always_full_pitzer = FALSE;
				
				diagonal_scale = TRUE;
				converge = model();
				if (converge == ERROR /*&& diagonal_scale == FALSE*/)
				{
					diagonal_scale = TRUE;
					always_full_pitzer = TRUE;
					set(TRUE);
					converge = model();
				}
				density_iterations++;
				if (solution_ref.Get_initial_data()->Get_calc_density())
				{
					solution_ref.Set_density(calc_dens());
					if (!equal(d0, solution_ref.Get_density(), 1e-8))
					{
						initial_data_ptr->Set_units(input_units);
						d0 = solution_ref.Get_density();
						if (count_iterations++ < 20) 
						{
							diag = (diagonal_scale == TRUE) ? true : false;
							continue;
						}
						else
						{
							error_msg(sformatf("%s %d.", "Density calculation failed for initial solution ", solution_ref.Get_n_user()),
								STOP);
						}
					}
				}
				break;
			} 
			diagonal_scale = (diag) ? TRUE : FALSE;
			converge1 = check_residuals();
			sum_species();
			viscosity();
			add_isotopes(solution_ref);
			punch_all();
			print_all();
			density_iterations = 0;
			/* free_model_allocs(); */
// remove pr_in
			for (size_t i = 0; i < count_unknowns; i++)
			{
				if (x[i]->type == SOLUTION_PHASE_BOUNDARY)
					x[i]->phase->pr_in = false;
			}

			if (converge == ERROR || converge1 == ERROR)
			{
				error_msg(sformatf("%s %d.", "Model failed to converge for initial solution ", solution_ref.Get_n_user()),
						  STOP);
			}
			n_user = solution_ref.Get_n_user();
			last = solution_ref.Get_n_user_end();
			/* copy isotope data */
			if (solution_ref.Get_isotopes().size() > 0)
			{
				isotopes_x = solution_ref.Get_isotopes();
			}
			else
			{
				isotopes_x.clear();
			}
			xsolution_save(n_user);
			Utilities::Rxn_copies(Rxn_solution_map, n_user, last);
		}
	}
	initial_solution_isotopes = FALSE;
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
initial_exchangers(int print)
/* ---------------------------------------------------------------------- */
{
/*
 *   Go through list of exchangers, make initial calculations
 *   for any marked "new" that are defined to be in equilibrium with a
 *   solution.
 */
	int i, converge, converge1;
	int last, n_user, print1;
	char token[2 * MAX_LENGTH];

	state = INITIAL_EXCHANGE;
	set_use();
	print1 = TRUE;
	dl_type_x = cxxSurface::NO_DL;
	//std::map<int, cxxExchange>::iterator it = Rxn_exchange_map.begin();
	//for ( ; it != Rxn_exchange_map.end(); it++)
	//{
	//for (size_t nn = 0; nn < Rxn_new_exchange.size(); nn++)
	//{
		//std::map<int, cxxExchange>::iterator it = Rxn_exchange_map.find(Rxn_new_exchange[nn]);
	for (std::set<int>::const_iterator nit = Rxn_new_exchange.begin(); nit != Rxn_new_exchange.end(); nit++)
	{
		std::map<int, cxxExchange>::iterator it = Rxn_exchange_map.find(*nit);
		if (it == Rxn_exchange_map.end())
		{
			assert(false);
		}
		if (!it->second.Get_new_def())
			continue;
		cxxExchange *exchange_ptr = &(it->second);
		n_user = exchange_ptr->Get_n_user();
		last = exchange_ptr->Get_n_user_end();
		exchange_ptr->Set_n_user_end(n_user);
		exchange_ptr->Set_new_def(false);
		if (exchange_ptr->Get_solution_equilibria())
		{
			if (print1 == TRUE && print == TRUE)
			{
				dup_print("Beginning of initial exchange"
						  "-composition calculations.", TRUE);
				print1 = FALSE;
			}
			if (print == TRUE)
			{
				sprintf(token, "Exchange %d.\t%.350s",
						exchange_ptr->Get_n_user(), exchange_ptr->Get_description().c_str());
				dup_print(token, FALSE);
			}
			use.Set_exchange_ptr(exchange_ptr);
			use.Set_solution_ptr(Utilities::Rxn_find(Rxn_solution_map, exchange_ptr->Get_n_solution()));
			if (use.Get_solution_ptr() == NULL)
			{
				error_msg
					("Solution not found for initial exchange calculation",
					 STOP);
			}

			prep();
			k_temp(use.Get_solution_ptr()->Get_tc(), use.Get_solution_ptr()->Get_patm());
			set(TRUE);
			converge = model();
			converge1 = check_residuals();
			sum_species();
			viscosity();
			species_list_sort();
			print_exchange();
			xexchange_save(n_user);
			punch_all();
			/* free_model_allocs(); */
			if (converge == ERROR || converge1 == ERROR)
			{
				error_msg
					("Model failed to converge for initial exchange calculation.",
					 STOP);
			}
		}
		for (i = n_user + 1; i <= last; i++)
		{
			Utilities::Rxn_copy(Rxn_exchange_map, n_user, i);
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
initial_gas_phases(int print)
/* ---------------------------------------------------------------------- */
{
/*
 *   Go through list of gas_phases, make initial calculations
 *   for any marked "new" that are defined to be in equilibrium with a
 *   solution.
 */
	int converge, converge1;
	int last, n_user, print1;
	char token[2 * MAX_LENGTH];
	class phase *phase_ptr;
	class rxn_token *rxn_ptr;
	LDBLE lp;
	bool PR = false;

	state = INITIAL_GAS_PHASE;
	set_use();
	print1 = TRUE;
	dl_type_x = cxxSurface::NO_DL;
	//std::map<int, cxxGasPhase>::iterator it = Rxn_gas_phase_map.begin();
	//for ( ; it != Rxn_gas_phase_map.end(); it++)
	//{
	//for (size_t nn = 0; nn < Rxn_new_gas_phase.size(); nn++)
	//{
		//std::map<int, cxxGasPhase>::iterator it = Rxn_gas_phase_map.find(Rxn_new_gas_phase[nn]);
	for (std::set<int>::const_iterator nit = Rxn_new_gas_phase.begin(); nit != Rxn_new_gas_phase.end(); nit++)
	{
		std::map<int, cxxGasPhase>::iterator it = Rxn_gas_phase_map.find(*nit);
		if (it == Rxn_gas_phase_map.end())
		{
			assert(false);
		}
		cxxGasPhase *gas_phase_ptr = &it->second;
		if (!gas_phase_ptr->Get_new_def())
			continue;
		n_user = gas_phase_ptr->Get_n_user();
		last = gas_phase_ptr->Get_n_user_end();
		gas_phase_ptr->Set_n_user_end(n_user);
		gas_phase_ptr->Set_new_def(false);
		if (gas_phase_ptr->Get_solution_equilibria())
		{
			if (print1 == TRUE && print == TRUE)
			{
				dup_print("Beginning of initial gas_phase"
						  "-composition calculations.", TRUE);
				print1 = FALSE;
			}
			if (print == TRUE)
			{
				sprintf(token, "Gas_Phase %d.\t%.350s",
						gas_phase_ptr->Get_n_user(), gas_phase_ptr->Get_description().c_str());
				dup_print(token, FALSE);
			}

			/* Try to obtain a solution pointer */
			use.Set_solution_ptr(Utilities::Rxn_find(Rxn_solution_map, gas_phase_ptr->Get_n_solution()));
			prep();
			k_temp(use.Get_solution_ptr()->Get_tc(), use.Get_solution_ptr()->Get_patm());
			set(TRUE);
			converge = model();
			converge1 = check_residuals();
			if (converge == ERROR || converge1 == ERROR)
			{
				/* free_model_allocs(); */
				error_msg
					("Model failed to converge for initial gas phase calculation.",
					 STOP);
			}
			use.Set_gas_phase_ptr(gas_phase_ptr);
			gas_phase_ptr->Set_total_p(0);
			gas_phase_ptr->Set_total_moles(0);
			for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
			{
				cxxGasComp * gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
				int k;
				phase_ptr = phase_bsearch(gc_ptr->Get_phase_name().c_str(), &k, FALSE);
				if (phase_ptr->in == TRUE)
				{
					lp = -phase_ptr->lk;
					for (rxn_ptr = &phase_ptr->rxn_x.token[0] + 1;
						 rxn_ptr->s != NULL; rxn_ptr++)
					{
						lp += rxn_ptr->s->la * rxn_ptr->coef;
					}
					phase_ptr->p_soln_x = exp(lp * LOG_10);
					gas_phase_ptr->Set_total_p(gas_phase_ptr->Get_total_p() + phase_ptr->p_soln_x);
					phase_ptr->moles_x = phase_ptr->p_soln_x *
						gas_phase_ptr->Get_volume() / (R_LITER_ATM * tk_x);
					gc_ptr->Set_moles(phase_ptr->moles_x);
					gas_phase_ptr->Set_total_moles(gas_phase_ptr->Get_total_moles() + phase_ptr->moles_x);
					if (phase_ptr->p_c || phase_ptr->t_c)
						PR = true;
				}
				else
				{
					phase_ptr->moles_x = 0;
				}
			}
			if (fabs(gas_phase_ptr->Get_total_p() - use.Get_solution_ptr()->Get_patm()) > 5)
			{
				sprintf(token,
					"WARNING: While initializing gas phase composition by equilibrating:\n%s (%.2f atm) %s (%.2f atm).\n%s.",
					"         Gas phase pressure",
					(double) gas_phase_ptr->Get_total_p(),
					"is not equal to solution-pressure",
					(double) use.Get_solution_ptr()->Get_patm(),
					"         Pressure effects on solubility may be incorrect");
					dup_print(token, FALSE);
			}

			print_gas_phase();
 			if (PR /*&& use.Get_gas_phase_ptr()->total_p > 1.0*/)
 				warning_msg("While initializing gas phase composition by equilibrating:\n"
				"         Found definitions of gas` critical temperature and pressure.\n"
				"         Going to use Peng-Robinson in subsequent calculations.\n");
			xgas_save(n_user);
			punch_all();
			/* free_model_allocs(); */
		}
		Utilities::Rxn_copies(Rxn_gas_phase_map, n_user, last);
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
initial_surfaces(int print)
/* ---------------------------------------------------------------------- */
{
/*
 *   Go through list of surfaces, make initial calculations
 *   for any marked "new" that are defined to be in equilibrium with a
 *   solution.
 */
	int last, n_user, print1;

	state = INITIAL_SURFACE;
	set_use();
	print1 = TRUE;

	//std::map<int, cxxSurface>::iterator it = Rxn_surface_map.begin();
	//for ( ; it != Rxn_surface_map.end(); it++)
	//{
	//for (size_t nn = 0; nn < Rxn_new_surface.size(); nn++)
	//{
		//std::map<int, cxxSurface>::iterator it = Rxn_surface_map.find(Rxn_new_surface[nn]);
	for (std::set<int>::const_iterator nit = Rxn_new_surface.begin(); nit != Rxn_new_surface.end(); nit++)
	{
		std::map<int, cxxSurface>::iterator it = Rxn_surface_map.find(*nit);
		if (it == Rxn_surface_map.end())
		{
			assert(false);
		}
		cxxSurface * surface_ptr = &it->second;
		if (!surface_ptr->Get_new_def())
			continue;
		n_user = surface_ptr->Get_n_user();
		last = surface_ptr->Get_n_user_end();
		surface_ptr->Set_n_user_end(n_user);
		if (surface_ptr->Get_solution_equilibria())
		{
			if (print1 == TRUE && print == TRUE)
			{
				dup_print
					("Beginning of initial surface-composition calculations.",
					 TRUE);
				print1 = FALSE;
			}
			if (print == TRUE)
			{
				std::ostringstream msg;
				msg << "Surface " << n_user << ".\t" << surface_ptr->Get_description().c_str();
				dup_print(msg.str().c_str(), FALSE);
			}
			use.Set_surface_ptr(surface_ptr);
			dl_type_x = use.Get_surface_ptr()->Get_dl_type();
			use.Set_solution_ptr(Utilities::Rxn_find(Rxn_solution_map, surface_ptr->Get_n_solution()));
			if (use.Get_solution_ptr() == NULL)
			{
				error_msg
					("Solution not found for initial surface calculation",
					 STOP);
			}
			set_and_run_wrapper(-1, FALSE, FALSE, -1, 0.0);
			species_list_sort();
			print_surface();
			/*print_all(); */
			punch_all();
			xsurface_save(n_user);
			/* free_model_allocs(); */
		}
		Utilities::Rxn_copies(Rxn_surface_map, n_user, last);
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
reactions(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Make all reaction calculation which could include:
 *      equilibrium with a pure-phase assemblage,
 *      equilibrium with an exchanger,
 *      equilibrium with an surface,
 *      equilibrium with a gas phase,
 *      equilibrium with a solid solution assemblage,
 *      kinetics,
 *      change of temperature,
 *      mixture,
 *      or irreversible reaction.
 */
	int count_steps, use_mix;
	char token[2 * MAX_LENGTH];
	class save save_data;
	LDBLE kin_time;
	cxxKinetics *kinetics_ptr;

	state = REACTION;
	/* last_model.force_prep = TRUE; */
	if (set_use() == FALSE)
		return (OK);
/*
 *   Find maximum number of steps
 */
	dup_print("Beginning of batch-reaction calculations.", TRUE);
	count_steps = 1;
	if (use.Get_reaction_in() == TRUE && use.Get_reaction_ptr() != NULL)
	{
		cxxReaction *reaction_ptr = use.Get_reaction_ptr();
		if (reaction_ptr->Get_reaction_steps() > count_steps)
			count_steps = reaction_ptr->Get_reaction_steps();
	}
	if (use.Get_kinetics_in() == TRUE && use.Get_kinetics_ptr() != NULL)
	{
		if (use.Get_kinetics_ptr()->Get_reaction_steps() > count_steps)
			count_steps = use.Get_kinetics_ptr()->Get_reaction_steps();
	}
	if (use.Get_temperature_in() == TRUE && use.Get_temperature_ptr() != NULL)
	{
		int count = use.Get_temperature_ptr()->Get_countTemps();
		if (count > count_steps)
		{
			count_steps = count;
		}
	}
	if (use.Get_pressure_in() == TRUE && use.Get_pressure_ptr() != NULL)
	{
		int count = use.Get_pressure_ptr()->Get_count();
		if (count > count_steps)
		{
			count_steps = count;
		}
	}
	count_total_steps = count_steps;
/*
 *  save data for saving solutions
 */
	// memcpy(&save_data, &save, sizeof(class save));
	save_data = save;
	/*
	 *Copy everything to -2
	 */
	copy_use(-2);
	rate_sim_time_start = 0;
	rate_sim_time = 0;
	for (reaction_step = 1; reaction_step <= count_steps; reaction_step++)
	{
		overall_iterations = 0;
		sprintf(token, "Reaction step %d.", reaction_step);
		if (reaction_step > 1 && incremental_reactions == FALSE)
		{
			copy_use(-2);
		}
		set_initial_moles(-2);
		dup_print(token, FALSE);
/*
 *  Determine time step for kinetics
 */
		kin_time = 0.0;

		if (use.Get_kinetics_in() == TRUE)
		{
			kinetics_ptr = Utilities::Rxn_find(Rxn_kinetics_map, -2);
			kin_time = kinetics_ptr->Current_step((incremental_reactions==TRUE), reaction_step);
		}
		if (incremental_reactions == FALSE ||
			(incremental_reactions == TRUE && reaction_step == 1))
		{
			use_mix = TRUE;
		}
		else
		{
			use_mix = FALSE;
		}
/*
 *   Run reaction step
 */
		run_reactions(-2, kin_time, use_mix, 1.0);
		if (incremental_reactions == TRUE)
		{
			rate_sim_time_start += kin_time;
			rate_sim_time = rate_sim_time_start;
		}
		else
		{
			rate_sim_time = kin_time;
		}
		if (state != ADVECTION)
		{
			punch_all();
			print_all();
		}
		/* saves back into -2 */
		if (reaction_step < count_steps)
		{
			saver();
		}
	}
/*
 *   save end of reaction
 */
	// memcpy(&save, &save_data, sizeof(class save));
	save = save_data;
	if (use.Get_kinetics_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_kinetics_map, -2, use.Get_n_kinetics_user());
	}
	saver();

	/* free_model_allocs(); */
//// set pr_in to false for following steps...
//	if (use.Get_pp_assemblage_in())
//	{
//		for (int i = 0; i < count_unknowns; i++)
//		{
//			if (x[i]->type == PP)
//				x[i]->phase->pr_in = false;
//		}
//	}
//	if (use.Get_gas_phase_in())
//	{
//		cxxGasPhase *gas_phase_ptr = use.Get_gas_phase_ptr();
//		for (size_t i = 0; i < gas_phase_ptr->Get_gas_comps().size(); i++)
//		{	
//			cxxGasComp *gc_ptr = &(gas_phase_ptr->Get_gas_comps()[i]);
//			int k;
//			class phase *phase_ptr = phase_bsearch(gc_ptr->Get_phase_name().c_str(), &k, FALSE);
//			assert(phase_ptr);
//			phase_ptr->pr_in = false;
//		}
//	}
	/* last_model.force_prep = TRUE; */
	rate_sim_time = 0;
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
saver(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save results of calcuations (data in variables with _x,
 *   in unknown structure x, in master, or s) into structure
 *   arrays.  Structure "save" has info on whether to save
 *   data for each entity (solution, ex, surf, pp, gas, or s_s).
 *   Initial calculation may be saved into multiple "n_user"
 *   slots.
 */
	int i, n;
	char token[MAX_LENGTH];

	if (save.solution == TRUE)
	{
		sprintf(token, "Solution after simulation %d.", simulation);
		description_x = token;
		n = save.n_solution_user;
		xsolution_save(n);
		for (i = save.n_solution_user + 1; i <= save.n_solution_user_end; i++)
		{
			Utilities::Rxn_copy(Rxn_solution_map, n, i);
		}
	}
	if (save.pp_assemblage == TRUE)
	{
		n = save.n_pp_assemblage_user;
		xpp_assemblage_save(n);
		Utilities::Rxn_copies(Rxn_pp_assemblage_map, save.n_pp_assemblage_user, save.n_pp_assemblage_user_end);
	}
	if (save.exchange == TRUE)
	{
		n = save.n_exchange_user;
		xexchange_save(n);
		for (i = save.n_exchange_user + 1; i <= save.n_exchange_user_end; i++)
		{
			Utilities::Rxn_copy(Rxn_exchange_map, n, i);
		}
	}
	if (save.surface == TRUE)
	{
		n = save.n_surface_user;
		xsurface_save(n);
		Utilities::Rxn_copies(Rxn_surface_map, n, save.n_surface_user_end);
	}
	if (save.gas_phase == TRUE)
	{
		n = save.n_gas_phase_user;
		xgas_save(n);
		for (i = save.n_gas_phase_user + 1; i <= save.n_gas_phase_user_end;
			 i++)
		{
			Utilities::Rxn_copy(Rxn_gas_phase_map, n, i);
		}
	}
	if (save.ss_assemblage == TRUE)
	{
		n = save.n_ss_assemblage_user;
		xss_assemblage_save(n);
		Utilities::Rxn_copies(Rxn_ss_assemblage_map, save.n_ss_assemblage_user, save.n_ss_assemblage_user_end);
	}
	if (save.kinetics == TRUE && use.Get_kinetics_in() == TRUE
	    /*&& use.Get_kinetics_ptr() != NULL */)
	{
		if (state == TRANSPORT || state == PHAST || state == ADVECTION)
		{
			use.Set_kinetics_ptr(Utilities::Rxn_find(Rxn_kinetics_map, use.Get_n_kinetics_user()));
		}
		else if (use.Get_kinetics_in() != FALSE)
		{
			use.Set_kinetics_ptr(Utilities::Rxn_find(Rxn_kinetics_map, -2));
		}
		if (use.Get_kinetics_ptr() != NULL)
		{
			n = use.Get_kinetics_ptr()->Get_n_user();
			for (i = save.n_kinetics_user; i <= save.n_kinetics_user_end; i++)
			{
				Utilities::Rxn_copy(Rxn_kinetics_map, n, i);
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
xexchange_save(int n_user)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save exchanger assemblage into structure exchange with user
 *   number n_user.
 */
	size_t i, j;
	char token[MAX_LENGTH];

	LDBLE charge;
	if (use.Get_exchange_ptr() == NULL)
		return (OK);

	cxxExchange temp_exchange = *use.Get_exchange_ptr();
/*
 *   Store data for structure exchange
 */
	temp_exchange.Set_n_user(n_user);
	temp_exchange.Set_n_user_end(n_user);
	temp_exchange.Set_new_def(false);
	sprintf(token, "Exchange assemblage after simulation %d.", simulation);
	temp_exchange.Set_description(token);
	temp_exchange.Set_solution_equilibria(false);
	temp_exchange.Set_n_solution(-999);
	temp_exchange.Get_exchange_comps().clear();

/*
 *   Write exch_comp structure for each exchange component
 */
	for (i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type == EXCH)
		{
			const cxxExchComp *comp_ptr = use.Get_exchange_ptr()->Find_comp(x[i]->exch_comp);
			if (!comp_ptr)
			{
				assert(false);
			}
			cxxExchComp xcomp = *comp_ptr;
			xcomp.Set_la(x[i]->master[0]->s->la);
/*
 *   Save element concentrations on exchanger
 */
			count_elts = 0;
			paren_count = 0;
			charge = 0.0;
			for (j = 0; j < species_list.size(); j++)
			{
				if (species_list[j].master_s == x[i]->master[0]->s)
				{
					add_elt_list(species_list[j].s->next_elt,
								 species_list[j].s->moles);
					charge += species_list[j].s->moles * species_list[j].s->z;
				}
			}
/*
 *   Keep exchanger related to phase even if none currently in solution
 */
			if (xcomp.Get_phase_name().size() != 0 && count_elts == 0)
			{
				add_elt_list(x[i]->master[0]->s->next_elt, 1e-20);
			}
/*
 *   Store list
 */
			xcomp.Set_charge_balance(charge);

			xcomp.Set_totals(elt_list_NameDouble());
/* debug
                        output_msg(sformatf( "Exchange charge_balance: %e\n", charge));
 */
			/* update unknown pointer */
			temp_exchange.Get_exchange_comps().push_back(xcomp);
		}
	}
/*
 *   Finish up
 */
	Rxn_exchange_map[n_user] = temp_exchange;

	use.Set_exchange_ptr(NULL);
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
xgas_save(int n_user)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Save gas composition into structure gas_phase with user
	 *   number n_user.
	 */
	char token[MAX_LENGTH];

	if (use.Get_gas_phase_ptr() == NULL)
		return (OK);
	cxxGasPhase* gas_phase_ptr = use.Get_gas_phase_ptr();
	cxxGasPhase temp_gas_phase(*gas_phase_ptr);
	/*
	 *   Store in gas_phase
	 */
	temp_gas_phase.Set_n_user(n_user);
	temp_gas_phase.Set_n_user_end(n_user);
	sprintf(token, "Gas phase after simulation %d.", simulation);
	temp_gas_phase.Set_description(token);
	temp_gas_phase.Set_new_def(false);
	temp_gas_phase.Set_solution_equilibria(false);
	temp_gas_phase.Set_n_solution(-99);
	/*
	 *   Update amounts
	 */
	bool PR = false;
	if (gas_phase_ptr->Get_v_m() >= 0.01) PR = true;
	for (size_t i = 0; i < temp_gas_phase.Get_gas_comps().size(); i++)
	{
		cxxGasComp* gc_ptr = &(temp_gas_phase.Get_gas_comps()[i]);
		int k;
		class phase* phase_ptr = phase_bsearch(gc_ptr->Get_phase_name().c_str(), &k, FALSE);
		assert(phase_ptr);
		if (PR)
		{
			gc_ptr->Set_moles(phase_ptr->moles_x);
			gc_ptr->Set_p(phase_ptr->p_soln_x);
			gc_ptr->Set_phi(phase_ptr->pr_phi);
			gc_ptr->Set_f(phase_ptr->p_soln_x * phase_ptr->pr_phi);
		}
		else
		{
			gc_ptr->Set_moles(phase_ptr->moles_x);
			gc_ptr->Set_p(phase_ptr->p_soln_x);
			gc_ptr->Set_phi(1.0);
			gc_ptr->Set_f(phase_ptr->p_soln_x);
		}
	}
	Rxn_gas_phase_map[n_user] = temp_gas_phase;

	use.Set_gas_phase_ptr(NULL);
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
xss_assemblage_save(int n_user)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save ss_assemblage composition into structure ss_assemblage with user
 *   number n_user.
 */
	cxxSSassemblage temp_ss_assemblage(this->phrq_io);

	if (use.Get_ss_assemblage_ptr() == NULL)
		return (OK);
/*
 *   Set ss_assemblage
 */
	temp_ss_assemblage.Set_n_user(n_user);
	temp_ss_assemblage.Set_n_user_end(n_user);
	std::ostringstream msg;
	msg << "Solid solution assemblage after simulation " << simulation;
	temp_ss_assemblage.Set_description(msg.str().c_str());
	temp_ss_assemblage.Set_new_def(false);
	temp_ss_assemblage.Set_SSs(use.Get_ss_assemblage_ptr()->Get_SSs());

	std::vector<cxxSS *> ss_ptrs = temp_ss_assemblage.Vectorize();
	for (size_t i = 0; i < ss_ptrs.size(); i++)
	{
		cxxSS *ss_ptr = ss_ptrs[i];
		/* set initial moles for quick setup */
		for (size_t j = 0; j < ss_ptr->Get_ss_comps().size(); j++)
		{
			cxxSScomp *comp_ptr = &(ss_ptr->Get_ss_comps()[j]);
			comp_ptr->Set_initial_moles(comp_ptr->Get_moles());
		}
	}
/*
 *   Finish up
 */
	Rxn_ss_assemblage_map[n_user] = temp_ss_assemblage;

	use.Set_ss_assemblage_ptr(NULL);
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
xpp_assemblage_save(int n_user)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save pure_phase assemblage into instance of cxxPPassemblage with user
 *   number n_user.
 */
	std::string token;
	cxxPPassemblage * pp_assemblage_ptr = use.Get_pp_assemblage_ptr();
	if (use.Get_pp_assemblage_ptr() == NULL)
		return (OK);

	cxxPPassemblage temp_pp_assemblage(*pp_assemblage_ptr);

	temp_pp_assemblage.Set_n_user(n_user);
	temp_pp_assemblage.Set_n_user_end(n_user);
	std::ostringstream desc;
	desc << "Pure-phase assemblage after simulation " << simulation << ".";
	temp_pp_assemblage.Set_description(desc.str().c_str());
	temp_pp_assemblage.Set_new_def(false);
/*
 *   Update amounts
 */
	for (int j = 0; j < count_unknowns; j++)
	{
		if (x[j]->type != PP)
			continue;
		cxxPPassemblageComp *comp = temp_pp_assemblage.Find(x[j]->pp_assemblage_comp_name);
		comp->Set_moles(x[j]->moles);
		comp->Set_delta(0.0);
	}
/*
 *   Finish up
 */

	Rxn_pp_assemblage_map[n_user] = temp_pp_assemblage;
	use.Set_pp_assemblage_ptr(NULL);
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
xsolution_save(int n_user)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save solution composition into structure solution with user number
 *   n_user.
 *
 *   input:  n_user is user solution number of target
 */
	class master *master_i_ptr, *master_ptr;
/*
 *   Malloc space for solution data
 */
	cxxSolution temp_solution;
	temp_solution.Set_n_user_both(n_user);
	temp_solution.Set_new_def(false);
	temp_solution.Set_description(description_x);
	temp_solution.Set_tc(tc_x);
	temp_solution.Set_patm(patm_x);
	temp_solution.Set_potV(potV_x);
	temp_solution.Set_ph(ph_x);
	temp_solution.Set_pe(solution_pe_x);
	temp_solution.Set_mu(mu_x);
	temp_solution.Set_ah2o(ah2o_x);
	//temp_solution.Set_density(density_x);
	temp_solution.Set_density(calc_dens());
	temp_solution.Set_total_h(total_h_x);
	temp_solution.Set_total_o(total_o_x);
	temp_solution.Set_cb(cb_x);	/* cb_x does not include surface charge sfter sum_species */
								/* does include surface charge after step */
	temp_solution.Set_mass_water(mass_water_aq_x);
	temp_solution.Set_total_alkalinity(total_alkalinity);
	temp_solution.Set_soln_vol(this->calc_solution_volume());
/*
 *   Copy pe data
 */
	/*
	 * Add in minor isotopes if initial solution calculation
	 */
	if (initial_solution_isotopes == TRUE)
	{
		for (int i = 0; i < (int)master_isotope.size(); i++)
		{
			if (master_isotope[i]->moles > 0)
			{
				master_i_ptr = master_bsearch(master_isotope[i]->name);
				master_ptr = master_isotope[i]->elt->master;
				if (master_isotope[i]->minor_isotope == TRUE)
				{
					master_i_ptr->total = master_isotope[i]->moles;
					if (master_ptr->total > 0)
					{
						master_i_ptr->s->la =
							master_ptr->s->la +
							log10(master_i_ptr->total / master_ptr->total);
					}
					else
					{
						master_i_ptr->s->la = master_ptr->s->la;
					}
				}
				else if (master_isotope[i]->minor_isotope == FALSE
						 && master_ptr->s != s_hplus
						 && master_ptr->s != s_h2o)
				{
					if (master_ptr->s->secondary != NULL)
					{
						master_ptr->s->secondary->total =
							master_isotope[i]->moles;
					}
					else
					{
						master_ptr->s->primary->total =
							master_isotope[i]->moles;
					}
				}
			}
		}
	}
/*
 *   Copy totals data
 */
	for (int i = 0; i < (int)master.size(); i++)
	{
		if (master[i]->s->type == EX ||
			master[i]->s->type == SURF || master[i]->s->type == SURF_PSI)
			continue;
		if (master[i]->s == s_hplus)
			continue;
		if (master[i]->s == s_h2o)
			continue;
/*
 *   Save list of log activities
 */
		if (master[i]->in != FALSE)
		{
			temp_solution.Get_master_activity()[master[i]->elt->name] = master[i]->s->la;
		}
		if (master[i]->total <= MIN_TOTAL)
		{
			master[i]->total = 0.0;
			master[i]->total_primary = 0.0;
			continue;
		}
/*
 *   Save list of concentrations
 */
		temp_solution.Get_totals()[master[i]->elt->name] = master[i]->total;
	}
	if (pitzer_model == TRUE || sit_model == TRUE)
	{
		for (int j = 0; j < (int)this->s_x.size(); j++)
		{
			if (s_x[j]->lg != 0.0)
			{
				temp_solution.Get_species_gamma()[s_x[j]->name] = s_x[j]->lg;
			}
		}
	}
/*
 *   Save isotope data
 */
	temp_solution.Set_isotopes(isotopes_x);
	std::map< std::string, cxxSolutionIsotope >::iterator it;
	for (it = temp_solution.Get_isotopes().begin(); it != temp_solution.Get_isotopes().end(); it++)
	{
		class master *iso_master_ptr = master_bsearch(it->second.Get_elt_name().c_str());
		if (iso_master_ptr != NULL)
		{
			it->second.Set_total(iso_master_ptr->total);
			if (iso_master_ptr == s_hplus->secondary)
			{
				it->second.Set_total(2 * mass_water_aq_x / gfw_water);
			}
			if (iso_master_ptr == s_h2o->secondary)
			{
				it->second.Set_total(mass_water_aq_x / gfw_water);
			}
		}
		else
		{
			error_string = sformatf("Ignoring failed attempt to interpret %s as an isotope of element %s.",
				it->second.Get_isotope_name().c_str(), it->second.Get_elt_name().c_str());
			warning_msg(error_string);
		}
	}
   if (this->save_species)
   {
	   // saves mol/L
	   temp_solution.Get_species_map().clear();
	   for (int i = 0; i < (int)this->s_x.size(); i++)
	   {
		   if (s_x[i]->type <= H2O)
		   {
			   temp_solution.Get_species_map()[s_x[i]->number] = s_x[i]->moles / temp_solution.Get_soln_vol();
		   }
	   }	 
	   // saves gamma
	   temp_solution.Get_log_gamma_map().clear();
	   for (int i = 0; i < (int)this->s_x.size(); i++)
	   {
		   if (s_x[i]->type <= H2O)
		   {
			   temp_solution.Get_log_gamma_map()[s_x[i]->number] = s_x[i]->lg;
		   }
	   }
	   // saves molalities
	   temp_solution.Get_log_molalities_map().clear();
	   for (int i = 0; i < (int)this->s_x.size(); i++)
	   {
		   if (s_x[i]->type <= H2O)
		   {
			   temp_solution.Get_log_molalities_map()[s_x[i]->number] = s_x[i]->lm;
		   }
	   }
   }
/*
 *   Save solution
 */
	Rxn_solution_map[n_user] = temp_solution;
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
xsurface_save(int n_user)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save surface data into structure surface with user
 *   number n_user.
 */
	LDBLE charge;
	if (use.Get_surface_ptr() == NULL)
		return (OK);
/*
 *   Store data for structure surface
 */
	cxxSurface temp_surface = *use.Get_surface_ptr();
	temp_surface.Set_n_user(n_user);
	temp_surface.Set_n_user_end(n_user);
	temp_surface.Set_new_def(false);
	temp_surface.Set_dl_type(dl_type_x);
	temp_surface.Set_solution_equilibria(false);
	temp_surface.Set_n_solution(-999);

	if (temp_surface.Get_type() == cxxSurface::NO_EDL)
	{
		temp_surface.Get_surface_charges().clear();
	}
/*
 *   Write surface_comp structure for each surf component into comps_ptr
 */
	/*
	 *  Initial entry of surface sites is random
	 *  Charge balance numbering follows the initial entry
	 *  Surface sites are then sorted alphabetically
	 *  Now when we save, the site order differs from the charge order
	 *  last_charge sets up logic to renumber charge balance equations.
	 */
	for (int i = 0; i < count_unknowns; i++)
	{
		if (x[i]->type == SURFACE)
		{
			cxxSurfaceComp *comp_ptr = temp_surface.Find_comp(x[i]->surface_comp);
			if (comp_ptr == NULL)
				continue; // appt in transport with different mobile and stagnant surfaces
			assert(comp_ptr);
			comp_ptr->Set_la(x[i]->master[0]->s->la);
			comp_ptr->Set_moles(0.);
/*
 *   Save element concentrations on surface
 */
			count_elts = 0;
			paren_count = 0;
			charge = 0.0;
			for (int j = 0; j < (int)species_list.size(); j++)
			{
				if (species_list[j].master_s == x[i]->master[0]->s)
				{
					add_elt_list(species_list[j].s->next_elt,
								 species_list[j].s->moles);
					//add_elt_list_multi_surf(species_list[j].s->next_elt,
					//			 species_list[j].s->moles, x[i]->master[0]->elt);
					charge += species_list[j].s->moles * species_list[j].s->z;
				}
			}
			{
				cxxNameDouble nd = elt_list_NameDouble();
				comp_ptr->Set_totals(nd);
			}
			comp_ptr->Set_charge_balance(charge);
		}
		else if (x[i]->type == SURFACE_CB && (use.Get_surface_ptr()->Get_type() == cxxSurface::DDL || use.Get_surface_ptr()->Get_type() == cxxSurface::CCM))
		{
			cxxSurfaceCharge *charge_ptr = temp_surface.Find_charge(x[i]->surface_charge);
			if (charge_ptr == NULL)
				continue; // appt in transport with different mobile and stagnant surfaces
			assert(charge_ptr);
			charge_ptr->Set_charge_balance(x[i]->f);
			charge_ptr->Set_la_psi(x[i]->master[0]->s->la);
/*
 *   Store moles from diffuse_layer
 */
			if (dl_type_x != cxxSurface::NO_DL)
			{
				sum_diffuse_layer(charge_ptr);
				cxxNameDouble nd = elt_list_NameDouble();
				charge_ptr->Set_diffuse_layer_totals(nd);
			}
		}
		else if (x[i]->type == SURFACE_CB
			&& use.Get_surface_ptr()->Get_type() == cxxSurface::CD_MUSIC)
		{
			cxxSurfaceCharge *charge_ptr = temp_surface.Find_charge(x[i]->surface_charge);
			assert(charge_ptr);
			if (dl_type_x != cxxSurface::NO_DL)
			{
				charge_ptr->Set_charge_balance(
					(charge_ptr->Get_sigma0() +
					 charge_ptr->Get_sigma1() +
					 charge_ptr->Get_sigma2() +
					 charge_ptr->Get_sigmaddl())
					* (charge_ptr->Get_specific_area() *
					   charge_ptr->Get_grams()) / F_C_MOL);
			}
			else
			{
				charge_ptr->Set_charge_balance(
					(charge_ptr->Get_sigma0() +
					 charge_ptr->Get_sigma1() +
					 charge_ptr->Get_sigma2())
					* (charge_ptr->Get_specific_area() *
					   charge_ptr->Get_grams()) / F_C_MOL);
			}
			charge_ptr->Set_la_psi(x[i]->master[0]->s->la);
/*
 *   Store moles from diffuse_layer
 */
			if (dl_type_x != cxxSurface::NO_DL)
			{
				sum_diffuse_layer(charge_ptr);
				cxxNameDouble nd = elt_list_NameDouble();
				charge_ptr->Set_diffuse_layer_totals(nd);
			}
		}
	}
	if (!(dl_type_x == cxxSurface::NO_DL))
	{
		cxxSurface *surface_ptr = &temp_surface;
		for (size_t i = 0; i < surface_ptr->Get_surface_charges().size(); i++)
		{
			cxxSurfaceCharge & charge_ref = surface_ptr->Get_surface_charges()[i];
			double mass_water_surface = charge_ref.Get_mass_water();
			for (int j = 0; j < (int)this->s_x.size(); j++)
			{
				if (s_x[j]->type > H2O)
					continue;
				double molality = under(s_x[j]->lm);
				double moles_excess = mass_water_aq_x * molality * charge_ref.Get_g_map()[s_x[j]->z].Get_g();
				double moles_surface = mass_water_surface * molality + moles_excess;
				charge_ref.Get_dl_species_map()[s_x[j]->number] = moles_surface/mass_water_surface;
				double g = charge_ref.Get_g_map()[s_x[j]->z].Get_g();
				//double moles_excess = mass_water_aq_x * molality * (g * s_x[j]->erm_ddl +
				//	mass_water_surface /
				//	mass_water_aq_x * (s_x[j]->erm_ddl - 1));

				//LDBLE g = charge_ptr->Get_g_map()[s_x[j]->z].Get_g();
				if (s_x[j]->erm_ddl != 1)
				{
					LDBLE ratio_aq = mass_water_surface / mass_water_aq_x;
					LDBLE g2 = g / ratio_aq + 1;
					g = ratio_aq * (g2 * s_x[j]->erm_ddl - 1);
				}
				moles_excess = mass_water_aq_x * molality * g;
				double c = (mass_water_surface * molality + moles_excess) / mass_water_surface;
				charge_ref.Get_dl_species_map()[s_x[j]->number] = c;
			}
			//charge_ref.Get_dl_species_map()[s_h2o->number] = 0.0;
			charge_ref.Get_dl_species_map()[s_h2o->number] = 1.0/gfw_water;
		}
	} 

/*
 *   Finish up
 */
	Rxn_surface_map[n_user] = temp_surface;
	use.Set_surface_ptr(NULL);
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
copy_use(int i)
/* ---------------------------------------------------------------------- */
{
/*
 *   Find mixture
 */
	if (use.Get_mix_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_mix_map, use.Get_n_mix_user(), i);
	}
/*
 *   Find solution
 */
	if (use.Get_solution_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_solution_map, use.Get_n_solution_user(), i);
	}
/*
 *   Always save solution to i, mixing or not
 */
	save.solution = TRUE;
	save.n_solution_user = i;
	save.n_solution_user_end = i;
/*
 *   Find pure phase assemblage
 */
	if (use.Get_pp_assemblage_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_pp_assemblage_map, use.Get_n_pp_assemblage_user(), i);
		save.pp_assemblage = TRUE;
		save.n_pp_assemblage_user = i;
		save.n_pp_assemblage_user_end = i;
	}
	else
	{
		save.pp_assemblage = FALSE;
	}
/*
 *   Find irrev reaction
 */
	if (use.Get_reaction_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_reaction_map, use.Get_n_reaction_user(), i);
		save.reaction = TRUE;
		save.n_reaction_user = i;
		save.n_reaction_user_end = i;
	}
	else
	{
		save.reaction = FALSE;
	}
/*
 *   Find exchange
 */
	if (use.Get_exchange_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_exchange_map, use.Get_n_exchange_user(), i);
		save.exchange = TRUE;
		save.n_exchange_user = i;
		save.n_exchange_user_end = i;
	}
	else
	{
		save.exchange = FALSE;
	}
/*
 *   Find kinetics
 */
	if (use.Get_kinetics_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_kinetics_map, use.Get_n_kinetics_user(), i);
		save.kinetics = TRUE;
		save.n_kinetics_user = i;
		save.n_kinetics_user_end = i;
	}
	else
	{
		save.kinetics = FALSE;
	}
/*
 *   Find surface
 */
	dl_type_x = cxxSurface::NO_DL;
	if (use.Get_surface_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_surface_map, use.Get_n_surface_user(), i);
		save.surface = TRUE;
		save.n_surface_user = i;
		save.n_surface_user_end = i;
	}
	else
	{
		save.surface = FALSE;
	}
/*
 *   Find temperature
 */
	if (use.Get_temperature_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_temperature_map, use.Get_n_temperature_user(), i);
	}
/*
 *   Find pressure
 */
	if (use.Get_pressure_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_pressure_map, use.Get_n_pressure_user(), i);
	}
/*
 *   Find gas
 */
	if (use.Get_gas_phase_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_gas_phase_map, use.Get_n_gas_phase_user(), i);
		save.gas_phase = TRUE;
		save.n_gas_phase_user = i;
		save.n_gas_phase_user_end = i;
	}
	else
	{
		save.gas_phase = FALSE;
	}
/*
 *   Find solid solution
 */
	if (use.Get_ss_assemblage_in() == TRUE)
	{
		Utilities::Rxn_copy(Rxn_ss_assemblage_map, use.Get_n_ss_assemblage_user(), i);
		save.ss_assemblage = TRUE;
		save.n_ss_assemblage_user = i;
		save.n_ss_assemblage_user_end = i;
	}
	else
	{
		save.ss_assemblage = FALSE;
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
step_save_exch(int n_user)
/* ---------------------------------------------------------------------- */
{
/*
 *   Save exchange composition
 *
 *   input:  n_user is user exchange number of target
 */

	if (use.Get_exchange_ptr() == NULL)
		return (OK);

	cxxExchange *temp_ptr = Utilities::Rxn_find(Rxn_exchange_map, use.Get_n_exchange_user());
	assert(temp_ptr);

	// Set all totals to 0.0
	cxxExchange temp_exchange = *temp_ptr;
	{
		for (size_t i = 0; i < temp_exchange.Get_exchange_comps().size(); i++)
		{
			temp_exchange.Get_exchange_comps()[i].Get_totals().multiply(0.0);
		}
	}

	// Set exchange total in one component
	for (int i = 0; i < (int)master.size(); i++)
	{
		if (master[i]->s->type != EX)
			continue;
		std::string e(master[i]->elt->name);
		for (size_t j = 0; j < temp_exchange.Get_exchange_comps().size(); j++)
		{
			cxxNameDouble *nd = &(temp_exchange.Get_exchange_comps()[j].Get_totals());
			cxxNameDouble::iterator nd_it = nd->find(e);
			if (nd_it != nd->end())
			{
				LDBLE coef;
				if (master[i]->total <= MIN_TOTAL)
				{
					coef = MIN_TOTAL;
				}
				else
				{
					coef = master[i]->total;
				}
				nd->insert(nd_it->first.c_str(), coef);
				break;
			}
		}
	}

	Rxn_exchange_map[n_user] = temp_exchange;
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
step_save_surf(int n_user)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Save surface for intermediate calculation
	 *   Amt of surface may have changed due to reaction or surface related
	 *   to kinetic reactant.
	 *
	 *   input:  n_user is user solution number of target
	 */
	if (use.Get_surface_ptr() == NULL)
		return (OK);
	Utilities::Rxn_copy(Rxn_surface_map, use.Get_surface_ptr()->Get_n_user(), n_user);
	cxxSurface *surface_ptr = Utilities::Rxn_find(Rxn_surface_map, n_user);
	for (int i = 0; i < (int)master.size(); i++)
	{
		if (master[i]->s->type != SURF)
			continue;
		for (size_t j = 0; j < surface_ptr->Get_surface_comps().size(); j++)
		{
			cxxSurfaceComp * comp_ptr = &(surface_ptr->Get_surface_comps()[j]);
			cxxNameDouble & totals = comp_ptr->Get_totals();
			if (totals.find(master[i]->elt->name) == totals.end())
			{
				continue;
			}
			else
			{
				LDBLE coef = master[i]->total;
				if (master[i]->total <= MIN_TOTAL)
				{
					coef = MIN_TOTAL;
				}
				totals[master[i]->elt->name] = coef;
				break;
			}
		}
	}
	/*
	 *   Update grams
	 */
	if ((surface_ptr->Get_type() == cxxSurface::DDL || surface_ptr->Get_type() == cxxSurface::CCM || surface_ptr->Get_type() == cxxSurface::CD_MUSIC)
		&& surface_ptr->Get_related_rate() && use.Get_kinetics_ptr() != NULL)
	{
		for (size_t j = 0; j < surface_ptr->Get_surface_comps().size(); j++)
		{
			cxxSurfaceComp *surface_comp_ptr = &(surface_ptr->Get_surface_comps()[j]);
			if (surface_comp_ptr->Get_rate_name().size() > 0)
			{
				cxxKinetics *kinetics_ptr = use.Get_kinetics_ptr();
				for (size_t m = 0; m < kinetics_ptr->Get_kinetics_comps().size(); m++)
				{
					cxxKineticsComp * kinetics_comp_ptr = &(kinetics_ptr->Get_kinetics_comps()[m]);
					if (strcmp_nocase
						(kinetics_comp_ptr->Get_rate_name().c_str(),
						surface_comp_ptr->Get_rate_name().c_str()) != 0)
						continue;
					cxxSurfaceCharge *charge_ptr = surface_ptr->Find_charge(surface_comp_ptr->Get_charge_name());
					charge_ptr->Set_grams(kinetics_comp_ptr->Get_m());
					break;
				}
			}
		}
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
copy_entities(void)
/* ---------------------------------------------------------------------- */
{
	int return_value;
	return_value = OK;
	for (size_t j = 0; j < copy_solution.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_solution_map, copy_solution.n_user[j]) != NULL)
		{
			for (size_t i = copy_solution.start[j]; i <= copy_solution.end[j]; i++)
			{
				if (i == copy_solution.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_solution_map, copy_solution.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_solution);

	for (size_t j = 0; j < copy_pp_assemblage.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_pp_assemblage_map, copy_pp_assemblage.n_user[j]) != NULL)
		{
			for (size_t i = copy_pp_assemblage.start[j]; i <= copy_pp_assemblage.end[j]; i++)
			{
				if (i == copy_pp_assemblage.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_pp_assemblage_map, copy_pp_assemblage.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_pp_assemblage);

	for (size_t j = 0; j < copy_reaction.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_reaction_map, copy_reaction.n_user[j]) != NULL)
		{
			for (size_t i = copy_reaction.start[j]; i <= copy_reaction.end[j]; i++)
			{
				if (i == copy_reaction.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_reaction_map, copy_reaction.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_reaction);

	for (size_t j = 0; j < copy_mix.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_mix_map, copy_mix.n_user[j]) != NULL)
		{
			for (size_t i = copy_mix.start[j]; i <= copy_mix.end[j]; i++)
			{
				if (i != copy_mix.n_user[j])
				{
					Utilities::Rxn_copy(Rxn_mix_map, copy_mix.n_user[j], (int)i);
				}
			}
		}
	}
	copier_clear(&copy_mix);

	for (size_t j = 0; j < copy_exchange.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_exchange_map, copy_exchange.n_user[j]) != NULL)
		{
			for (size_t i = copy_exchange.start[j]; i <= copy_exchange.end[j]; i++)
			{
				if (i == copy_exchange.n_user[j]) continue;
				Utilities::Rxn_copy(Rxn_exchange_map, copy_exchange.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_exchange);

	for (size_t j = 0; j < copy_surface.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_surface_map, copy_surface.n_user[j]) != NULL)
		{
			for (size_t i = copy_surface.start[j]; i <= copy_surface.end[j]; i++)
			{
				if (i == copy_surface.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_surface_map, copy_surface.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_surface);

	for (size_t j = 0; j < copy_temperature.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_temperature_map, copy_temperature.n_user[j]) != NULL)
		{
			for (size_t i = copy_temperature.start[j]; i <= copy_temperature.end[j]; i++)
			{
				if (i != copy_temperature.n_user[j])
				{
					Utilities::Rxn_copy(Rxn_temperature_map, copy_temperature.n_user[j], (int)i);
				}
			}
		}
	}
	copier_clear(&copy_temperature);

	for (size_t j = 0; j < copy_pressure.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_pressure_map, copy_pressure.n_user[j]) != NULL)
		{
			for (size_t i = copy_pressure.start[j]; i <= copy_pressure.end[j]; i++)
			{
				if (i != copy_pressure.n_user[j])
				{
					Utilities::Rxn_copy(Rxn_pressure_map, copy_pressure.n_user[j], (int)i);
				}
			}
		}
	}
	copier_clear(&copy_pressure);

	for (size_t j = 0; j < copy_gas_phase.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_gas_phase_map, copy_gas_phase.n_user[j]) != NULL)
		{
			for (size_t i = copy_gas_phase.start[j]; i <= copy_gas_phase.end[j]; i++)
			{
				if (i == copy_gas_phase.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_gas_phase_map, copy_gas_phase.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_gas_phase);

	for (size_t j = 0; j < copy_kinetics.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_kinetics_map, copy_kinetics.n_user[j]) != NULL)
		{
			for (size_t i = copy_kinetics.start[j]; i <= copy_kinetics.end[j]; i++)
			{
				if (i == copy_kinetics.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_kinetics_map, copy_kinetics.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_kinetics);

	for (size_t j = 0; j < copy_ss_assemblage.n_user.size(); j++)
	{
		if (Utilities::Rxn_find(Rxn_ss_assemblage_map, copy_ss_assemblage.n_user[j]) != NULL)
		{
			for (size_t i = copy_ss_assemblage.start[j]; i <= copy_ss_assemblage.end[j]; i++)
			{
				if (i == copy_ss_assemblage.n_user[j])
					continue;
				Utilities::Rxn_copy(Rxn_ss_assemblage_map, copy_ss_assemblage.n_user[j], (int)i);
			}
		}
	}
	copier_clear(&copy_ss_assemblage);

	new_copy = FALSE;
	return return_value;
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_database(void)
/* ---------------------------------------------------------------------- */
{
	simulation = 0;

/*
 *   Prepare error handling
 */
	try
	{
		set_reading_database(TRUE);
		dup_print("Reading data base.", TRUE);
		read_input();
		tidy_model();
		status(0, NULL);
	}
	catch (const PhreeqcStop&)
	{
		return get_input_errors();
	}
	set_reading_database(FALSE);
	return 0;
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
run_simulations(void)
/* ---------------------------------------------------------------------- */
{
	char token[MAX_LENGTH];
#if defined(_MSC_VER) && (_MSC_VER < 1900)  // removed in vs2015
	unsigned int old_exponent_format;
	old_exponent_format = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
/*
 *   Prepare error handling
 */
	try
	{
/*
 *   Read input data for simulation
 */
		for (simulation = 1;; simulation++)
		{
#ifdef TEST_COPY_OPERATOR
			{
				//int simulation_save = simulation;
				Phreeqc phreeqc_new;
				phreeqc_new = *this;
				PHRQ_io *temp_io = this->phrq_io;
				std::vector<std::ostream *> so_ostreams;
				{
					std::map<int, SelectedOutput>::iterator so_it = this->SelectedOutput_map.begin();
					for (; so_it != this->SelectedOutput_map.end(); so_it++)
					{
						so_ostreams.push_back(so_it->second.Get_punch_ostream());
						so_it->second.Set_punch_ostream(NULL);
					}
				}
				this->clean_up();
				this->init();
				this->initialize();
				this->phrq_io = temp_io;
				this->InternalCopy(&phreeqc_new);		
				{
					size_t i = 0;
					std::map<int, SelectedOutput>::iterator so_it = this->SelectedOutput_map.begin();
					for (; so_it != this->SelectedOutput_map.end(); so_it++)
					{
						so_it->second.Set_punch_ostream(so_ostreams[i++]);
					}
				}
				//this->simulation = simulation_save;
				//delete phreeqc_new.Get_phrq_io();
			}
#endif
#if defined PHREEQ98
			AddSeries = !connect_simulations;
#endif

#if defined PHREEQCI_GUI
			sprintf(token, "\nSimulation %d\n", simulation);
			screen_msg(token);
#endif
			sprintf(token, "Reading input data for simulation %d.", simulation);

			dup_print(token, TRUE);
			if (read_input() == EOF)
				break;

			if (title_x.size() > 0)
			{
				sprintf(token, "TITLE");
				dup_print(token, TRUE);
				if (pr.headings == TRUE)
				{
					output_msg(sformatf("%s\n\n", title_x.c_str()));
				}
			}
			tidy_model();
/*
 *   Calculate distribution of species for initial solutions
 */
			if (new_solution)
			{
				initial_solutions(TRUE);
			}

/*
 *   Calculate distribution for exchangers
 */
			if (new_exchange)
				initial_exchangers(TRUE);
/*
 *   Calculate distribution for surfaces
 */
			if (new_surface)
				initial_surfaces(TRUE);
/*
 *   Calculate initial gas composition
 */
			if (new_gas_phase)
				initial_gas_phases(TRUE);
/*
 *   Calculate reactions
 */
			reactions();
/*
 *   Calculate inverse models
 */
			inverse_models();
/*
 *   Calculate advection
 */
			if (use.Get_advect_in())
			{
				dup_print("Beginning of advection calculations.", TRUE);
				advection();
			}
/*
 *   Calculate transport
 */
			if (use.Get_trans_in())
			{
				dup_print("Beginning of transport calculations.", TRUE);
				transport();
			}
/*
 *   run
 */
			run_as_cells();
/*
 *   Calculate mixes
 */
			do_mixes();

/*
 *   Copy
 */
			if (new_copy) copy_entities();
/*
 *   dump
 */
			dump_entities();
/*
 *   delete
 */
			delete_entities();
/*
 *   End of simulation
 */
			dup_print("End of simulation.", TRUE);
			output_flush();
			error_flush();
		}
	}
	catch (const PhreeqcStop&)
	{
		return get_input_errors();
	}
	return 0;
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
do_initialize(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Prepare error handling
 */
	try {

		state = INITIALIZE;

		initialize();
	}
	catch (const PhreeqcStop&)
	{
		return get_input_errors();
	}
	return 0;
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
do_status(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Prepare error handling
 */
	try {

		if (pr.status == TRUE)
		{
			status(0, "\nDone.");
			screen_msg("\n");
		}
		//pr.headings = TRUE; // set in class_main; not set for IPhreeqc
#ifndef TESTING
		LDBLE ext = (double)clock() / CLOCKS_PER_SEC;
		dup_print(sformatf("End of Run after %g Seconds.", ext), TRUE);
		screen_msg(sformatf("\nEnd of Run after %g Seconds.\n", ext));
#endif
// appt this gives output when the charts are active...
		phrq_io->output_flush();
		phrq_io->error_flush();
	}
	catch (const PhreeqcStop&)
	{
		return get_input_errors();
	}
	return 0;
}
void Phreeqc::
save_init(int i)
{
	save.solution = i;
	save.n_solution_user = i;
	save.n_solution_user_end = i;
	save.mix = i;
	save.n_mix_user = i;
	save.n_mix_user_end = i;
	save.reaction = i;
	save.n_reaction_user = i;
	save.n_reaction_user_end = i;
	save.pp_assemblage = i;
	save.n_pp_assemblage_user = i;
	save.n_pp_assemblage_user_end = i;
	save.exchange = i;
	save.n_exchange_user = i;
	save.n_exchange_user_end = i;
	save.kinetics = i;
	save.n_kinetics_user = i;
	save.n_kinetics_user_end = i;
	save.surface = i;
	save.n_surface_user = i;
	save.n_surface_user_end = i;
	save.gas_phase = i;
	save.n_gas_phase_user = i;
	save.n_gas_phase_user_end = i;
	save.ss_assemblage = i;
	save.n_ss_assemblage_user = i;
	save.n_ss_assemblage_user_end = i;
}
void
Phreeqc::do_mixes(void)
{
	bool surf, exch, kin, min;
	surf = (Rxn_surface_mix_map.size() > 0);
	exch = (Rxn_exchange_mix_map.size() > 0);
	kin = (Rxn_kinetics_mix_map.size() > 0);
	min = (Rxn_pp_assemblage_mix_map.size() > 0);
	Utilities::Rxn_mix(Rxn_solution_mix_map, Rxn_solution_map, this);
	Utilities::Rxn_mix(Rxn_exchange_mix_map, Rxn_exchange_map, this);
	Utilities::Rxn_mix(Rxn_gas_phase_mix_map, Rxn_gas_phase_map, this);
	Utilities::Rxn_mix(Rxn_kinetics_mix_map, Rxn_kinetics_map, this);
	Utilities::Rxn_mix(Rxn_pp_assemblage_mix_map, Rxn_pp_assemblage_map, this);
	Utilities::Rxn_mix(Rxn_ss_assemblage_mix_map, Rxn_ss_assemblage_map, this);
	Utilities::Rxn_mix(Rxn_surface_mix_map, Rxn_surface_map, this);
	if (exch || kin) update_kin_exchange();
	if (exch || min) update_min_exchange();
	if (surf || min) update_min_surface();
	if (surf || kin) update_kin_surface();
}
