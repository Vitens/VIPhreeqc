#ifdef __EMSCRIPTEN__
#if defined(__cplusplus)
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>

#include "IPhreeqc.h"
#include "IPhreeqc.hpp"


// only if the emscripten framework is used, we want the bind code to be compiled
#include <emscripten/bind.h>

using namespace emscripten;

std::string c_str_to_std_string(const char* string) {
	if(string != nullptr)
	{
		return std::string(string);
	}
	return std::string();
}

// create wrapper functions that use std::string instead of const char * as return values for simplification
std::string SGetComponent(int id, int n) {
	return c_str_to_std_string(GetComponent(id, n));
}
std::string SGetDumpFileName(int id) {
	return c_str_to_std_string(GetDumpFileName(id));
}
std::string SGetDumpString(int id) {
	return c_str_to_std_string(GetDumpString(id));
}
std::string SGetDumpStringLine(int id, int n) {
	return c_str_to_std_string(GetDumpStringLine(id, n));
}
std::string SGetErrorFileName(int id) {
	return c_str_to_std_string(GetErrorFileName(id));
}
std::string SGetErrorString(int id) {
	return c_str_to_std_string(GetErrorString(id));
}
std::string SGetErrorStringLine(int id, int n) {
	return c_str_to_std_string(GetErrorStringLine(id, n));
}
std::string SGetLogFileName(int id) {
	return c_str_to_std_string(GetLogFileName(id));
}
std::string SGetLogString(int id) {
	return c_str_to_std_string(GetLogString(id));
}
std::string SGetLogStringLine(int id, int n) {
	return c_str_to_std_string(GetLogStringLine(id, n));
}
std::string SGetOutputFileName(int id) {
	return c_str_to_std_string(GetOutputFileName(id));
}
std::string SGetOutputString(int id) {
	return c_str_to_std_string(GetOutputString(id));
}
std::string SGetOutputStringLine(int id, int n) {
	return c_str_to_std_string(GetOutputStringLine(id, n));
}
std::string SGetSelectedOutputFileName(int id) {
	return c_str_to_std_string(GetSelectedOutputFileName(id));
}
std::string SGetSelectedOutputString(int id) {
	return c_str_to_std_string(GetSelectedOutputString(id));
}
std::string SGetSelectedOutputStringLine(int id, int n) {
	return c_str_to_std_string(GetSelectedOutputStringLine(id, n));
}
std::string SGetVersionString(void) {
	return c_str_to_std_string(GetVersionString());
}
std::string SGetWarningString(int id) {
	return c_str_to_std_string(GetWarningString(id));
}
std::string SGetWarningStringLine(int id, int n) {
	return c_str_to_std_string(GetWarningStringLine(id, n));
}
std::string SGetGasComponents(int id, int solution) {
	return c_str_to_std_string(GetGasComponents(id, solution));
}
std::string SGetEquilibriumPhaseComponents(int id, int phase) {
	return c_str_to_std_string(GetEquilibriumPhaseComponents(id, phase));
}
std::string SGetSpecies(int id, int solution) {
	return c_str_to_std_string(GetSpecies(id, solution));
}
std::string SGetSpeciesMasters(int id, int solution) {
	return c_str_to_std_string(GetSpeciesMasters(id, solution));
}
std::string SGetPhases(int id, int solution) {
	return c_str_to_std_string(GetPhases(id, solution));
}
std::string SGetElements(int id, int solution) {
	return c_str_to_std_string(GetElements(id, solution));
}
std::string SGetSolutionList(int id) {
	return c_str_to_std_string(GetSolutionList(id));
}
// create wrapper functions that use std::string instead of const char * as argument for simplification
IPQ_RESULT SAccumulateLine(int id, const std::string& line) {
	return AccumulateLine(id, line.c_str());
}
int SAddError(int id, const std::string& error_msg) {
	return AddError(id, error_msg.c_str());
}
int SAddWarning(int id, const std::string& warn_msg) {
	return AddWarning(id, warn_msg.c_str());
}
int SLoadDatabase(int id, const std::string& filename) {
	return LoadDatabase(id, filename.c_str());
}
int SLoadDatabaseString(int id, const std::string& input) {
	return LoadDatabaseString(id, input.c_str());
}
int SRunFile(int id, const std::string& filename) {
	return RunFile(id, filename.c_str());
}
int SRunString(int id, const std::string& input) {
	return RunString(id, input.c_str());
}
IPQ_RESULT SSetDumpFileName(int id, const std::string& filename) {
	return SetDumpFileName(id, filename.c_str());
}
IPQ_RESULT SSetErrorFileName(int id, const std::string& filename) {
	return SetErrorFileName(id, filename.c_str());
}
IPQ_RESULT SSetLogFileName(int id, const std::string& filename) {
	return SetLogFileName(id, filename.c_str());
}
IPQ_RESULT SSetOutputFileName(int id, const std::string& filename) {
	return SetOutputFileName(id, filename.c_str());
}
IPQ_RESULT SSetSelectedOutputFileName(int id, const std::string& filename) {
	return SetSelectedOutputFileName(id, filename.c_str());
}
double SGetGasComponentMoles(int id, int solution, const std::string& component) {
	return GetGasComponentMoles(id, solution, component.c_str());
}
double SGetEquilibriumPhaseComponentMoles(int id, int solution, const std::string& component) {
	return GetEquilibriumPhaseComponentMoles(id, solution, component.c_str());
}
double SGetTotal(int id, int solution, const std::string& string) {
	return GetTotal(id, solution, string.c_str());
}
double SGetTotalIon(int id, int solution, const std::string& string) {
	return GetTotalIon(id, solution, string.c_str());
}
double SGetTotalElement(int id, int solution, const std::string& string) {
	return GetTotalElement(id, solution, string.c_str());
}
double SGetMoles(int id, int solution, const std::string& species) {
	return GetMoles(id, solution, species.c_str());
}
double SGetActivity(int id, int solution, const std::string& species) {
	return GetActivity(id, solution, species.c_str());
}
double SGetMolality(int id, int solution, const std::string& species) {
	return GetMolality(id, solution, species.c_str());
}
double SGetDiffusion(int id, int solution, const std::string& species) {
	return GetDiffusion(id, solution, species.c_str());
}
double SGetSI(int id, int solution, const std::string& phase) {
	return GetSI(id, solution, phase.c_str());
}
EMSCRIPTEN_BINDINGS(VIPhreeqcModule) {
    enum_<IPQ_RESULT>("IPQ_RESULT")
        .value("IPQ_OK", IPQ_OK)
        .value("IPQ_OUTOFMEMORY", IPQ_RESULT::IPQ_OUTOFMEMORY)
		.value("IPQ_BADVARTYPE", IPQ_RESULT::IPQ_BADVARTYPE)
		.value("IPQ_INVALIDARG", IPQ_RESULT::IPQ_INVALIDARG)
		.value("IPQ_INVALIDROW", IPQ_RESULT::IPQ_INVALIDROW)
		.value("IPQ_INVALIDCOL", IPQ_RESULT::IPQ_INVALIDCOL)
		.value("IPQ_BADINSTANCE", IPQ_RESULT::IPQ_BADINSTANCE)
        ;
	enum_<VAR_TYPE>("VAR_TYPE")
        .value("TT_EMPTY", VAR_TYPE::TT_EMPTY)
        .value("TT_ERROR", VAR_TYPE::TT_ERROR)
		.value("TT_LONG", VAR_TYPE::TT_LONG)
		.value("TT_DOUBLE", VAR_TYPE::TT_DOUBLE)
		.value("TT_STRING", VAR_TYPE::TT_STRING)
        ;
	enum_<VRESULT>("VRESULT")
        .value("VR_OK", VRESULT::VR_OK)
        .value("VR_OUTOFMEMORY", VRESULT::VR_OUTOFMEMORY)
		.value("VR_BADVARTYPE", VRESULT::VR_BADVARTYPE)
		.value("VR_INVALIDARG", VRESULT::VR_INVALIDARG)
		.value("VR_INVALIDROW", VRESULT::VR_INVALIDROW)
		.value("VR_INVALIDCOL", VRESULT::VR_INVALIDCOL)
        ;
	value_object<VAR>("VAR")
		.field("VAR_TYPE",&VAR::type)
		.field("lVal",&VAR::lVal)
		.field("dVal",&VAR::dVal)
		.field("sVal",&VAR::get_sVal, &VAR::set_sVal);
	
	function("c_str_to_std_string", &c_str_to_std_string, allow_raw_pointers());
	function("AccumulateLine", &SAccumulateLine);
	function("AddError", &SAddError);
	function("AddWarning", &SAddWarning);
	function("ClearAccumulatedLines", &ClearAccumulatedLines);
	function("CreateIPhreeqc", &CreateIPhreeqc);
	function("DestroyIPhreeqc", &DestroyIPhreeqc);
	function("GetComponent", &SGetComponent);
	function("GetComponentCount", &GetComponentCount);
	function("GetCurrentSelectedOutputUserNumber", &GetCurrentSelectedOutputUserNumber);
	function("GetDumpFileName", &SGetDumpFileName);
	function("GetDumpFileOn", &GetDumpFileOn);
	function("GetDumpString", &SGetDumpString);
	function("GetDumpStringLine", &SGetDumpStringLine);
	function("GetDumpStringLineCount", &GetDumpStringLineCount);
	function("GetDumpStringOn", &GetDumpStringOn);
	function("GetErrorFileName", &SGetErrorFileName);
	function("GetErrorFileOn", &GetErrorFileOn);
	function("GetErrorOn", &GetErrorOn);
	function("GetErrorString", &SGetErrorString);
	function("GetErrorStringLine", &SGetErrorStringLine);
	function("GetErrorStringLineCount", &GetErrorStringLineCount);
	function("GetErrorStringOn", &GetErrorStringOn);
	function("GetLogFileName", &SGetLogFileName);
	function("GetLogFileOn", &GetLogFileOn);
	function("GetLogString", &SGetLogString);
	function("GetLogStringLine", &SGetLogStringLine);
	function("GetLogStringLineCount", &GetLogStringLineCount);
	function("GetLogStringOn", &GetLogStringOn);
	function("GetNthSelectedOutputUserNumber", &GetNthSelectedOutputUserNumber);
	function("GetOutputFileName", &SGetOutputFileName);
	function("GetOutputFileOn", &GetOutputFileOn);
	function("GetOutputString", &SGetOutputString);
	function("GetOutputStringLine", &SGetOutputStringLine);
	function("GetOutputStringLineCount", &GetOutputStringLineCount);
	function("GetOutputStringOn", &GetOutputStringOn);
	function("GetSelectedOutputColumnCount", &GetSelectedOutputColumnCount);
	function("GetSelectedOutputCount", &GetSelectedOutputCount);
	function("GetSelectedOutputFileName", &SGetSelectedOutputFileName);
	function("GetSelectedOutputFileOn", &GetSelectedOutputFileOn);
	function("GetSelectedOutputRowCount", &GetSelectedOutputRowCount);
	function("GetSelectedOutputString", &SGetSelectedOutputString);
	function("GetSelectedOutputStringLine", &SGetSelectedOutputStringLine);
	function("GetSelectedOutputStringLineCount", &GetSelectedOutputStringLineCount);
	function("GetSelectedOutputStringOn", &GetSelectedOutputStringOn);
	function("GetSelectedOutputValue", &GetSelectedOutputValue, allow_raw_pointers());
	function("GetSelectedOutputValue2", &GetSelectedOutputValue2, allow_raw_pointers());
	function("GetVersionString", &SGetVersionString);
	function("GetWarningString", &SGetWarningString);
	function("GetWarningStringLine", &SGetWarningStringLine);
	function("GetWarningStringLineCount", &GetWarningStringLineCount);
	function("LoadDatabase", &SLoadDatabase);
	function("LoadDatabaseString", &SLoadDatabaseString);
	function("OutputAccumulatedLines", &OutputAccumulatedLines);
	function("OutputErrorString", &OutputErrorString);
	function("OutputWarningString", &OutputWarningString);
	function("RunAccumulated", &RunAccumulated);
	function("RunFile", &SRunFile);
	function("RunString", &SRunString);
	//function("SetBasicCallback", &SetBasicCallback);
	function("SetCurrentSelectedOutputUserNumber", &SetCurrentSelectedOutputUserNumber);
	function("SetDumpFileName", &SSetDumpFileName);
	function("SetDumpFileOn", &SetDumpFileOn);
	function("SetDumpStringOn", &SetDumpStringOn);
	function("SetErrorFileName", &SSetErrorFileName);
	function("SetErrorFileOn", &SetErrorFileOn);
	function("SetErrorOn", &SetErrorOn);
	function("SetErrorStringOn", &SetErrorStringOn);
	function("SetLogFileName", &SSetLogFileName);
	function("SetLogFileOn", &SetLogFileOn);
	function("SetLogStringOn", &SetLogStringOn);
	function("SetOutputFileName", &SSetOutputFileName);
	function("SetOutputFileOn", &SetOutputFileOn);
	function("SetOutputStringOn", &SetOutputStringOn);
	function("SetSelectedOutputFileName", &SSetSelectedOutputFileName);
	function("SetSelectedOutputFileOn", &SetSelectedOutputFileOn);
	function("SetSelectedOutputStringOn", &SetSelectedOutputStringOn);
	/// VITENS VIPHREEQC Extension Functions
	function("GetGasVolume", &GetGasVolume);
	function("GetGasPressure", &GetGasPressure);
	function("GetGasTotalMoles", &GetGasTotalMoles);
	function("GetGasComponentMoles", &SGetGasComponentMoles);
	function("GetGasComponents", &SGetGasComponents);
	function("GetEquilibriumPhaseComponents", &SGetEquilibriumPhaseComponents);
	function("GetEquilibriumPhaseComponentMoles", &SGetEquilibriumPhaseComponentMoles);
	function("GetPH", &GetPH);
	function("GetSC", &GetSC);
	function("GetMu", &GetMu);
	function("GetTemperature", &GetTemperature);
	function("GetPe", &GetPe);
	function("GetMass", &GetMass);
	function("GetVolume", &GetVolume);
	function("GetDensity", &GetDensity);
	function("GetTotal", &SGetTotal);
	function("GetTotalElement", &SGetTotalElement);
	function("GetTotalIon", &SGetTotalIon);
	function("GetMoles", &SGetMoles);
	function("GetActivity", &SGetActivity);
	function("GetDiffusion", &SGetDiffusion);
	function("GetMolality", &SGetMolality);
	function("GetSpecies", &SGetSpecies);
	function("GetSpeciesMasters", &SGetSpeciesMasters);
	function("GetSI", &SGetSI);
	function("GetPhases", &SGetPhases);
	function("GetElements", &SGetElements);
	function("GetSolutionList", &SGetSolutionList);
	// class interface
	// Create a derived class, to avoid char*
	class IPQ_DLL_EXPORT VIPhreeqc : public IPhreeqc 
	{
		public:
			VIPhreeqc(void) : IPhreeqc::IPhreeqc() {
				
			};
			virtual ~VIPhreeqc(void) {
			};
			VRESULT AccumulateLine(const std::string& line) {
				return IPhreeqc::AccumulateLine(line.c_str());
			};
			size_t AddError(const std::string& error_msg) {
				return IPhreeqc::AddError(error_msg.c_str());
			};
			size_t AddWarning(const std::string& warning_msg) {
				return IPhreeqc::AddWarning(warning_msg.c_str());
			};
			std::string GetComponent(int n) {
				return c_str_to_std_string(IPhreeqc::GetComponent(n));
			}
			std::string GetDumpFileName(void)const {
				return c_str_to_std_string(IPhreeqc::GetDumpFileName());
			}
			std::string GetDumpString(void)const {
				return c_str_to_std_string(IPhreeqc::GetDumpString());
			}
			std::string GetDumpStringLine(int n) {
				return c_str_to_std_string(IPhreeqc::GetDumpStringLine(n));
			}
			std::string GetErrorFileName(void)const {
				return c_str_to_std_string(IPhreeqc::GetErrorFileName());
			}
			std::string GetErrorString(void) {
				return c_str_to_std_string(IPhreeqc::GetErrorString());
			}
			std::string GetErrorStringLine(int n) {
				return c_str_to_std_string(IPhreeqc::GetErrorStringLine(n));
			}
			std::string GetLogFileName(void)const {
				return c_str_to_std_string(IPhreeqc::GetLogFileName());
			}
			std::string GetLogString(void)const {
				return c_str_to_std_string(IPhreeqc::GetLogString());
			}
			std::string GetLogStringLine(int n)const {
				return c_str_to_std_string(IPhreeqc::GetLogStringLine(n));
			}
			std::string GetOutputFileName(void)const {
				return c_str_to_std_string(IPhreeqc::GetOutputFileName());
			}
			std::string GetOutputString(void)const {
				return c_str_to_std_string(IPhreeqc::GetOutputString());
			}
			std::string GetOutputStringLine(int n)const {
				return c_str_to_std_string(IPhreeqc::GetOutputStringLine(n));
			}
			std::string GetSelectedOutputFileName(void)const {
				return c_str_to_std_string(IPhreeqc::GetSelectedOutputFileName());
			}
			std::string GetSelectedOutputString(void)const {
				return c_str_to_std_string(IPhreeqc::GetSelectedOutputString());
			}
			std::string GetSelectedOutputStringLine(int n) {
				return c_str_to_std_string(IPhreeqc::GetSelectedOutputStringLine(n));
			}
			std::string GetVersionString(void) {
				return c_str_to_std_string(IPhreeqc::GetVersionString());
			}
			std::string GetWarningString(void) {
				return c_str_to_std_string(IPhreeqc::GetWarningString());
			}
			std::string GetWarningStringLine(int n) {
				return c_str_to_std_string(IPhreeqc::GetWarningStringLine(n));
			}
			int LoadDatabase(const std::string& filename) {
				return IPhreeqc::LoadDatabase(filename.c_str());
			};
			int LoadDatabaseString(const std::string& input) {
				return IPhreeqc::LoadDatabaseString(input.c_str());
			};
			int RunFile(const std::string& filename) {
				return IPhreeqc::RunFile(filename.c_str());
			};
			int RunString(const std::string& input) {
				return IPhreeqc::RunString(input.c_str());
			};
			void SetDumpFileName(const std::string& filename) {
				return IPhreeqc::SetDumpFileName(filename.c_str());
			};
			void SetErrorFileName(const std::string& filename) {
				return IPhreeqc::SetErrorFileName(filename.c_str());
			};
			void SetLogFileName(const std::string& filename) {
				return IPhreeqc::SetLogFileName(filename.c_str());
			};
			void SetOutputFileName(const std::string& filename) {
				return IPhreeqc::SetOutputFileName(filename.c_str());
			};
			void SetSelectedOutputFileName(const std::string& filename) {
				return IPhreeqc::SetSelectedOutputFileName(filename.c_str());
			};
			double GetGasComponentMoles(int gas_phase, const std::string& component) {
				return IPhreeqc::GetGasComponentMoles(gas_phase, component.c_str());
			};
			double GetEquilibriumPhaseComponentMoles(int eq_phase, const std::string& component) {
				return IPhreeqc::GetEquilibriumPhaseComponentMoles(eq_phase, component.c_str());
			};
			double GetTotal(int solution, const std::string& string) {
				return IPhreeqc::GetTotal(solution, string.c_str());
			};
			double GetTotalElement(int solution, const std::string& string) {
				return IPhreeqc::GetTotalElement(solution, string.c_str());
			};
			double GetActivity(int solution, const std::string& species) {
				return IPhreeqc::GetActivity(solution, species.c_str());
			};
			double GetTotalIon(int solution, const std::string& string) {
				return IPhreeqc::GetTotalIon(solution, string.c_str());
			};
			double GetMoles(int solution, const std::string& species) {
				return IPhreeqc::GetMoles(solution, species.c_str());
			};
			double GetMolality(int solution, const std::string& species) {
				return IPhreeqc::GetMolality(solution, species.c_str());
			};
			double GetDiffusion(int solution, const std::string& species) {
				return IPhreeqc::GetDiffusion(solution, species.c_str());
			};
			double GetSI(int solution, const std::string& phase) {
				return IPhreeqc::GetSI(solution, phase.c_str());
			};
	};
	// define baseclass
	class_<PHRQ_io>("PHRQ_io")
	.constructor<>();
	// define derived class
	class_<IPhreeqc, base<PHRQ_io>>("IPhreeqc")
    .constructor<>()
	.function("ClearAccumulatedLines",&IPhreeqc::ClearAccumulatedLines)
	.function("GetAccumulatedLines",&IPhreeqc::GetAccumulatedLines)
	.function("GetComponentCount",&IPhreeqc::GetComponentCount)
	.function("GetCurrentSelectedOutputUserNumber",&IPhreeqc::GetCurrentSelectedOutputUserNumber)
	.function("GetDumpFileOn",&IPhreeqc::GetDumpFileOn)
	.function("GetDumpStringLineCount",&IPhreeqc::GetDumpStringLineCount)
	.function("GetDumpStringOn",&IPhreeqc::GetDumpStringOn)
	.function("GetErrorFileOn",&IPhreeqc::GetErrorFileOn)
	.function("GetErrorOn",&IPhreeqc::GetErrorOn)
	.function("GetErrorStringLineCount",&IPhreeqc::GetErrorStringLineCount)
	.function("GetErrorStringOn",&IPhreeqc::GetErrorStringOn)
	.function("GetId",&IPhreeqc::GetId)
	.function("GetLogFileOn",&IPhreeqc::GetLogFileOn)
	.function("GetLogStringLineCount",&IPhreeqc::GetLogStringLineCount)
	.function("GetLogStringOn",&IPhreeqc::GetLogStringOn)
	.function("GetNthSelectedOutputUserNumber",&IPhreeqc::GetNthSelectedOutputUserNumber)
	.function("GetOutputFileOn",&IPhreeqc::GetOutputFileOn)
	.function("GetOutputStringLineCount",&IPhreeqc::GetOutputStringLineCount)
	.function("GetOutputStringOn",&IPhreeqc::GetOutputStringOn)
	.function("GetSelectedOutputColumnCount",&IPhreeqc::GetSelectedOutputColumnCount)
	.function("GetSelectedOutputCount",&IPhreeqc::GetSelectedOutputCount)
	.function("GetSelectedOutputFileOn",&IPhreeqc::GetSelectedOutputFileOn)
	.function("GetSelectedOutputRowCount",&IPhreeqc::GetSelectedOutputRowCount)
	.function("GetSelectedOutputStringLineCount",&IPhreeqc::GetSelectedOutputStringLineCount)
	.function("GetSelectedOutputStringOn",&IPhreeqc::GetSelectedOutputStringOn)
	.function("GetSelectedOutputValue",&IPhreeqc::GetSelectedOutputValue,allow_raw_pointers())
	.function("GetSelectedOutputValue2",&IPhreeqc::GetSelectedOutputValue2,allow_raw_pointers())
	.function("GetWarningStringLineCount",&IPhreeqc::GetWarningStringLineCount)
	.function("ListComponents",&IPhreeqc::ListComponents)
	.function("OutputAccumulatedLines",&IPhreeqc::OutputAccumulatedLines)
	.function("OutputErrorString",&IPhreeqc::OutputErrorString)
	.function("OutputWarningString",&IPhreeqc::OutputWarningString)
	.function("RunAccumulated",&IPhreeqc::RunAccumulated)
	.function("SetCurrentSelectedOutputUserNumber",&IPhreeqc::SetCurrentSelectedOutputUserNumber)
	.function("SetDumpFileOn",&IPhreeqc::SetDumpFileOn)
	.function("SetDumpStringOn",&IPhreeqc::SetDumpStringOn)
	.function("SetErrorFileOn",&IPhreeqc::SetErrorFileOn)
	.function("SetErrorOn",&IPhreeqc::SetErrorOn)
	.function("SetErrorStringOn",&IPhreeqc::SetErrorStringOn)
	.function("SetLogFileOn",&IPhreeqc::SetLogFileOn)
	.function("SetLogStringOn",&IPhreeqc::SetLogStringOn)
	.function("SetOutputFileOn",&IPhreeqc::SetOutputFileOn)
	.function("SetOutputStringOn",&IPhreeqc::SetOutputStringOn)
	.function("SetSelectedOutputFileOn",&IPhreeqc::SetSelectedOutputFileOn)
	.function("SetSelectedOutputStringOn",&IPhreeqc::SetSelectedOutputStringOn)
	.function("GetGasVolume",&IPhreeqc::GetGasVolume)
	.function("GetGasPressure",&IPhreeqc::GetGasPressure)
	.function("GetGasTotalMoles",&IPhreeqc::GetGasTotalMoles)
	.function("GetGasComponents",&IPhreeqc::GetGasComponents)
	.function("GetEquilibriumPhaseComponents",&IPhreeqc::GetEquilibriumPhaseComponents)
	.function("GetPH",&IPhreeqc::GetPH)
	.function("GetPe",&IPhreeqc::GetPe)
	.function("GetSC",&IPhreeqc::GetSC)
	.function("GetMu",&IPhreeqc::GetMu)
	.function("GetTemperature",&IPhreeqc::GetTemperature)
	.function("GetMass",&IPhreeqc::GetMass)
	.function("GetDensity",&IPhreeqc::GetDensity)
	.function("GetVolume",&IPhreeqc::GetVolume)
	.function("GetSpecies",&IPhreeqc::GetSpecies)
	.function("GetSpeciesMasters",&IPhreeqc::GetSpeciesMasters)
	.function("GetPhases",&IPhreeqc::GetPhases)
	.function("GetElements",&IPhreeqc::GetElements)
	.function("GetSolutionList2",&IPhreeqc::GetSolutionList2)
	;
	// define derived class
	class_<VIPhreeqc, base<IPhreeqc>>("VIPhreeqc")
    .constructor<>()
	.function("AccumulateLine",&VIPhreeqc::AccumulateLine)
	.function("AddError",&VIPhreeqc::AddError)
	.function("AddWarning",&VIPhreeqc::AddWarning)
	.function("GetComponent",&VIPhreeqc::GetComponent)
	.function("GetDumpFileName",&VIPhreeqc::GetDumpFileName)
	.function("GetDumpString",&VIPhreeqc::GetDumpString)
	.function("GetDumpStringLine",&VIPhreeqc::GetDumpStringLine)
	.function("GetErrorFileName",&VIPhreeqc::GetErrorFileName)
	.function("GetErrorString",&VIPhreeqc::GetErrorString)
	.function("GetErrorStringLine",&VIPhreeqc::GetErrorStringLine)
	.function("GetLogFileName",&VIPhreeqc::GetLogFileName)
	.function("GetLogString",&VIPhreeqc::GetLogString)
	.function("GetLogStringLine",&VIPhreeqc::GetLogStringLine)
	.function("GetOutputFileName",&VIPhreeqc::GetOutputFileName)
	.function("GetOutputString",&VIPhreeqc::GetOutputString)
	.function("GetOutputStringLine",&VIPhreeqc::GetOutputStringLine)
	.function("GetSelectedOutputFileName",&VIPhreeqc::GetSelectedOutputFileName)
	.function("GetSelectedOutputString",&VIPhreeqc::GetSelectedOutputString)
	.function("GetSelectedOutputStringLine",&VIPhreeqc::GetSelectedOutputStringLine)
	.function("GetVersionString",&VIPhreeqc::GetVersionString)
	.function("GetWarningString",&VIPhreeqc::GetWarningString)
	.function("GetWarningStringLine",&VIPhreeqc::GetWarningStringLine)
	.function("LoadDatabase",&VIPhreeqc::LoadDatabase)
	.function("LoadDatabaseString",&VIPhreeqc::LoadDatabaseString)
	.function("RunFile",&VIPhreeqc::RunFile)
	.function("RunString",&VIPhreeqc::RunString)
	.function("SetDumpFileName",&VIPhreeqc::SetDumpFileName)
	.function("SetErrorFileName",&VIPhreeqc::SetErrorFileName)
	.function("SetLogFileName",&VIPhreeqc::SetLogFileName)
	.function("SetOutputFileName",&VIPhreeqc::SetOutputFileName)
	.function("SetSelectedOutputFileName",&VIPhreeqc::SetSelectedOutputFileName)
	.function("GetGasComponentMoles",&VIPhreeqc::GetGasComponentMoles)
	.function("GetEquilibriumPhaseComponentMoles",&VIPhreeqc::GetEquilibriumPhaseComponentMoles)
	.function("GetTotal",&VIPhreeqc::GetTotal)
	.function("GetTotalElement",&VIPhreeqc::GetTotalElement)
	.function("GetActivity",&VIPhreeqc::GetActivity)
	.function("GetTotalIon",&VIPhreeqc::GetTotalIon)
	.function("GetMoles",&VIPhreeqc::GetMoles)
	.function("GetMolality",&VIPhreeqc::GetMolality)
	.function("GetDiffusion",&VIPhreeqc::GetDiffusion)
	.function("GetSI",&VIPhreeqc::GetSI)
	;
}
#endif
#endif