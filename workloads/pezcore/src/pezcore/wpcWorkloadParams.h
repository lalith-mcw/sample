/************************************

SPEC GWPG/WPC Workload Input Parameter handler
Class that handles input parameters to workloads.

*************************************/

#pragma once
#include <Windows.h>
#include "wpcWorkloads.h"
typedef GenericDocument<UTF16<> > WDocument;
typedef GenericStringBuffer<UTF16<>> WStringBuffer;
typedef GenericValue<UTF16<> > WValue;

class wpcWorkloadParams
{
public:
	wpcWorkloadParams(); // Defaults to workload.json in the same directory
	wpcWorkloadParams(const wchar_t* workload_definition_file);
	~wpcWorkloadParams();

	//handle built in parameters
	unsigned int getNumThreads() { return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS); };

	const WDocument& getDoc() { return wlParameters; }
	unsigned int getNumberOfTests() { return numberOfTests; }
	bool testHasError(unsigned int testIndex);
	unsigned int getNumberOfPermutations(unsigned int testIndex);
	unsigned int getNumberOfParamsInPermutation(unsigned int testIndex, unsigned int permIndex);
	std::wstring getParameterString(unsigned int testIndex, unsigned int permIndex, unsigned int paramIndex);
	std::wstring getParameterValue(unsigned int testIndex, unsigned int permIndex, unsigned int paramIndex);
	std::wstring getParameterDescription(unsigned int testIndex, unsigned int permIndex, unsigned int paramIndex);
	bool getTestNameByIndex(unsigned int testIndex, std::wstring& testName);
	bool hasCustomRequirements() { return customRequirements; };
	bool getCustomRequirements(WDocument& requirementDOM);

	//To get parameters object and paste it into the result file
	WValue getParameterObject(unsigned int testIndex, unsigned int permIndex);

private:
	WDocument wlParameters; // The rapidjson DOM version of the JSON
	unsigned int numberOfTests = 0;
	bool customRequirements = false;

	template<typename T> void safe_delete(T*& p) { delete p;	p = NULL; }
	bool parseJsonFile(const wchar_t* filename);
	bool isValidParamJSON();
};

