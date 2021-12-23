#include "wpcWorkloadParams.h"

// Parse the default parameter filename and validate it
// Throws an exception string on failure
wpcWorkloadParams::wpcWorkloadParams()
{
	if (!parseJsonFile(WPC_PARAMETERS_FILENAME)) {
		throw(L"Unable to parse parameter file! Exiting.");
	}

	if (!isValidParamJSON()) {
		throw(L"Unable to parse parameter file! Exiting.");
	}
}

// Parse the parameter filename provided and validate it
// Throws an exception string on failure
wpcWorkloadParams::wpcWorkloadParams(const wchar_t* wld_filename)
{
	if (!parseJsonFile(wld_filename)) {
		throw(L"Unable to parse parameter file! Exiting.");
	}

	if (!isValidParamJSON()) {
		throw(L"Unable to parse parameter file! Exiting.");
	}
}

// Parse a Json file, returning a RapidJSON Document
// Returns false on failure, true otherwise
bool wpcWorkloadParams::parseJsonFile(const wchar_t* filename)
{
	std::wstring line, text;
	std::wifstream in(filename);
	if (!in.is_open()) {
		std::wcerr << "Unable to open filename " << filename << "\n";
		return false;
	}

	while (std::getline(in, line))
	{
		text += line + L"\n";
	}

	wlParameters.Parse(text.c_str());
	if (!wlParameters.IsObject()) {
		std::wcerr << "Parser failed to parse " << filename << "\n";
		return false;
	}

	return true;
}

bool wpcWorkloadParams::isValidParamJSON()
{
	/*************************************
	* TODO : update this comment block for params file
	ASSUMPTIONS
	1. doc is an object
	2. workload defined by doc has a name
	3. workload has a reference run time
	4. workload has some number of tests
	5. every test has a name
	6. every test has a command line
	6. every test has a baseline score
	*************************************/
	/*
		// Type name checking
		static const char* kTypeNames[] =
		{ "Null", "False", "True", "Object", "Array", "String", "Number" };

		for (auto& m : doc[WPC_TESTS_STR][1].GetObject())
			std::cout << "Type of member " << m.name.GetString() << " is " << kTypeNames[m.value.GetType()] << std::endl;

		for (Value::ConstValueIterator itr = doc[WPC_TESTS_STR].Begin(); itr != doc[WPC_TESTS_STR].End(); ++itr) {
			std::cout << "Test found: " << itr << std::endl;
		}
	*/
	if (!wlParameters.IsObject())
		return false;

	if (wlParameters.HasMember(WPC_PARAMETERS_REQS))
		customRequirements = true;

	if (!wlParameters.HasMember(WPC_TESTS_STR))
		return false;

	if (!wlParameters[WPC_TESTS_STR].IsArray())
		return false;

	bool tests_valid = wlParameters[WPC_TESTS_STR][0].HasMember(WPC_NAME_STR);

	for (SizeType i = 0; i < wlParameters[WPC_TESTS_STR].Size(); i++) {
		if (!wlParameters[WPC_TESTS_STR][i].HasMember(WPC_NAME_STR))
			tests_valid = false;
		if (!wlParameters[WPC_TESTS_STR][i].HasMember(WPC_PARAMETERS_PERMS))
			tests_valid = false;
		else
			numberOfTests++;
	}

	return tests_valid;
}

// Check if a test has an error field
bool wpcWorkloadParams::testHasError(unsigned int testIndex)
{
	try {
		if (wlParameters[WPC_TESTS_STR][testIndex].HasMember(WPC_ERRORS_STR)) {
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

//getNumberOfPermutations
unsigned int wpcWorkloadParams::getNumberOfPermutations(unsigned int testIndex)
{
	return wlParameters[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS].Size();
}

unsigned int wpcWorkloadParams::getNumberOfParamsInPermutation(unsigned int testIndex, unsigned int permIndex)
{
	return wlParameters[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS][permIndex].Size();
}

std::wstring wpcWorkloadParams::getParameterString(unsigned int testIndex, unsigned int permIndex, unsigned int paramIndex)
{
	return wlParameters[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS][permIndex][paramIndex][WPC_PARAMETERS_PNAME].GetString();
}

std::wstring wpcWorkloadParams::getParameterValue(unsigned int testIndex, unsigned int permIndex, unsigned int paramIndex)
{
	return wlParameters[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS][permIndex][paramIndex][WPC_PARAMETERS_VALUE].GetString();
}

std::wstring wpcWorkloadParams::getParameterDescription(unsigned int testIndex, unsigned int permIndex, unsigned int paramIndex)
{
	return wlParameters[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS][permIndex][paramIndex][WPC_PARAMETERS_DESC].GetString();
}

WValue wpcWorkloadParams::getParameterObject(unsigned int testIndex, unsigned int permIndex)
{
	return wlParameters[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS][permIndex].GetArray();
}

bool wpcWorkloadParams::getTestNameByIndex(unsigned int testIndex, std::wstring& testName)
{
	try {
		if (wlParameters[WPC_TESTS_STR][testIndex].HasMember(WPC_NAME_STR)) {
			testName = wlParameters[WPC_TESTS_STR][testIndex][WPC_NAME_STR].GetString();
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

bool wpcWorkloadParams::getCustomRequirements(WDocument& requirementDOM)
{
	try {
		if (wlParameters.HasMember(WPC_PARAMETERS_REQS)) {
			requirementDOM.CopyFrom(wlParameters[WPC_PARAMETERS_REQS], requirementDOM.GetAllocator());
#ifdef DEBUGOUTPUT
			WStringBuffer buffer;
			buffer.Clear();
			Writer<WStringBuffer, UTF16<>, UTF16<>> writer(buffer);
			requirementDOM.Accept(writer);
			std::wcerr << "requirementDOM : " << std::wstring(buffer.GetString()) << std::endl;
#endif
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}