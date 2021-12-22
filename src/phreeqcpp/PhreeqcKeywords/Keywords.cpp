#include "Keywords.h"

#if defined(PHREEQCI_GUI)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

Keywords::Keywords(void)
{
}


Keywords::~Keywords(void)
{
}

Keywords::KEYWORDS Keywords::Keyword_search(std::string key)
{
	std::map<const std::string, Keywords::KEYWORDS>::const_iterator it;
	it = phreeqc_keywords.find(key);
	if (it != Keywords::phreeqc_keywords.end())
	{
		return it->second;
	}
	return Keywords::KEY_NONE;
}

const std::string & Keywords::Keyword_name_search(Keywords::KEYWORDS key)
{
	std::map<Keywords::KEYWORDS, const std::string>::const_iterator it;
	it = phreeqc_keyword_names.find(key);
	if (it != Keywords::phreeqc_keyword_names.end())
	{
		return it->second;
	}
	it = phreeqc_keyword_names.find(KEY_NONE);
	return it->second;
}

const std::map<const std::string, Keywords::KEYWORDS>::value_type temp_keywords[] = {
std::map<const std::string, Keywords::KEYWORDS>::value_type("eof",							Keywords::KEY_END),
std::map<const std::string, Keywords::KEYWORDS>::value_type("end", 							Keywords::KEY_END),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_species", 			Keywords::KEY_SOLUTION_SPECIES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_master_species", 		Keywords::KEY_SOLUTION_MASTER_SPECIES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution", 					Keywords::KEY_SOLUTION),
std::map<const std::string, Keywords::KEYWORDS>::value_type("phases", 						Keywords::KEY_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("pure_phases", 					Keywords::KEY_EQUILIBRIUM_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction", 					Keywords::KEY_REACTION),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix", 							Keywords::KEY_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("use", 							Keywords::KEY_USE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("save", 						Keywords::KEY_SAVE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("exchange_species", 			Keywords::KEY_EXCHANGE_SPECIES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("exchange_master_species", 		Keywords::KEY_EXCHANGE_MASTER_SPECIES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("exchange", 					Keywords::KEY_EXCHANGE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("surface_species", 				Keywords::KEY_SURFACE_SPECIES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("surface_master_species", 		Keywords::KEY_SURFACE_MASTER_SPECIES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("surface", 						Keywords::KEY_SURFACE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_temperature", 		Keywords::KEY_REACTION_TEMPERATURE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("inverse_modeling", 			Keywords::KEY_INVERSE_MODELING),
std::map<const std::string, Keywords::KEYWORDS>::value_type("gas_phase", 					Keywords::KEY_GAS_PHASE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("transport", 					Keywords::KEY_TRANSPORT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("debug", 						Keywords::KEY_KNOBS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("selected_output", 				Keywords::KEY_SELECTED_OUTPUT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("select_output", 				Keywords::KEY_SELECTED_OUTPUT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("knobs", 						Keywords::KEY_KNOBS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("print", 						Keywords::KEY_PRINT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium_phases", 			Keywords::KEY_EQUILIBRIUM_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibria", 					Keywords::KEY_EQUILIBRIUM_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium", 					Keywords::KEY_EQUILIBRIUM_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("pure", 						Keywords::KEY_EQUILIBRIUM_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("title", 						Keywords::KEY_TITLE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("comment", 						Keywords::KEY_TITLE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("advection", 					Keywords::KEY_ADVECTION),
std::map<const std::string, Keywords::KEYWORDS>::value_type("kinetics", 					Keywords::KEY_KINETICS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("incremental_reactions", 		Keywords::KEY_INCREMENTAL_REACTIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("incremental", 					Keywords::KEY_INCREMENTAL_REACTIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("rates", 						Keywords::KEY_RATES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_s", 					Keywords::KEY_SOLUTION_SPREAD),
std::map<const std::string, Keywords::KEYWORDS>::value_type("user_print", 					Keywords::KEY_USER_PRINT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("user_punch", 					Keywords::KEY_USER_PUNCH),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solutions", 				Keywords::KEY_SOLID_SOLUTIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solution", 				Keywords::KEY_SOLID_SOLUTIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_spread", 				Keywords::KEY_SOLUTION_SPREAD),
std::map<const std::string, Keywords::KEYWORDS>::value_type("spread_solution", 				Keywords::KEY_SOLUTION_SPREAD),
std::map<const std::string, Keywords::KEYWORDS>::value_type("selected_out", 				Keywords::KEY_SELECTED_OUTPUT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("select_out", 					Keywords::KEY_SELECTED_OUTPUT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("user_graph", 					Keywords::KEY_USER_GRAPH),
std::map<const std::string, Keywords::KEYWORDS>::value_type("llnl_aqueous_model_parameters",Keywords::KEY_LLNL_AQUEOUS_MODEL_PARAMETERS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("llnl_aqueous_model", 			Keywords::KEY_LLNL_AQUEOUS_MODEL_PARAMETERS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("database",						Keywords::KEY_DATABASE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("named_analytical_expression", 	Keywords::KEY_NAMED_EXPRESSIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("named_analytical_expressions", Keywords::KEY_NAMED_EXPRESSIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("named_expressions", 			Keywords::KEY_NAMED_EXPRESSIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("named_log_k", 					Keywords::KEY_NAMED_EXPRESSIONS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("isotopes", 					Keywords::KEY_ISOTOPES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("calculate_values", 			Keywords::KEY_CALCULATE_VALUES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("isotope_ratios", 				Keywords::KEY_ISOTOPE_RATIOS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("isotope_alphas", 				Keywords::KEY_ISOTOPE_ALPHAS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("copy", 						Keywords::KEY_COPY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("pitzer", 						Keywords::KEY_PITZER),
std::map<const std::string, Keywords::KEYWORDS>::value_type("sit", 							Keywords::KEY_SIT),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium_phase", 			Keywords::KEY_EQUILIBRIUM_PHASES),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_raw", 				Keywords::KEY_SOLUTION_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("exchange_raw", 				Keywords::KEY_EXCHANGE_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("surface_raw", 					Keywords::KEY_SURFACE_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium_phases_raw", 		Keywords::KEY_EQUILIBRIUM_PHASES_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("kinetics_raw", 				Keywords::KEY_KINETICS_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solutions_raw", 			Keywords::KEY_SOLID_SOLUTIONS_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("gas_phase_raw", 				Keywords::KEY_GAS_PHASE_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_raw", 				Keywords::KEY_REACTION_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_raw", 						Keywords::KEY_MIX_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_temperature_raw", 	Keywords::KEY_REACTION_TEMPERATURE_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("dump", 						Keywords::KEY_DUMP),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_modify", 				Keywords::KEY_SOLUTION_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium_phases_modify", 	Keywords::KEY_EQUILIBRIUM_PHASES_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("exchange_modify", 				Keywords::KEY_EXCHANGE_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("surface_modify", 				Keywords::KEY_SURFACE_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solutions_modify", 		Keywords::KEY_SOLID_SOLUTIONS_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("gas_phase_modify", 			Keywords::KEY_GAS_PHASE_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("kinetics_modify", 				Keywords::KEY_KINETICS_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("delete", 						Keywords::KEY_DELETE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("run_cells", 					Keywords::KEY_RUN_CELLS),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_modify", 				Keywords::KEY_REACTION_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_temperature_modify", 	Keywords::KEY_REACTION_TEMPERATURE_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solution_modify", 		Keywords::KEY_SOLID_SOLUTIONS_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_pressure", 			Keywords::KEY_REACTION_PRESSURE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_pressures", 			Keywords::KEY_REACTION_PRESSURE),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_pressure_raw", 		Keywords::KEY_REACTION_PRESSURE_RAW),
std::map<const std::string, Keywords::KEYWORDS>::value_type("reaction_pressure_modify", 	Keywords::KEY_REACTION_PRESSURE_MODIFY),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solution_mix", 	            Keywords::KEY_SOLUTION_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_solution", 	            Keywords::KEY_SOLUTION_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("exchange_mix", 	            Keywords::KEY_EXCHANGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_exchange", 	            Keywords::KEY_EXCHANGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("gas_phase_mix", 	            Keywords::KEY_GAS_PHASE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_gas_phase", 	            Keywords::KEY_GAS_PHASE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("kinetics_mix", 	            Keywords::KEY_KINETICS_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_kinetics", 	            Keywords::KEY_KINETICS_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium_phases_mix", 	    Keywords::KEY_PPASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_equilibrium_phases", 	    Keywords::KEY_PPASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("equilibrium_phase_mix", 	    Keywords::KEY_PPASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_equilibrium_phase", 	    Keywords::KEY_PPASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solutions_mix", 	        Keywords::KEY_SSASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_solid_solutions", 	        Keywords::KEY_SSASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("solid_solution_mix", 	        Keywords::KEY_SSASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_solid_solution", 	        Keywords::KEY_SSASSEMBLAGE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("surface_mix", 	                Keywords::KEY_SURFACE_MIX),
std::map<const std::string, Keywords::KEYWORDS>::value_type("mix_surface", 	                Keywords::KEY_SURFACE_MIX)
};
const std::map<const std::string, Keywords::KEYWORDS> Keywords::phreeqc_keywords(temp_keywords, temp_keywords + sizeof temp_keywords / sizeof temp_keywords[0]);

const std::map<Keywords::KEYWORDS, std::string>::value_type temp_keyword_names[] = {
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_NONE,							"UNKNOWN"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_END,							"END"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION_SPECIES,				"SOLUTION_SPECIES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION_MASTER_SPECIES,		"SOLUTION_MASTER_SPECIES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION,						"SOLUTION"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_PHASES,						"PHASES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION,						"REACTION"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_MIX,							"MIX"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_USE,							"USE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SAVE,							"SAVE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EXCHANGE_SPECIES,				"EXCHANGE_SPECIES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EXCHANGE_MASTER_SPECIES,		"EXCHANGE_MASTER_SPECIES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EXCHANGE,						"EXCHANGE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SURFACE_SPECIES,				"SURFACE_SPECIES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SURFACE_MASTER_SPECIES,		"SURFACE_MASTER_SPECIES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SURFACE,						"SURFACE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_TEMPERATURE,			"REACTION_TEMPERATURE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_INVERSE_MODELING,				"INVERSE_MODELING"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_GAS_PHASE,					"GAS_PHASE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_TRANSPORT,					"TRANSPORT"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SELECTED_OUTPUT,				"SELECTED_OUTPUT"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_KNOBS,						"KNOBS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_PRINT,						"PRINT"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EQUILIBRIUM_PHASES,			"EQUILIBRIUM_PHASES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_TITLE,						"TITLE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_ADVECTION,					"ADVECTION"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_KINETICS,						"KINETICS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_INCREMENTAL_REACTIONS,		"INCREMENTAL_REACTIONS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_RATES,						"RATES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_USER_PRINT,					"USER_PRINT"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_USER_PUNCH,					"USER_PUNCH"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLID_SOLUTIONS,				"SOLID_SOLUTIONS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION_SPREAD,				"SOLUTION_SPREAD"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_USER_GRAPH,					"USER_GRAPH"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_LLNL_AQUEOUS_MODEL_PARAMETERS,"LLNL_AQUEOUS_MODEL_PARAMETERS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_DATABASE,						"DATABASE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_NAMED_EXPRESSIONS,			"NAMED_EXPRESSIONS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_ISOTOPES,						"ISOTOPES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_CALCULATE_VALUES,				"CALCULATE_VALUES"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_ISOTOPE_RATIOS,				"ISOTOPE_RATIOS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_ISOTOPE_ALPHAS,				"ISOTOPE_ALPHAS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_COPY,							"COPY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_PITZER,						"PITZER"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SIT,							"SIT"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION_RAW,					"SOLUTION_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EXCHANGE_RAW,					"EXCHANGE_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SURFACE_RAW,					"SURFACE_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EQUILIBRIUM_PHASES_RAW,		"EQUILIBRIUM_PHASES_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_KINETICS_RAW,					"KINETICS_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLID_SOLUTIONS_RAW,			"SOLID_SOLUTIONS_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_GAS_PHASE_RAW,				"GAS_PHASE_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_RAW,					"REACTION_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_MIX_RAW,						"MIX_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_TEMPERATURE_RAW,		"REACTION_TEMPERATURE_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_DUMP,							"DUMP"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION_MODIFY,				"SOLUTION_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EQUILIBRIUM_PHASES_MODIFY,	"EQUILIBRIUM_PHASES_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EXCHANGE_MODIFY,				"EXCHANGE_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SURFACE_MODIFY,				"SURFACE_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLID_SOLUTIONS_MODIFY,		"SOLID_SOLUTIONS_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_GAS_PHASE_MODIFY,				"GAS_PHASE_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_KINETICS_MODIFY,				"KINETICS_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_DELETE,						"DELETE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_RUN_CELLS,					"RUN_CELLS"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_MODIFY,				"REACTION_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_TEMPERATURE_MODIFY,	"REACTION_TEMPERATURE_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_PRESSURE,			"REACTION_PRESSURE"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_PRESSURE_RAW,		"REACTION_PRESSURE_RAW"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_REACTION_PRESSURE_MODIFY,		"REACTION_PRESSURE_MODIFY"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SOLUTION_MIX,		            "SOLUTION_MIX"),	
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_EXCHANGE_MIX,		            "EXCHANGE_MIX"),	
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_GAS_PHASE_MIX,		        "GAS_PHASE_MIX"),	
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_KINETICS_MIX,		            "KINETICS_MIX"),	
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_PPASSEMBLAGE_MIX,		        "EQUILIBRIUM_PHASES_MIX"),
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SSASSEMBLAGE_MIX,		        "SOLID_SOLUTIONS_MIX"),	
std::map<Keywords::KEYWORDS, const std::string>::value_type(Keywords::KEY_SURFACE_MIX,		            "SURFACE_MIX")	
};
const std::map<Keywords::KEYWORDS, const std::string> Keywords::phreeqc_keyword_names(temp_keyword_names, temp_keyword_names + sizeof temp_keyword_names / sizeof temp_keyword_names[0]);
