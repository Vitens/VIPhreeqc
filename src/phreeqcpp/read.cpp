#include <complex>
#include <assert.h>
#include <time.h>

#include "Utils.h"	
#include "Phreeqc.h"
#include "phqalloc.h"
#include "Pressure.h"
#include "Temperature.h"
#include "Parser.h"
#include "cxxMix.h"
#include "Exchange.h"
#include "GasPhase.h"
#include "Reaction.h"
#include "PPassemblage.h"
#include "SSassemblage.h"
#include "cxxKinetics.h"
#include "Solution.h"
#include "SelectedOutput.h"
#include "UserPunch.h"

#if defined(PHREEQCI_GUI)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_input(void)
/* ---------------------------------------------------------------------- */
{
	int i, j, l;
	const char* cptr;
	char token[2 * MAX_LENGTH];
#define LAST_C_KEYWORD 61

	parse_error = 0;
	input_error = 0;
	next_keyword = Keywords::KEY_NONE;
	count_warnings = 0;

	Rxn_new_exchange.clear();
	Rxn_new_gas_phase.clear();
	Rxn_new_kinetics.clear();      // not used
	Rxn_new_mix.clear();           // not used
	Rxn_new_pp_assemblage.clear();
	Rxn_new_pressure.clear();      // not used
	Rxn_new_reaction.clear();      // not used
	Rxn_new_solution.clear();
	Rxn_new_ss_assemblage.clear();
	Rxn_new_surface.clear();
	Rxn_new_temperature.clear();   // not used
	phrq_io->Set_echo_on(true);
/*
 *  Initialize keyword counters
 */
	for (i = 0; i < Keywords::KEY_COUNT_KEYWORDS; i++)
	{
		keycount[i] = 0;
	}
/*
 *  Initialize use and save pointers
 */
	use.init();

	save.solution = FALSE;
	save.mix = FALSE;
	save.reaction = FALSE;
	save.kinetics = FALSE;
	save.pp_assemblage = FALSE;
	save.exchange = FALSE;
	save.surface = FALSE;
	save.gas_phase = FALSE;
	save.ss_assemblage = FALSE;
	title_x.clear(); 

	while ((i =	check_line("Subroutine Read", FALSE, TRUE, TRUE, TRUE)) != KEYWORD)
	{
		/* empty, eof, keyword, print */

		if (i == EOF)
			return (EOF);
		error_string = sformatf(
				"Unknown input, no keyword has been specified.");
		warning_msg(error_string);
	}

	for (;;)
	{
		if (next_keyword > 0)
		{
			if (next_keyword != Keywords::KEY_DATABASE && !reading_database())
			{
				first_read_input = FALSE;
			}
		}
		// mark keyword read
		if (next_keyword > 0 && next_keyword < Keywords::KEY_COUNT_KEYWORDS)
		{
			keycount[next_keyword]++;
		}
		switch (next_keyword)
		{
		case Keywords::KEY_NONE:				/* Have not read line with keyword */
			error_string = sformatf( "Unknown input, no keyword has been specified.");
			warning_msg(error_string);
			while ((j =	check_line("No keyword", FALSE, TRUE, TRUE, TRUE)) != KEYWORD && j != EOF)
			{
				warning_msg(error_string);
			}
			break;
		case Keywords::KEY_END:
			goto END_OF_SIMULATION_INPUT;
		case Keywords::KEY_SOLUTION_SPECIES:				/* Read aqueous model */
			read_species();
			break;
		case Keywords::KEY_SOLUTION_MASTER_SPECIES:		/* Read master species */
			read_master_species();
			break;
		case Keywords::KEY_SOLUTION:				/* Read solution data */
			read_solution();
			break;
		case Keywords::KEY_PHASES:
			read_phases();
			break;
		case Keywords::KEY_EQUILIBRIUM_PHASES:
			read_pp_assemblage();
			break;
		case Keywords::KEY_REACTION:
			read_reaction();
			break;
		case Keywords::KEY_MIX:
			read_mix();
			break;
		case Keywords::KEY_SOLUTION_MIX:
			//read_solution_mix();
			read_entity_mix(Rxn_solution_mix_map);
			break;
		case Keywords::KEY_EXCHANGE_MIX:
			read_entity_mix(Rxn_exchange_mix_map);
			break;
		case Keywords::KEY_GAS_PHASE_MIX:
			read_entity_mix(Rxn_gas_phase_mix_map);
			break;
		case Keywords::KEY_KINETICS_MIX:
			read_entity_mix(Rxn_kinetics_mix_map);
			break;
		case Keywords::KEY_PPASSEMBLAGE_MIX:
			read_entity_mix(Rxn_pp_assemblage_mix_map);
			break;
		case Keywords::KEY_SSASSEMBLAGE_MIX:
			read_entity_mix(Rxn_ss_assemblage_mix_map);
			break;
		case Keywords::KEY_SURFACE_MIX:
			read_entity_mix(Rxn_surface_mix_map);
			break;
		case Keywords::KEY_USE:
			read_use();
			break;
		case Keywords::KEY_SAVE:
			read_save();
			break;
		case Keywords::KEY_EXCHANGE_SPECIES:
			read_exchange_species();
			break;
		case Keywords::KEY_EXCHANGE_MASTER_SPECIES:
			read_exchange_master_species();
			break;
		case Keywords::KEY_EXCHANGE:
			read_exchange();
			break;
		case Keywords::KEY_SURFACE_SPECIES:
			read_surface_species();
			break;
		case Keywords::KEY_SURFACE_MASTER_SPECIES:
			read_surface_master_species();
			break;
		case Keywords::KEY_SURFACE:
			read_surface();
			break;
		case Keywords::KEY_REACTION_TEMPERATURE:
			read_temperature();
			break;
		case Keywords::KEY_INVERSE_MODELING:
			read_inverse();
			break;
		case Keywords::KEY_GAS_PHASE:
			read_gas_phase();
			break;
		case Keywords::KEY_TRANSPORT:
			read_transport();
			break;
		case Keywords::KEY_KNOBS:
			read_debug();
			break;
		case Keywords::KEY_SELECTED_OUTPUT:
			read_selected_output();
			break;
		case Keywords::KEY_PRINT:
			read_print();
			break;
		case Keywords::KEY_TITLE:
			read_title();
			break;
		case Keywords::KEY_ADVECTION:
			read_advection();
			break;
		case Keywords::KEY_KINETICS:
			read_kinetics();
			break;
		case Keywords::KEY_INCREMENTAL_REACTIONS:
			read_incremental_reactions();
			break;
		case Keywords::KEY_RATES:
			read_rates();
			break;
		case Keywords::KEY_SOLUTION_SPREAD:
			read_solution_spread();
			break;
		case Keywords::KEY_USER_PRINT:
			read_user_print();
			break;
		case Keywords::KEY_USER_PUNCH:
			read_user_punch();
			break;
		case Keywords::KEY_SOLID_SOLUTIONS:
			read_solid_solutions();
			break;
		case Keywords::KEY_USER_GRAPH:
#if defined PHREEQ98 
			read_user_graph();
#elif defined MULTICHART
			read_user_graph_handler();
# else
			for (;;)
			{
				j = check_line("Reading user_graph", FALSE, TRUE, TRUE, TRUE);
				if (j == EOF || j == KEYWORD)
				{
					break;
				}
			}
#endif
			break;
		case Keywords::KEY_LLNL_AQUEOUS_MODEL_PARAMETERS:
			read_llnl_aqueous_model_parameters();
			break;
		case Keywords::KEY_DATABASE:
			if (reading_database())
			{
				/* warning_msg("DATABASE is ignored in the database file."); */
			}
			else if (first_read_input == FALSE)
			{
				error_msg("DATABASE must be the first keyword in the input file.", CONTINUE);
				input_error++;
			}
			else
			{
				cptr = line;
				copy_token(token, &cptr, &l);
#if defined(SWIG_SHARED_OBJ)
				warning_msg("DATABASE keyword is ignored by IPhreeqc.");
#else
				user_database = cptr;
				string_trim(user_database);
				if (user_database.size() == 0)
				{
					error_msg("DATABASE file name is missing.", CONTINUE);
					input_error++;
				}
				first_read_input = FALSE;
#endif
			}
			j = check_line("Reading after DATABASE", FALSE, TRUE, TRUE, TRUE);
			break;
		case Keywords::KEY_NAMED_EXPRESSIONS:
			read_named_logk();
			break;
		case Keywords::KEY_ISOTOPES:
			read_isotopes();
			break;
		case Keywords::KEY_CALCULATE_VALUES:
			read_calculate_values();
			break;
		case Keywords::KEY_ISOTOPE_RATIOS:
			read_isotope_ratios();
			break;
		case Keywords::KEY_ISOTOPE_ALPHAS:
			read_isotope_alphas();
			break;
		case Keywords::KEY_COPY:
			read_copy();
			break;
		case Keywords::KEY_PITZER:
			read_pitzer();
			break;
		case Keywords::KEY_SIT:
			read_sit();
			break;
		case Keywords::KEY_SOLUTION_RAW:
			Utilities::Rxn_read_raw(Rxn_solution_map, Rxn_new_solution, this);
			break;
		case Keywords::KEY_EXCHANGE_RAW:		
			Utilities::Rxn_read_raw(Rxn_exchange_map, Rxn_new_exchange, this);
			break;
		case Keywords::KEY_SURFACE_RAW:
			Utilities::Rxn_read_raw(Rxn_surface_map, Rxn_new_surface, this);
			break;
		case Keywords::KEY_EQUILIBRIUM_PHASES_RAW:		
			Utilities::Rxn_read_raw(Rxn_pp_assemblage_map, Rxn_new_pp_assemblage, this);
			break;
		case Keywords::KEY_KINETICS_RAW:
			Utilities::Rxn_read_raw(Rxn_kinetics_map, Rxn_new_kinetics, this);
			break;
		case Keywords::KEY_SOLID_SOLUTIONS_RAW:
			Utilities::Rxn_read_raw(Rxn_ss_assemblage_map, Rxn_new_ss_assemblage, this);
			break;
		case Keywords::KEY_GAS_PHASE_RAW:		
			Utilities::Rxn_read_raw(Rxn_gas_phase_map, Rxn_new_gas_phase, this);
			break;
		case Keywords::KEY_REACTION_RAW:		
			Utilities::Rxn_read_raw(Rxn_reaction_map, Rxn_new_reaction, this);
			break;
		case Keywords::KEY_MIX_RAW:		
			Utilities::Rxn_read_raw(Rxn_mix_map, Rxn_new_mix, this);
			break;
		case Keywords::KEY_REACTION_TEMPERATURE_RAW:
			Utilities::Rxn_read_raw(Rxn_temperature_map, Rxn_new_temperature, this);
			break;
		case Keywords::KEY_DUMP:		
			read_dump();
			break;
		case Keywords::KEY_SOLUTION_MODIFY:
			Utilities::Rxn_read_modify(Rxn_solution_map, Rxn_new_solution, this);
			break;
		case Keywords::KEY_EQUILIBRIUM_PHASES_MODIFY:
			Utilities::Rxn_read_modify(Rxn_pp_assemblage_map, Rxn_new_pp_assemblage, this);
			break;
		case Keywords::KEY_EXCHANGE_MODIFY:
			Utilities::Rxn_read_modify(Rxn_exchange_map, Rxn_new_exchange, this);
			break;
		case Keywords::KEY_SURFACE_MODIFY:
			Utilities::Rxn_read_modify(Rxn_surface_map, Rxn_new_surface, this);
			break;
		case Keywords::KEY_SOLID_SOLUTIONS_MODIFY:
			Utilities::Rxn_read_modify(Rxn_ss_assemblage_map, Rxn_new_ss_assemblage, this);
			break;
		case Keywords::KEY_GAS_PHASE_MODIFY:
			Utilities::Rxn_read_modify(Rxn_gas_phase_map, Rxn_new_gas_phase, this);
			break;
		case Keywords::KEY_KINETICS_MODIFY:
			Utilities::Rxn_read_modify(Rxn_kinetics_map, Rxn_new_kinetics, this);
			break;
		case Keywords::KEY_DELETE:
			read_delete();
			break;
		case Keywords::KEY_RUN_CELLS:
			read_run_cells();
			break;
		case Keywords::KEY_REACTION_MODIFY:
			Utilities::Rxn_read_modify(Rxn_reaction_map, Rxn_new_reaction, this);
			break;
		case Keywords::KEY_REACTION_TEMPERATURE_MODIFY:
			Utilities::Rxn_read_modify(Rxn_temperature_map, Rxn_new_temperature, this);
			break;
		//case LAST_C_KEYWORD + 22:		//reaction_temperature_modify
		//	keyword[LAST_C_KEYWORD + 22].keycount++;
		//	read_reaction_temperature_modify();
		//	break;
		case Keywords::KEY_REACTION_PRESSURE:
			read_reaction_pressure();
			break;
		case Keywords::KEY_REACTION_PRESSURE_RAW:
			Utilities::Rxn_read_raw(Rxn_pressure_map, Rxn_new_pressure, this);
			break;
		case Keywords::KEY_REACTION_PRESSURE_MODIFY:
			Utilities::Rxn_read_modify(Rxn_pressure_map, Rxn_new_pressure, this);
			break;
		default:
			error_msg("Error in keyword switch", STOP);
			break;		  
		}
	}
  END_OF_SIMULATION_INPUT:
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_exchange_species(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read data for exchange species, parse equations
 */
	int i;
	int association;
	char token[MAX_LENGTH];
	const char* cptr;
	class phase *phase_ptr;

	class species *s_ptr;
	const class elt_list *next_elt;
	//LDBLE exchange_coef;
	LDBLE offset;

	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"no_check",				/* 0 */
		"check",				/* 1 */
		"mb",					/* 2 */
		"mass_balance",			/* 3 */
		"log_k",				/* 4 */
		"logk",					/* 5 */
		"delta_h",				/* 6 */
		"deltah",				/* 7 */
		"analytical_expression",	/* 8 */
		"a_e",					/* 9 */
		"ae",					/* 10 */
		"mole_balance",			/* 11 */
		"gamma",				/* 12 */
		"davies",				/* 13 */
		"offset",				/* 14 */
		"llnl_gamma",			/* 15 */
		"add_logk",				/* 16 */
		"add_log_k",			/* 17 */
		"add_constant",			/* 18 */
		"vm"	/* 19, molar volume, must replace delta_v */
	};
	int count_opt_list = 20;

	association = TRUE;
	s_ptr = NULL;
/*
 *   Read eqn from file and call parser
 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in EXCHANGE_SPECIES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* no_check */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->check_equation = FALSE;
			break;
		case 1:				/* check */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->check_equation = TRUE;
			break;
		case 2:				/* mb */
		case 3:				/* mass_balance */
		case 11:				/* mole_balance */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			count_elts = 0;
			paren_count = 0;
			copy_token(token, &next_char, &i);
			s_ptr->mole_balance = string_hsave(token);
			cptr = token;
			get_secondary_in_species(&cptr, 1.0);
			s_ptr->next_secondary.clear();
			s_ptr->next_secondary = elt_list_vsave();
/* debug
			for (i = 0; i < count_elts; i++) {
				output_msg(sformatf("%s\t%f\n", elt_list[i].elt->name,
					elt_list[i].coef));
			}
 */
			opt_save = OPTION_DEFAULT;
			break;
		case 4:				/* log_k */
		case 5:				/* logk */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_log_k_only(next_char, &s_ptr->logk[0]);
			opt_save = OPTION_DEFAULT;
			break;
		case 6:				/* delta_h */
		case 7:				/* deltah */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_delta_h_only(next_char, &s_ptr->logk[1],
							  &s_ptr->original_units);
			opt_save = OPTION_DEFAULT;
			break;
		case 8:				/* analytical_expression */
		case 9:				/* a_e */
		case 10:				/* ae */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_analytical_expression_only(next_char, &(s_ptr->logk[T_A1]));
			opt_save = OPTION_DEFAULT;
			break;
		case 12:				/* gamma */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->exch_gflag = 2;
			s_ptr->a_f = 0;
			i = sscanf(next_char, SCANFORMAT SCANFORMAT SCANFORMAT, &s_ptr->dha,
					   &s_ptr->dhb, &s_ptr->a_f);
			if (i < 2)
			{
				error_string = sformatf( "Expecting 2 activity-"
						"coefficient parameters, a and b.");
				warning_msg(error_string);
			}
			if (s_ptr->dha == 0 && s_ptr->dhb == 0)
			{
				s_ptr->dhb = 99.9;
				s_ptr->exch_gflag = 1;
			}
			opt_save = OPTION_DEFAULT;
			break;
		case 13:				/* davies */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->exch_gflag = 1;
			s_ptr->dha = 0;
			s_ptr->dhb = 99.9;
			opt_save = OPTION_DEFAULT;
			break;
		case 14:				/* offset */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			if (sscanf(next_char, SCANFORMAT, &offset) != 1)
			{
				error_msg("No offset for log K given", STOP);
			}
			s_ptr->logk[0] += offset;
			opt_save = OPTION_DEFAULT;
			break;
		case 15:				/* llnl_gamma */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->exch_gflag = 7;	/* llnl D-H */
			i = sscanf(next_char, SCANFORMAT, &s_ptr->dha);
			if (i < 1)
			{
				error_string = sformatf(
						"Expecting activity-coefficient parameter, a.");
				warning_msg(error_string);
			}
			opt_save = OPTION_DEFAULT;
			break;
		case 16:				/* add_logk */
		case 17:				/* add_log_k */
		{
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = s_ptr->add_logk.size();
			s_ptr->add_logk.resize(count_add_logk + 1);
			/* read name */
			if (copy_token(token, &next_char, &i) == EMPTY)
			{
				input_error++;
				error_string = sformatf(
					"Expected the name of a NAMED_EXPRESSION.");
				error_msg(error_string, CONTINUE);
				break;
			}
			s_ptr->add_logk[count_add_logk].name = string_hsave(token);
			/* read coef */
			i = sscanf(next_char, SCANFORMAT,
				&s_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				s_ptr->add_logk[count_add_logk].coef = 1;
			}
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 18:				/* add_constant */
		{
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = s_ptr->add_logk.size();
			s_ptr->add_logk.resize(count_add_logk + 1);
			i = sscanf(next_char, SCANFORMAT,
				&s_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				input_error++;
				error_string = sformatf(
					"Expected the constant to add for log_K definition.");
				error_msg(error_string, CONTINUE);
				break;
			}
			/* set name */
			s_ptr->add_logk[count_add_logk].name =
				string_hsave("XconstantX");
			/* read coef */
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 19:            /* vm, molar volume */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
				opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_vm_only(next_char, &s_ptr->logk[vm0],
				&s_ptr->original_deltav_units);
			opt_save = OPTION_DEFAULT;
			break;

		case OPTION_DEFAULT:
			/*
			 *   Get exchange species information and parse equation
			 */
		{
			s_ptr = NULL;
			std::vector<class elt_list> new_elt_list;
			if (parse_eq(line, new_elt_list, association) == ERROR)
			{
					parse_error++;
					error_msg("Parsing equation.", CONTINUE);
					error_msg(line_save, CONTINUE);
					break;
				}
			/*
			 *   Get pointer to each species in the reaction, store new species if necessary
			 */
			trxn.token[0].s = s_store(trxn.token[0].name, trxn.token[0].z, TRUE);
			for (i = 1; i < count_trxn; i++)
			{
				trxn.token[i].s = s_store(trxn.token[i].name, trxn.token[i].z, FALSE);
			}
			/*
			 *   Save element list and carbon, hydrogen, and oxygen in species
			 */
			trxn.token[0].s->next_elt = new_elt_list;
			next_elt = &trxn.token[0].s->next_elt[0];
			for (; next_elt->elt != NULL; next_elt++)
			{
				if (strcmp(next_elt->elt->name, "C") == 0)
				{
					trxn.token[0].s->carbon = next_elt->coef;
				}
				if (strcmp(next_elt->elt->name, "H") == 0)
				{
					trxn.token[0].s->h = next_elt->coef;
				}
				if (strcmp(next_elt->elt->name, "O") == 0)
				{
					trxn.token[0].s->o = next_elt->coef;
				}
			}
			/*
			 *   Copy reaction to reaction for species
			 */
			trxn_copy(trxn.token[0].s->rxn);
			/*
			 *   Set type for species
			 */
			trxn.token[0].s->type = EX;
			s_ptr = trxn.token[0].s;
			/*
			 *   Set gamma data
			 */
			s_ptr->gflag = 4;
			s_ptr->exch_gflag = 3;
			s_ptr->dha = 0.0;
			s_ptr->dhb = 0.0;
			opt_save = OPTION_DEFAULT;
			/*
			 *  Save as a phase for inverse modeling only
			 */
			phase_ptr = phase_store(s_ptr->name);
			if (phase_ptr == NULL)
			{
				input_error++;
				error_string = sformatf("Copying exchange to phases.");
				error_msg(error_string, CONTINUE);
			}
			else
			{
				phase_ptr->formula = s_ptr->name;
				phase_ptr->check_equation = FALSE;
				phase_ptr->type = EX;
				phase_ptr->next_elt = s_ptr->next_elt;
				phase_ptr->rxn = s_ptr->rxn;
			}
		}
		break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_exchange(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads exchange data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int n_user;
	LDBLE conc;
	const char* cptr;
	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"equilibrate",			/* 0 */
		"equil",				/* 1 */
		"pitzer_exchange_gammas",	/* 2 */
		"exchange_gammas",		/* 3 */
		"gammas",				/* 4 */
		"equilibrium"           /* 5 */
	};
	int count_opt_list = 6;
/*
 * kin_exch is for exchangers, related to kinetically reacting minerals
 *    they are defined if "sites" is followed by mineral name:
 *    Z	 Manganite		('equi' or 'kine')      0.25
 *    ^Name     ^equi or kinetic mineral ^switch		  ^prop.factor
 */
/*
 *   Default values + n_user, description
 */
	cxxExchange temp_exchange;
	cptr = line;
	temp_exchange.read_number_description(cptr);
	n_user = temp_exchange.Get_n_user();
	cxxExchComp *comp_ptr = NULL;
	temp_exchange.Set_new_def(true);
/*
 *   Set use data
 */
	if (use.Get_exchange_in() == FALSE)
	{
		use.Set_exchange_in(true);
		use.Set_n_exchange_user(n_user);
	}
/*
 *   Read exchange data
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in EXCHANGE keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* equilibrate */
		case 1:             /* equil */
		case 5:             /* equilibrium */
			/*
			 *   Read solution to equilibrate with
			 */
			for (;;)
			{
				std::string token;
				int i = copy_token(token, &next_char);
				if (i == DIGIT)
				{
					int n_solution;
					(void)sscanf(token.c_str(), "%d", &n_solution);
					temp_exchange.Set_n_solution(n_solution);
					temp_exchange.Set_new_def(true);
					temp_exchange.Set_solution_equilibria(true);
					break;
				}
				if (i == EMPTY)
				{
					error_msg
						("Expected a solution number with which to equilibrate exchanger.",
						 CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
					break;
				}
			}
			break;
		case 2:				/* pitzer_exchange_gammas */
		case 3:				/* exchange_gammas */
		case 4:				/* gammas */
			temp_exchange.Set_pitzer_exchange_gammas(
				get_true_false(next_char, TRUE) == TRUE);
			break;
		case OPTION_DEFAULT:
			{
				std::string token;
				cptr = line;
				int i = copy_token(token, &cptr);
				/*
				*   Species formula is stored in token
			    */
				if (i != UPPER && token[0] != '[')
				{ /* maybe a bit more clear? */
					error_string = sformatf(
						"Expected exchanger name to begin with a capital letter, but found:\n %s",
						line_save);
					error_msg(error_string, CONTINUE);
					input_error++;
					break;
				}
				cxxExchComp temp_comp(this->phrq_io);
				temp_exchange.Get_exchange_comps().push_back(temp_comp);
				comp_ptr = &(temp_exchange.Get_exchange_comps().back());
				comp_ptr->Set_formula(token.c_str());
				prev_next_char = cptr;
				std::string token1;
				i = copy_token(token1, &cptr);
				if (i == DIGIT)
				{
					/*
					*   Read exchange concentration
				 */

					/* exchanger conc. is read directly .. */
					if (sscanf(token1.c_str(), SCANFORMAT, &conc) < 1)
					{
						error_string = sformatf(
							"Expected concentration of exchanger, but found:\n %s",
							line_save);
						error_msg(error_string, CONTINUE);
						input_error++;
						break;
					}
					prev_next_char = cptr;
					int j = copy_token(token1, &cptr);
					if (j == UPPER || j == LOWER)
					{
						comp_ptr->Set_rate_name(token1.c_str());
						if (copy_token(token1, &cptr) != DIGIT)
						{
							error_string = sformatf(
								"Expected a coefficient to relate exchange to kinetic reaction, but found:\n %s",
								prev_next_char);
							error_msg(error_string, CONTINUE);
							input_error++;
							break;
						}
						LDBLE p;
						(void)sscanf(token1.c_str(), SCANFORMAT, &p);
						comp_ptr->Set_phase_proportion(p);
					}
					/*
					*   Read equilibrium phase name or kinetics rate name
				 */
				}
				else if (i != EMPTY)
				{

					/* exchanger conc. is related to mineral or kinetics */
					comp_ptr->Set_phase_name(token1.c_str());
					prev_next_char = cptr;
					int j = copy_token(token1, &cptr);
					if (j != DIGIT)
					{
						if (token1[0] == 'K' || token1[0] == 'k')
						{
							comp_ptr->Set_rate_name(comp_ptr->Get_phase_name().c_str());
							comp_ptr->Set_phase_name("");
						}
						else if (token1[0] != 'E' && token1[0] != 'e')
						{
							error_string = sformatf(
								"Character string expected to be 'equilibrium_phase' or 'kinetics'\n to relate exchange to mineral or kinetic reaction, but found:\n %s",
								prev_next_char);
							error_msg(error_string, CONTINUE);
							input_error++;
							break;
						}
						prev_next_char = cptr;
						j = copy_token(token1, &cptr);
					}


					if (j != DIGIT)
					{
						error_string = sformatf(
							"Expected a coefficient to relate exchanger to mineral or kinetic reaction, but found:\n %s",
							prev_next_char);
						error_msg(error_string, CONTINUE);
						input_error++;
						break;
					}
					LDBLE p;
					(void)sscanf(token1.c_str(), SCANFORMAT, &p);
					comp_ptr->Set_phase_proportion(p);
					/* real conc must be defined in tidy_model */
					conc = 1.0;
				}
				else
				{
					error_msg
						("Expected concentration of exchanger, mineral name, or kinetic reaction name.",
						CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
					break;
				}
				/*
				*   Accumulate elements in elt_list
			    */
				count_elts = 0;
				paren_count = 0;
				std::string formula = token.c_str();
				cptr = formula.c_str();
				get_elts_in_species(&cptr, conc);
				
				/*
				*   save formula for adjusting number of exchange sites
			    */
				cptr = formula.c_str();
				std::string name;
				LDBLE z;
				int l;
				get_token(&cptr, name, &z, &l);
				comp_ptr->Set_formula_z(z);
				/*
				*   Save elt_list
			    */
				comp_ptr->Set_totals(elt_list_NameDouble());
				comp_ptr->Set_charge_balance(0.0);
			}
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	Rxn_exchange_map[n_user] = temp_exchange;
	Rxn_new_exchange.insert(n_user);
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_exchange_master_species(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads master species data from data file or input file
 */
	int j, l;
	const char* cptr, *cptr1;
	LDBLE l_z;
	class element *elts_ptr;
	class species *s_ptr;
	char token[MAX_LENGTH];
	for (;;)
	{
		j = check_line("Exchange species equation", FALSE, TRUE, TRUE, TRUE);
		if (j == EOF || j == KEYWORD)
		{
			break;
		}
/*
 *   Get element name with valence, allocate space, store
 */
		cptr = line;
/*
 *   Get element name and save pointer to character string
 */
		if (copy_token(token, &cptr, &l) != UPPER && token[0] != '[')
		{
			parse_error++;
			error_msg("Reading element for master species.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
		/*
		   if (token[0] == '[') {
		   cptr1 = token;
		   get_elt(&cptr, element, &l);
		   strcpy(token, element);
		   }
		 */
		replace("(+", "(", token);
/*
 *   Delete master if it exists
 */
		master_delete(token);
/*
 *   Increase pointer array, if necessary,  and malloc space
 */
		size_t count_master = master.size();
		master.resize(count_master + 1);
		master[count_master] = master_alloc();
/*
 *   Set type to EX
 */
		master[count_master]->type = EX;
/*
 *   Save element name
 */
		master[count_master]->elt = element_store(token);
/*
 *   Save pointer to species data for master species
 */
		if ((copy_token(token, &cptr, &l) != UPPER) &&
			token[0] != '[' && (strcmp_nocase_arg1(token, "e-") != 0))
		{
			parse_error++;
			error_msg("Reading master species name.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
		s_ptr = s_search(token);
		if (s_ptr != NULL)
		{
			master[count_master]->s = s_ptr;
		}
		else
		{
			cptr1 = token;
			std::string token1;
			get_token(&cptr1, token1, &l_z, &l);
			master[count_master]->s = s_store(token1.c_str(), l_z, FALSE);
		}
/*
 *   MAKE LISTS OF PRIMARY AND SECONDARY MASTER SPECIES
 */
		master[count_master]->primary = TRUE;
		if (strcmp(master[count_master]->elt->name, "E") != 0)
		{
			elts_ptr = element_store(master[count_master]->elt->name);
			elts_ptr->gfw = 0.0;
		}
	}
	return (j);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_gas_phase(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads gas phase data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int i, j, l;
	int n_user;
	const char* cptr;
	char token[MAX_LENGTH];
	cxxGasPhase temp_gas_phase(this->phrq_io);
	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"pressure",				/* 0 */
		"volume",				/* 1 */
		"temp",					/* 2 */
		"temperature",			/* 3 */
		"fixed_pressure",		/* 4 */
		"fixed_volume",			/* 5 */
		"equilibrium",			/* 6 */
		"equilibrate",			/* 7 */
		"equil"					/* 8 */
	};
	int count_opt_list = 9;

	cptr = line;
	temp_gas_phase.read_number_description(cptr);
	n_user = temp_gas_phase.Get_n_user();
	temp_gas_phase.Set_new_def(true);
/*
 *   Set use data to first read
 */
	if (use.Get_gas_phase_in() == FALSE)
	{
		use.Set_gas_phase_in(true);
		use.Set_n_gas_phase_user(n_user);
	}
/*
 *   Read phases
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in GAS_PHASE keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* pressure */
			(void)sscanf(next_char, SCANFORMAT, &dummy);
			temp_gas_phase.Set_total_p(dummy);
			break;
		case 1:				/* Volume */
			(void)sscanf(next_char, SCANFORMAT, &dummy);
			temp_gas_phase.Set_volume(dummy);
			break;
		case 2:				/* Temperature */
		case 3:
			j = sscanf(next_char, SCANFORMAT, &dummy);
			if (j == 1)
			{
				temp_gas_phase.Set_temperature(dummy + 273.15);
			}
			break;
		case 4:				/* fixed_pressure */
			temp_gas_phase.Set_type(cxxGasPhase::GP_PRESSURE);
			break;
		case 5:				/* fixed_volume */
			temp_gas_phase.Set_type(cxxGasPhase::GP_VOLUME);
			break;
		case 6:				/* equilibrate */
		case 7:				/* equilibrium */
		case 8:				/* equil */
/*
 *   Read solution to equilibrate with
 */
			for (;;)
			{
				i = copy_token(token, &next_char, &l);
				if (i == DIGIT)
				{
					(void)sscanf(token, "%d", &l);
					temp_gas_phase.Set_n_solution(l);
					temp_gas_phase.Set_new_def(true);
					temp_gas_phase.Set_solution_equilibria(true);
					break;
				}
				if (i == EMPTY)
				{
					error_msg
						("Expected a solution number with which to equilibrate gas phase.",
						 CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
					break;
				}
			}
			break;
		case OPTION_DEFAULT:
			{
				cxxGasComp temp_comp;
				/*
				*   Read name
				*/
				cptr = line;
				copy_token(token, &cptr, &l);
				temp_comp.Set_phase_name(token);
				if ((j = copy_token(token, &cptr, &l)) == EMPTY)
				{
					temp_comp.Set_p_read(NAN);
					temp_gas_phase.Get_gas_comps().push_back(temp_comp);
					break;
				}
				/*
				*   Read initial partial pressure of gas
				*/

				j = sscanf(token, SCANFORMAT, &dummy);
				
				if (j != 1)
				{
					error_msg("Expected partial pressure of gas in gas phase.",
						CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else
				{
					temp_comp.Set_p_read(dummy);
					temp_gas_phase.Get_gas_comps().push_back(temp_comp);
				}
			}
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}

	// sort components
	std::map<std::string, cxxGasComp> gc;
	for (size_t i = 0; i < temp_gas_phase.Get_gas_comps().size(); i++)
	{
		cxxGasComp *gc_ptr = &(temp_gas_phase.Get_gas_comps()[i]);
		gc[gc_ptr->Get_phase_name()] = *gc_ptr;
	}
	std::vector<cxxGasComp> vgc;
	std::map<std::string, cxxGasComp>::iterator it = gc.begin();
	for ( ; it != gc.end(); it++)
	{
		vgc.push_back(it->second);
	}
	temp_gas_phase.Set_gas_comps(vgc);


	Rxn_gas_phase_map[n_user] = temp_gas_phase;
	Rxn_new_gas_phase.insert(n_user);
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_inverse(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads data for mass_balance calculations
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int n, j;
	int n_user, n_user_end;
	const char* cptr;
	char *description;
	LDBLE range_max, inv_tol, water_uncertainty;

	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"solutions",			/* 0 */
		"uncertainty",			/* 1 */
		"uncertainties",		/* 2 */
		"balances",				/* 3 */
		"phase_data",			/* 4 */
		"range",				/* 5 */
		"minimal",				/* 6 */
		"minimum",				/* 7 */
		"balance",				/* 8 */
		"bal",					/* 9 */
		"sol",					/* 10 */
		"phases",				/* 11 */
		"ranges",				/* 12 */
		"tolerance",			/* 13 */
		"u_water",				/* 14 */
		"uncertainty_water",	/* 15 */
		"force",				/* 16 */
		"force_solution",		/* 17 */
		"force_solutions",		/* 18 */
		"isotopes",				/* 19 */
		"mineral_water",		/* 20 */
		"phase",				/* 21 */
		"multiple_precision",	/* 22 */
		"mp_tolerance",			/* 23 */
		"censor_mp",			/* 24 */
		"lon_netpath",			/* 25 */
		"pat_netpath"			/* 26 */
	};
	int count_opt_list = 27;

	cptr = line;
/*
 *   Read solution number and description
 */
	read_number_description(cptr, &n_user, &n_user_end, &description);
/*
 *   Malloc space for solution data
 */
	if (inverse_search(n_user, &n) != NULL)
	{
		inverse_delete(n);
	}
	inverse_alloc();
	n = count_inverse - 1;
/*
 *   Initialize structure and use
 */
	inverse[n].new_def = TRUE;
	inverse[n].n_user = n_user;
	inverse[n].range = FALSE;
	inverse[n].range_max = 1000.;
	inverse[n].tolerance = 1e-10;
	inverse[n].minimal = FALSE;
	inverse[n].description = description;
	inverse[n].water_uncertainty = 0.0;
	inverse[n].mineral_water = TRUE;
	inverse[n].mp = FALSE;
	inverse[n].mp_tolerance = 1e-12;
	inverse[n].mp_censor = 1e-20;
	inverse[n].netpath = NULL;
	inverse[n].pat = NULL;
/*
 *   Read data for inverse modeling
 */
	opt_save = OPTION_ERROR;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = opt;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in INVERSE_MODELING keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* solutions */
		case 10:				/* solution */
			read_vector_ints(&next_char, inverse[n].solns, TRUE);
			inverse[n].count_solns = (int)inverse[n].solns.size();
			opt_save = OPTION_ERROR;
			break;
		case 1:				/* uncertainty */
		case 2:				/* uncertainties */
			read_vector_doubles(&next_char, inverse[n].uncertainties);
			opt_save = OPTION_ERROR;
			break;
		case 3:				/* balances */
		case 8:				/* balance */
		case 9:				/* bal */
			read_inv_balances(&(inverse[n]), next_char);
			break;
		case 4:				/* phase_data */
		case 11:				/* phases */
		case 21:				/* phase */
			read_inv_phases(&(inverse[n]), next_char);
			break;
		case 5:				/* range */
		case 12:				/* ranges */
			inverse[n].range = TRUE;
			j = sscanf(next_char, SCANFORMAT, &range_max);
			if (j == 1)
			{
				inverse[n].range_max = range_max;
			}
			opt_save = OPTION_ERROR;
			break;
		case 6:				/* minimal */
		case 7:				/* minimum */
			inverse[n].minimal = TRUE;
			opt_save = OPTION_ERROR;
			break;
		case 13:				/* tolerance */
			j = sscanf(next_char, SCANFORMAT, &inv_tol);
			if (j == 1)
			{
				inverse[n].tolerance = inv_tol;
			}
			opt_save = OPTION_ERROR;
			break;
		case 14:				/* u_water */
		case 15:				/* uncertainty_water */
			j = sscanf(next_char, SCANFORMAT, &water_uncertainty);
			if (j == 1)
			{
				inverse[n].water_uncertainty = water_uncertainty;
			}
			opt_save = OPTION_ERROR;
			break;
		case 16:				/* force */
		case 17:				/* force_solution */
		case 18:				/* force_solutions */
			inverse[n].force_solns.clear();
			read_vector_t_f(&next_char, inverse[n].force_solns);
			opt_save = OPTION_ERROR;
			break;
		case 19:				/* isotope values */
			read_inv_isotopes(&(inverse[n]), next_char);
			break;
		case 20:				/* mineral_water */
			inverse[n].mineral_water = get_true_false(next_char, TRUE);
			opt_save = OPTION_ERROR;
			break;
		case 22:				/* multiple_precision */
			inverse[n].mp = get_true_false(next_char, TRUE);
			opt_save = OPTION_ERROR;
			break;
		case 23:				/* mp_tolerance */
			j = sscanf(next_char, SCANFORMAT, &inv_tol);
			if (j == 1)
			{
				inverse[n].mp_tolerance = fabs(inv_tol);
			}
			opt_save = OPTION_ERROR;
			break;
		case 24:				/* censor_mp */
			j = sscanf(next_char, SCANFORMAT, &inv_tol);
			if (j == 1)
			{
				inverse[n].mp_censor = fabs(inv_tol);
			}
			opt_save = OPTION_ERROR;
			break;
		case 25:				/* lon_netpath */
		{
			std::string temp_name(next_char);
			string_trim(temp_name);
			if (temp_name.size() > 0)
			{
				inverse[n].netpath = string_hsave(temp_name.c_str());
			}
			else
			{
				inverse[n].netpath = string_hsave("netpath");
			}
			opt_save = OPTION_ERROR;
		}
		break;
		case 26:				/* pat_netpath */
		{
			std::string temp_name(next_char);
			string_trim(temp_name);
			if (temp_name.size() > 0)
			{
				inverse[n].pat = string_hsave(temp_name.c_str());
			}
			else
			{
				inverse[n].pat = string_hsave("netpath");
			}
			opt_save = OPTION_ERROR;
		}
		break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
/*
 *  Default: soln 1 -> soln 2
 */
	if (inverse[n].count_solns == 0)
	{
		inverse[n].solns.push_back(1);
		inverse[n].solns.push_back(2);
		inverse[n].count_solns = 2;
	}
/*
 *   Sort isotopes
 */
	if (inverse[n].isotopes.size() > 1)
	{
		qsort(&inverse[n].isotopes[0],
			  inverse[n].isotopes.size(),
			  sizeof(class inv_isotope), inverse_isotope_compare);
	}

	if (inverse[n].i_u.size() > 1)
	{
		qsort(&inverse[n].i_u[0],
			  inverse[n].i_u.size(),
			  (size_t) sizeof(class inv_isotope), inverse_isotope_compare);
	}

	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_inv_balances(class inverse *inverse_ptr, const char* cptr)
/* ---------------------------------------------------------------------- */
{
	int j, l;
	char token[MAX_LENGTH];
/*
 *   Read element name
 */
	j = copy_token(token, &cptr, &l);
	if (j == EMPTY)
	{
		return (OK);
	}
	else if (j == LOWER && strcmp_nocase_arg1(token, "ph") != 0)
	{
		error_msg("Expecting element name.", CONTINUE);
		error_msg(line_save, CONTINUE);
		input_error++;
	}
	else if (strcmp_nocase_arg1(token, "ph") != 0)
	{
		size_t count_elts = inverse_ptr->elts.size();
		inverse_ptr->elts.resize(count_elts + 1);
		replace("(+", "(", token);
		inverse_ptr->elts[count_elts].name = string_hsave(token);
/*
 *   Read element uncertainties
 */
		read_vector_doubles(&cptr, inverse_ptr->elts[count_elts].uncertainties);
	}
	else if (strcmp_nocase_arg1(token, "ph") == 0)
	{
		inverse_ptr->ph_uncertainties.clear();
		read_vector_doubles(&cptr, inverse_ptr->ph_uncertainties);
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_inv_isotopes(class inverse *inverse_ptr, const char* cptr)
/* ---------------------------------------------------------------------- */
{
	int i, j, l, l1, l2;
	LDBLE isotope_number;
	char token[MAX_LENGTH], token1[MAX_LENGTH];
	const char* cptr1, *ptr2;
	const char * redox_name, *element_name;
/*
 *   Read element name
 */
	cptr1 = cptr;
	j = copy_token(token, &cptr1, &l);
/*
 *   cptr1 is start of uncertainties
 */
	if (j == EMPTY)
	{
		return (OK);
	}
	else if (j != DIGIT)
	{
		error_msg("Expecting isotope to begin with isotope number.",
				  CONTINUE);
		error_msg(line_save, CONTINUE);
		input_error++;
		return (ERROR);
	}
/*
 *   Read isotope name
 */
	ptr2 = token;
	get_num(&ptr2, &isotope_number);
	if (ptr2[0] == '\0' || isupper((int) ptr2[0]) == FALSE)
	{
		error_msg("Expecting element name.", CONTINUE);
		error_msg(line_save, CONTINUE);
		input_error++;
		return (ERROR);
	}

	/* redox state name with parentheses */
	redox_name = string_hsave(ptr2);

	copy_token(token, &ptr2, &l1);
	replace("(", " ", token);
	ptr2 = token;

	/* element name, without parentheses */
	copy_token(token1, &ptr2, &l2);
	element_name = string_hsave(token1);

/*
 *  add element name to inv_ptr->isotopes
 */
	for (i = 0; i < inverse_ptr->isotopes.size(); i++)
	{
		if (element_name == inverse_ptr->isotopes[i].elt_name)
			break;
	}
	if (i == inverse_ptr->isotopes.size())
	{
		size_t count_isotopes = inverse_ptr->isotopes.size();
		inverse_ptr->isotopes.resize(count_isotopes + 1);
		inverse_ptr->isotopes[count_isotopes].isotope_number = isotope_number;
		inverse_ptr->isotopes[count_isotopes].elt_name = element_name;
		inverse_ptr->isotopes[count_isotopes].uncertainties.clear();
	}
/*
 *  add redox state name to inv_ptr->i_u
 */
	size_t count_i_u = inverse_ptr->i_u.size();
	inverse_ptr->i_u.resize(count_i_u + 1);
	inverse_ptr->i_u[count_i_u].elt_name = redox_name;
	inverse_ptr->i_u[count_i_u].isotope_number = isotope_number;
/*
 *   Read isotope uncertainties
 */
	read_vector_doubles(&cptr1, inverse_ptr->i_u[count_i_u].uncertainties);
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_inv_phases(class inverse *inverse_ptr, const char* cptr)
/* ---------------------------------------------------------------------- */
{
	int j, l;
	char token[MAX_LENGTH], token1[MAX_LENGTH];
	const char* cptr1;
	std::vector <cxxSolutionIsotope> isotopes;
/*
 *   Read phase name
 */
	j = copy_token(token, &cptr, &l);
	if (j == EMPTY)
		return (OK);

	size_t count_phases = inverse_ptr->phases.size();
	inverse_ptr->phases.resize(count_phases + 1);
	inverse_ptr->phases[count_phases].name = string_hsave(token);
/*
 *   Read constraint, force, and isotopes
 */
	inverse_ptr->phases[count_phases].constraint = EITHER;
	inverse_ptr->phases[count_phases].force = FALSE;
	for (;;)
	{
		cxxSolutionIsotope temp_isotope;
		j = copy_token(token, &cptr, &l);
		if (j == EMPTY)
			break;
		strcpy(token1, token);
		str_tolower(token1);
		if (token1[0] == 'p')
		{
			inverse_ptr->phases[count_phases].constraint = PRECIPITATE;
		}
		else if (token1[0] == 'd')
		{
			inverse_ptr->phases[count_phases].constraint = DISSOLVE;
		}
		else if (token[0] == 'f')
		{
			inverse_ptr->phases[count_phases].force = TRUE;
		}
		else if (j == DIGIT)
		{
/*
 *   read isotope data
 */
			cptr1 = token;

			/* isotope number */
			get_num(&cptr1, &dummy);
			temp_isotope.Set_isotope_number(dummy);
			if (cptr1[0] == '\0' || isupper((int) cptr1[0]) == FALSE)
			{
				error_string = sformatf( "Expecting element name: %s.", cptr1);
				error_msg(error_string, CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
				break;
			}

			/* element name */
			temp_isotope.Set_elt_name(cptr1);

			/* ratio */
			j = copy_token(token, &cptr, &l);
			if (j != DIGIT)
			{
				error_msg("Expecting isotope ratio for phase.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
				break;
			}
			(void)sscanf(token, SCANFORMAT, &dummy);
			temp_isotope.Set_ratio(dummy);

			/* read and store isotope ratio uncertainty */
			prev_next_char = cptr;
			if (copy_token(token, &cptr, &l) != DIGIT)
			{
				input_error++;
				error_string = sformatf(
					"Expected numeric value for uncertainty in isotope ratio, but found:\n %s",
					prev_next_char);
				error_msg(error_string, CONTINUE);
				continue;
			}
			(void)sscanf(token, SCANFORMAT, &dummy);
			temp_isotope.Set_ratio_uncertainty(dummy);
			temp_isotope.Set_ratio_uncertainty_defined(true);
			isotopes.push_back(temp_isotope);
		}
		else
		{
			error_string = sformatf(
					"Unknown option for inverse modeling phase.");
			warning_msg(error_string);
		}
	}
	if (isotopes.size() > 0)
	{
		inverse_ptr->phases[count_phases].isotopes.resize(isotopes.size());
		for (size_t i = 0; i < isotopes.size(); i++)
		{
			class isotope *iso_ptr = &(inverse_ptr->phases[count_phases].isotopes[i]);
			iso_ptr->isotope_number = isotopes[i].Get_isotope_number();
			iso_ptr->elt_name = string_hsave(isotopes[i].Get_elt_name().c_str());
			iso_ptr->isotope_name = string_hsave(isotopes[i].Get_isotope_name().c_str());
			iso_ptr->total = isotopes[i].Get_total();
			iso_ptr->ratio = isotopes[i].Get_ratio();
			if (isotopes[i].Get_ratio_uncertainty_defined())
				iso_ptr->ratio_uncertainty = isotopes[i].Get_ratio_uncertainty();
			else
				iso_ptr->ratio_uncertainty = NAN;
			iso_ptr->x_ratio_uncertainty = isotopes[i].Get_x_ratio_uncertainty();
			iso_ptr->coef = isotopes[i].Get_coef();
			iso_ptr->master = NULL;
			iso_ptr->primary = NULL;
		}
	}
	else
	{
		inverse_ptr->phases[count_phases].isotopes.clear();
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_kinetics(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads kinetics data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
/*
 *   Read kinetics
 */
	const char* cptr;
	std::string token;
	int n_user;
	LDBLE step;

	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"tol",					/* 0 */
		"m",					/* 1 */
		"m0",					/* 2 */
		"parms",				/* 3 */
		"formula",				/* 4 */
		"steps",				/* 5 */
		"step_divide",			/* 6 */
		"parameters",			/* 7 */
		"runge-kutta",			/* 8 */
		"runge_kutta",			/* 9 */
		"rk",					/* 10 */
		"bad_step_max",			/* 11 */
		"cvode",				/* 12 */
		"cvode_steps",			/* 13 */
		"cvode_order",			/* 14 */
		"time_steps"			/* 15 */
	};
	int count_opt_list = 16;

	cxxKinetics temp_kinetics(this->phrq_io);
	cptr = line;
	temp_kinetics.read_number_description(cptr);
	n_user = temp_kinetics.Get_n_user();
	cxxKineticsComp *kinetics_comp_ptr = NULL;
	std::string stdunits;
/*
 *   Set use data to first read
 */
	if (use.Get_kinetics_in() == FALSE)
	{
		use.Set_kinetics_in(true);
		use.Set_n_kinetics_user(n_user);
	}
/*
 *   Read kinetics data
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:	/* allocate space, read new name */
			if (kinetics_comp_ptr)
			{
				temp_kinetics.Get_kinetics_comps().push_back(*kinetics_comp_ptr);
				delete kinetics_comp_ptr;
			}
			kinetics_comp_ptr = new cxxKineticsComp;
			cptr = line;
			copy_token(token, &cptr);
			kinetics_comp_ptr->Set_rate_name(token.c_str());
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in KINETICS keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* tolerance */
			if (kinetics_comp_ptr == NULL)
			{
				error_string = sformatf( "No rate name has been defined.");
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			else
			{
				prev_next_char = next_char;
				if (copy_token(token, &next_char) == DIGIT)
				{
					char* ptr;
					kinetics_comp_ptr->Set_tol(strtod(token.c_str(), &ptr));
				}
				else
				{
					error_string = sformatf(
						"Expecting numerical value for tolerance, but found:\n %s",
						prev_next_char);
					error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		case 1:				/* m */
			if (kinetics_comp_ptr == NULL)
			{
				error_string = sformatf( "No rate name has been defined.");
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			else
			{
				prev_next_char = next_char;
				if (copy_token(token, &next_char) == DIGIT)
				{
					char* ptr;
					kinetics_comp_ptr->Set_m(strtod(token.c_str(), &ptr));
				}
				else
				{
					error_string = sformatf(
						"Expecting numerical value for moles of reactant (m), but found:\n %s",
						prev_next_char);
					error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		case 2:				/* m0 */
			if (kinetics_comp_ptr == NULL)
			{
				error_string = sformatf( "No rate name has been defined.");
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			else
			{
				prev_next_char = next_char;
				if (copy_token(token, &next_char) == DIGIT)
				{
					char* ptr;
					kinetics_comp_ptr->Set_m0(strtod(token.c_str(), &ptr));
				}
				else
				{
					error_string = sformatf(
					"Expecting numerical value for initial moles of reactant (m0), but found:\n %s",
					prev_next_char);
				error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		case 3:				/* parms */
		case 7:				/* parameters */
			if (kinetics_comp_ptr == NULL)
			{
				error_string = sformatf( "No rate name has been defined.");
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			else
			{
				int j;
				while ((j = copy_token(token, &next_char)) != EMPTY)
				{
					/*
					 *   Store a LDBLE parameter
					 */
					if (j == DIGIT)
					{
						char* ptr;
						kinetics_comp_ptr->Get_d_params().push_back(strtod(token.c_str(), &ptr));
					}
					else
					{
						/*
						 *   Store a character parameter
						 */
						kinetics_comp_ptr->Get_c_params().push_back(token);
					}
				}
			}
			break;
		case 4:				/* formula */
			if (kinetics_comp_ptr == NULL)
			{
				error_string = sformatf( "No rate name has been defined.");
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			else
			{
				/*
				 *   Store reactant name, default coefficient
				 */
				cptr = next_char;
				bool have_name = false;
				std::string name;
				LDBLE coef = 1;
				while (copy_token(token, &cptr) != EMPTY)
				{
					coef = 1;
					if (isalpha((int) token[0]) || (token[0] == '(')
						|| (token[0] == '['))
					{
						if (have_name)
						{
							kinetics_comp_ptr->Get_namecoef().add(name.c_str(), coef);
						}
						name = token;
						have_name = true;
					}
					else
					{
						if (!have_name)
						{
							error_string = sformatf( "No phase or chemical formula has been defined.");
							error_msg(error_string, CONTINUE);
							input_error++;
						}
						/*
						 *   Store relative coefficient
						 */
						int j = sscanf(token.c_str(), SCANFORMAT, &coef);

						if (j == 1)
						{
							kinetics_comp_ptr->Get_namecoef().add(name.c_str(), coef);
							have_name = false;
						}
						else
						{
							error_msg
								("Reading relative coefficient of reactant.",
								 CONTINUE);
							error_msg(line_save, CONTINUE);
							input_error++;
						}
					}
				}
				if (have_name)
				{
					kinetics_comp_ptr->Get_namecoef().add(name.c_str(), coef);
				}
			}
			break;
		case 5:				/* steps */
		case 15:			/* time_steps */
			/*
			 *   Read one or more kinetics time increments
			 */
			{
				int j;
				while ((j = copy_token(token, &next_char)) != EMPTY)
				{
					if (j == DIGIT)
					{
						/*  Read next step increment(s) */
		/* multiple, equal timesteps 15 aug. 2005 */
						if (Utilities::replace("*", " ", token))
						{
							int k;
							if (sscanf(token.c_str(), "%d" SCANFORMAT, &k, &step) == 2)
							{
								for (int i = 0; i < k; i++)
								{
									temp_kinetics.Get_steps().push_back(step);
								}
							}
							else
							{
								input_error++;
								error_msg
									("Format error in multiple, equal KINETICS timesteps.\nCorrect is (for example): 20 4*10 2*5 3\n",
									 CONTINUE);
							}
						}
						else
						{
							char* ptr;
							step = strtod(token.c_str(), &ptr);
							temp_kinetics.Get_steps().push_back(step);
						}
					}
					else
					{
						Utilities::str_tolower(token);
						if (token.substr(0,1) == "i" )
						{
							/*
							 *   Read number of increments
							 */
							if (temp_kinetics.Get_steps().size() != 1)
							{
								error_msg
									("To define equal time increments, only one total time should be defined.",
									 CONTINUE);
								input_error++;
								break;
							}
							temp_kinetics.Set_equalIncrements(true);
							do
							{
								int i = 1;
								j = sscanf(token.c_str(), "%d", &i);
								if (j == 1)
								{
									temp_kinetics.Set_count(abs(i));
									break;
								}
								else if (j == 1 && i < 0)
								{
									error_msg
										("Expecting positive number for number of equal "
										 "time increments for kinetics.", CONTINUE);
									error_msg(line_save, CONTINUE);
									input_error++;
									break;
								}
							}
							while (copy_token(token, &next_char) != EMPTY);
						}
						else 
						{
							stdunits = token;
						}
					}
				}
			}
			break;
		case 6:				/* step_divide */
			if (copy_token(token, &next_char) == DIGIT)
			{
				(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
				temp_kinetics.Set_step_divide(dummy);
			}
			else
			{
				error_string = sformatf(
						"Expecting numerical value for step_divide.");
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			break;
		case 8:				/* runge-kutta */
		case 9:				/* runge_kutta */
		case 10:				/* rk */
			{
				int j = copy_token(token, &next_char);
				if (j == DIGIT)
				{
					char* ptr;
					temp_kinetics.Set_rk((int) strtod(token.c_str(), &ptr));
				}
				else if (j == EMPTY)
				{
				}
				else
				{
					error_string = sformatf(
						"Expecting order for Runge-Kutta method.");
					error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		case 11:				/* bad_step_max */
			{
				int j = copy_token(token, &next_char);
				if (j == DIGIT)
				{
					char* ptr;
					temp_kinetics.Set_bad_step_max((int) strtod(token.c_str(), &ptr));
				}
				else if (j == EMPTY)
				{
				}
				else
				{
					error_string = sformatf( "Expecting maximal bad steps number.");
					error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		case 12:				/* cvode */
			temp_kinetics.Set_use_cvode(get_true_false(next_char, TRUE) == TRUE);
			break;
		case 13:				/* cvode_steps */
			{
				int j = copy_token(token, &next_char);
				if (j == DIGIT)
				{
					char* ptr;
					temp_kinetics.Set_cvode_steps((int) strtod(token.c_str(), &ptr));
				}
				else if (j == EMPTY)
				{
				}
				else
				{
					error_string = sformatf(
						"Expecting maximum number of steps for one call to cvode.");
					error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		case 14:				/* cvode_order */
			{
				int j = copy_token(token, &next_char);
				if (j == DIGIT)
				{
					char* ptr;
					temp_kinetics.Set_cvode_order((int) strtod(token.c_str(), &ptr));
				}
				else if (j == EMPTY)
				{
				}
				else
				{
					error_string = sformatf(
						"Expecting number of terms (order) used in cvode (1-5).");
					error_msg(error_string, CONTINUE);
					input_error++;
				}
			}
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}

	// save last comp
	if (kinetics_comp_ptr)
	{
		temp_kinetics.Get_kinetics_comps().push_back(*kinetics_comp_ptr);
		delete kinetics_comp_ptr;
	}
/*
 *   Default reactant
 */
	for (size_t i = 0; i < temp_kinetics.Get_kinetics_comps().size(); i++)
	{
		cxxKineticsComp *kinetics_comp_ptr = &(temp_kinetics.Get_kinetics_comps()[i]);
		if (kinetics_comp_ptr->Get_namecoef().size() == 0)
		{
			kinetics_comp_ptr->Get_namecoef().add(kinetics_comp_ptr->Get_rate_name().c_str(), 1.0); 
		}
	}
/*
 *   Default 1 sec
 */
	if (temp_kinetics.Get_steps().size() == 0)
	{
		temp_kinetics.Get_steps().push_back(1.0);
	}
	else if (stdunits.size() > 0)
	{
		std::vector<LDBLE>::iterator it;
		for (it = temp_kinetics.Get_steps().begin(); it != temp_kinetics.Get_steps().end(); it++)
		{
			*it = Utilities::convert_time(*it, stdunits, "s");
		}
	}
/*
 *   set defaults for moles
 */
	for (size_t i = 0; i < temp_kinetics.Get_kinetics_comps().size(); i++)
	{
		cxxKineticsComp *kinetics_comp_ptr = &(temp_kinetics.Get_kinetics_comps()[i]);
		if (kinetics_comp_ptr->Get_m0() < 0)
		{
			if (kinetics_comp_ptr->Get_m() < 0)
			{
				kinetics_comp_ptr->Set_m0(1);
			}
			else
			{
				kinetics_comp_ptr->Set_m0(kinetics_comp_ptr->Get_m());
			}
		}
		if (kinetics_comp_ptr->Get_m() < 0)
		{
			kinetics_comp_ptr->Set_m(kinetics_comp_ptr->Get_m0());
		}
	}
	Rxn_kinetics_map[n_user] = temp_kinetics;
	return (return_value);
}
/* ---------------------------------------------------------------------- */
bool Phreeqc::
read_vector_doubles(const char** cptr, std::vector<double>& v)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Reads a list of LDBLE numbers until end of line is reached or
	 *   a LDBLE cannot be read from a token.
	 */
	double value;
	std::istringstream iss(*cptr);
	while (iss >> value)
	{
		v.push_back(value);
	}
	return true;
}
/* ---------------------------------------------------------------------- */
bool Phreeqc::
read_vector_ints(const char** cptr, std::vector<int>& v, int positive)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Reads a list of int numbers until end of line is reached or
	 *   an int cannot be read from a token.
	 */
	int value;
	std::istringstream iss(*cptr);
	while (iss >> value)
	{
		v.push_back(value);
		if (value <= 0 && positive == TRUE)
		{
			error_msg("Expected an integer greater than zero.", CONTINUE);
			error_msg(line_save, CONTINUE);
			input_error++;
			return false;
		}
	}
	return true;
}

/* ---------------------------------------------------------------------- */
int * Phreeqc::
read_list_ints_range(const char **cptr, int *count_ints, int positive, int *int_list)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads a list of int numbers until end of line is reached or
 *   an int cannot be read from a token.
 *
 *      Arguments:
 *	 cptr    entry: points to line to read from
 *		exit:  points to next non-int token or end of line
 *
 *	 count_ints entry: number of ints already in list
 *
 *	 positive  entry: if TRUE, expects to read only positive integers
 *
 *      Returns:
 *	 pointer to a list of count_ints ints
 */
	char token[MAX_LENGTH];
	int value, value1, value2;
	int i, l;
	const char* cptr_save;

	if (int_list == NULL)
	{
		int_list = (int *) PHRQ_malloc(sizeof(int));
		if (int_list == NULL)
		{
			malloc_error();
			return (NULL);
		}
		*count_ints = 0;
	}
	cptr_save = *cptr;
	while (copy_token(token, cptr, &l) != EMPTY)
	{
		if (sscanf(token, "%d", &value) == 1)
		{
			/* Read an integer */
			(*count_ints)++;
			int_list =
				(int *) PHRQ_realloc(int_list,
									 (size_t) (*count_ints) * sizeof(int));
			if (int_list == NULL)
			{
				malloc_error();
				return (NULL);
			}
			int_list[(*count_ints) - 1] = value;
			if (value <= 0 && positive == TRUE)
			{
				error_msg("Expected an integer greater than zero.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
			}
			/* Read range of integers */
			if (replace("-", " ", token) == TRUE)
			{
				if (sscanf(token, "%d %d", &value1, &value2) != 2)
				{
					error_msg("Expected an integer range n-m.", CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else if (value2 < value1)
				{
					error_msg("Expected an integer range n-m, with n <= m.",
							  CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else if (value2 <= 0 && positive == TRUE)
				{
					error_msg("Expected an integer greater than zero.",
							  CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else
				{
					for (i = value1 + 1; i <= value2; i++)
					{
						(*count_ints)++;
						int_list =
							(int *) PHRQ_realloc(int_list,
												 (size_t) (*count_ints) *
												 sizeof(int));
						if (int_list == NULL)
						{
							malloc_error();
							return (NULL);
						}
						int_list[(*count_ints) - 1] = i;
					}
				}
			}
			cptr_save = *cptr;
		}
		else
		{
			*cptr = cptr_save;
			break;
		}
	}
	return (int_list);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_list_ints_range(const char** cptr, bool positive, std::vector<int> &int_list)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Reads a list of int numbers until end of line is reached or
	 *   an int cannot be read from a token.
	 *
	 *      Arguments:
	 *	 cptr    entry: points to line to read from
	 *		exit:  points to next non-int token or end of line
	 *
	 *	 count_ints entry: number of ints already in list
	 *
	 *	 positive  entry: if TRUE, expects to read only positive integers
	 *
	 *      Returns:
	 *	 pointer to a list of count_ints ints
	 */
	char token[MAX_LENGTH];
	int value, value1, value2;
	int i, l;
	const char* cptr_save;
	int count_start = (int)int_list.size();

	cptr_save = *cptr;
	while (copy_token(token, cptr, &l) != EMPTY)
	{
		if (sscanf(token, "%d", &value) == 1)
		{
			/* Read an integer */
			int_list.push_back(value);
			if (value <= 0 && positive)
			{
				error_msg("Expected an integer greater than zero.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
			}
			/* Read range of integers */
			if (replace("-", " ", token) == TRUE)
			{
				if (sscanf(token, "%d %d", &value1, &value2) != 2)
				{
					error_msg("Expected an integer range n-m.", CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else if (value2 < value1)
				{
					error_msg("Expected an integer range n-m, with n <= m.",
						CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else if (value2 <= 0 && positive == TRUE)
				{
					error_msg("Expected an integer greater than zero.",
						CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
				else
				{
					for (i = value1 + 1; i <= value2; i++)
					{
						int_list.push_back(i);
					}
				}
			}
			cptr_save = *cptr;
		}
		else
		{
			*cptr = cptr_save;
			break;
		}
	}
	return ((int )int_list.size() - count_start);
}

/* ---------------------------------------------------------------------- */
bool Phreeqc::
read_vector_t_f(const char** cptr, std::vector<bool>& v)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Reads a list of true and false until end of line is reached or
	 *   until non- t or f is found
	 */
	std::string token;
	while (copy_token(token, cptr) != EMPTY)
	{
		str_tolower(token);
		if (token[0] == 't')
		{
			v.push_back(true);
		}
		else if (token[0] == 'f')
		{
			v.push_back(false);
		}
		else
		{
			error_msg("Expected TRUE or FALSE.", CONTINUE);
			error_msg(line_save, CONTINUE);
			input_error++;
			return false;
		}

	}
	return true;
}


/* ---------------------------------------------------------------------- */
int Phreeqc::
read_log_k_only(const char* cptr_in, LDBLE * log_k)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read log k
 */
	*log_k = 0.0;
	std::string stds(cptr_in);
	replace(stds, "=", " ");
	//replace("=", " ", cptr);
	if (sscanf(stds.c_str(), SCANFORMAT, log_k) < 1)
	{
		input_error++;
		error_msg("Expecting log k.", CONTINUE);
		return (ERROR);
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_t_c_only(const char* cptr_in, LDBLE *t_c)
/* ---------------------------------------------------------------------- */
{
	*t_c = 0.0;
	std::string stds(cptr_in);
	replace(stds, "=", " ");
	//replace("=", " ", cptr);
	if (sscanf(stds.c_str(), SCANFORMAT, t_c) < 1)
	{
		input_error++;
		error_msg("Expecting numeric value for critical temperature T_c (K)", CONTINUE);
		return (ERROR);
	}
	return OK;
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_p_c_only(const char* cptr, LDBLE * p_c)
/* ---------------------------------------------------------------------- */
{
	*p_c = 0.0;
	std::string stds(cptr);
	replace(stds, "=", " ");
	if (sscanf(stds.c_str(), SCANFORMAT, p_c) < 1)
	{
		input_error++;
		error_msg("Expecting numeric value for critical pressure P_c (atm)", CONTINUE);
		return (ERROR);
	}
	return OK;
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_omega_only(const char* cptr, LDBLE *omega)
/* ---------------------------------------------------------------------- */
{
	*omega = 0.0;
	std::string stds(cptr);
	replace(stds, "=", " ");
	if (sscanf(stds.c_str(), SCANFORMAT, omega) < 1)
	{
		input_error++;
		error_msg("Expecting numeric value for acentric factor Omega", CONTINUE);
		return (ERROR);
	}
	return OK;
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_aq_species_vm_parms(const char* cptr, LDBLE * delta_v)
/* ---------------------------------------------------------------------- */
{
	int j;
	/*
	*   Read supcrt parms and Ionic strength terms
	*/
	for (j = 0; j < 11; j++)
	{
		delta_v[j] = 0.0;
	}
	delta_v[9] = 1.0;
/* Vmax, dmax...
	delta_v[10] = 999.0;
	delta_v[11] = 1.0; */
	j = sscanf(cptr, SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT ,
		/* a1..a4 */
		&(delta_v[0]), &(delta_v[1]), &(delta_v[2]), &(delta_v[3]),
		/* wref */
		&(delta_v[4]),
		/* b_Av */
		&(delta_v[5]),
		/* c1..c4 */
		&(delta_v[6]), &(delta_v[7]), &(delta_v[8]), &(delta_v[9]),
		 //vmP, exP
		&(delta_v[10]), &(delta_v[11])); 
	if (j < 1)
	{
		input_error++;
		error_msg("Expecting numeric values for calculating the species molar volume.",
			CONTINUE);
		return (ERROR);
	}
	/* multiply with factors. a1 is in cal/mol/bar. a2 in  cal/mol, a3, a4 in cal K/mol
	  41.84004 converts cal/mol/bar to cm3/mol. */
	delta_v[0] *= 41.84004e-1;
	delta_v[1] *= 41.84004e2;
	delta_v[2] *= 41.84004;
	delta_v[3] *= 41.84004e4;
	/* wref in cal/mol/bar */
	delta_v[4] *= 1e5;

	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_vm_only(const char* cptr, LDBLE * delta_v, DELTA_V_UNIT * units)
/* ---------------------------------------------------------------------- */
{
	int j, l;
	char token[MAX_LENGTH];
	/*
	*   Read analytical expression
	*/			
	for (j = 0; j < 9; j++)
	{
		delta_v[j] = 0.0;
	}
	j = sscanf(cptr, SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT,
		&(delta_v[0]), &(delta_v[1]), &(delta_v[2]), &(delta_v[3]),
		&(delta_v[4]), &(delta_v[5]), &(delta_v[6]), &(delta_v[7]));
	if (j < 1)
	{
		input_error++;
		error_msg("Expecting numeric value for the phase's molar volume, vm.",
			CONTINUE);
		return (ERROR);
	}
	/*
	*   Read delta V units
	*/
	*units = cm3_per_mol;
	do
	{
		j = copy_token(token, &cptr, &l);
	} while (j == DIGIT); 

	if (j == EMPTY)
	{
		return (OK);
	}

	LDBLE factor = 1.0;
	if (j == UPPER || j == LOWER)
	{
		str_tolower(token);
		if (strstr(token, "cm3") != NULL)
		{
			/* cm3/mol */
			;
		}
		else if (strstr(token, "dm3") != NULL)
		{
			/* Convert dm3/mol to cm3/mol */
			factor = 1e3;
		}
		else if (strstr(token, "m3") != NULL)
		{
			/* Convert m3/mol to cm3/mol */
			factor = 1e6;
		}

		for (int i = 0; i < 8; i++)
		{
			delta_v[i] *= factor;
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_phase_vm(const char* cptr, LDBLE * delta_v, DELTA_V_UNIT * units)
/* ---------------------------------------------------------------------- */
{
	int j, l;
	char token[MAX_LENGTH];
	/*
	*   Read analytical expression
	*/
	for (j = 0; j < 1; j++)
	{
		delta_v[j] = 0.0;
	}
	j = sscanf(cptr, SCANFORMAT /*SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT*/,
		&(delta_v[0])/*, &(delta_v[1]), &(delta_v[2]), &(delta_v[3]),
		&(delta_v[4]), &(delta_v[5]), &(delta_v[6]), &(delta_v[7])*/);
	if (j < 1)
	{
		input_error++;
		error_msg("Expecting numeric value for the phase's molar volume, vm.",
			CONTINUE);
		return (ERROR);
	}
	/*
	*   Read delta V units
	*/
	*units = cm3_per_mol;
	do
	{
		j = copy_token(token, &cptr, &l);
	} while (j == DIGIT); 

	if (j == EMPTY)
	{
		return (OK);
	}

	LDBLE factor = 1.0;
	if (j == UPPER || j == LOWER)
	{
		str_tolower(token);
		if (strstr(token, "cm3") != NULL)
		{
			/* cm3/mol */
			;
		}
		else if (strstr(token, "dm3") != NULL)
		{
			/* Convert dm3/mol to cm3/mol */
			factor = 1e3;
			*units = dm3_per_mol;
		}
		else if (strstr(token, "m3") != NULL)
		{
			/* Convert m3/mol to cm3/mol */
			factor = 1e6;
			*units = m3_per_mol;
		}

		for (int i = 0; i < 1; i++)
		{
			delta_v[i] *= factor;
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_delta_h_only(const char* cptr_in, LDBLE * delta_h, DELTA_H_UNIT * units)
/* ---------------------------------------------------------------------- */
{
	int j, l, kilo, joul;
	char token[MAX_LENGTH];
/*
 *   Read delta H
 */
	*delta_h = 0.0;
	std::string stds(cptr_in);
	replace(stds, "=", " ");
	const char* cptr = stds.c_str();
	j = copy_token(token, &cptr, &l);
	if (j == EMPTY)
	{
		input_error++;
		error_msg("Expecting numeric value for delta H.", CONTINUE);
		return (ERROR);
	}
	if (sscanf(token, SCANFORMAT, delta_h) < 1)
	{
		input_error++;
		error_msg("Expecting numeric value for delta H.", CONTINUE);
		return (ERROR);
	}
/*
 *   Read delta H units
 */
	j = copy_token(token, &cptr, &l);
	*units = kjoules;
	kilo = TRUE;
	joul = TRUE;
	if (j == EMPTY)
	{
		return (OK);
	}
	if (j == UPPER || j == LOWER)
	{
		str_tolower(token);
		if (strstr(token, "k") != token)
		{
			/* convert to kilo */
			kilo = FALSE;
			*delta_h /= 1000.;
		}
		if (strstr(token, "c") != NULL)
		{
			/* convert to joules */
			*delta_h *= JOULES_PER_CALORIE;
			joul = FALSE;
		}
	}
	if (kilo == TRUE && joul == TRUE)
	{
		*units = kjoules;
	}
	else if (kilo == FALSE && joul == TRUE)
	{
		*units = joules;
	}
	else if (kilo == TRUE && joul == FALSE)
	{
		*units = kcal;
	}
	else if (kilo == FALSE && joul == FALSE)
	{
		*units = cal;
	}
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_analytical_expression_only(const char* cptr, LDBLE * log_k)
/* ---------------------------------------------------------------------- */
{
	int j;
	int num_terms = T_A6 - T_A1 + 1;
/*
 *   Read analytical expression
 */
	for (j = 0; j < num_terms; j++)
	{
		log_k[j] = 0.0;
	}
	j = sscanf(cptr, SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT,
			   &(log_k[0]), &(log_k[1]), &(log_k[2]), &(log_k[3]),
			   &(log_k[4]), &(log_k[5]));
	if (j < 1)
	{
		input_error++;
		error_msg("Expecting numeric values for analytical expression.",
				  CONTINUE);
		return (ERROR);
	}
	return (OK);
}

/* VP: Density Start */
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_millero_abcdef (const char* cptr, LDBLE * abcdef)
/* ---------------------------------------------------------------------- */
{
  int j;
/*
 *   Read a, b, c, d, e, f, and kappa parameters for Millero density model.
 */
  for (j = 0; j < 7; j++)
  {
    abcdef[j] = 0.0;
  }
  j = sscanf (cptr, SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT,
	    &(abcdef[0]), &(abcdef[1]), &(abcdef[2]), &(abcdef[3]), &(abcdef[4]), &(abcdef[5]), &(abcdef[6]));
  if (j < 1)
  {
    input_error++;
    error_msg ("Expecting numeric values for analytical expression.",
	       CONTINUE);
    return (ERROR);
  }
  return (OK);
}
/* VP: Density End */

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_viscosity_parms(const char* cptr, LDBLE * Jones_Dole)
/* ---------------------------------------------------------------------- */
{
  int j;
/*
 *   Read .
 */
  for (j = 0; j <= 9; j++)
  {
    Jones_Dole[j] = 0.0;
  }
  j = sscanf (cptr, SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT,
	&(Jones_Dole[0]), &(Jones_Dole[1]), &(Jones_Dole[2]), &(Jones_Dole[3]), &(Jones_Dole[4]), &(Jones_Dole[5]), &(Jones_Dole[6]), &(Jones_Dole[7]), &(Jones_Dole[8]), &(Jones_Dole[9]));
  if (j < 1)
  {
    input_error++;
    error_msg ("Expecting numeric values for viscosity calculation.",
	       CONTINUE);
    return (ERROR);
  }
  return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_incremental_reactions(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Define flow only
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int j, l;
	const char* cptr;
	char token[MAX_LENGTH];

	cptr = line;
	/* read keyword */
	copy_token(token, &cptr, &l);

	/* read true or false */
	incremental_reactions = get_true_false(cptr, TRUE);
/*
 *   find next keyword
 */
	while ((j =
			check_line("Subroutine Read", FALSE, TRUE, TRUE,
					   FALSE)) != KEYWORD)
	{
		/* empty, eof, keyword, print */
		if (j == EOF)
			return (EOF);
		error_string = sformatf( "Unknown input: %s", line);
		error_msg(error_string, CONTINUE);
		input_error++;
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_master_species(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads master species data from data file or input file
 */
	int j, i, l;
	const char* cptr, *cptr1;
	LDBLE l_z;
	class element *elts_ptr;
	class species *s_ptr;
	char token[MAX_LENGTH];

	elts_ptr = NULL;
	for (;;)
	{
		j = check_line("Master species", FALSE, TRUE, TRUE, TRUE);
		if (j == EOF || j == KEYWORD)
		{
			break;
		}
/*
 *   Get element name with valence, allocate space, store
 */
		cptr = line;
/*
 *   Get element name and save pointer to character string
 */
		if (copy_token(token, &cptr, &l) != UPPER && token[0] != '[')
		{
			parse_error++;
			error_msg("Reading element for master species.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
		/*
		   if (token[0] == '[') {
		   cptr1 = token;
		   get_elt(&cptr, element, &l);
		   strcpy(token, element);
		   }
		 */
		replace("(+", "(", token);
/*
 *   Delete master if it exists
 */
		master_delete(token);
/*
 *   Increase pointer array, if necessary,  and malloc space
 */
		size_t count_master = master.size();
		master.resize(count_master + 1);
		master[count_master] = master_alloc();
/*
 *   Set type to AQ
 */
		master[count_master]->type = AQ;
/*
 *   Save element name
 */
		master[count_master]->elt = element_store(token);
		std::string ename = token;
/*
 *   Save pointer to species data for master species
 */
		if ((copy_token(token, &cptr, &l) != UPPER) &&
			token[0] != '[' && (strcmp_nocase_arg1(token, "e-") != 0))
		{
			parse_error++;
			error_msg("Reading master species name.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}

		s_ptr = s_search(token);
		if (s_ptr != NULL)
		{
			master[count_master]->s = s_ptr;
		}
		else
		{
			cptr1 = token;
			std::string token1;
			get_token(&cptr1, token1, &l_z, &l);
			master[count_master]->s = s_store(token1.c_str(), l_z, FALSE);
		}
		
		std::string sname = token;
		replace("("," ", ename);
		std::istringstream iss(ename);
		iss >> ename;
		if (ename != "e" && ename != "E" && ename != "Alkalinity" && std::string::npos == sname.find(ename))
		{
			input_error++;
			std::ostringstream oss;
			oss << "Master species, " << sname << " must contain the element, " << ename;
			error_msg(oss.str().c_str(), CONTINUE);
			continue;
		}

/*
 *   Read alkalinity for species
 */
		copy_token(token, &cptr, &l);
		i = sscanf(token, SCANFORMAT, &master[count_master]->alk);
		if (i != 1)
		{
			input_error++;
			if (elts_ptr != NULL)
			{
				error_string = sformatf(
						"Expected alkalinity for master species, %s, in master species input.",
						elts_ptr->name);
			}
			else
			{
				error_string = sformatf(
						"Expected alkalinity for master species in master species input.");
			}
			error_msg(error_string, CONTINUE);
			continue;
		}
/*
 *   Read default gfw for species
 */
		i = copy_token(token, &cptr, &l);
		if (i == DIGIT)
		{
			(void)sscanf(token, SCANFORMAT, &master[count_master]->gfw);
		}
		else if (i == UPPER)
		{
			master[count_master]->gfw_formula = string_hsave(token);
		}
		else
		{
			input_error++;
			if (elts_ptr != NULL)
			{
				error_string = sformatf(
						"Expected gram formula weight for master species, %s, in master species input.",
						elts_ptr->name);
			}
			else
			{
				error_string = sformatf(
						"Expected gram formula weight for master species in master species input.");
			}
			error_msg(error_string, CONTINUE);
			continue;
		}
/*
 *   MAKE LISTS OF PRIMARY AND SECONDARY MASTER SPECIES
 */
		if (strchr(master[count_master]->elt->name, '(') == NULL)
		{
			master[count_master]->primary = TRUE;
			/* Read gram formula weight for primary */
			if (strcmp(master[count_master]->elt->name, "E") != 0)
			{
				elts_ptr = master[count_master]->elt;
				i = copy_token(token, &cptr, &l);
				if (i == DIGIT)
				{
					(void)sscanf(token, SCANFORMAT, &elts_ptr->gfw);
				}
				else
				{
					input_error++;
					if (elts_ptr != NULL)
					{
						error_string = sformatf(
								"Expected gram formula weight for element, %s.",
								elts_ptr->name);
					}
					else
					{
						error_string = sformatf(
								"Expected gram formula weight for element.");
					}

					error_msg(error_string, CONTINUE);
					continue;
				}
			}
		}
		else
		{
			master[count_master]->primary = FALSE;
		}
	}
	gfw_map.clear();
	return (j);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_mix(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads mixing fractions
 */
	int n_user;
	int return_value;
	int n_solution;
	LDBLE fraction;
	int j, i, l;
	const char* cptr;
	char token[MAX_LENGTH];
	cxxMix temp_mix;

	cptr = line;
	temp_mix.read_number_description(cptr);
	n_user = temp_mix.Get_n_user();
/*
 *   Set use data to first read
 */
	if (use.Get_mix_in() == FALSE)
	{
		use.Set_mix_in(true);
		use.Set_n_mix_user(n_user);
	}
/*
 *   Read mixture data
 */
	for (;;)
	{
		return_value = check_line("Mixture data", FALSE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		if (return_value == EOF || return_value == KEYWORD)
		{
			break;
		}
		cptr = line;
/*
 *   Read n_user
 */
		i = copy_token(token, &cptr, &l);
		if (i == DIGIT)
		{
			(void)sscanf(token, "%d ", &n_solution);
		}
		else
		{
			input_error++;
			error_msg("Expected a solution number in mix input.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
/*
 *   Read fraction for solution
 */
		copy_token(token, &cptr, &l);
		j = sscanf(token, SCANFORMAT, &fraction);
		if (j != 1)
		{
			input_error++;
			error_msg("Expected a mixing fraction.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
		
/*
 *   Save data
 */
		temp_mix.Add(n_solution ,fraction);
	}
	if (temp_mix.Get_mixComps().size() == 0)
	{
		input_error++;
		error_msg
			("Must define at least one solution number and mixing fraction for MIX input.",
			 CONTINUE);
	}
	Rxn_mix_map[n_user] = temp_mix;

	// copy if needed
	if (temp_mix.Get_n_user_end() > n_user)
	{
		int i;
		for (i = n_user + 1; i <= temp_mix.Get_n_user_end(); i++)
		{
			Utilities::Rxn_copy(Rxn_mix_map, n_user, i);
		}
	}

	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_entity_mix(std::map<int, cxxMix> &mix_map)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads mixing fractions
 */
	int return_value;
	int n_solution;
	LDBLE fraction;
	int j, i, l;
	const char* cptr;
	char token[MAX_LENGTH];
	cxxMix temp_mix;

/*
 *   Read mix number
 */
	cptr = line;
	temp_mix.read_number_description(line);
/*
 *   Read mixture data
 */
	for (;;)
	{
		return_value = check_line("Mix raw data", FALSE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		if (return_value == EOF || return_value == KEYWORD)
		{
			break;
		}
		cptr = line;
/*
 *   Read n_user
 */
		i = copy_token(token, &cptr, &l);
		if (i == DIGIT)
		{
			(void)sscanf(token, "%d ", &n_solution);
		}
		else
		{
			input_error++;
			error_msg("Expected a number in mix input.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
/*
 *   Read fraction for entity
 */
		copy_token(token, &cptr, &l);
		j = sscanf(token, SCANFORMAT, &fraction);
		if (j != 1)
		{
			input_error++;
			error_msg("Expected a mixing fraction.", CONTINUE);
			error_msg(line_save, CONTINUE);
			continue;
		}
/*
 *   Save data
 */
		temp_mix.Add(n_solution ,fraction);
	}
	if (temp_mix.Get_mixComps().size() == 0)
	{
		input_error++;
		error_msg
			("Must define at least one number and mixing fraction for mix input.",
			 CONTINUE);
	}
	mix_map[temp_mix.Get_n_user()] = temp_mix;
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_number_description(const char* cptr, int *n_user,
						int *n_user_end, char **description, int allow_negative)
/* ---------------------------------------------------------------------- */
{
	int l, n;
	char token[MAX_LENGTH];
	const char* cptr1;
/*
 *   Read user number, allow negative numbers Oct 3, 2011
 */
	copy_token(token, &cptr, &l);  // keyword
	cptr1 = cptr;
	copy_token(token, &cptr, &l);

	if (!isdigit(token[0]) && token[0] != '-')
	{
		*n_user = 1;
		*n_user_end = 1;
	}
	else
	{
		if (replace("-", " ", &token[1]))
		{
			n = sscanf(token, "%d%d", n_user, n_user_end);
			if (n != 2)
			{
				if (n == 0)
				{
					*n_user_end = *n_user = 1;
				}
				else
				{
					*n_user_end = *n_user;
				}
				if (next_keyword >= 0)
				{
					error_string = sformatf( "Reading number range for %s.", Keywords::Keyword_name_search(next_keyword).c_str());
				}
				else
				{
					error_string = sformatf( "Reading number range for keyword.");
				}
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			cptr1 = cptr;
		}
		else
		{
			n = sscanf(token, "%d", n_user);
			if (n != 1)
			{
				if (next_keyword >= 0)
				{
					error_string = sformatf( "Reading number range for %s.", Keywords::Keyword_name_search(next_keyword).c_str());
				}
				else
				{
					error_string = sformatf( "Reading number range for keyword.");
				}
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			*n_user_end = *n_user;
			cptr1 = cptr;
		};
	}
	if (*n_user < 0 && allow_negative == FALSE)
	{
		error_string = sformatf( "Negative number in number range not allowed for keyword.");
		error_msg(error_string, CONTINUE);
		input_error++;
	}
/*
 *   Read description
 */
	for (; isspace((int) cptr1[0]); cptr1++);
	*description = string_duplicate(cptr1);
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_phases(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read data for phases, parse equations
 */
	int j, i, l;
	int association;
	const char* cptr;
	char token[MAX_LENGTH];
	char token1[MAX_LENGTH];
	class phase *phase_ptr;
	class rxn_token *token_ptr;

	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"no_check",				/* 0 */
		"check",				/* 1 */
		"log_k",				/* 2 */
		"logk",					/* 3 */
		"delta_h",				/* 4 */
		"deltah",				/* 5 */
		"analytical_expression",	/* 6 */
		"a_e",					/* 7 */
		"ae",					/* 8 */
		"add_logk",				/* 9 */
		"add_log_k",			/* 10 */
		"add_constant",			/* 11 */
		"t_c",					/* 12 */
		"p_c",					/* 13 */
		"omega",				/* 14 */
		"vm"	/* 15, molar volume, must replace delta_v */
	};
	int count_opt_list = 16;
	association = FALSE;
/*
 *   Read eqn from file and call parser
 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	phase_ptr = NULL;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in PHASES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* no_check */
			if (phase_ptr == NULL)
				break;
			phase_ptr->check_equation = FALSE;
			break;
		case 1:				/* check */
			if (phase_ptr == NULL)
				break;
			phase_ptr->check_equation = TRUE;
			break;
		case 2:				/* log_k */
		case 3:				/* logk */
			if (phase_ptr == NULL)
				break;
			read_log_k_only(next_char, &phase_ptr->logk[0]);
			opt_save = OPTION_DEFAULT;
			break;
		case 4:				/* delta_h */
		case 5:				/* deltah */
			if (phase_ptr == NULL)
				break;
			read_delta_h_only(next_char, &phase_ptr->logk[1],
							  &phase_ptr->original_units);
			opt_save = OPTION_DEFAULT;
			break;
		case 6:				/* analytical_expression */
		case 7:				/* a_e */
		case 8:				/* ae */
			if (phase_ptr == NULL)
				break;
			read_analytical_expression_only(next_char, &(phase_ptr->logk[T_A1]));
			opt_save = OPTION_DEFAULT;
			break;
		case 9:				/* add_logk */
		case 10:				/* add_log_k */
		{
			if (phase_ptr == NULL)
				break;
			size_t count_add_logk = phase_ptr->add_logk.size();
			phase_ptr->add_logk.resize(count_add_logk + 1);
			/* read name */
			if (copy_token(token, &next_char, &i) == EMPTY)
			{
				input_error++;
				error_string = sformatf(
					"Expected the name of a NAMED_EXPRESSION.");
				error_msg(error_string, CONTINUE);
				break;
			}
			phase_ptr->add_logk[count_add_logk].name =
				string_hsave(token);
			/* read coef */
			i = sscanf(next_char, SCANFORMAT,
				&phase_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				phase_ptr->add_logk[count_add_logk].coef = 1;
			}
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 11:				/* add_constant */
		{
			if (phase_ptr == NULL)
				break;
			size_t count_add_logk = phase_ptr->add_logk.size();
			phase_ptr->add_logk.resize(count_add_logk + 1);
			i = sscanf(next_char, SCANFORMAT,
				&phase_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				input_error++;
				error_string = sformatf(
					"Expected the constant to add for log_K definition.");
				error_msg(error_string, CONTINUE);
				break;
			}
			/* set name */
			phase_ptr->add_logk[count_add_logk].name =
				string_hsave("XconstantX");
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 12:				/* T_c */
			if (phase_ptr == NULL)
				break;
			read_t_c_only(next_char, &phase_ptr->t_c);
			opt_save = OPTION_DEFAULT;
			break;
		case 13:				/* P_c */
			if (phase_ptr == NULL)
				break;
			read_p_c_only(next_char, &phase_ptr->p_c);
			opt_save = OPTION_DEFAULT;
			break;
		case 14:				/* Omega */
			if (phase_ptr == NULL)
				break;
			read_omega_only(next_char, &phase_ptr->omega);
			opt_save = OPTION_DEFAULT;
			break;
		case 15:            /* vm, molar volume */
			if (phase_ptr == NULL)
				break;
			read_phase_vm(next_char, &(phase_ptr->logk[vm0]),
				&phase_ptr->original_deltav_units);
			phase_ptr->delta_v[1] = phase_ptr->logk[vm0];
			opt_save = OPTION_DEFAULT;
			break;
		case OPTION_DEFAULT:
		{
			/*
			 *   Get element name and save pointer to character string
			 */
			phase_ptr = NULL;
			cptr = line;
			copy_token(token, &cptr, &l);
			/*
			 *   Get and parse equation
			 */
			j = check_line("Phase equation", FALSE, TRUE, TRUE, TRUE);
			if (j == EOF || j == KEYWORD)
			{
				return_value = j;
				break;
			}
			if (j == OPTION)
			{
				parse_error++;
				error_string = sformatf("Expecting equation for phase %s.", token);
				error_msg(error_string, CONTINUE);
				error_msg("Parsing equation.", CONTINUE);
				error_msg(line_save, CONTINUE);
				break;
			}
			std::vector<class elt_list> new_elt_list;
			if (parse_eq(line, new_elt_list, association) == ERROR)
			{
				parse_error++;
				error_msg("Parsing equation.", CONTINUE);
				error_msg(line_save, CONTINUE);
				break;
			}
			phase_ptr = phase_store(token);
			/*
			 *   Get pointer to each species in the reaction, store new species if necessary
			 */
			strcpy(token1, trxn.token[0].name);
			replace("(g)", "", token1);
			replace("(s)", "", token1);
			replace("(G)", "", token1);
			replace("(S)", "", token1);
			phase_ptr->formula = string_hsave(token1);
			for (i = 1; i < count_trxn; i++)
			{
				if ((strstr(trxn.token[i].name, "(s)") == NULL) &&
					(strstr(trxn.token[i].name, "(g)") == NULL) &&
					(strstr(trxn.token[i].name, "(S)") == NULL) &&
					(strstr(trxn.token[i].name, "(G)") == NULL))
				{
					strcpy(token1, trxn.token[i].name);
					replace("(aq)", "", token1);
					replace("(AQ)", "", token1);
					replace("H2O(l)", "H2O", token1);
					replace("(H2O(L)", "H2O", token1);
					trxn.token[i].s = s_store(token1, trxn.token[i].z, FALSE);
				}
				else
				{
					trxn.token[i].s = NULL;
				}
			}
			/*
			 *   Save element list
			 */
			phase_ptr->next_elt = new_elt_list;
 /*
  *   Copy reaction to reaction for phase, first token (token[0]) is not used
  *   except to check that coef of phase formula = 1.0
  */
			trxn_copy(phase_ptr->rxn);
			token_ptr = &phase_ptr->rxn.token[0];
			token_ptr[0].name = trxn.token[1].name;
			token_ptr[i].s = NULL;
			token_ptr[i].name = NULL;
			/*
			 *   Set type for phase
			 */
			phase_ptr->type = SOLID;
			opt_save = OPTION_DEFAULT;
		}
		break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_pp_assemblage(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads pp_assemblage data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int j;
	int return_value;
	int n_user;
	const char* cptr;
	std::string token;
	int opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"force_equality"		/* 0 */
	};
	int count_opt_list = 1;
	/*
	 *   Find pp_assemblage or realloc space for pp_assemblage
	 */
	cxxPPassemblage temp_pp_assemblage;
	cptr = line;
	temp_pp_assemblage.read_number_description(cptr);
	n_user = temp_pp_assemblage.Get_n_user();
	cxxPPassemblageComp *comp = NULL;
	std::map<std::string, cxxPPassemblageComp> comps;
	temp_pp_assemblage.Set_new_def(true);
	/*
	 *   Set use data to first read
	 */
	if (use.Get_pp_assemblage_in() == FALSE)
	{
		use.Set_pp_assemblage_in(true);
		use.Set_n_pp_assemblage_user(n_user);
	}
	/*
	 *  Read equilibrium phase data
	 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in EQUILIBRIUM_PHASES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* force_equality */
			if (comp == NULL)
			{
				error_msg
					("Force_equality defined before equilibrium phase has been defined.",
					 CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
			}
			else
			{
				comp->Set_force_equality(get_true_false(next_char, TRUE) == TRUE);
			}
			break;
		case OPTION_DEFAULT:
			/*
			 *   Make space, set default
			 */
			if (comp)
			{
				comps[comp->Get_name()] = *comp;
			}
			delete comp;
			comp = new cxxPPassemblageComp;
			/*
			 *   Read name
			 */
			cptr = line;
			copy_token(token, &cptr);
			comp->Set_name(token.c_str());

			if ((j = copy_token(token, &cptr)) == EMPTY)
				continue;
			/*
			 *   Read saturation index
			 */
			j = sscanf(token.c_str(), SCANFORMAT, &dummy);
			comp->Set_si(dummy);
			comp->Set_si_org(dummy);
			if (j != 1)
			{
				error_msg("Expected saturation index.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
				continue;
			}
			/*
			 *   Adding a reaction to the phase boundary
			 */
			if ((j = copy_token(token, &cptr)) == EMPTY)
				continue;
			if (j == UPPER || j == LOWER)
			{
				comp->Set_add_formula(token.c_str());
				j = copy_token(token, &cptr);
			}
			/*
			 *   Read amount
			 */
			if (j == EMPTY)
				continue;
			j = sscanf(token.c_str(), SCANFORMAT, &dummy);
			if (dummy < 0)
			{
				error_string = sformatf( "Moles of mineral < 0, reset to 0.");
				dummy = 0;
				warning_msg(error_string);
			}
			comp->Set_moles(dummy);
			if (j != 1)
			{
				error_msg("Expected amount of mineral.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
				continue;
			}
			if ((j = copy_token(token, &cptr)) == EMPTY)
				continue;
			Utilities::str_tolower(token);
			if (strstr(token.c_str(), "d") == token.c_str())
			{
				comp->Set_dissolve_only(true);
				comp->Set_precipitate_only(false);
			} else if (strstr(token.c_str(), "p") == token.c_str())
			{
				comp->Set_precipitate_only(true);
				comp->Set_dissolve_only(false);
			}
			else
			{
				error_msg
					("Unexpected data at end of equilibrium-phase definition.",
					 CONTINUE);
				input_error++;
				continue;
			}
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	if (comp)
	{
		comps[comp->Get_name()] = *comp;
		delete comp;
		comp = NULL;
	}
	temp_pp_assemblage.Set_pp_assemblage_comps(comps);
	Rxn_pp_assemblage_map[n_user] = temp_pp_assemblage;
	Rxn_new_pp_assemblage.insert(n_user);
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_reaction(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads reaction data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
/*
 *   Read reaction
 */
	int l;
	const char* cptr;
	char token[MAX_LENGTH];
	int return_value;
	int n_user;
/*
 *   Defaults
 */
	cxxReaction temp_reaction;
	cptr = line;
	temp_reaction.read_number_description(cptr);
	n_user = temp_reaction.Get_n_user();
/*
 *   Set use data to first read
 */
	if (use.Get_reaction_in() == FALSE)
	{
		use.Set_reaction_in(true);
		use.Set_n_reaction_user(n_user);
	}
/*
 *   Read reaction data
 */
	for (;;)
	{
/*
 *   Read line
 */
		return_value = check_line("Reaction data", FALSE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		if (return_value == EOF || return_value == KEYWORD)
		{
			break;
		}
		cptr = line;
		copy_token(token, &cptr, &l);
		if (isalpha((int) token[0]) || (token[0] == '(') || (token[0] == '['))
		{
/*
 *   Read reactant information
 */
			read_reaction_reactants(&temp_reaction);
		}
		else
		{
/*
 *   Read steps information
 */
			read_reaction_steps(&temp_reaction);
		}
	}
/*
 *   Default 1 mol of reaction
 */
	if (temp_reaction.Get_steps().size() == 0)
	{
		std::vector<LDBLE> v;
		v.push_back(1.0);
		temp_reaction.Set_steps(v);
	}
	if (temp_reaction.Get_equalIncrements())
	{
		if (temp_reaction.Get_countSteps() == 0)
		{
			temp_reaction.Set_countSteps(1);
		}
	}
	Rxn_reaction_map[n_user] = temp_reaction;
	// copy if needed
	Utilities::Rxn_copies(Rxn_reaction_map, n_user, temp_reaction.Get_n_user_end());

	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_reaction_reactants(cxxReaction *reaction_ptr)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read reactants, may be a chemical formula or a pure_phase name
 *   followed by relative reaction coefficient, default 1.0.
 *
 */
	std::string token, last_token;
	LDBLE coef;
	const char* cptr;
/*
 *   Read one or more reactants
 */
	cptr = line;
	while (copy_token(token, &cptr) != EMPTY)
	{
/*
 *   Store reactant name, default coefficient
 */
		if (isalpha((int) token[0]) || (token[0] == '(') || (token[0] == '['))
		{

			reaction_ptr->Get_reactantList()[token] = 1.0;
			last_token = token;
/*
 *   Store relative coefficient
 */
		}
		else
		{
			int j = sscanf(token.c_str(), SCANFORMAT, &coef);
			if (j == 1 && last_token.size() > 0)
			{
				reaction_ptr->Get_reactantList()[last_token] = coef;
			}
			else
			{
				error_msg("Reading relative coefficient of reactant.",
						  CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
			}
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_reaction_steps(cxxReaction *reaction_ptr)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read amount(s) of irrev reactions in one of three forms:
 *
 *   6 millimoles in 6 steps   or
 *
 *   1 2 3 4 5 6 millimoles    or
 *
 *   6*1 millimoles
 *   INCREMENTAL_REACTIONS
 */

	const char* cptr;
	std::string token, token1;

	cptr = line;
/*
 *   Read one or more reaction increments
 */
	for (;;)
	{
		if (copy_token(token, &cptr) == EMPTY)
		{
			return (OK);
		}
/*
 *   Read next step increment
 */
/* begin modif 29 july 2005... */
		if (replace("*", " ", token))
		{
			int n;
			LDBLE value;
			if (sscanf(token.c_str(), "%d" SCANFORMAT, &n, &value) == 2)
			{
				for (int i = 0; i < n; i++)
				{
					reaction_ptr->Get_steps().push_back(value);
				}
			}
			else
			{
				input_error++;
				error_msg
					("Format error in multiple, equal REACTION steps.\nCorrect is (for example): 0.2 4*0.1 2*0.5 0.3\n",
					 CONTINUE);
			}
		}
		else
		{
			LDBLE step;
			int j = sscanf(token.c_str(), SCANFORMAT, &step);
			if (j == 1)
			{
				reaction_ptr->Get_steps().push_back(step);
			}
			else
			{
				break;
			}
		}
/* ...end modif 29 july 2005 */
	}
/*
 *   Read units
 */
	token1 = token;
	token1.append("/l");
	std::string t1 = token1;
	if (check_units(t1, false, false, NULL, false) == OK)
	{
		replace("/l", "", t1);
		if (strstr(t1.c_str(), "Mol") == NULL)
		{
			error_string = sformatf( "Units of steps not in moles, %s.", token.c_str());
			error_msg(error_string, CONTINUE);
			input_error++;
			return (ERROR);
		}
		else
		{
			reaction_ptr->Set_units(t1.c_str());
		}
		if (copy_token(token, &cptr) == EMPTY)
		{
			return (OK);
		}
	}
/*
 *  Read number of equal increments, store as negative integer
 */
	if (reaction_ptr->Get_reaction_steps() != 1)
	{
		error_msg
			("To define equal increments, only one reaction increment should be defined.",
			 CONTINUE);
		input_error++;
		return (ERROR);
	}
	do
	{
		int i;
		int j = sscanf(token.c_str(), "%d", &i);
		if (j == 1 && i > 0)
		{
			reaction_ptr->Set_countSteps(i);
			reaction_ptr->Set_equalIncrements(true);
			return (OK);
		}
		else if (j == 1 && i <= 0)
		{
			break;
		}
	}
	while (copy_token(token, &cptr) != EMPTY);

	error_msg("Expecting positive number for number of equal "
			  "increments to add.", CONTINUE);
	error_msg(line_save, CONTINUE);
	input_error++;
	return (ERROR);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_save(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads solution, mix, irreversible reaction, and pure phases to use
 *   in reaction calculation
 */
	int i, l, n, n_user, n_user_end;
	const char* cptr;
	char token[MAX_LENGTH];
/*
 *   Read "save"
 */
	cptr = line;
	copy_token(token, &cptr, &l);
/*
 *   Read keyword
 */
	copy_token(token, &cptr, &l);
	check_key(token);
/*
 *   Read number
 */
	for (;;)
	{
		i = copy_token(token, &cptr, &l);
		if (i == DIGIT)
		{
			replace("-", " ", token);
			n = sscanf(token, "%d%d", &n_user, &n_user_end);
			if (n == 1)
			{
				n_user_end = n_user;
			}
			if (n_user < 0)
			{
				error_msg("Number must be a positive integer.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
			}
			break;
		}
		else if (i == EMPTY)
		{
			error_string = sformatf( "No number given, 1 assumed.");
			warning_msg(error_string);
			n_user = 1;
			n_user_end = 1;
			break;
		}
	}

	switch (next_keyword)
	{
	case Keywords::KEY_SOLUTION:								/* Solution */
		save.solution = TRUE;
		save.n_solution_user = n_user;
		save.n_solution_user_end = n_user_end;
		break;
	case Keywords::KEY_EQUILIBRIUM_PHASES:					/* Pure phases */
		save.pp_assemblage = TRUE;
		save.n_pp_assemblage_user = n_user;
		save.n_pp_assemblage_user_end = n_user_end;
		break;
	case Keywords::KEY_EXCHANGE:								/* exchange */
		save.exchange = TRUE;
		save.n_exchange_user = n_user;
		save.n_exchange_user_end = n_user_end;
		break;
	case Keywords::KEY_SURFACE:								/* surface */
		save.surface = TRUE;
		save.n_surface_user = n_user;
		save.n_surface_user_end = n_user_end;
		break;
	case Keywords::KEY_GAS_PHASE:								/* gas_phase */
		save.gas_phase = TRUE;
		save.n_gas_phase_user = n_user;
		save.n_gas_phase_user_end = n_user_end;
		break;
	case Keywords::KEY_SOLID_SOLUTIONS:						/* solid_solutions */
		save.ss_assemblage = TRUE;
		save.n_ss_assemblage_user = n_user;
		save.n_ss_assemblage_user_end = n_user_end;
		break;
	default:
		input_error++;
		error_msg
			("Expecting keyword solution, equilibrium_phases, exchange, surface, gas_phase, or solid_solutions.",
			CONTINUE);
		error_msg(line_save, CONTINUE);
		check_line("End of save", FALSE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		return (ERROR);
	}

	check_line("End of save", FALSE, TRUE, TRUE, TRUE);
	/* empty, eof, keyword, print */
	return (OK);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_selected_output(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read data for to output to flat file
 */
	int value;
	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"file",					/* 0 */
		"totals",				/* 1 */
		"molalities",			/* 2 */
		"activities",			/* 3 */
		"pure_phases",			/* 4 */
		"si",					/* 5 */
		"saturation_indices",	/* 6 */
		"gases",				/* 7 */
		"equilibrium_phases",	/* 8 */
		"equilibria",       	/* 9 */
		"equilibrium",			/* 10 */
		"pure",					/* 11 */
		"inverse",				/* 12 */
		"kinetic_reactants",	/* 13 */
		"kinetics",				/* 14 */
		"solid_solutions",		/* 15 */
		"inverse_modeling",		/* 16 */
		"reset",				/* 17 */
		"simulation",			/* 18 */
		"sim",					/* 19 */
		"state",				/* 20 */
		"solution",				/* 21 */
		"soln",					/* 22 */
		"distance",				/* 23 */
		"dist",					/* 24 */
		"time",					/* 25 */
		"step",					/* 26 */
		"reaction",				/* 27 */
		"rxn",					/* 28 */
		"temperature",			/* 29 */
		"temp",					/* 30 */
		"ph",					/* 31 */
		"pe",					/* 32 */
		"alkalinity",			/* 33 */
		"alk",					/* 34 */
		"ionic_strength",		/* 35 */
		"mu",					/* 36 */
		"water",				/* 37 */
		"high_precision",		/* 38 */
		"user_punch",			/* 39 */
		"mol",					/* 40 */
		"kin",					/* 41 */
		"charge_balance",		/* 42 */
		"percent_error",		/* 43 */
		"selected_out",			/* 44 */
		"selected_output",		/* 45 */
		"isotopes",				/* 46 */
		"calculate_values",		/* 47 */
		"equilibrium_phase",    /* 48 */
		"active",               /* 49 */
		"new_line"            /* 50 */
	};
	int count_opt_list = 51;

	int i, l;
	char token[MAX_LENGTH];
	std::string file_name;
	const char* cptr;
	int n_user;

	SelectedOutput temp_selected_output;
	cptr = line;
	temp_selected_output.read_number_description(cptr);
	n_user = temp_selected_output.Get_n_user();
	temp_selected_output.Set_new_def(false);
	temp_selected_output.Set_file_name(n_user);
	// find if it exists
	std::map< int, SelectedOutput >::iterator so = SelectedOutput_map.find(n_user);
	if (n_user == 1 && so != SelectedOutput_map.end())
	{
		// n_user = 1, old definition, keep old definition
		SelectedOutput & so_ref = so->second;
		temp_selected_output.Set_active           ( so_ref.Get_active() );
		temp_selected_output.Set_new_line         ( so_ref.Get_new_line() );
		temp_selected_output.Set_inverse          ( so_ref.Get_inverse() );
		temp_selected_output.Set_sim              ( so_ref.Get_sim() );
		temp_selected_output.Set_state            ( so_ref.Get_state() );
		temp_selected_output.Set_soln             ( so_ref.Get_soln() );
		temp_selected_output.Set_dist             ( so_ref.Get_dist() );
		temp_selected_output.Set_time             ( so_ref.Get_time() );
		temp_selected_output.Set_step             ( so_ref.Get_step() );
		temp_selected_output.Set_rxn              ( so_ref.Get_rxn() );
		temp_selected_output.Set_temp             ( so_ref.Get_temp() );
		temp_selected_output.Set_ph               ( so_ref.Get_ph() );
		temp_selected_output.Set_pe               ( so_ref.Get_pe() );
		temp_selected_output.Set_alk              ( so_ref.Get_alk() );
		temp_selected_output.Set_mu               ( so_ref.Get_mu() );
		temp_selected_output.Set_water            ( so_ref.Get_water() );
		temp_selected_output.Set_high_precision   ( so_ref.Get_high_precision() );
		temp_selected_output.Set_user_punch       ( so_ref.Get_user_punch() );
		temp_selected_output.Set_charge_balance   ( so_ref.Get_charge_balance() );
		temp_selected_output.Set_percent_error    ( so_ref.Get_percent_error() );
		temp_selected_output.Set_have_punch_name  ( so_ref.Get_have_punch_name() );
		temp_selected_output.Set_file_name        ( so_ref.Get_file_name() );
#if PHREEQCI_GUI
		assert(false);
#endif
	}
	else if (n_user == 1 && so == SelectedOutput_map.end())
	{
		// n_user = 1, new definition, do nothing use; constructor default
		temp_selected_output.Set_new_def(true);
	}
	else 
	{
		// n_user != 1 then reset false

		temp_selected_output.Reset(false);
		if (so == SelectedOutput_map.end())
		{
			temp_selected_output.Set_new_def(true);
		}
	}
	CParser parser(this->phrq_io);

/*
 *   Read eqn from file and call parser
 */
	opt_save = OPTION_ERROR;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = opt;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SELECTED_OUTPUT keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* file name */
		{
			temp_selected_output.Set_new_def(true);
			std::string temp_name(next_char);
			string_trim(temp_name);
			if (temp_name.size() > 0)
			{
				file_name = temp_name;
				temp_selected_output.Set_file_name(file_name);
				temp_selected_output.Set_have_punch_name(true);
			}
			opt_save = OPTION_ERROR;
		}
		break;
		case 1:				/* totals */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				if (i != UPPER && token[0] != '[')
				{
					error_string = sformatf( "Expected element name to"
							" begin with upper case letter.");
					warning_msg(error_string);
				}
				else
				{
					std::pair< std::string, void *> t_pair(token, ((void *)0));
					temp_selected_output.Get_totals().push_back(t_pair);
				}
			}
			break;
		case 2:				/* molalities */
		case 40:				/* mol */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				if (i != UPPER && token[0] != '(' && (token[0] != '['))
				{
					error_string = sformatf( "Expected species name to"
							" begin with upper case letter.");
					warning_msg(error_string);
				}
				else
				{
					std::pair< std::string, void *> t_pair(token, ((void *)0));
					temp_selected_output.Get_molalities().push_back(t_pair);
				}
			}
			break;
		case 3:				/* activities */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				if (i != UPPER && token[0] != '(' && (token[0] != '['))
				{
					error_string = sformatf( "Expected species name to"
							" begin with upper case letter.");
					warning_msg(error_string);
				}
				else
				{
					std::pair< std::string, void *> t_pair(token, ((void *)0));
					temp_selected_output.Get_activities().push_back(t_pair);
				}
			}
			break;
		case 4:				/* pure_phases */
		case 8:				/* equilibrium_phases */
		case 9:				/* equilibria */
		case 10:			/* equilibrium */
		case 11:			/* pure */
		case 48:            /* equilibrium_phase */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				std::pair< std::string, void *> t_pair(token, ((void *)0));
				temp_selected_output.Get_pure_phases().push_back(t_pair);
			}
			break;
		case 5:				/* si */
		case 6:				/* saturation_index */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				std::pair< std::string, void *> t_pair(token, ((void *)0));
				temp_selected_output.Get_si().push_back(t_pair);
			}
			break;
		case 7:				/* gases */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				std::pair< std::string, void *> t_pair(token, ((void *)0));
				temp_selected_output.Get_gases().push_back(t_pair);
			}
			break;
		case 12:				/* inverse */
		case 16:				/* inverse_modeling */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_inverse(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 13:				/* kinetic_reactants */
		case 14:				/* kinetics */
		case 41:				/* kin */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				std::pair< std::string, void *> t_pair(token, ((void *)0));
				temp_selected_output.Get_kinetics().push_back(t_pair);
			}
			break;
		case 15:				/* solid_solutions */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				std::pair< std::string, void *> t_pair(token, ((void *)0));
				temp_selected_output.Get_s_s().push_back(t_pair);
			}
			break;
		case 46:				/* isotopes */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				if (i != UPPER && token[0] != '[')
				{
					error_string = sformatf( "Expected element name to"
							" begin with upper case letter.");
					warning_msg(error_string);
				}
				else
				{
					std::pair< std::string, void *> t_pair(token, ((void *)0));
					temp_selected_output.Get_isotopes().push_back(t_pair);
				}
			}
			break;
		case 47:				/* calculate_values */
			temp_selected_output.Set_new_def(true);
			while ((i = copy_token(token, &next_char, &l)) != EMPTY)
			{
				std::pair< std::string, void *> t_pair(token, ((void *)0));
				temp_selected_output.Get_calculate_values().push_back(t_pair);
			}
			break;
		case 17:				/* reset */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			/* matches print order */
			temp_selected_output.Reset(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 18:				/* simulation */
		case 19:				/* sim */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_sim(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 20:				/* state */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_state(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 21:				/* solution */
		case 22:				/* soln */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_soln(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 23:				/* distance */
		case 24:				/* dist */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_dist(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 25:				/* time */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_time(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 26:				/* step */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_step(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 27:				/* reaction */
		case 28:				/* rxn */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_rxn(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 29:				/* temperature */
		case 30:				/* temp */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_temp(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 31:				/* ph */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_ph(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 32:				/* pe */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_pe(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 33:				/* alkalinity */
		case 34:				/* alk */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_alk(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 35:				/* ionic strength */
		case 36:				/* mu */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_mu(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 37:				/* water */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_water(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 38:				/* high_precision */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_high_precision(value!=FALSE);
			//if (n_user == 1)
			//{
			//	high_precision = (value != FALSE);
			//}
			if (value == TRUE)
			{
				convergence_tolerance = 1e-12;
			}
			opt_save = OPTION_ERROR;
			break;
		case 39:				/* user_punch */
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_user_punch(value!=FALSE);
			if (so != SelectedOutput_map.end())
			{
				so->second.Set_user_punch(value!=FALSE);
			}			opt_save = OPTION_ERROR;
			break;
		case 42:				/* charge_balance */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_charge_balance(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 43:				/* percent_error */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_percent_error(value!=FALSE);
			opt_save = OPTION_ERROR;
			break;
		case 44:				/* selected_out */
		case 45:				/* selected_output */
			//warning_msg("Use PRINT; -selected_output, not SELECTED_OUTPUT; -selected_output");
			//value = get_true_false(next_char, TRUE);
			//temp_selected_output.Set_active(value!=FALSE);
			//opt_save = OPTION_ERROR;
			//break;
		case 49:				/* active */
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_active(value!=FALSE);
			if (so != SelectedOutput_map.end())
			{
				so->second.Set_active(value!=FALSE);
			}
			opt_save = OPTION_ERROR;
			break;
		case 50:				/* new_line */
			temp_selected_output.Set_new_def(true);
			value = get_true_false(next_char, TRUE);
			temp_selected_output.Set_new_line(value != FALSE);
			opt_save = OPTION_ERROR;
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	
	if (temp_selected_output.Get_new_def() || so == SelectedOutput_map.end())
	{
		// delete if exists
		if (so != SelectedOutput_map.end())
		{
			SelectedOutput_map.erase(so);
		}

		// store new selected output
		SelectedOutput_map[n_user] = temp_selected_output;

		if (punch_open(SelectedOutput_map[n_user].Get_file_name().c_str(), n_user))
		{
			if (phrq_io)
			{
				SelectedOutput_map[n_user].Set_punch_ostream(phrq_io->Get_punch_ostream());
				phrq_io->Set_punch_ostream(NULL);
			}
		}
		else
		{
			error_string = sformatf( "Can`t open file, %s.", SelectedOutput_map[n_user].Get_file_name().c_str());
			input_error++;
			error_msg(error_string, CONTINUE);
		}
	}

	//if (!have_punch_name)
	//{
	//	punch_close();
	//	if (!punch_open("selected.out"))
	//	{
	//		error_string = sformatf( "Can`t open file, %s.", "selected.out");
	//		input_error++;
	//		error_msg(error_string, CONTINUE);
	//	}
	//}

	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_solution(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads solution data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int n_user;
	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"temp",					/* 0 */
		"temperature",			/* 1 */
		"dens",					/* 2 */
		"density",				/* 3 */
		"units",				/* 4 */
		"redox",				/* 5 */
		"ph",					/* 6 */
		"pe",					/* 7 */
		"unit",					/* 8 */
		"isotope",				/* 9 */
		"water",				/* 10 */
		"press",				/* 11 */
		"pressure",				/* 12 */
		"potential"				/* 13 */
	};
	int count_opt_list = 14;

	cxxSolution temp_solution;
	const char* cptr = line;
	temp_solution.read_number_description(cptr);
	n_user = temp_solution.Get_n_user();
	temp_solution.Set_new_def(true);
	temp_solution.Create_initial_data();
	cxxISolution *isoln_ptr = temp_solution.Get_initial_data();
	CParser parser(this->phrq_io);

	if (!use.Get_solution_in())
	{
		use.Set_solution_in(true);
		use.Set_n_solution_user(n_user);
	}

/*
 *   Read concentration data
 */
	std::string token;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			cptr = next_char;
			if (copy_token(token, &cptr) == CParser::TT_DIGIT)
			{
				opt = 9;
			}
		}
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SOLUTION keyword.", PHRQ_io::OT_CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* temperature */
		case 1:
			if (sscanf(next_char, SCANFORMAT, &dummy) == 1)
			{
				temp_solution.Set_tc(dummy);
			}
			break;
		case 2:				/* density */
		case 3:
			{
				copy_token(token, &next_char);
				if (sscanf(token.c_str(), SCANFORMAT, &dummy) != 1)
				{
						error_msg("Expecting numeric value for density.", PHRQ_io::OT_CONTINUE);
						error_msg(line_save, PHRQ_io::OT_CONTINUE);
						input_error++;
				}
				else
				{
					temp_solution.Set_density(dummy);
				}
				int j = copy_token(token, &next_char);
				if (j != EMPTY)
				{
					if (token[0] != 'c' && token[0] != 'C')
					{
						error_msg("Only option following density is c[alculate].", PHRQ_io::OT_CONTINUE);
						error_msg(line_save, PHRQ_io::OT_CONTINUE);
						input_error++;
					}
					else
					{
						isoln_ptr->Set_calc_density(true);
					}
				}
			}
			break;
		case 4:				/* units */
		case 8:				/* unit */
			if (copy_token(token, &next_char) == CParser::TT_EMPTY)
				break;
			{
				if (check_units(token, false, false, "mMol/kgw", false) == CParser::PARSER_OK)
				{
					isoln_ptr->Set_units(token);
				}
				else
				{
					input_error++;
				}
			}
			break;
		case 5:				/* redox */
			if (copy_token(token, &next_char) == CParser::TT_EMPTY)
				break;
			if (parser.parse_couple(token) == CParser::PARSER_OK)
			{
				const char * str = string_hsave(token.c_str());
				//isoln_ptr->Set_default_pe(token);
				isoln_ptr->Set_default_pe(str);
				CReaction temp_chem_reaction;
				isoln_ptr->Get_pe_reactions()[token] = temp_chem_reaction;
			}
			else
			{
				input_error++;
			}
			break;
		case 6:				/* ph */
			{
				cxxISolutionComp temp_comp(this->phrq_io);
				if (temp_comp.read(line, &temp_solution) == CParser::PARSER_ERROR)
				{
					input_error++;
					break;
				}
				
				temp_solution.Set_ph(temp_comp.Get_input_conc());
				
				if (temp_comp.Get_equation_name().size() == 0)
				{
					break;
					
				}
				temp_comp.Set_description("H(1)");
				isoln_ptr->Get_comps()[temp_comp.Get_description()] = temp_comp;
			}
			break;
		case 7:				/* pe */
			{
				cxxISolutionComp temp_comp(this->phrq_io);
				if (temp_comp.read(line, &temp_solution) == CParser::PARSER_ERROR)
				{
					input_error++;
					break;
				}
				temp_solution.Set_pe(temp_comp.Get_input_conc());
				if (temp_comp.Get_equation_name().size() == 0)
				{
					break;
				}
				temp_comp.Set_description("E");
				isoln_ptr->Get_comps()[temp_comp.Get_description()] = temp_comp;
			}
			break;
		case 9:				/* isotope */
			{
				cxxSolutionIsotope temp_isotope;
				if (copy_token(token, &next_char) !=  CParser::TT_DIGIT)
				{
					input_error++;
					error_string = sformatf( "Expected isotope name to"
						" begin with an isotopic number.");
					error_msg(error_string, PHRQ_io::OT_CONTINUE);
					error_string = sformatf( "In read_solution\n");
					error_msg(error_string, PHRQ_io::OT_CONTINUE);
					error_string = sformatf( "\t%s\t%s\n", "token:     ", token.c_str());
					error_msg(error_string, PHRQ_io::OT_CONTINUE);
					error_string = sformatf( "\t%s\t%s\n", "next_char: ", next_char);
					error_msg(error_string, PHRQ_io::OT_CONTINUE);
					error_string = sformatf( "\t%s\t%s\n", "line_save: ", line_save);
					error_msg(error_string, PHRQ_io::OT_CONTINUE);
					continue;
				}
				temp_isotope.Set_isotope_name(token.c_str());
				/* read and save element name */
				{
					std::string temp_iso_name = token.c_str();
					const char* cptr1 = temp_iso_name.c_str();
					get_num(&cptr1, &dummy);
					temp_isotope.Set_isotope_number(dummy);
					if (cptr1[0] == '\0' || isupper((int) cptr1[0]) == FALSE)
					{
						error_msg("Expecting element name.", PHRQ_io::OT_CONTINUE);
						error_msg(line_save, PHRQ_io::OT_CONTINUE);
						input_error++;
						return (CParser::PARSER_ERROR);
					}
					temp_isotope.Set_elt_name(cptr1);
				}
				/* read and store isotope ratio */
				if (copy_token(token, &next_char) != CParser::TT_DIGIT)
				{
					input_error++;
					error_string = sformatf(
						"Expected numeric value for isotope ratio.");
					error_msg(error_string, CONTINUE);
					continue;
				}
				(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
				temp_isotope.Set_ratio(dummy);
				temp_isotope.Set_ratio_uncertainty(NAN);

				/* read and store isotope ratio uncertainty */
				int j;
				if ((j = copy_token(token, &next_char)) != CParser::TT_EMPTY)
				{
					if (j != DIGIT)
					{
						input_error++;
						error_string = sformatf(
							"Expected numeric value for uncertainty in isotope ratio.");
						error_msg(error_string, PHRQ_io::OT_CONTINUE);
						continue;
					}
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					temp_isotope.Set_ratio_uncertainty(dummy);
					temp_isotope.Set_ratio_uncertainty_defined(true);
				}
				temp_solution.Get_isotopes()[temp_isotope.Get_isotope_name()] = temp_isotope;
			}
			break;
		case 10:				/* water */
			{
				int j = copy_token(token, &next_char);
				if (j == EMPTY)
				{
					temp_solution.Set_mass_water(1.0);
				}
				else if (j != DIGIT)
				{
					input_error++;
					error_string = sformatf(
						"Expected numeric value for mass of water in solution.");
					error_msg(error_string, CONTINUE);
				}
				else
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					temp_solution.Set_mass_water(dummy);
				}
			}
			break;
		case 11: /* pressure */
		case 12:
			{
				if (sscanf(next_char, SCANFORMAT, &dummy) != 1)
				{
					temp_solution.Set_patm(1);
				}
				else
				{
					temp_solution.Set_patm(dummy);
				}
			}
			break;
		case 13: /* potential, Volt */
			{
				if (sscanf(next_char, SCANFORMAT, &dummy) != 1)
				{
					temp_solution.Set_potV(0);
				}
				else
				{
					temp_solution.Set_potV(dummy);
				}
			}
			break;
		case OPTION_DEFAULT:
/*
 *   Read concentration
 */
			{
				cxxISolutionComp temp_comp(this->phrq_io);
				if (temp_comp.read(line, &temp_solution) == CParser::PARSER_ERROR)
				{
					input_error++;
					break;
				}
				isoln_ptr->Get_comps()[temp_comp.Get_description()] = temp_comp;
				if (temp_comp.Get_pe_reaction().size() > 0)
				{
					CReaction temp_chem_reaction;
					isoln_ptr->Get_pe_reactions()[temp_comp.Get_pe_reaction()] = temp_chem_reaction;
				}
			}
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
/*
 *   fix up default units and default pe
 */
	std::map < std::string, cxxISolutionComp >::iterator it;
	for (it = isoln_ptr->Get_comps().begin(); it != isoln_ptr->Get_comps().end(); it++)
	{
		token = it->first;
		Utilities::str_tolower(token);
		if (it->second.Get_units().size() == 0)
		{
			it->second.Set_units(isoln_ptr->Get_units().c_str());
		}
		else
		{
			bool alk = false;
			if (strstr(token.c_str(), "alk") == token.c_str())
				alk = true;
			std::string token1 = it->second.Get_units();
			if (check_units(token1, alk, true, isoln_ptr->Get_units().c_str(), true) ==	CParser::PARSER_ERROR)
			{
				input_error++;
			}
			else
			{
				it->second.Set_units(token1.c_str());
			}
		}
		if (it->second.Get_pe_reaction().size() == 0)
		{
			it->second.Set_pe_reaction(isoln_ptr->Get_default_pe());
		}
	}
	Rxn_solution_map[n_user] = temp_solution;
	Rxn_new_solution.insert(n_user);
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_species(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read data for aqueous species, parse equations
 */
	int i;
	int association;
	class species *s_ptr;
	const class elt_list *next_elt;
	const char* cptr;
	char token[MAX_LENGTH];
	//bool vm_read = false;
	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"no_check",				/* 0 */
		"check",				/* 1 */
		"gamma",				/* 2 */
		"mb",					/* 3 */
		"mass_balance",			/* 4 */
		"log_k",				/* 5 */
		"logk",					/* 6 */
		"delta_h",				/* 7 */
		"deltah",				/* 8 */
		"analytical_expression",	/* 9 */
		"a_e",					/* 10 */
		"ae",					/* 11 */
		"mole_balance",			/* 12 */
		"llnl_gamma",			/* 13 */
		"co2_llnl_gamma",		/* 14 */
		"activity_water",		/* 15 */
		"add_logk",				/* 16 */
		"add_log_k",			/* 17 */
		"add_constant",			/* 18 */
		"dw",					/* 19 */
		"erm_ddl",				/* 20 */
/* VP: Density Start */
		"millero",				/* 21 */
/* VP: Density End */
		"vm",		    /* 22, parms for molar volume, a1..a4 and w_ref, I terms */
		"viscosity"		/* 23, b and d parms for viscosity, (b1 + b2 * exp(-b3 * tc)) * c + (d1 * exp(-d2 * tc)) * c ^ d3 */
	};
	int count_opt_list = 24;
	association = TRUE;
	s_ptr = NULL;
/*
 *   Read eqn from file and call parser
 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
			//vm_read = false;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SOLUTION_SPECIES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* no_check */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->check_equation = FALSE;
			break;
		case 1:				/* check */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->check_equation = TRUE;
			break;
		case 2:				/* gamma data */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->gflag = 2;	/* Wateq D-H */
			i = sscanf(next_char, SCANFORMAT SCANFORMAT, &s_ptr->dha,
					   &s_ptr->dhb);
			if (i < 2)
			{
				error_string = sformatf( "Expecting 2 activity-"
						"coefficient parameters, a and b.");
				warning_msg(error_string);
			}
			opt_save = OPTION_DEFAULT;
			break;
		case 3:				/* mb */
		case 4:				/* mass_balance */
		case 12:				/* mole_balance */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			count_elts = 0;
			paren_count = 0;
			copy_token(token, &next_char, &i);
			s_ptr->mole_balance = string_hsave(token);
			cptr = token;
			s_ptr->next_secondary.clear();
			get_secondary_in_species(&cptr, 1.0);
			s_ptr->next_secondary = elt_list_vsave();
/* debug
			for (i = 0; i < count_elts; i++) {
				output_msg(sformatf("%s\t%f\n", elt_list[i].elt->name,
					elt_list[i].coef));
			}
 */
			opt_save = OPTION_DEFAULT;
			break;
		case 5:				/* log_k */
		case 6:				/* logk */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_log_k_only(next_char, &s_ptr->logk[0]);
			opt_save = OPTION_DEFAULT;
			break;
		case 7:				/* delta_h */
		case 8:				/* deltah */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_delta_h_only(next_char, &s_ptr->logk[1],
							  &s_ptr->original_units);
			opt_save = OPTION_DEFAULT;
			break;
		case 9:				/* analytical_expression */
		case 10:				/* a_e */
		case 11:				/* ae */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_analytical_expression_only(next_char, &(s_ptr->logk[T_A1]));
			opt_save = OPTION_DEFAULT;
			break;
		case 13:				/* llnl_gamma */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->gflag = 7;	/* llnl D-H */
			i = sscanf(next_char, SCANFORMAT, &s_ptr->dha);
			if (i < 1)
			{
				error_string = sformatf(
						"Expecting activity-coefficient parameter, a.");
				warning_msg(error_string);
			}
			opt_save = OPTION_DEFAULT;
			break;
		case 14:				/* co2_llnl_gamma */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->gflag = 8;	/* llnl CO2 D-H */
			opt_save = OPTION_DEFAULT;
			break;
		case 15:				/* activity water */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->gflag = 9;	/* activity_water/55.5 for HDO, D2O, H2[O18], etc */
			opt_save = OPTION_DEFAULT;
			break;
		case 16:				/* add_logk */
		case 17:				/* add_log_k */
		{
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = s_ptr->add_logk.size();
			s_ptr->add_logk.resize(count_add_logk + 1);
			/* read name */
			if (copy_token(token, &next_char, &i) == EMPTY)
			{
				input_error++;
				error_string = sformatf(
					"Expected the name of a NAMED_EXPRESSION.");
				error_msg(error_string, CONTINUE);
				break;
			}
			s_ptr->add_logk[count_add_logk].name = string_hsave(token);
			/* read coef */
			i = sscanf(next_char, SCANFORMAT,
				&s_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				s_ptr->add_logk[count_add_logk].coef = 1;
			}
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 18:				/* add_constant */
		{
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = s_ptr->add_logk.size();
			s_ptr->add_logk.resize(count_add_logk + 1);
			i = sscanf(next_char, SCANFORMAT,
				&s_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				input_error++;
				error_string = sformatf(
					"Expected the constant to add for log_K definition.");
				error_msg(error_string, CONTINUE);
				break;
			}
			/* set name */
			s_ptr->add_logk[count_add_logk].name =
				string_hsave("XconstantX");
			/* read coef */
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 19:				/* tracer diffusion coefficient */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->dw_t = 0;  s_ptr->dw_a = 0; s_ptr->dw_a2 = 0; s_ptr->dw_a_visc = 0;
			i = sscanf(next_char, SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT SCANFORMAT, &s_ptr->dw, &s_ptr->dw_t,
				&s_ptr->dw_a, &s_ptr->dw_a2, &s_ptr->dw_a_visc);
			s_ptr->dw_corr = s_ptr->dw;
			opt_save = OPTION_DEFAULT;
			break;
		case 20:				/* enrichment factor in the DDL */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			i = sscanf(next_char, SCANFORMAT, &s_ptr->erm_ddl);
			if (s_ptr->erm_ddl < 0)
			{
				error_string = sformatf( "Expecting enrichment factor > 0, "
						"resetting to erm_ddl = 1.");
				warning_msg(error_string);
				s_ptr->erm_ddl = 1.0;
			}
			opt_save = OPTION_DEFAULT;
			break;
/* VP: Density Start */
		case 21:			/* Millero a, b, c, d, e and f coefficients */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			print_density = read_millero_abcdef (next_char, &(s_ptr->millero[0]));
			//if (!vm_read)
			//{
			///* copy millero parms into logk, for calculating pressure dependency... */
			//	for (i = 0; i < 7; i++)
			//		s_ptr->logk[vm0 + i] = s_ptr->millero[i];
			//	s_ptr->logk[vm0 + i] = 0;
			//}
			opt_save = OPTION_DEFAULT;
			break;
/* VP: Density End */
		case 22:            /* vm, supcrt parms + Ionic strength terms */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
				opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_aq_species_vm_parms(next_char, &s_ptr->logk[vma1]);
			//vm_read = true;
			print_density = OK;
			opt_save = OPTION_DEFAULT;
			break;
		case 23:            /* viscosity parms for the Jones-Dole eqn */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
				opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_viscosity_parms(next_char, &s_ptr->Jones_Dole[0]);
			print_viscosity = OK;
			opt_save = OPTION_DEFAULT;
			break;
		case OPTION_DEFAULT:
		{
			/*
			 *   Get space for species information and parse equation
			 */
			s_ptr = NULL;
			std::vector<class elt_list> new_elt_list;
			if (parse_eq(line, new_elt_list, association) == ERROR)
			{
				parse_error++;
				error_msg("Parsing equation.", CONTINUE);
				error_msg(line_save, CONTINUE);
				break;
			}
			/*
			 *   Get pointer to each species in the reaction, store new species if necessary
			 */
			trxn.token[0].s =
				s_store(trxn.token[0].name, trxn.token[0].z, TRUE);
			for (i = 1; i < count_trxn; i++)
			{
				trxn.token[i].s =
					s_store(trxn.token[i].name, trxn.token[i].z, FALSE);
			}
			/*
			 *   Save element list and carbon, hydrogen, and oxygen in species
			 */
			trxn.token[0].s->next_elt = new_elt_list;
			trxn.token[0].s->next_secondary.clear();
			next_elt = &trxn.token[0].s->next_elt[0];
			for (; next_elt->elt != NULL; next_elt++)
			{
				if (strcmp(next_elt->elt->name, "C") == 0)
				{
					trxn.token[0].s->carbon = next_elt->coef;
				}
				if (strcmp(next_elt->elt->name, "H") == 0)
				{
					trxn.token[0].s->h = next_elt->coef;
				}
				if (strcmp(next_elt->elt->name, "O") == 0)
				{
					trxn.token[0].s->o = next_elt->coef;
				}
			}
			/*
			 *   Copy reaction to reaction for species
			 */
			trxn_copy(trxn.token[0].s->rxn);
			s_ptr = trxn.token[0].s;
			/*
			 *   Default gamma data
			 */
			s_ptr->dha = 0.0;
			s_ptr->dhb = 0.0;
			if (equal(s_ptr->z, 0.0, TOL) == TRUE)
			{
				s_ptr->gflag = 0;	/* Uncharged */
				s_ptr->dhb = 0.1;
			}
			else
			{
				s_ptr->gflag = 1;	/* Davies */
			}
			/*
			 *   Set type for species
			 */
			if (strcmp(trxn.token[0].s->name, "H+") == 0)
			{
				s_hplus = trxn.token[0].s;
				s_hplus->type = HPLUS;
			}
			else if (strcmp(trxn.token[0].s->name, "H3O+") == 0)
			{
				s_h3oplus = trxn.token[0].s;
				s_h3oplus->type = HPLUS;
			}
			else if (strcmp(trxn.token[0].s->name, "e-") == 0)
			{
				s_eminus = trxn.token[0].s;
				s_eminus->type = EMINUS;
				s_eminus->gflag = 3;	/* Always 1 */
			}
			else if (strcmp(trxn.token[0].s->name, "H2O") == 0)
			{
				s_h2o = trxn.token[0].s;
				s_h2o->type = H2O;
				s_h2o->gflag = 3;	/* Always 1 */
			}
			else if (strstr(trxn.token[0].s->name, "(s)") != NULL)
			{
				trxn.token[0].s->type = SOLID;
			}
			else if (strcmp(trxn.token[0].s->name, "H2") == 0)
			{
				trxn.token[0].s->type = AQ;
				s_h2 = trxn.token[0].s;
			}
			else if (strcmp(trxn.token[0].s->name, "O2") == 0)
			{
				trxn.token[0].s->type = AQ;
				s_o2 = trxn.token[0].s;
			}
			else
			{
				trxn.token[0].s->type = AQ;
			}
			opt_save = OPTION_DEFAULT;
		}
		break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_use(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads solution, mix, irreversible reaction, and pure phases to use
 *   in reaction calculation
 */
	int i, l, n_user, return_value;
	const char* cptr;
	char token[MAX_LENGTH], token1[MAX_LENGTH];;
/*
 *   Read "use"
 */
	cptr = line;
	copy_token(token, &cptr, &l);
/*
 *   Read keyword
 */
	copy_token(token, &cptr, &l);
	check_key(token);
	if (next_keyword != Keywords::KEY_SOLUTION				&&
		next_keyword != Keywords::KEY_MIX					&&
		next_keyword != Keywords::KEY_KINETICS				&&
		next_keyword != Keywords::KEY_REACTION				&&
		next_keyword != Keywords::KEY_REACTION_TEMPERATURE	&&
		next_keyword != Keywords::KEY_REACTION_PRESSURE		&&
		next_keyword != Keywords::KEY_EQUILIBRIUM_PHASES	&&
		next_keyword != Keywords::KEY_EXCHANGE				&&
		next_keyword != Keywords::KEY_SURFACE				&&
		next_keyword != Keywords::KEY_GAS_PHASE				&&
		next_keyword != Keywords::KEY_SOLID_SOLUTIONS)
	{
		input_error++;
		error_msg("Unknown item in USE keyword", CONTINUE);
		error_msg(line_save, CONTINUE);
		check_line("End of use", FALSE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		return (ERROR);
	}
/*
 *   Read number
 */
	strcpy(token1, token);
	for (;;)
	{
		i = copy_token(token, &cptr, &l);
		if (i == DIGIT)
		{
			(void)sscanf(token, "%d", &n_user);
			if (n_user < 0)
			{
				error_msg("Number must be a positive integer.", CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
			}
			if (strstr(token, "-") != NULL)
			{
				error_string = sformatf(
						"USE does not accept a range of numbers, %s.", token);
				warning_msg(error_string);
				error_string = sformatf(
						"Only %s %d will be used in the batch-reaction calculation.",
						token1, n_user);
				warning_msg(error_string);
				error_string = sformatf(
						"NOTE--USE is not needed for ADVECTION and TRANSPORT calculations.");
				warning_msg(error_string);

			}
			break;
		}
		else if (i == EMPTY)
		{
			error_string = sformatf( "No number given, 1 assumed.");
			warning_msg(error_string);
			n_user = 1;
			break;
		}
		else if (token[0] == 'N' || token[0] == 'n')
		{
			n_user = -2;
			break;
		}
	}
	switch (next_keyword)
	{
	case Keywords::KEY_SOLUTION:					/* Solution */
		use.Set_n_solution_user(n_user);
		if (n_user >= 0)
		{
			use.Set_solution_in(true);
		}
		else
		{
			use.Set_solution_in(false);
		}
		break;
	case Keywords::KEY_EQUILIBRIUM_PHASES:					/* Pure phases */
		use.Set_n_pp_assemblage_user(n_user);
		if (n_user >= 0)
		{
			use.Set_pp_assemblage_in(true);
		}
		else
		{
			use.Set_pp_assemblage_in(false);
		}
		break;
	case Keywords::KEY_REACTION:					/* Reaction */
		use.Set_n_reaction_user(n_user);
		if (n_user >= 0)
		{
			use.Set_reaction_in(true);
		}
		else
		{
			use.Set_reaction_in(false);
		}
		break;
	case Keywords::KEY_MIX:					/* Mix */
		use.Set_n_mix_user(n_user);
		if (n_user >= 0)
		{
			use.Set_mix_in(true);
		}
		else
		{
			use.Set_mix_in(false);
		}
		break;
	case Keywords::KEY_EXCHANGE:					/* Ex */
		use.Set_n_exchange_user(n_user);
		if (n_user >= 0)
		{
			use.Set_exchange_in(true);
		}
		else
		{
			use.Set_exchange_in(false);
		}
		break;
	case Keywords::KEY_SURFACE:					/* Surface */
		use.Set_n_surface_user(n_user);
		if (n_user >= 0)
		{
			use.Set_surface_in(true);
		}
		else
		{
			use.Set_surface_in(false);
		}
		break;
	case Keywords::KEY_REACTION_TEMPERATURE:					/* Temperature */
		use.Set_n_temperature_user(n_user);
		if (n_user >= 0)
		{
			use.Set_temperature_in(true);
		}
		else
		{
			use.Set_temperature_in(false);
		}
		break;
	case Keywords::KEY_REACTION_PRESSURE:					/* pressure */
		use.Set_n_pressure_user(n_user);
		if (n_user >= 0)
		{
			use.Set_pressure_in(true);
		}
		else
		{
			use.Set_pressure_in(false);
		}
		break;
	case Keywords::KEY_GAS_PHASE:					/* Gas */
		use.Set_n_gas_phase_user(n_user);
		if (n_user >= 0)
		{
			use.Set_gas_phase_in(true);
		}
		else
		{
			use.Set_gas_phase_in(false);
		}
		break;
	case Keywords::KEY_KINETICS:					/* Kinetics */
		use.Set_n_kinetics_user(n_user);
		if (n_user >= 0)
		{
			use.Set_kinetics_in(true);
		}
		else
		{
			use.Set_kinetics_in(false);
		}
		break;
	case Keywords::KEY_SOLID_SOLUTIONS:					/* solid_solutions */
		use.Set_n_ss_assemblage_user(n_user);
		if (n_user >= 0)
		{
			use.Set_ss_assemblage_in(true);
		}
		else
		{
			use.Set_ss_assemblage_in(false);
		}
		break;
	default:
		input_error++;
		error_msg(line_save, CONTINUE);
		error_msg("Error in switch for USE.", STOP);
		break;
	}
	return_value = check_line("End of use", FALSE, TRUE, TRUE, TRUE);
	/* empty, eof, keyword, print */
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_surface_species(void)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Read data for surface species, parse equations
	 */
	int i, j;
	int association;
	char token[MAX_LENGTH];
	const char* cptr;
	LDBLE offset;

	class species *s_ptr;
	const class elt_list *next_elt;

	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"no_check",				/* 0 */
		"check",				/* 1 */
		"mb",					/* 2 */
		"mass_balance",			/* 3 */
		"log_k",				/* 4 */
		"logk",					/* 5 */
		"delta_h",				/* 6 */
		"deltah",				/* 7 */
		"analytical_expression",	/* 8 */
		"a_e",					/* 9 */
		"ae",					/* 10 */
		"mole_balance",			/* 11 */
		"offset",				/* 12 */
		"add_logk",				/* 13 */
		"add_log_k",			/* 14 */
		"add_constant",			/* 15 */
		"cd_music",				/* 16 */
		"music",				/* 17 */
		"vm"					/* 18 */
	};
	int count_opt_list = 19;
	association = TRUE;
	/*
	 *   Read eqn from file and call parser
	 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	s_ptr = NULL;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SURFACE_SPECIES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* no_check */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->check_equation = FALSE;
			break;
		case 1:				/* check */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			s_ptr->check_equation = TRUE;
			break;
		case 2:				/* mb */
		case 3:				/* mass_balance */
		case 11:				/* mole_balance */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			count_elts = 0;
			paren_count = 0;
			copy_token(token, &next_char, &i);
			s_ptr->mole_balance = string_hsave(token);
			cptr = token;
			s_ptr->next_secondary.clear();
			get_secondary_in_species(&cptr, 1.0);
			s_ptr->next_secondary = elt_list_vsave();
			/* debug
			   for (i = 0; i < count_elts; i++) {
			   output_msg(sformatf("%s\t%f\n", elt_list[i].elt->name,
			   elt_list[i].coef));
			   }
			 */
			opt_save = OPTION_DEFAULT;
			break;
		case 4:				/* log_k */
		case 5:				/* logk */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_log_k_only(next_char, &s_ptr->logk[0]);
			opt_save = OPTION_DEFAULT;
			break;
		case 6:				/* delta_h */
		case 7:				/* deltah */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_delta_h_only(next_char, &s_ptr->logk[1],
							  &s_ptr->original_units);
			opt_save = OPTION_DEFAULT;
			break;
		case 8:				/* analytical_expression */
		case 9:				/* a_e */
		case 10:				/* ae */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_analytical_expression_only(next_char, &(s_ptr->logk[T_A1]));
			opt_save = OPTION_DEFAULT;
			break;
		case 12:				/* offset */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			if (sscanf(next_char, SCANFORMAT, &offset) != 1)
			{
				error_msg("No offset for log K given", STOP);
			}
			s_ptr->logk[0] += offset;
			opt_save = OPTION_DEFAULT;
			break;
		case 13:				/* add_logk */
		case 14:				/* add_log_k */
		{
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = s_ptr->add_logk.size();
			s_ptr->add_logk.resize(count_add_logk + 1);
			/* read name */
			if (copy_token(token, &next_char, &i) == EMPTY)
			{
				input_error++;
				error_string = sformatf(
					"Expected the name of a NAMED_EXPRESSION.");
				error_msg(error_string, CONTINUE);
				break;
			}
			s_ptr->add_logk[count_add_logk].name = string_hsave(token);
			/* read coef */
			i = sscanf(next_char, SCANFORMAT,
				&s_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				s_ptr->add_logk[count_add_logk].coef = 1;
			}
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 15:				/* add_constant */
		{
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = s_ptr->add_logk.size();
			s_ptr->add_logk.resize(count_add_logk + 1);
			i = sscanf(next_char, SCANFORMAT, &s_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				input_error++;
				error_string = sformatf(
					"Expected the constant to add for log_K definition.");
				error_msg(error_string, CONTINUE);
				break;
			}
			/* set name */
			s_ptr->add_logk[count_add_logk].name =
				string_hsave("XconstantX");
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 16:				/* cd_music */
		case 17:				/* music */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			for (j = 0; j < 5; j++)
			{
				if (copy_token(token, &next_char, &i) == EMPTY)
					break;
				if (sscanf(token, SCANFORMAT, &s_ptr->cd_music[j]) != 1)
					break;
			}
			s_ptr->dz[0] = s_ptr->cd_music[0] + s_ptr->cd_music[3] * s_ptr->cd_music[4];
			s_ptr->dz[1] = s_ptr->cd_music[1] + (1 - s_ptr->cd_music[3]) *	s_ptr->cd_music[4];
			s_ptr->dz[2] = s_ptr->cd_music[2];
			for (j = 0; j < 3; j++)
			{
				s_ptr->rxn.dz[j] = s_ptr->dz[j];
			}
			opt_save = OPTION_DEFAULT;
			break;
		case 18:            /* vm, molar volume */
			if (s_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_vm_only(next_char, &s_ptr->logk[vm0],
				&s_ptr->original_deltav_units);
			opt_save = OPTION_DEFAULT;
			break;
		case OPTION_DEFAULT:
		{
			/*
			 *   Get surface species information and parse equation
			 */
			s_ptr = NULL;
			std::vector<class elt_list> new_elt_list;
			if (parse_eq(line, new_elt_list, association) == ERROR)
			{
				parse_error++;
				error_msg("Parsing equation.", CONTINUE);
				error_msg(line_save, CONTINUE);
				break;
			}
			/*
			 *   Get pointer to each species in the reaction, store new species if necessary
			 */
			trxn.token[0].s = s_store(trxn.token[0].name, trxn.token[0].z, TRUE);
			for (i = 1; i < count_trxn; i++)
			{
				trxn.token[i].s =
					s_store(trxn.token[i].name, trxn.token[i].z, FALSE);
			}
			/*
			 *   Save element list and carbon, hydrogen, and oxygen in species
			 */
			trxn.token[0].s->next_elt = new_elt_list;
			next_elt = &trxn.token[0].s->next_elt[0];
			for (; next_elt->elt != NULL; next_elt++)
			{
				if (strcmp(next_elt->elt->name, "C") == 0)
				{
					trxn.token[0].s->carbon = next_elt->coef;
				}
				if (strcmp(next_elt->elt->name, "H") == 0)
				{
					trxn.token[0].s->h = next_elt->coef;
				}
				if (strcmp(next_elt->elt->name, "O") == 0)
				{
					trxn.token[0].s->o = next_elt->coef;
				}
			}
			/*
			 *   Copy reaction to reaction for species
			 */
			trxn_copy(trxn.token[0].s->rxn);
			/*
			 *   Set type for species
			 */
			trxn.token[0].s->type = SURF;
			s_ptr = trxn.token[0].s;
			/*
			 *   Read gamma data
			 */
			s_ptr->gflag = 6;
			s_ptr->dha = 0.0;
			s_ptr->dhb = 0.0;
			opt_save = OPTION_DEFAULT;
		}
		break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_surface(void)
/* ---------------------------------------------------------------------- */
{
	/*
	 *      Reads surface data
	 *
	 *      Arguments:
	 *   none
	 *
	 *      Returns:
	 *   KEYWORD if keyword encountered, input_error may be incremented if
	 *	  a keyword is encountered in an unexpected position
	 *   EOF     if eof encountered while reading mass balance concentrations
	 *   ERROR   if error occurred reading data
	 *
	 */
	int n_user;
	LDBLE conc;
	const char* cptr, *cptr1;
	std::string token, token1, name;
	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"equilibrate",          /* 0 */
		"equil",                /* 1 */
		"diff",                 /* 2 */
		"diffuse_layer",        /* 3 */
		"no_edl",               /* 4 */
		"no_electrostatic",     /* 5 */
		"only_counter_ions",    /* 6 */
		"donnan",               /* 7 */
		"cd_music",             /* 8 */
		"capacitances",         /* 9 */
		"sites",                /* 10 */
		"sites_units",			/* 11 */
		"constant_capacitance", /* 12 */
		"ccm",                  /* 13 */
        "equilibrium",          /* 14 */
		"site_units",           /* 15 */
		"ddl"                   /* 16 */
	};
	int count_opt_list = 17;
	/*
	 * kin_surf is for Surfaces, related to kinetically reacting minerals
	 *    they are defined if "sites" is followed by mineral name:
	 *    Surf_wOH  Manganite  [equilibrium_phases or kinetics]      0.25    4000
	 *    ^Name     mineral    ^switch		 ^prop.factor ^m2/mol
	 */
	cxxSurface temp_surface;
	cptr = line;
	temp_surface.read_number_description(cptr);
	n_user = temp_surface.Get_n_user();
	cxxSurfaceComp *comp_ptr = NULL;
	cxxSurfaceCharge *charge_ptr = NULL;
	temp_surface.Set_new_def(true);

	if (use.Get_surface_in() == FALSE)
	{
		use.Set_surface_in(true);
		use.Set_n_surface_user(n_user);
	}
	/*
	 *   Read surface data
	 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SURFACE keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* equilibrate */
		case 1:             /* equil */
		case 14:			/* equilibrium */
			for (;;)
			{
				int i = copy_token(token, &next_char);
				if (i == DIGIT)
				{
					int j;
					(void)sscanf(token.c_str(), "%d", &j);
					temp_surface.Set_solution_equilibria(true);
					temp_surface.Set_n_solution(j);
					break;
				}
				if (i == EMPTY)
				{
					error_msg
						("Expected a solution number with which to equilibrate surface.",
						 CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
					break;
				}
			}
			break;
		case 2:				/* diffuse_layer */
		case 3:
			{
				temp_surface.Set_thickness(1e-8);
				temp_surface.Set_dl_type(cxxSurface::BORKOVEK_DL);
				int j = sscanf(next_char, SCANFORMAT, &dummy);
				if (j == 1)
				{
					temp_surface.Set_thickness(dummy);
				}
			}
			 break;
		case 4:				/* no electrostatic */
		case 5:
			temp_surface.Set_type(cxxSurface::NO_EDL);
			break;
		case 6:
			temp_surface.Set_only_counter_ions(get_true_false(next_char, TRUE) == TRUE);
			break;
		case 7:				/* donnan for DL conc's */
			{
				temp_surface.Set_dl_type(cxxSurface::DONNAN_DL);
				LDBLE thickness = 0.0;
				for (;;)
				{
					int i = copy_token(token, &next_char);
					if (i == DIGIT)
					{
						(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
						temp_surface.Set_thickness(dummy);
						thickness = 1;
						continue;
					}
					else if (i != EMPTY)
					{
						if (token[0] == 'D' || token[0] == 'd')
						{
							if (thickness != 0)
							{
								error_msg
									("You must enter EITHER thickness OR Debye lengths (1/k),\n	   and relative DDL viscosity, DDL limit.\nCorrect is (for example): -donnan 1e-8 viscosity 0.5\n or (default values):     -donnan debye_lengths 1 viscosity 1 limit 0.8",
									CONTINUE);
								error_msg(line_save, CONTINUE);
								input_error++;
								break;
							}
							int j = copy_token(token1, &next_char);
							if (j == DIGIT)
							{
								(void)sscanf(token1.c_str(), SCANFORMAT, &dummy);
								temp_surface.Set_debye_lengths(dummy);
								continue;
							}
							else if (j != EMPTY)
							{
								error_msg
									("Expected number of Debye lengths (1/k).",
									CONTINUE);
								error_msg(line_save, CONTINUE);
								input_error++;
								break;
							}
						}
						else if (token[0] == 'V' || token[0] == 'v')
						{
							int j = copy_token(token1, &next_char);
							if (j == DIGIT)
							{
								(void)sscanf(token1.c_str(), SCANFORMAT, &dummy);
								temp_surface.Set_DDL_viscosity(dummy);
								continue;
							}
							else if (j != EMPTY)
							{
								error_msg
									("Expected number for relative DDL viscosity.",
									CONTINUE);
								error_msg(line_save, CONTINUE);
								input_error++;
								break;
							}
						}
						else if (token[0] == 'L' || token[0] == 'l')
						{
							int j = copy_token(token1, &next_char);
							if (j == DIGIT)
							{
								(void)sscanf(token1.c_str(), SCANFORMAT, &dummy);
								temp_surface.Set_DDL_limit(dummy);
								continue;
							}
							else if (j != EMPTY)
							{
								error_msg
									("Expected number for maximum of DDL water as fraction of bulk water.",
									CONTINUE);
								error_msg(line_save, CONTINUE);
								input_error++;
								break;
							}
						}
						else
						{
							error_msg
								("Expected diffuse layer thickness (m) or Debye lengths (1/k) for \n\tcalculating the thickness, and relative DDL viscosity and DDL limit.\nCorrect is (for example): -donnan 1e-8 visc 0.5\n or (default values):     -donnan debye_lengths 1 visc 1 lim 0.8",
								CONTINUE);
							error_msg(line_save, CONTINUE);
							input_error++;
							break;
						}
					}
					else
						break;
				}
			}
			break;
		case 8:				/* cd_music */
			temp_surface.Set_type(cxxSurface::CD_MUSIC);
			break;
		case 9:				/* capacitances */
			/*
			 *   Read capacitance for CD_MUSIC
			 */
			if (charge_ptr == NULL)
			{
				error_msg
					("Surface component has not been defined for capacitances.",
					 CONTINUE);
				error_msg(line_save, CONTINUE);
				input_error++;
				break;
			}
			copy_token(token1, &next_char);
			if (sscanf(token1.c_str(), SCANFORMAT, &dummy) == 1)
			{
				charge_ptr->Set_capacitance0(dummy);
			}
			copy_token(token1, &next_char);
			if (sscanf(token1.c_str(), SCANFORMAT, &dummy) == 1)
			{
				charge_ptr->Set_capacitance1(dummy);
			}
			break;
		case 10:				/* sites */
		case 11:				/* sites_units */
		case 15:				/* site_units */
			{
				int j = copy_token(token1, &next_char);
				if (j != EMPTY && (token1[0] == 'A' || token1[0] == 'a'))
				{
					temp_surface.Set_sites_units(cxxSurface::SITES_ABSOLUTE);
				}
				else if (j != EMPTY && (token1[0] == 'D' || token1[0] == 'd'))
				{
					temp_surface.Set_sites_units(cxxSurface::SITES_DENSITY);
				}
				else
				{
					error_msg
						("Character string expected to be 'Absolute' or 'Density' to define the units of the first item in the definition of a surface component.\n",
						CONTINUE);
					input_error++;
					break;
				}
			}
			break;
			// CCM not implemented yet
		case 12:			    /* constant_capacitance */
		case 13:			    /* ccm */
			temp_surface.Set_type(cxxSurface::CCM);
			copy_token(token1, &next_char);
			if (charge_ptr == NULL)
			{
				error_msg("A surface must be defined before the capacitance is defined.\n",
					 CONTINUE);
				input_error++;
				break;
			}
			if (sscanf(token1.c_str(), SCANFORMAT, &dummy) != 1)
			{
				error_msg("Expected capacitance for constant_capacitance model.\n",
					 CONTINUE);
				input_error++;
				break;
			}
			else
			{
				charge_ptr->Set_capacitance0(dummy); 
			}

			/* constant capacitance model not implemented yet */
			//error_msg("Constant capacitance model not implemented.", CONTINUE);
			//input_error++;
			break;
		case 16:				/* ddl */
			temp_surface.Set_type(cxxSurface::DDL);
			break;
		case OPTION_DEFAULT:
			/*
			 *   Read surface component
			 */
			{
				cptr = line;
				int i = copy_token(token, &cptr);
				if (i != UPPER && token[0] != '[')
				{
					error_msg
						("Expected surface name to begin with a capital letter.",
						CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
					break;
				}

				cxxSurfaceComp temp_comp(this->phrq_io);

				temp_surface.Get_surface_comps().push_back(temp_comp);
				comp_ptr = &(temp_surface.Get_surface_comps().back());
				comp_ptr->Set_formula(token.c_str());

				i = copy_token(token1, &cptr);
				if (i == DIGIT)
				{
					/*
					*   Read surface concentration
					*/
					/* surface concentration is read directly */
					if (sscanf(token1.c_str(), SCANFORMAT, &conc) < 1)
					{
						error_msg("Expected number of surface sites in moles.",
							CONTINUE);
						error_msg(line_save, CONTINUE);
						input_error++;
						break;
					}
					comp_ptr->Set_moles(conc);
					/*
					*   Read equilibrium phase name or kinetics rate name
					*/
				}
				else if (i != EMPTY)
				{

					/* surface conc. is related to mineral or kinetics */
					comp_ptr->Set_phase_name(token1.c_str());
					int j = copy_token(token1, &cptr);

					/* read optional 'equilibrium_phases' or 'kinetics' */
					if (j != DIGIT)
					{
						if (token1[0] == 'K' || token1[0] == 'k')
						{
							comp_ptr->Set_rate_name(comp_ptr->Get_phase_name().c_str());
							comp_ptr->Set_phase_name(NULL);
						}
						else if (token1[0] == 'E' || token1[0] == 'e')
						{
							comp_ptr->Set_rate_name(NULL);
						}
						else
						{
							error_msg
								("Character string expected to be 'equilibrium_phase' or 'kinetics' to relate surface to mineral or kinetic reaction.\n",
								CONTINUE);
							input_error++;
							break;
						}
						j = copy_token(token1, &cptr);
					}

					/* read proportion */
					if (j != DIGIT)
					{
						error_msg
							("Expected a coefficient to relate surface to mineral or kinetic reaction.\n",
							CONTINUE);
						input_error++;
						break;
					}
					(void)sscanf(token1.c_str(), SCANFORMAT, &dummy);
					comp_ptr->Set_phase_proportion(dummy);
					/* real conc must be defined in tidy_model */
					conc = 1.0;
				}
				else
				{
					error_msg
						("Expected concentration of surface, mineral name, or kinetic reaction name.",
						CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
					break;
				}
				/*
				*   Accumulate elements in elt_list
				*/
				count_elts = 0;
				paren_count = 0;
				std::string formula = token.c_str();
				cptr1 = formula.c_str();
				get_elts_in_species(&cptr1, conc);
				/*
				*   save formula for adjusting number of exchange sites
				*/
				cptr1 = formula.c_str();
				int l;
				std::string name;
				get_token(&cptr1, name, &dummy, &l);
				comp_ptr->Set_formula_z(dummy);
				cxxNameDouble nd = elt_list_NameDouble();
				comp_ptr->Set_totals(nd);
				/*
				*   Search for charge structure
				*/
				cptr1 = formula.c_str();
				get_elt(&cptr1, name, &l);
				{
					std::string::size_type pos = name.find('_');
					if (pos != std::string::npos)
					{
						name = name.substr(0, pos);
					}
				}
				charge_ptr = temp_surface.Find_charge(name);
				if (charge_ptr == NULL)
				{
					cxxSurfaceCharge temp_charge(this->phrq_io);
					temp_charge.Set_name(name.c_str());
					if (comp_ptr->Get_phase_name().size() == 0
						&& comp_ptr->Get_rate_name().size() == 0)
					{
						temp_charge.Set_specific_area(600.0);
						temp_charge.Set_grams(0.0);
					}
					else
					{
						temp_charge.Set_specific_area(0.0);
						temp_charge.Set_grams(1.0);
					}
					temp_surface.Get_surface_charges().push_back(temp_charge);
					charge_ptr = temp_surface.Find_charge(name);
				}
				comp_ptr->Set_charge_name(name.c_str());
				/*
				*   Read surface area (m2/g)
				*/
				copy_token(token1, &cptr);
				if (sscanf(token1.c_str(), SCANFORMAT, &dummy) == 1)
				{
					charge_ptr->Set_specific_area(dummy);
				}
				else
				{
					break;
				}
				/*
				*   Read grams of solid (g)
				*/
				copy_token(token1, &cptr);
				if (sscanf(token1.c_str(), SCANFORMAT, &dummy) == 1)
				{
					charge_ptr->Set_grams(dummy);
				}
				/* read Dw */
				copy_token(token1, &cptr);
				Utilities::str_tolower(token1);
				if (strcmp(token1.c_str(), "dw") == 0)
				{
					int j = copy_token(token1, &cptr);
					if (j != DIGIT)
					{
						error_msg
							("Expected surface diffusion coefficient (m2/s).\n",
							CONTINUE);
						input_error++;
						break;
					}
					else
					{
						(void)sscanf(token1.c_str(), SCANFORMAT, &dummy);
						comp_ptr->Set_Dw(dummy);
						if (dummy > 0)
						{
							temp_surface.Set_transport(true);
							if (temp_surface.Get_related_rate()
								|| temp_surface.Get_related_phases())
							{
								error_msg
									("Can`t transport surfaces related to phases or rates (yet).",
									CONTINUE);
								input_error++;
							}
						}
						break;
					}
				}
			}
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}

	// Sort comps and charge
	temp_surface.Sort_comps();

	/*
	 * copy Dw > 0 to all comps with the same charge structure, check edl & dif for surface transport
	 */
	if (temp_surface.Get_transport())
	{
		if (temp_surface.Get_type() <= cxxSurface::NO_EDL)
		{
			input_error++;
			error_msg
				("Must use default Dzombak and Morel or -cd_music for surface transport.",
				 CONTINUE);
		}
		if (temp_surface.Get_dl_type() <= cxxSurface::NO_DL)
		{
			input_error++;
			error_msg
				("Must use -donnan or -diffuse_layer for surface transport.",
				 CONTINUE);
		}
		for (size_t i = 0; i < temp_surface.Get_surface_comps().size(); i++)
		{
			comp_ptr = &(temp_surface.Get_surface_comps()[i]);
			if (comp_ptr->Get_Dw() > 0)
			{
				std::string name = comp_ptr->Get_charge_name();
				for (size_t j = 0; j < temp_surface.Get_surface_comps().size(); j++)
				{
					cxxSurfaceComp *comp_ptr1 = &(temp_surface.Get_surface_comps()[j]);
					std::string name1 = comp_ptr1->Get_charge_name();
					if (name == name1)
					{
						comp_ptr1->Set_Dw(comp_ptr->Get_Dw());
					}
				}
			}
		}
	}
	/*
	 *   Make sure surface area is defined
	 */
	if (temp_surface.Get_type() == cxxSurface::DDL || temp_surface.Get_type() == cxxSurface::CCM || temp_surface.Get_type() == cxxSurface::CD_MUSIC)
	{
		for (size_t i = 0; i < temp_surface.Get_surface_charges().size(); i++)
		{
			charge_ptr = &(temp_surface.Get_surface_charges()[i]);
			if (charge_ptr->Get_grams() *
				charge_ptr->Get_specific_area() <= 0)
			{
				error_string = sformatf( "Surface area not defined for %s.\n",
						charge_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
		}
	}
	/*
	 *  Logical checks
	 */
	if (temp_surface.Get_type() == cxxSurface::UNKNOWN_DL)
	{
		error_string = sformatf( "Unknown surface type.\n");
		error_msg(error_string, CONTINUE);
		input_error++;
	}
	else if (temp_surface.Get_type() == cxxSurface::NO_EDL)
	{
		if (temp_surface.Get_dl_type() != cxxSurface::NO_DL)
		{
			error_string = sformatf(
					"No electrostatic term calculations do not allow calculation of the diffuse layer composition.\n");
			warning_msg(error_string);
			temp_surface.Set_dl_type(cxxSurface::NO_DL);
		}
	}
	else if (temp_surface.Get_type() == cxxSurface::DDL || temp_surface.Get_type() == cxxSurface::CCM)
	{
		/* all values of dl_type are valid */
	}
	else if (temp_surface.Get_type() == cxxSurface::CD_MUSIC)
	{
		if (temp_surface.Get_dl_type() == cxxSurface::BORKOVEK_DL)
		{
			error_string = sformatf(
					"Borkovec and Westall diffuse layer calculation is not allowed with a CD_MUSIC surface.\n\tUsing Donnan diffuse layer calculation.");
			warning_msg(error_string);
			temp_surface.Set_dl_type(cxxSurface::DONNAN_DL);
		}
		if (temp_surface.Get_debye_lengths() > 0)
		{
			error_msg
				("Variable DDL thickness is not permitted in CD_MUSIC. Fix DDL thickness\n\tfor example (default value): -donnan 1e-8",
				 CONTINUE);
			input_error++;
		}
	}
	temp_surface.Sort_comps();
	Rxn_surface_map[n_user] = temp_surface;
	Rxn_new_surface.insert(n_user);
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_surface_master_species(void)
/* ---------------------------------------------------------------------- */
{
	/*
	 *   Reads master species data from data file or input file
	 */
	int l, return_value;
	const char* cptr, *cptr1;
	LDBLE l_z;
	class species *s_ptr;
	char token[MAX_LENGTH], token1[MAX_LENGTH];
	int opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"capacitance",			/* 0 */
		"cd_music_capacitance"	/* 1 */
	};
	int count_opt_list = 0;
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SURFACE_MASTER_SPECIES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case OPTION_DEFAULT:
			/*
			 *   Get "element" name with valence, allocate space, store
			 */
			cptr = line;
			/*
			 *   Get element name and save pointer to character string
			 */
			if (copy_token(token, &cptr, &l) != UPPER && token[0] != '[')
			{
				parse_error++;
				error_msg("Reading element for master species.", CONTINUE);
				error_msg(line_save, CONTINUE);
				continue;
			}
			replace("(+", "(", token);
			/*
			 *   Delete master if it exists
			 */
			master_delete(token);
			/*
			 *   Save values in master and species structure for surface sites
			 */
			size_t count_master = master.size();
			master.resize(count_master + 1);
			master[count_master] = master_alloc();
			master[count_master]->type = SURF;
			master[count_master]->elt = element_store(token);
			if (copy_token(token, &cptr, &l) != UPPER && token[0] != '[')
			{
				parse_error++;
				error_msg("Reading surface master species name.", CONTINUE);
				error_msg(line_save, CONTINUE);
				continue;
			}
			s_ptr = s_search(token);
			if (s_ptr != NULL)
			{
				master[count_master]->s = s_ptr;
			}
			else
			{
				cptr1 = token;
				std::string token1;
				get_token(&cptr1, token1, &l_z, &l);
				master[count_master]->s = s_store(token1.c_str(), l_z, FALSE);
			}
			master[count_master]->primary = TRUE;
			strcpy(token, master[count_master]->elt->name);
			count_master++;
			/*
			 *   Save values in master and species structure for surface psi
			 */
			strcpy(token1, token);
			replace("_", " ", token1);
			cptr1 = token1;
			copy_token(token, &cptr1, &l);
			strcat(token, "_psi");
			add_psi_master_species(token);
			opt_save = OPTION_DEFAULT;
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
add_psi_master_species(char *token)
/* ---------------------------------------------------------------------- */
{
	class species *s_ptr;
	class master *master_ptr;
	const char* cptr;
	char token1[MAX_LENGTH];
	int i, n, plane;

	strcpy(token1, token);
	for (plane = SURF_PSI; plane <= SURF_PSI2; plane++)
	{
		strcpy(token, token1);
		switch (plane)
		{
		case SURF_PSI:
			break;
		case SURF_PSI1:
			strcat(token, "b");
			break;
		case SURF_PSI2:
			strcat(token, "d");
			break;
		}
		master_ptr = master_search(token, &n);
		if (master_ptr == NULL)
		{
			size_t count_master = master.size();
			master.resize(count_master + 1);
			master[count_master] = master_alloc();
			master[count_master]->type = plane;
			master[count_master]->elt = element_store(token);
			s_ptr = s_search(token);
			if (s_ptr != NULL)
			{
				master[count_master]->s = s_ptr;
			}
			else
			{
				master[count_master]->s = s_store(token, 0.0, FALSE);
			}
			count_elts = 0;
			paren_count = 0;
			cptr = token;
			get_elts_in_species(&cptr, 1.0);
			master[count_master]->s->next_elt = elt_list_vsave();
			master[count_master]->s->type = plane;
			master[count_master]->primary = TRUE;

			master[count_master]->s->rxn.token.resize(3);
			/*
			 *   Define reaction for psi
			 */
			for (i = 0; i < MAX_LOG_K_INDICES; i++)
			{
				master[count_master]->s->rxn.logk[i] = 0.0;
			}
			master[count_master]->s->rxn.token[0].s =
				master[count_master]->s;
			master[count_master]->s->rxn.token[0].coef = -1.0;
			master[count_master]->s->rxn.token[1].s =
				master[count_master]->s;
			master[count_master]->s->rxn.token[1].coef = 1.0;
			master[count_master]->s->rxn.token[2].s = NULL;
			count_master++;
		}
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_title(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads title for simulation
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	const char* cptr, *cptr1;
	int l;
	int return_value;
	char token[MAX_LENGTH];
/*
 *   Read anything after keyword
 */
	cptr = line;
	copy_token(token, &cptr, &l);
	cptr1 = cptr;
	title_x.clear();
	if (copy_token(token, &cptr, &l) != EMPTY)
	{
		title_x = cptr1;
	}

/*
 *   Read additonal lines
 */
	for (;;)
	{
		return_value = check_line("title", TRUE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		if (return_value == EOF || return_value == KEYWORD)
			break;
/*
 *   append line to title_x
 */
		if (title_x.size() > 0)
		{
			title_x.append("\n");
		}
		title_x.append(line);
	}
	last_title_x = title_x;
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_advection(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads advection information
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
/*
 *   Read advection parameters:
 *	number of cells;
 *	number of shifts;
 */
	std::vector<int> punch_temp, print_temp;
	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"cells",				/* 0 */
		"shifts",				/* 1 */
		"print",				/* 2 */
		"selected_output",		/* 3 */
		"punch",				/* 4 */
		"print_cells",			/* 5 */
		"selected_cells",		/* 6 */
		"time_step",			/* 7 */
		"timest",				/* 8 */
		"output",				/* 9 */
		"output_frequency",		/* 10 */
		"selected_output_frequency",	/* 11 */
		"punch_frequency",		/* 12 */
		"print_frequency",		/* 13 */
		"punch_cells",			/* 14 */
		"initial_time",			/* 15 */
		"warning",				/* 16 */
		"warnings"				/* 17 */
	};
	int count_opt_list = 18;
/*
 *   Set use data
 */
	use.Set_advect_in(true);
	count_ad_cells = 0;
	count_ad_shifts = 0;
	print_ad_modulus = 1;
	punch_ad_modulus = 1;
/*
 *   Read lines
 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in ADVECTION keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* cells */
			(void)sscanf(next_char, "%d", &count_ad_cells);
			opt_save = OPTION_DEFAULT;
			break;
		case 1:				/* shifts */
			(void)sscanf(next_char, "%d", &count_ad_shifts);
			opt_save = OPTION_DEFAULT;
			break;
		case 2:				/* print */
		case 5:				/* print_cells */
		{
			(void)read_list_ints_range(&next_char, false, print_temp);
			opt_save = 2;
		}
		break;
		case 3:				/* selected_output */
		case 11:			/* selected_output_frequency */
		case 12:			/* punch_frequency */
			(void)sscanf(next_char, "%d", &punch_ad_modulus);
			opt_save = OPTION_DEFAULT;
			if (punch_ad_modulus <= 0)
			{
				error_string = sformatf(
						"Punch frequency must be greater than 0. Frequency set to 1000.");
				warning_msg(error_string);
				punch_ad_modulus = 1000;
			}
			break;
		case 4:				/* punch */
		case 14:			/* punch_cells */
		case 6:				/* selected_cells */
		{
			(void) read_list_ints_range(&next_char, false, punch_temp);
			opt_save = 4;
			break;
		}
		case 7:				/* time_step */
		case 8:				/* timest */
			(void)sscanf(next_char, SCANFORMAT, &advection_kin_time);
			{
				std::string token;
				int j = copy_token(token, &next_char);
				j = copy_token(token, &next_char);
				if (j == UPPER || j == LOWER)
				{
					advection_kin_time = Utilities::convert_time(advection_kin_time, token, "s");
				}
			}
			advection_kin_time_defined = TRUE;
			opt_save = OPTION_DEFAULT;
			break;
		case 9:				/* output */
		case 10:				/* output_frequency */
		case 13:				/* print_frequency */
			(void)sscanf(next_char, "%d", &print_ad_modulus);
			opt_save = OPTION_DEFAULT;
			if (print_ad_modulus <= 0)
			{
				error_string = sformatf(
						"Print frequency must be greater than 0. Frequency set to 1000.");
				warning_msg(error_string);
				print_ad_modulus = 1000;
			}
			break;
		case 15:				/* initial_time */
			char token[MAX_LENGTH];
			int j;
			if (copy_token(token, &next_char, &j) == DIGIT)
				(void)sscanf(token, SCANFORMAT, &initial_total_time);
			{
				std::string stdtoken;
				j = copy_token(stdtoken, &next_char);
				if (j == UPPER || j == LOWER)
				{
					initial_total_time = Utilities::convert_time(initial_total_time, stdtoken, "s");
				}
			}
			opt_save = OPTION_DEFAULT;
			break;
		case 16:				/* warning */
		case 17:				/* warnings */
			advection_warnings = get_true_false(next_char, TRUE);
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
/*
 *   Fill in data for punch
 */
	advection_punch.resize(count_ad_cells + 1);
	if (punch_temp.size() != 0)
	{
		for (size_t i = 0; i < count_ad_cells; i++)
			advection_punch[i] = FALSE;
		for (size_t i = 0; i < punch_temp.size(); i++)
		{
			if (punch_temp[i] > count_ad_cells || punch_temp[i] < 1)
			{
				error_string = sformatf(
						"Cell number for punch is out of range, %d. Request ignored.",
						punch_temp[i]);
				warning_msg(error_string);
			}
			else
			{
				advection_punch[punch_temp[i] - 1] = TRUE;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < count_ad_cells; i++)
			advection_punch[i] = TRUE;
	}
	punch_temp.clear();
/*
 *   Fill in data for print
 */
	advection_print.resize(count_ad_cells + 1);
	if (print_temp.size() != 0)
	{
		for (size_t i = 0; i < count_ad_cells; i++)
			advection_print[i] = FALSE;
		for (size_t i = 0; i < print_temp.size(); i++)
		{
			if (print_temp[i] > count_ad_cells || print_temp[i] < 1)
			{
				error_string = sformatf(
						"Cell number for print is out of range, %d. Request ignored.",
						print_temp[i]);
				warning_msg(error_string);
			}
			else
			{
				advection_print[print_temp[i] - 1] = TRUE;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < count_ad_cells; i++)
			advection_print[i] = TRUE;
	}
	print_temp.clear();
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_debug(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads knobs and debugging info
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"iterations",			/* 0 */
		"tolerance",			/* 1 */
		"step_size",			/* 2 */
		"pe_step_size",			/* 3 */
		"scale_pure_phases",	/* 4 */
		"diagonal_scale",		/* 5 */
		"debug_model",			/* 6 */
		"debug_prep",			/* 7 */
		"debug_set",			/* 8 */
		"debug_inverse",		/* 9 */
		"logfile",				/* 10 */
		"log_file",				/* 11 */
		"debug_diffuse_layer",	/* 12 */
		"delay_mass_water",		/* 13 */
		"convergence_tolerance",	/* 14 */
		"numerical_derivatives",	/* 15 */
		"tries",					/* 16 */
		"try",						       /* 17 */
		"numerical_fixed_volume",          /* 18 */
		"force_numerical_fixed_volume",    /* 19 */
		"equi_delay",                      /* 20 */
		"minimum_total",                   /* 21 */  
		"min_total",                       /* 22 */   
		"debug_mass_action",               /* 23 */
		"debug_mass_balance"               /* 24 */
	};
	int count_opt_list = 25;
/*
 *   Read parameters:
 *	ineq_tol;
 *	step_size;
 *	pe_step_size;
 *	pp_scale;
 *	diagonal_scale;
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in KNOBS keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* iterations */
			(void)sscanf(next_char, "%d", &itmax);
			break;
		case 1:				/* tolerance */
			(void)sscanf(next_char, SCANFORMAT, &ineq_tol);
			break;
		case 2:				/* step_size */
			(void)sscanf(next_char, SCANFORMAT, &step_size);
			break;
		case 3:				/* pe_step_size */
			(void)sscanf(next_char, SCANFORMAT, &pe_step_size);
			break;
		case 4:				/* pp_scale */
			(void)sscanf(next_char, SCANFORMAT, &pp_scale);
			break;
		case 5:				/* diagonal_scale */
			diagonal_scale = get_true_false(next_char, TRUE);
			break;
		case 6:				/* debug_model */
			debug_model = get_true_false(next_char, TRUE);
			break;
		case 7:				/* debug_prep */
			debug_prep = get_true_false(next_char, TRUE);
			break;
		case 8:				/* debug_set */
			debug_set = get_true_false(next_char, TRUE);
			break;
		case 9:				/* debug_inverse */
			debug_inverse = get_true_false(next_char, TRUE);
			break;
		case 10:				/* logfile */
		case 11:				/* log_file */
			pr.logfile = get_true_false(next_char, TRUE);
			if (phast == TRUE)
			{
				pr.logfile = FALSE;
				warning_msg("PHREEQC log file is disabled in PHAST");
			}
			phrq_io->Set_log_on(pr.logfile == TRUE);
			break;
		case 12:				/* debug_diffuse_layer */
			debug_diffuse_layer = get_true_false(next_char, TRUE);
			break;
		case 13:				/* delay_mass_water */
			delay_mass_water = get_true_false(next_char, TRUE);
			break;
		case 14:				/* convergence_tolerance */
			{
				LDBLE ct;
				(void)sscanf(next_char, SCANFORMAT, &ct);
				convergence_tolerance = ct;
			}
			break;
		case 15:				/* numerical_derivatives */
			numerical_deriv = get_true_false(next_char, TRUE);
			break;
		case 16:				/* tries */
		case 17:				/* try */
			(void)sscanf(next_char, "%d", &max_tries);
			break;
		case 18:				/* debug_inverse */
			numerical_fixed_volume = (get_true_false(next_char, TRUE) == TRUE);
			break;
		case 19:				/* debug_inverse */
			force_numerical_fixed_volume = (get_true_false(next_char, TRUE) == TRUE);
			break;
		case 20:				/* equi_delay */
			(void)sscanf(next_char, "%d", &equi_delay);
			break;
		case 21:				/* minimum_total */
		case 22:				/* min_total */
			(void)sscanf(next_char, SCANFORMAT, &MIN_TOTAL);
			MIN_TOTAL_SS = MIN_TOTAL/100;
			MIN_RELATED_SURFACE = MIN_TOTAL*100;
			break;
		case 23:				/* debug_mass_action */
			debug_mass_action = get_true_false(next_char, TRUE);
			break;
		case 24:				/* debug_mass_balance */
			debug_mass_balance = get_true_false(next_char, TRUE);
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_print(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads printing info
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int return_value, opt, l;
	const char* next_char;
	char token[MAX_LENGTH];
	LDBLE num;
	const char *opt_list[] = {
		"reset",				/* 0 */
		"gas_phase",			/* 1 */
		"pure_phase",			/* 2 */
		"surface",				/* 3 */
		"exchange",				/* 4 */
		"totals",				/* 5 */
		"eh",					/* 6 */
		"species",				/* 7 */
		"saturation_indices",	/* 8 */
		"si",					/* 9 same a 8 */
		"reaction",				/* 10 irrev, not used, pr.use */
		"mix",					/* 11 */
		"use",					/* 12 */
		"selected_output",		/* 13 */
		"equilibrium_phases",	/* 14 same as 2 */
		"equilibria",	        /* 15 same as 2 */
		"equilibrium",			/* 16 same as 2 */
		"pure",					/* 17 same as 2 */
		"other",				/* 18 same as 12 */
		"status",				/* 19 */
		"inverse",				/* 20 */
		"kinetics",				/* 21 */
		"dump",					/* 22 */
		"user_print",			/* 23 */
		"user_pr",				/* 24 */
		"solid_solution",		/* 25 */
		"solid_solutions",		/* 26 */
		"inverse_modeling",		/* 27 */
		"headings",				/* 28 */
		"heading",				/* 29 */
		"user_graph",			/* 30 */
		"echo_input",			/* 31 */
		"warning",				/* 32 */
		"warnings",				/* 33 */
		"initial_isotopes",		/* 34 */
		"isotope_ratios",		/* 35 */
		"isotope_alphas",		/* 36 */
		"censor_species",		/* 37 */
		"alkalinity",			/* 38 */
		"equilibrium_phase",    /* 39 */
		"high_precision"        /* 40 */
	};

	int count_opt_list = 41;
	int value;
/*
 *   Read flags:
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in PRINT keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* reset */
			value = get_true_false(next_char, TRUE);
			pr.kinetics = value;
			pr.gas_phase = value;
			pr.pp_assemblage = value;
			pr.surface = value;
			pr.exchange = value;
			pr.totals = value;
			pr.eh = value;
			pr.species = value;
			pr.saturation_indices = value;
			pr.irrev = value;
			pr.mix = value;
			pr.reaction = value;
			pr.use = value;
			pr.inverse = value;
			pr.user_print = value;
			pr.ss_assemblage = value;
			pr.headings = value;
			pr.initial_isotopes = value;
			pr.isotope_ratios = value;
			pr.isotope_alphas = value;
			pr.echo_input = value;
			break;
		case 1:				/* gas_phase */
			pr.gas_phase = get_true_false(next_char, TRUE);
			break;
		case 2:				/* pure_phase */
		case 14:				/* equilibrium_phases */
		case 15:				/* equilibria */
		case 16:				/* equilibrium */
		case 17:				/* pure */
		case 39:                /* equilibrium_phase */
			pr.pp_assemblage = get_true_false(next_char, TRUE);
			break;
		case 3:				/* surface */
			pr.surface = get_true_false(next_char, TRUE);
			break;
		case 4:				/* exchange */
			pr.exchange = get_true_false(next_char, TRUE);
			break;
		case 5:				/* totals */
			pr.totals = get_true_false(next_char, TRUE);
			break;
		case 6:				/* eh */
			pr.eh = get_true_false(next_char, TRUE);
			break;
		case 7:				/* species */
			pr.species = get_true_false(next_char, TRUE);
			break;
		case 8:
		case 9:				/* saturation_indices */
			pr.saturation_indices = get_true_false(next_char, TRUE);
			break;
		case 10:				/* reaction, not used, pr.use controls */
			pr.irrev = get_true_false(next_char, TRUE);
			break;
		case 11:				/* mix, not used, pr.use controls */
			pr.mix = get_true_false(next_char, TRUE);
			break;
		case 12:				/* use */
		case 18:				/* other */
			pr.use = get_true_false(next_char, TRUE);
			break;
		case 13:				/* selected_output */
			pr.punch = get_true_false(next_char, TRUE);
			phrq_io->Set_punch_on(pr.punch == TRUE);
			break;
		case 19:				/* status */
			{
				int j;
				pr.status = get_true_false(next_char, TRUE);
				j = copy_token(token, &next_char, &l);
				if  (j == UPPER || j == LOWER)
				{
					j = copy_token(token, &next_char, &l);
				}
				if (j == DIGIT)
				{
					const char * tptr = token;
					get_num(&tptr, &num);
					num = floor(num);
					if (num < 0.0) num = 0.0;
					//status_interval = (int) floor(num);
					status_interval = (clock_t) num;
				}
			}
			//if (status_interval < 0)
			//	status_interval = 0;
			break;
		case 20:				/* inverse */
		case 27:				/* inverse_modeling */
			pr.inverse = get_true_false(next_char, TRUE);
			break;
		case 21:				/* kinetics */
			pr.kinetics = get_true_false(next_char, TRUE);
			break;
		case 22:				/* dump */
			pr.dump = get_true_false(next_char, TRUE);
			phrq_io->Set_dump_on(pr.dump == TRUE);
			break;
		case 23:				/* user_print */
		case 24:				/* user_pr */
			pr.user_print = get_true_false(next_char, TRUE);
			break;
		case 25:				/* solid_solution */
		case 26:				/* solid_solutions */
			pr.ss_assemblage = get_true_false(next_char, TRUE);
			break;
		case 28:				/* headings */
		case 29:				/* heading */
			pr.headings = get_true_false(next_char, TRUE);
			break;
		case 30:				/* user_graph */
			pr.user_graph = get_true_false(next_char, TRUE);
			break;
		case 31:				/* echo_input */
			pr.echo_input = get_true_false(next_char, TRUE);
			if (pr.echo_input == 0)
				phrq_io->Set_echo_on(false);
			else
				phrq_io->Set_echo_on(true);
			break;
		case 32:				/* warning */
		case 33:				/* warnings */
			(void)sscanf(next_char, "%d", &pr.warnings);
			break;
		case 34:				/* initial_isotopes */
			pr.initial_isotopes = get_true_false(next_char, TRUE);
			break;
		case 35:				/* isotope_ratios */
			pr.isotope_ratios = get_true_false(next_char, TRUE);
			break;
		case 36:				/* isotope_alphas */
			pr.isotope_alphas = get_true_false(next_char, TRUE);
			break;
		case 37:				/* censor_species */
			if (copy_token(token, &next_char, &l) != EMPTY)
			{
				(void)sscanf(token, SCANFORMAT, &censor);
			}
			else
			{
				censor = 0;
			}
			break;
		case 38:				/* alkalinity */
			pr.alkalinity = get_true_false(next_char, TRUE);
			break;
		case 40:				/* high_precision */
			value = get_true_false(next_char, TRUE);
			high_precision = (value != FALSE);
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}

/* ---------------------------------------------------------------------- */
/*
 *   Utilitity routines for read
 */
/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
int Phreeqc::
check_key(const char *str)
/* ---------------------------------------------------------------------- */
{
/*
 *   Check if string begins with a keyword, returns TRUE or FALSE
 *
 *   Arguments:
 *      *str is pointer to character string to be checked for keywords
 *   Returns:
 *      TRUE,
 *      FALSE.
 */
	const char* cptr;
	std::string stdtoken;
	char * token1;
	token1 = string_duplicate(str);

	cptr = token1;
	int j = copy_token(stdtoken, &cptr);
	Utilities::str_tolower(stdtoken);
	std::string key(stdtoken);

	if (j == EMPTY)
	{
		next_keyword = Keywords::KEY_END;
	}
	else
	{
		next_keyword = Keywords::Keyword_search(key);
	}

	free_check_null(token1);
	if (next_keyword > 0)
	{
		return TRUE;
	}
	return (FALSE);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
check_units(std::string &tot_units, bool alkalinity, bool check_compatibility,
			const char *default_units, bool print)
/* ---------------------------------------------------------------------- */
{
#define NUNITS (sizeof(units) / sizeof(char *))
/*
 *   Check if legitimate units
 *   Input:
 *	   tot_units	   character string to check,
 *	   alkalinity	  TRUE if alkalinity, FALSE if any other total,
 *	   check_compatibility TRUE check alk and default units, FALSE otherwise
 *	   default_units       character string of default units (check /L, /kg, etc)
 *	   print	       TRUE print warning messages
 *   Output:
 *	   tot_units	   standard form for unit
 */
	const char *units[] = {
		"Mol/l",				/* 0 */
		"mMol/l",				/* 1 */
		"uMol/l",				/* 2 */
		"g/l",					/* 3 */
		"mg/l",					/* 4 */
		"ug/l",					/* 5 */
		"Mol/kgs",				/* 6 */
		"mMol/kgs",				/* 7 */
		"uMol/kgs",				/* 8 */
		"g/kgs",				/* 9 = ppt */
		"mg/kgs",				/* 10 = ppm */
		"ug/kgs",				/* 11 = ppb */
		"Mol/kgw",				/* 12 = mol/kg H2O */
		"mMol/kgw",				/* 13 = mmol/kg H2O */
		"uMol/kgw",				/* 14 = umol/kg H2O */
		"g/kgw",				/* 15 = mol/kg H2O */
		"mg/kgw",				/* 16 = mmol/kg H2O */
		"ug/kgw",				/* 17 = umol/kg H2O */
		"eq/l",					/* 18 */
		"meq/l",				/* 19 */
		"ueq/l",				/* 20 */
		"eq/kgs",				/* 21 */
		"meq/kgs",				/* 22 */
		"ueq/kgs",				/* 23 */
		"eq/kgw",				/* 24 */
		"meq/kgw",				/* 25 */
		"ueq/kgw",				/* 26 */
	};
	Utilities::squeeze_white(tot_units);
	Utilities::str_tolower(tot_units);
	replace("milli", "m", tot_units);
	replace("micro", "u", tot_units);
	replace("grams", "g", tot_units);
	replace("gram", "g", tot_units);
	replace("moles", "Mol", tot_units);
	replace("mole", "Mol", tot_units);
	replace("mol", "Mol", tot_units);
	replace("liter", "l", tot_units);
	replace("kgh", "kgw", tot_units);
	replace("ppt", "g/kgs", tot_units);
	replace("ppm", "mg/kgs", tot_units);
	replace("ppb", "ug/kgs", tot_units);
	replace("equivalents", "eq", tot_units);
	replace("equivalent", "eq", tot_units);
	replace("equiv", "eq", tot_units);

	size_t pos;
	if ((pos = tot_units.find("/l")) != std::string::npos)
	{
		tot_units = tot_units.substr(0, pos + 2);
	}
	else if ((pos = tot_units.find("/kgs")) != std::string::npos)
	{
		tot_units = tot_units.substr(0, pos + 4);
	}
	else if ((pos = tot_units.find("/kgw")) != std::string::npos)
	{
		tot_units = tot_units.substr(0, pos + 4);
	}
/*
 *   Check if unit in list
 */
	bool found = false;
	size_t i;
	for (i = 0; i < NUNITS; i++)
	{
		if (strcmp(tot_units.c_str(), units[i]) == 0)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		if (print)
		{
			error_string = sformatf( "Unknown unit, %s.", tot_units.c_str());
			error_msg(error_string, CONTINUE);
		}
		return (ERROR);
	}

/*
 *   Check if units are compatible with default_units
 */
	if (!check_compatibility)
		return (OK);
/*
 *   Special cases for alkalinity
 */
	if (alkalinity && strstr(tot_units.c_str(), "Mol") != NULL)
	{
		if (print == TRUE)
		{
			error_string = sformatf(
					"Alkalinity given in moles, assumed to be equivalents.");
			warning_msg(error_string);
		}
		replace("Mol", "eq", tot_units);
	}
	if (!alkalinity && strstr(tot_units.c_str(), "eq") != NULL)
	{
		if (print)
		{
			error_msg("Only alkalinity can be entered in equivalents.",
					  CONTINUE);
		}
		return (ERROR);
	}
/*
 *   See if default_units are compatible with tot_units
 */
	if (strstr(default_units, "/l") && strstr(tot_units.c_str(), "/l"))
		return (OK);
	if (strstr(default_units, "/kgs") && strstr(tot_units.c_str(), "/kgs"))
		return (OK);
	if (strstr(default_units, "/kgw") && strstr(tot_units.c_str(), "/kgw"))
		return (OK);

	std::string string = default_units;
	Utilities::replace("kgs", "kg solution", string);
	Utilities::replace("kgs", "kg solution", tot_units);
	Utilities::replace("kgw", "kg water", string);
	Utilities::replace("kgw", "kg water", tot_units);
	Utilities::replace("/l", "/L", string);
	Utilities::replace("Mol", "mol", string);
	Utilities::replace("/l", "/L", tot_units);
	Utilities::replace("Mol", "mol", tot_units);
	if (print == TRUE)
	{
		error_string = sformatf(
				"Units for master species, %s, are not compatible with default units, %s.",
				tot_units.c_str(), string.c_str());
		error_msg(error_string, CONTINUE);
	}
	return (ERROR);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
find_option(const char *item, int *n, const char **list, int count_list, int exact)
/* ---------------------------------------------------------------------- */
{
/*
 *   Compares a string value to match beginning letters of a list of options
 *
 *      Arguments:
 *	 item    entry: pointer to string to compare
 *	 n       exit:  item in list that was matched
 *	 list    entry: pointer to list of character values, assumed to
 *		 be lower case
 *	 count_list entry: number of character values in list
 *
 *      Returns:
 *	 OK      item matched
 *	 ERROR   item not matched
 *	 n       -1      item not matched
 *		 i       position of match in list
 */
	int i;
	std::string stdtoken(item);

	Utilities::str_tolower(stdtoken);
	for (i = 0; i < count_list; i++)
	{
		if (exact == TRUE)
		{
			if (strcmp(list[i], stdtoken.c_str()) == 0)
			{
				*n = i;
				return (OK);
			}
		}
		else
		{
			if (strstr(list[i], stdtoken.c_str()) == list[i])
			{
				*n = i;
				return (OK);
			}
		}
	}
	*n = -1;
	return (ERROR);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
get_true_false(const char *string, int default_value)
/* ---------------------------------------------------------------------- */
{
/*
 *   Returns true unless string starts with "F" or "f"
 */
	int l;
	char token[MAX_LENGTH];
	const char* cptr;

	cptr = string;

	if (copy_token(token, &cptr, &l) == EMPTY)
	{
		return (default_value);
	}
	else
	{
		if (token[0] == 'F' || token[0] == 'f')
		{
			return (FALSE);
		}
	}
	return (TRUE);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
get_option(const char **opt_list, int count_opt_list, const char **next_char)
/* ---------------------------------------------------------------------- */
{
/*
 *   Read a line and check for options
 */
	int j;
	int opt;
	const char *opt_ptr;
	std::string stdoption;
/*
 *   Read line
 */
	j = check_line("get_option", FALSE, TRUE, TRUE, FALSE);
	if (j == EOF)
	{
		j = OPTION_EOF;
	}
	else if (j == KEYWORD)
	{
		j = OPTION_KEYWORD;
	}
	else if (j == OPTION)
	{
		opt_ptr = line;
		copy_token(stdoption, &opt_ptr);
		if (find_option(&(stdoption.c_str()[1]), &opt, opt_list, count_opt_list, FALSE) == OK)
		{
			j = opt;
			replace(stdoption.c_str(), opt_list[j], line_save);
			replace(stdoption.c_str(), opt_list[j], line);
			opt_ptr = line;
			copy_token(stdoption, &opt_ptr);
			*next_char = opt_ptr;
			if (pr.echo_input == TRUE)
			{
				if (!reading_database())
					output_msg(sformatf( "\t%s\n", line_save));
			}
		}
		else
		{
			if (!reading_database())
				output_msg(sformatf( "\t%s\n", line_save));
			error_msg("Unknown option.", CONTINUE);
			error_msg(line_save, CONTINUE);
			input_error++;
			j = OPTION_ERROR;
			*next_char = line;
		}
	}
	else
	{
		opt_ptr = line;
		copy_token(stdoption, &opt_ptr);
		if (find_option(&(stdoption.c_str()[0]), &opt, opt_list, count_opt_list, TRUE) == OK)
		{
			j = opt;
			*next_char = opt_ptr;
		}
		else
		{
			j = OPTION_DEFAULT;
			*next_char = line;
		}
		if (pr.echo_input == TRUE)
		{
			if (!reading_database())
				output_msg(sformatf( "\t%s\n", line_save));
		}
	}
	return (j);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_rates(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads basic code with which to calculate rates
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	const char* cptr;
	int l, n;
	int return_value, opt, opt_save;
	char token[MAX_LENGTH];
	class rate *rate_ptr;
	const char* next_char;
	const char *opt_list[] = {
		"start",				/* 0 */
		"end"					/* 1 */
	};
	int count_opt_list = 2;

	n = -1;
	opt_save = OPTION_DEFAULT;
/*
 *   Read lines
 */
	return_value = UNKNOWN;
	rate_ptr = NULL;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in RATES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* start */
			opt_save = OPT_1;
			break;
		case 1:				/* end */
			opt_save = OPTION_DEFAULT;
			break;
		case OPTION_DEFAULT:	/* read rate name */
			cptr = line;
			copy_token(token, &cptr, &l);
			{
				const char *name = string_hsave(token);
				rate_ptr = rate_search(name, &n);
			}
			if (rate_ptr == NULL)
			{
				size_t count_rates = rates.size();
				rates.resize(count_rates + 1);
				rate_ptr = &rates[count_rates];
			}
			else
			{
				rate_free(rate_ptr);
			}
			rate_ptr->new_def = TRUE;
			rate_ptr->commands.clear();
			rate_ptr->name = string_hsave(token);
			rate_ptr->linebase = NULL;
			rate_ptr->varbase = NULL;
			rate_ptr->loopbase = NULL;
			opt_save = OPT_1;
			break;
		case OPT_1:			/* read command */
			if (rate_ptr == NULL)
			{
				input_error++;
				error_string = sformatf( "No rate name has been defined.");
				error_msg(error_string, CONTINUE);
				opt_save = OPT_1;
				break;
			}
			rate_ptr->commands.append(";\0");
			rate_ptr->commands.append(line);
			opt_save = OPT_1;
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
/*	output_msg(sformatf( "%s", rates[0].commands));
 */ 
	rates_map.clear();
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_user_print(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads basic code with which to calculate rates
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"start",				/* 0 */
		"end"					/* 1 */
	};
	int count_opt_list = 2;
	opt_save = OPTION_DEFAULT;
/*
 *   Read lines
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in USER_PRINT keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* start */
			opt_save = OPTION_DEFAULT;
			break;
		case 1:				/* end */
			opt_save = OPTION_DEFAULT;
			break;
		case OPTION_DEFAULT:	/* read first command */
			rate_free(user_print);
			user_print->new_def = TRUE;
			user_print->commands.clear();
			user_print->linebase = NULL;
			user_print->varbase = NULL;
			user_print->loopbase = NULL;
			user_print->name =
				string_hsave("user defined Basic print routine");
		case OPT_1:			/* read command */
			user_print->commands.append(";\0");
			user_print->commands.append(line);
			opt_save = OPT_1;
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
/*	output_msg(sformatf( "%s", rates[0].commands));
 */ return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_user_punch(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads basic code with which to calculate rates
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int return_value, opt, opt_save;
	std::string stdtoken;
	const char* next_char;
	const char *opt_list[] = {
		"start",				/* 0 */
		"end",					/* 1 */
		"heading",				/* 2 */
		"headings"				/* 3 */
	};
	int count_opt_list = 4;
	opt_save = OPTION_DEFAULT;
/*
 *   Read lines
 */

	int n_user;

	UserPunch temp_user_punch;
	const char* cptr = line;
	temp_user_punch.read_number_description(cptr);
	n_user = temp_user_punch.Get_n_user();
	temp_user_punch.Set_PhreeqcPtr(this);

	//std::map < int, UserPunch >::iterator up = UserPunch_map.find(n_user);
	//if (up != UserPunch_map.end())
	//{
	//	UserPunch_map.erase(up);
	//}
	
	// Malloc rate structure
	class rate* r = new class rate;
	r->commands.clear();
	r->new_def = TRUE;
	r->linebase = NULL;
	r->varbase = NULL;
	r->loopbase = NULL;
	r->name = string_hsave("user defined Basic punch routine");

	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in USER_PUNCH keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* start */
			opt_save = OPTION_DEFAULT;
			break;
		case 1:				/* end */
			opt_save = OPTION_DEFAULT;
			break;
		case 2:				/* headings */
		case 3:				/* heading */
			while (copy_token(stdtoken, &next_char) != EMPTY)
			{
				temp_user_punch.Get_headings().push_back(stdtoken);
			}
			break;
		case OPTION_DEFAULT:	/* read first command */
		{
			r->commands.clear();
		}
		case OPT_1:			/* read command */
			r->commands.append(";\0");
			r->commands.append(line);
			opt_save = OPT_1;
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	UserPunch_map.erase(n_user); 
	UserPunch_map[n_user] = temp_user_punch;
	UserPunch_map[n_user].Set_rate(r);

	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_solid_solutions(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads solid solution data
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	int n_user;
	std::string token;

	int return_value, opt;
	const char* next_char;
	const char *opt_list[] = {
		"component",			/* 0 */
		"comp",					/* 1 */
		"parms",				/* 2 */
		"gugg_nondimensional",	/* 3 */
		"gugg_kj",				/* 4 */
		"activity_coefficients",	/* 5 */
		"distribution_coefficients",	/* 6 */
		"miscibility_gap",		/* 7 */
		"spinodal_gap",			/* 8 */
		"critical_point",		/* 9 */
		"alyotropic_point",		/* 10 */
		"temp",					/* 11 */
		"tempk",				/* 12 */
		"tempc",				/* 13 */
		"thompson",				/* 14 */
		"margules",				/* 15 */
		"comp1",				/* 16 */
		"comp2"					/* 17 */
	};
	int count_opt_list = 18;
/*
 *   Read ss_assemblage number
 */
	cxxSSassemblage temp_ss_assemblage;

	const char* cptr = line;
	temp_ss_assemblage.read_number_description(cptr);
	n_user = temp_ss_assemblage.Get_n_user();
	temp_ss_assemblage.Set_new_def(true);

	std::vector<cxxSScomp> comps;
	cxxSScomp * comp0_ptr = NULL;
	cxxSScomp * comp1_ptr = NULL;
	cxxSS * ss_ptr = NULL;
/*
 *   Set use data to first read
 */
	if (!use.Get_ss_assemblage_in())
	{
		use.Set_ss_assemblage_in(true);
		use.Set_n_ss_assemblage_user(n_user);
	}
/*
 *   Read solid solutions
 */
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SOLID_SOLUTIONS keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
/*
 * New component
 */
		case 0:				/* component */
		case 1:				/* comp */
			{
				cxxSScomp comp(this->phrq_io);
				/*
				*   Read phase name of component
				*/
				cptr = next_char;
				copy_token(token, &cptr);
				comp.Set_name(token);
				/*
				*   Read moles of component
				*/
				
				if (copy_token(token, &cptr) == EMPTY)
				{
					comp.Set_moles(NAN);
				}
				else
				{
					int j = sscanf(token.c_str(), SCANFORMAT, &dummy);
					comp.Set_moles(dummy);
					if (j != 1)
					{
						error_msg("Expected moles of solid solution.", CONTINUE);
						error_msg(line_save, CONTINUE);
						input_error++;
					}
				}
				comps.push_back(comp);
			}
			break;
		case 2:				/* parms */
		case 3:				/* gugg_nondimensional */
			/*
			 *   Read parameters
			 */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			if (copy_token(token, &cptr) != EMPTY)
			{
				(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
				ss_ptr->Get_p()[0] = dummy;
			}
			if (copy_token(token, &cptr) != EMPTY)
			{
				(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
				ss_ptr->Get_p()[1] = dummy;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_A0_A1);
			break;
		case 4:				/* gugg_kj */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			if (copy_token(token, &cptr) != EMPTY)
			{
				(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
				ss_ptr->Get_p()[0] = dummy;
			}
			if (copy_token(token, &cptr) != EMPTY)
			{
				(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
				ss_ptr->Get_p()[1] = dummy;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_DIM_GUGG);
			break;
		case 5:				/* activity coefficients */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 4; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 4)
			{
				error_string = sformatf(
					"Expected 4 parameters to calculate a0 and a1 from two activity coefficients, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_GAMMAS);
			break;
		case 6:				/* distribution coefficients */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 4; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 4)
			{
				error_string = sformatf(
					"Expected 4 parameters to calculate a0 and a1 from two distribution coefficients, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_DIST_COEF);
			break;
		case 7:				/* miscibility_gap */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 2; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 2)
			{
				error_string = sformatf(
						"Expected 2 miscibility gap fractions of component 2 to calculate a0 and a1, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_MISCIBILITY);
			break;
		case 8:				/* spinodal_gap */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 2; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 2)
			{
				error_string = sformatf(
						"Expected 2 spinodal gap fractions of component 2 to calculate a0 and a1, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_SPINODAL);
			break;
		case 9:				/* critical point */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 2; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 2)
			{
				error_string = sformatf(
						"Expected fraction of component 2 and critical temperature to calculate a0 and a1, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_CRITICAL);
			break;
		case 10:				/* alyotropic point */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 2; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 2)
			{
				error_string = sformatf(
						"Expected fraction of component 2 and sigma pi at alyotropic point to calculate a0 and a1, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_ALYOTROPIC);
			break;
		case 12:				/* tempk */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			{
				cptr = next_char;
				int j = 0;
				if (copy_token(token, &cptr) != EMPTY)
				{
					j = sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Set_tk(dummy);
				}
				if (j != 1)
				{
					error_string = sformatf(
						"Expected temperature (Kelvin) for parameters, assemblage %d, solid solution %s, using 298.15 K",
					n_user,
					ss_ptr->Get_name().c_str());
					warning_msg(error_string);
					ss_ptr->Set_tk(298.15);
				}
			}
			break;
		case 11:				/* temp */
		case 13:				/* tempc */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			{
				cptr = next_char;
				int j = 0;
				if (copy_token(token, &cptr) != EMPTY)
				{
					j = sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Set_tk(dummy + 298.15);
				}
				if (j != 1)
				{
					error_string = sformatf(
						"Expected temperature (Celcius) for parameters, assemblage %d, solid solution %s, using 25 C",
					n_user,
					ss_ptr->Get_name().c_str());
					warning_msg(error_string);
					ss_ptr->Set_tk(298.15);
				}
			}
			break;
		case 14:				/* Thompson and Waldbaum */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 2; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 2)
			{
				error_string = sformatf(
						"Expected Wg2 and Wg1 Thompson-Waldbaum parameters to calculate a0 and a1, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_WALDBAUM);
			break;
		case 15:				/* Margules */
			if (!ss_ptr)
			{
				error_msg("Solid solution name has not been defined", CONTINUE);
				break;
			}
			cptr = next_char;
			ss_ptr->Get_p().clear();
			for (int i = 0; i < 2; i++)
			{
				if (copy_token(token, &cptr) != EMPTY)
				{
					(void)sscanf(token.c_str(), SCANFORMAT, &dummy);
					ss_ptr->Get_p().push_back(dummy);
				}
			}
			if (ss_ptr->Get_p().size() != 2)
			{
				error_string = sformatf(
						"Expected alpha2 and alpha3 Margules parameters to calculate a0 and a1, assemblage %d, solid solution %s",
					n_user,
					ss_ptr->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
			ss_ptr->Set_input_case(cxxSS::SS_PARM_MARGULES);
			break;
		case 16:				/* comp1 */

			/*
			 *   Read phase name of component
			 */
			delete comp0_ptr;
			comp0_ptr = new cxxSScomp;
			cptr = next_char;
			copy_token(token, &cptr);
			comp0_ptr->Set_name(token);
			/*
			 *   Read moles of component
			 */
			if (copy_token(token, &cptr) == EMPTY)
			{
				comp0_ptr->Set_moles(NAN);
			}
			else
			{
				int j = sscanf(token.c_str(), SCANFORMAT, &dummy);
				comp0_ptr->Set_moles(dummy);
				if (j != 1)
				{
					error_msg("Expected moles of solid solution.", CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
			}
			break;
		case 17:				/* comp2 */
			delete comp1_ptr;
			comp1_ptr = new cxxSScomp;
			/*
			 *   Read phase name of component
			 */
			cptr = next_char;
			copy_token(token, &cptr);
			comp1_ptr->Set_name(token);
			/*
			 *   Read moles of component
			 */
			if (copy_token(token, &cptr) == EMPTY)
			{
				comp1_ptr->Set_moles(NAN);
			}
			else
			{
				int j = sscanf(token.c_str(), SCANFORMAT, &dummy);
				comp1_ptr->Set_moles(dummy);
				if (j != 1)
				{
					error_msg("Expected moles of solid solution.", CONTINUE);
					error_msg(line_save, CONTINUE);
					input_error++;
				}
			}
			break;
/*
 * New solid solution
 */
		case OPTION_DEFAULT:
			if(ss_ptr)
			{
				if (comp1_ptr)
				{
					comps.insert(comps.begin(), *comp1_ptr);
				}
				if (comp0_ptr)
				{
					comps.insert(comps.begin(), *comp0_ptr);
				}
				ss_ptr->Set_ss_comps(comps);
				temp_ss_assemblage.Get_SSs()[ss_ptr->Get_name()] = *ss_ptr;
				delete ss_ptr;
				ss_ptr = NULL;
				comps.clear();
				delete comp0_ptr;
				delete comp1_ptr;
				comp0_ptr = comp1_ptr = NULL;
			}
			ss_ptr = new cxxSS;
			/*
			 *   Read solid solution name
			 */
			cptr = line;
			copy_token(token, &cptr);
			ss_ptr->Set_name(token);
			ss_ptr->Set_total_moles(0.0);
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}

	// add last ss and clean up 
	if (comp1_ptr)
		comps.insert(comps.begin(), *comp1_ptr);
	if (comp0_ptr)
		comps.insert(comps.begin(), *comp0_ptr);
	if (ss_ptr != NULL && comps.size() > 0)
	{
		ss_ptr->Set_ss_comps(comps);
	}
	if (ss_ptr != NULL && ss_ptr->Get_name().size() > 0)
	{
		temp_ss_assemblage.Get_SSs()[ss_ptr->Get_name()] = *ss_ptr;
	}
	delete ss_ptr;
	ss_ptr = NULL;
	comps.clear();
	delete comp0_ptr;
	delete comp1_ptr;
	comp0_ptr = comp1_ptr = NULL;

	// check non ideal ss
	std::vector<cxxSS *> ss_v;
	for (size_t i = 0; i < ss_v.size(); i++)
	{
		if (ss_v[i]->Get_p()[0] != 0.0 ||
			ss_v[i]->Get_p()[1] != 0.0)
		{
			if (ss_v[i]->Get_ss_comps().size() != 2)
			{
				error_string = sformatf(
						"Solid solution, %s, is nonideal. Must define exactly two components (-comp1 and -comp2).",
						ss_v[i]->Get_name().c_str());
				error_msg(error_string, CONTINUE);
				input_error++;
			}
		}
	}

	// Add to map
	Rxn_ss_assemblage_map[n_user] = temp_ss_assemblage;
	Rxn_new_ss_assemblage.insert(n_user);
	return (return_value);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_llnl_aqueous_model_parameters(void)
/* ---------------------------------------------------------------------- */
{
	/*
	 *      Reads aqueous model parameters
	 *
	 *      Arguments:
	 *	 none
	 *
	 *      Returns:
	 *	 KEYWORD if keyword encountered, input_error may be incremented if
	 *		    a keyword is encountered in an unexpected position
	 *	 EOF     if eof encountered while reading mass balance concentrations
	 *	 ERROR   if error occurred reading data
	 *
	 */
	int return_value, opt;
	const char* next_char;
	const char* opt_list[] = {
		"temperatures",			/* 0 */
		"temperature",			/* 1 */
		"temp",					/* 2 */
		"adh",					/* 3 */
		"debye_huckel_a",		/* 4 */
		"dh_a",					/* 5 */
		"bdh",					/* 6 */
		"debye_huckel_b",		/* 7 */
		"dh_b",					/* 8 */
		"bdot",					/* 9 */
		"b_dot",				/* 10 */
		"c_co2",				/* 11 */
		"co2_coefs"				/* 12 */
	};
	int count_opt_list = 13;
	/*
	 *   Initialize
	 */
	 /*
	  *   Read aqueous model parameters
	  */
	return_value = UNKNOWN;
	int opt_save = OPTION_DEFAULT;
	opt_save = OPTION_DEFAULT;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_DEFAULT:
		case OPTION_ERROR:
			input_error++;
			error_msg
			("Unknown input in LLNL_AQUEOUS_MODEL_PARAMETERS keyword.",
				CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* temperatures */
		case 1:				/* temperature */
		case 2:				/* temp */
		{
			std::istringstream iss(next_char);
			while (iss >> dummy)
			{
				llnl_temp.push_back(dummy);
			}
			opt_save = 2;
		}
		break;
		case 3:				/* adh */
		case 4:				/* debye_huckel_a */
		case 5:				/* dh_a */
		{
			std::istringstream iss(next_char);
			while (iss >> dummy)
			{
				llnl_adh.push_back(dummy);
			}
			opt_save = 5;
		}
		break;
		case 6:				/* bdh */
		case 7:				/* debye_huckel_b */
		case 8:				/* dh_b */
		{
			std::istringstream iss(next_char);
			while (iss >> dummy)
			{
				llnl_bdh.push_back(dummy);
			}
			opt_save = 8;
		}
		break;
		case 9:				/* bdot */
		case 10:				/* b_dot */
		{
			std::istringstream iss(next_char);
			while (iss >> dummy)
			{
				llnl_bdot.push_back(dummy);
			}
			opt_save = 10;
		}
		break;
		case 11:				/* c_co2 */
		case 12:				/* co2_coefs */
		{
			std::istringstream iss(next_char);
			while (iss >> dummy)
			{
				llnl_co2_coefs.push_back(dummy);
			}
			opt_save = 12;
		}
		break;
		}
		return_value = check_line_return;
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	/* check consistency */
	if ((llnl_temp.size() == 0) ||
		(llnl_temp.size() != llnl_adh.size()) ||
		(llnl_temp.size() != llnl_bdh.size()) ||
		(llnl_temp.size() != llnl_bdot.size()))
	{
		error_msg
		("Must define equal number (>0) of temperatures, dh_a, dh_b, and bdot parameters\nin LLNL_AQUEOUS_MODEL",
			CONTINUE);
		input_error++;
	}
	if (llnl_co2_coefs.size() != 5)
	{
		error_msg
		("Must define 5 CO2 activity coefficient parameters in LLNL_AQUEOUS_MODEL",
			CONTINUE);
		input_error++;
	}
	for (size_t i = 1; i < llnl_temp.size(); i++)
	{
		if (llnl_temp[i - 1] > llnl_temp[i])
		{
			error_msg
			("Temperatures must be in ascending order in LLNL_AQUEOUS_MODEL",
				CONTINUE);
			input_error++;
		}
	}

	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
next_keyword_or_option(const char **opt_list, int count_opt_list)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads to next keyword or option or eof
 *
 *   Returns:
 *       KEYWORD
 *       OPTION
 *       EOF
 */
	int opt;
	const char* next_char;

	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_EOF)
		{						/* end of file */
			break;
		}
		else if (opt == OPTION_KEYWORD)
		{						/* keyword */
			break;
		}
		else if (opt >= 0 && opt < count_opt_list)
		{
			break;
		}
		else
		{
			error_msg("Expected a keyword or option.", CONTINUE);
			error_msg(line_save, CONTINUE);
			input_error++;
		}
	}
	return (opt);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_named_logk(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads K's that can be used to calculate K's for species
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */

	int l;
	int i, empty;
	class logk *logk_ptr;
	char token[MAX_LENGTH];

	int return_value, opt, opt_save;
	const char* next_char;
	const char *opt_list[] = {
		"log_k",				/* 0 */
		"logk",					/* 1 */
		"delta_h",				/* 2 */
		"deltah",				/* 3 */
		"analytical_expression",	/* 4 */
		"a_e",					/* 5 */
		"ae",					/* 6 */
		"ln_alpha1000",			/* 7 */
		"add_logk",				/* 8 */
		"add_log_k",			/* 9 */
		"vm"					/* 10 */
	};
	int count_opt_list = 11;
	logk_ptr = NULL;
/*
 *   Read name followed by options
 */
	opt_save = OPTION_DEFAULT;
	return_value = UNKNOWN;
	for (;;)
	{
		opt = get_option(opt_list, count_opt_list, &next_char);
		if (opt == OPTION_DEFAULT)
		{
			opt = opt_save;
		}
		opt_save = OPTION_DEFAULT;
		switch (opt)
		{
		case OPTION_EOF:		/* end of file */
			return_value = EOF;
			break;
		case OPTION_KEYWORD:	/* keyword */
			return_value = KEYWORD;
			break;
		case OPTION_ERROR:
			input_error++;
			error_msg("Unknown input in SPECIES keyword.", CONTINUE);
			error_msg(line_save, CONTINUE);
			break;
		case 0:				/* log_k */
		case 1:				/* logk */
			if (logk_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_log_k_only(next_char, &logk_ptr->log_k[0]);
			logk_copy2orig(logk_ptr);
			opt_save = OPTION_DEFAULT;
			break;
		case 2:				/* delta_h */
		case 3:				/* deltah */
			if (logk_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_delta_h_only(next_char, &logk_ptr->log_k[1],
							  &logk_ptr->original_units);
			logk_copy2orig(logk_ptr);
			opt_save = OPTION_DEFAULT;
			break;
		case 4:				/* analytical_expression */
		case 5:				/* a_e */
		case 6:				/* ae */
			if (logk_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_analytical_expression_only(next_char, &(logk_ptr->log_k[T_A1]));
			logk_copy2orig(logk_ptr);
			opt_save = OPTION_DEFAULT;
			break;
		case 7:				/* ln_alpha1000 */
			if (logk_ptr == NULL)
			{
				error_string = sformatf(
						"No reaction defined before option, %s.",
						opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			empty = TRUE;
			for (i = T_A1; i <= T_A6; i++)
			{
				if (logk_ptr->log_k[i] != 0.0)
				{
					empty = FALSE;
					logk_ptr->log_k[i] = 0.0;
				}
			}
			if (empty == FALSE)
			{
				error_string = sformatf(
						"Analytical expression previously defined for %s in NAMED_EXPRESSIONS\nAnalytical expression will be overwritten.",
						logk_ptr->name);
				warning_msg(error_string);
			}
			read_analytical_expression_only(next_char, &(logk_ptr->log_k[T_A1]));
			for (i = T_A1; i < T_A6; i++)
			{
				logk_ptr->log_k[i] /= 1000. * LOG_10;
			}
			logk_copy2orig(logk_ptr);
			opt_save = OPTION_DEFAULT;
			break;
		case 8:				/* add_logk */
		case 9:				/* add_log_k */
		{
			if (logk_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
					opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			size_t count_add_logk = logk_ptr->add_logk.size();
			logk_ptr->add_logk.resize(count_add_logk + 1);
			/* read name */
			if (copy_token(token, &next_char, &i) == EMPTY)
			{
				input_error++;
				error_string = sformatf(
					"Expected the name of a NAMED_EXPRESSION.");
				error_msg(error_string, CONTINUE);
				break;
			}
			logk_ptr->add_logk[count_add_logk].name =
				string_hsave(token);
			/* read coef */
			i = sscanf(next_char, SCANFORMAT,
				&logk_ptr->add_logk[count_add_logk].coef);
			if (i <= 0)
			{
				logk_ptr->add_logk[count_add_logk].coef = 1;
			}
			opt_save = OPTION_DEFAULT;
		}
		break;
		case 10:            /* vm, molar volume */
			if (logk_ptr == NULL)
			{
				error_string = sformatf(
					"No reaction defined before option, %s.",
				opt_list[opt]);
				error_msg(error_string, CONTINUE);
				input_error++;
				break;
			}
			read_vm_only(next_char, &logk_ptr->log_k[vm0],
				&logk_ptr->original_deltav_units);
			logk_copy2orig(logk_ptr);
			opt_save = OPTION_DEFAULT;
			break;
		case OPTION_DEFAULT:
/*
 *   Get space for logk information
 */
			logk_ptr = NULL;
			copy_token(token, &next_char, &l);

			logk_ptr = logk_store(token, TRUE);
/*
 *   Get pointer to each species in the reaction, store new species if necessary
 */
			opt_save = OPTION_DEFAULT;
			break;
		}
		if (return_value == EOF || return_value == KEYWORD)
			break;
	}
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_copy(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Reads solution,
 *	 equilibrium_phases,
 *	 exchange,
 *	 surface,
 *	 solid_solution,
 *	 gas_phase,
 *	 kinetics,
 *	 mix,
 *	 reaction,
 *	 reaction_temperature
 *
 */
	int i, l, n, n_user, n_user_start, n_user_end, return_value;
	const char* cptr;
	char token[MAX_LENGTH], token1[MAX_LENGTH], nonkeyword[MAX_LENGTH];
/*
 *   Read "copy"
 */
	cptr = line;
	copy_token(token, &cptr, &l);
/*
 *   Read keyword
 */
	copy_token(token, &cptr, &l);
	check_key(token);

	switch (next_keyword)
	{
	case Keywords::KEY_NONE:					/* Have not read line with keyword */
		strcpy(nonkeyword, token);
		break;
	case Keywords::KEY_SOLUTION:				/* Solution */
	case Keywords::KEY_EQUILIBRIUM_PHASES:		/* Pure phases */
	case Keywords::KEY_REACTION:				/* Reaction */
	case Keywords::KEY_MIX:						/* Mix */
	case Keywords::KEY_EXCHANGE:				/* Ex */
	case Keywords::KEY_SURFACE:					/* Surface */
	case Keywords::KEY_REACTION_TEMPERATURE:	/* Temperature */
	case Keywords::KEY_REACTION_PRESSURE:		/* Pressure */
	case Keywords::KEY_GAS_PHASE:				/* Gas */
	case Keywords::KEY_KINETICS:				/* Kinetics */
	case Keywords::KEY_SOLID_SOLUTIONS:			/* solid_solutions */
		break;
	default:
		input_error++;
		error_msg
			("Expecting keyword solution, mix, kinetics, reaction, reaction_pressure, reaction_temperature, equilibrium_phases, exchange, surface, gas_phase, or solid_solutions, or cell.",
			 CONTINUE);
		error_msg(line_save, CONTINUE);
		check_line("End of use", FALSE, TRUE, TRUE, TRUE);
		/* empty, eof, keyword, print */
		return (ERROR);
	}
/*
 *   Read source index
 */
	strcpy(token1, token);
	i = copy_token(token, &cptr, &l);
	if (i == DIGIT)
	{
		(void)sscanf(token, "%d", &n_user);
		//if (n_user < 0)
		//{
		//	error_msg("Source index number must be a positive integer.",
		//			  CONTINUE);
		//	error_msg(line_save, CONTINUE);
		//	input_error++;
		//	return (ERROR);
		//}
		//if (strstr(token, "-") != NULL)
		//{
		//	error_msg
		//		("COPY does not accept a range of numbers for source index",
		//		 CONTINUE);
		//	error_msg(line_save, CONTINUE);
		//	input_error++;
		//	return (ERROR);
		//}
	}
	else
	{
		error_msg("Source index number must be an integer.",
				  CONTINUE);
		error_msg(line_save, CONTINUE);
		input_error++;
		return (ERROR);
	}
/*
 *   Read target index or range of indices
 */
	i = copy_token(token, &cptr, &l);
	if (i == DIGIT)
	{
		replace("-", " ", &token[1]);
		n = sscanf(token, "%d%d", &n_user_start, &n_user_end);
		if (n == 1)
		{
			n_user_end = n_user_start;
		}
	}
	else
	{
		error_msg("Target index number must be an integer.",
				  CONTINUE);
		error_msg(line_save, CONTINUE);
		input_error++;
		return (ERROR);
	}

	switch (next_keyword)
	{
	case Keywords::KEY_NONE:
		str_tolower(nonkeyword);
		if (strstr(nonkeyword, "cell") != nonkeyword)
		{
			error_msg("Unknown input in COPY data block.", CONTINUE);
			error_msg(line_save, CONTINUE);
			input_error++;
			return (ERROR);
		}
		copier_add(&copy_solution, n_user, n_user_start, n_user_end);
		copier_add(&copy_pp_assemblage, n_user, n_user_start, n_user_end);
		copier_add(&copy_reaction, n_user, n_user_start, n_user_end);
		copier_add(&copy_mix, n_user, n_user_start, n_user_end);
		copier_add(&copy_exchange, n_user, n_user_start, n_user_end);
		copier_add(&copy_surface, n_user, n_user_start, n_user_end);
		copier_add(&copy_temperature, n_user, n_user_start, n_user_end);
		copier_add(&copy_pressure, n_user, n_user_start, n_user_end);
		copier_add(&copy_gas_phase, n_user, n_user_start, n_user_end);
		copier_add(&copy_kinetics, n_user, n_user_start, n_user_end);
		copier_add(&copy_ss_assemblage, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_SOLUTION:								/* Solution */
		copier_add(&copy_solution, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_EQUILIBRIUM_PHASES:					/* Pure phases */
		copier_add(&copy_pp_assemblage, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_REACTION:								/* Reaction */
		copier_add(&copy_reaction, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_MIX:										/* Mix */
		copier_add(&copy_mix, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_EXCHANGE:								/* Ex */
		copier_add(&copy_exchange, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_SURFACE:									/* Surface */
		copier_add(&copy_surface, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_REACTION_TEMPERATURE:					/* Temperature */
		copier_add(&copy_temperature, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_REACTION_PRESSURE:						/* Pressure */
		copier_add(&copy_pressure, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_GAS_PHASE:								/* Gas */
		copier_add(&copy_gas_phase, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_KINETICS:								/* Kinetics */
		copier_add(&copy_kinetics, n_user, n_user_start, n_user_end);
		break;
	case Keywords::KEY_SOLID_SOLUTIONS:							/* solid_solutions */
		copier_add(&copy_ss_assemblage, n_user, n_user_start, n_user_end);
		break;
	default:
		error_msg("Error in switch for READ_COPY.", STOP);
		break;
	}
	return_value = check_line("End of COPY", FALSE, TRUE, TRUE, TRUE);
	/* empty, eof, keyword, print */
	return (return_value);
}

/* ---------------------------------------------------------------------- */
int Phreeqc::
read_reaction_pressure(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads REACTION_PRESSURE data block
 *
 *      Arguments:
 *         none
 *
 *      Returns:
 *         KEYWORD if keyword encountered, input_error may be incremented if
 *                    a keyword is encountered in an unexpected position
 *         EOF     if eof encountered while reading mass balance concentrations
 *         ERROR   if error occurred reading data
 *
 */
	assert(!reading_database());

	// Make instance, set n_user, n_user_end, description
	cxxPressure atm(this->phrq_io);
	const char* cptr = line;

	atm.read_number_description(cptr);
	int n_user = atm.Get_n_user();
	/*
	 *  Make parser
	 */
	CParser parser(this->phrq_io);
	if (pr.echo_input == FALSE) parser.set_echo_file(CParser::EO_NONE);
	atm.read(parser);
	if (atm.Get_base_error_count() == 0)
	{
		Rxn_pressure_map[n_user] = atm;
	}

	if (use.Get_pressure_in() == FALSE)
	{
		use.Set_pressure_in(true);
		use.Set_n_pressure_user(atm.Get_n_user());
	}

	// Make copies if necessary
	if (atm.Get_n_user_end() > n_user)
	{
		int i;
		for (i = n_user + 1; i <= atm.Get_n_user_end(); i++)
		{
			Utilities::Rxn_copy(Rxn_pressure_map, n_user, i);
		}
	}

	return cleanup_after_parser(parser);
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_reaction_pressure_raw(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads REACTION_PRESSURE_RAW data block
 *
 *      Arguments:
 *         none
 *
 *      Returns:
 *         KEYWORD if keyword encountered, input_error may be incremented if
 *                    a keyword is encountered in an unexpected position
 *         EOF     if eof encountered while reading mass balance concentrations
 *         ERROR   if error occurred reading data
 *
 */
	assert(!reading_database());

	cxxPressure atm(this->phrq_io);
	/*
	 *  Make parser
	 */
	CParser parser(this->phrq_io);
	if (pr.echo_input == FALSE) parser.set_echo_file(CParser::EO_NONE);
	atm.read_raw(parser);

	// Store
	if (atm.Get_base_error_count() == 0)
	{
		Rxn_pressure_map[atm.Get_n_user()] = atm;
	}

	// Make copies if necessary

	Utilities::Rxn_copies(Rxn_pressure_map, atm.Get_n_user(), atm.Get_n_user_end());


	return cleanup_after_parser(parser);
}
int Phreeqc::
cleanup_after_parser(CParser &parser)
{
	// check_key sets next_keyword
	if (parser.get_m_line_type() == PHRQ_io::LT_EOF)
	{
		strcpy(line, "");
		strcpy(line_save, "");
		next_keyword = Keywords::KEY_END;
		return(TRUE); 
	}
	int return_value = check_key(parser.line().c_str());

	// copy parser line to line and line_save
	// make sure there is enough space
	size_t l1 = strlen(parser.line().c_str()) + 1;
	size_t l2 = strlen(parser.line_save().c_str()) + 1;
	size_t l = (l1 > l2) ? l1 : l2;
	if (l >= (size_t) max_line)
	{
		max_line = (int) l * 2;
		line_save =	(char *) PHRQ_realloc(line_save,
			(size_t) max_line * sizeof(char));
		if (line_save == NULL)
			malloc_error();
		line = (char *) PHRQ_realloc(line, (size_t) max_line * sizeof(char));
		if (line == NULL)
			malloc_error();
	}
	strcpy(line, parser.line().c_str());
	strcpy(line_save, parser.line_save().c_str());
	return return_value;
}
/* ---------------------------------------------------------------------- */
int Phreeqc::
read_temperature(void)
/* ---------------------------------------------------------------------- */
{
/*
 *      Reads REACTION_TEMPERATURE data block
 *
 *      Arguments:
 *         none
 *
 *      Returns:
 *         KEYWORD if keyword encountered, input_error may be incremented if
 *                    a keyword is encountered in an unexpected position
 *         EOF     if eof encountered while reading mass balance concentrations
 *         ERROR   if error occurred reading data
 *
 */
	assert(!reading_database());

	// Make instance, set n_user, n_user_end, description
	cxxTemperature t_react(this->phrq_io);
	const char* cptr = line;
	t_react.read_number_description(cptr);
	int n_user = t_react.Get_n_user();
	/*
	 *  Make parser
	 */
	CParser parser(this->phrq_io);
	if (pr.echo_input == FALSE) parser.set_echo_file(CParser::EO_NONE);
	t_react.read(parser);
	if (t_react.Get_base_error_count() == 0)
	{
		Rxn_temperature_map[n_user] = t_react;
	}

	if (use.Get_temperature_in() == FALSE)
	{
		use.Set_temperature_in(true);
		use.Set_n_temperature_user(t_react.Get_n_user());
	}

	// Make copies if necessary
	if (t_react.Get_n_user_end() > n_user)
	{
		int i;
		for (i = n_user + 1; i <= t_react.Get_n_user_end(); i++)
		{
			Utilities::Rxn_copy(Rxn_temperature_map, n_user, i);
		}
	}

	return cleanup_after_parser(parser);
}
