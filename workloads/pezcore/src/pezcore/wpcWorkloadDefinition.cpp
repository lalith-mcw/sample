/************************************

SPEC GWPG/WPC Workload Definition
Class that manages the workload definition, which is encoded as a JSON file
according to the workload architecture specification.

Provides useful functions for fetching relevant information from the
workload definition file and invoking workloads as defined by the file.

*************************************/

#include "wpcWorkloadDefinition.h"

// Parse the default workload definition filename and validate it
// Throws an exception string on failure
wpcWorkloadDefinition::wpcWorkloadDefinition()
{
	if (!parseJsonFile(WPC_WORKLOAD_DEFINITION_FILENAME)) {
		throw(L"Unable to parse workload definition file! Exiting.");
	}

	if (!isValidWorkload()) {
		throw(L"Unable to parse workload definition file! Exiting.");
	}
}

// Parse the workload definition filename provided and validate it
// Throws an exception string on failure
wpcWorkloadDefinition::wpcWorkloadDefinition(const wchar_t * wld_filename)
{
	if (!parseJsonFile(wld_filename)) {
		throw(L"Unable to parse workload definition file! Exiting.");
	}
	
	if (!isValidWorkload()) {
		throw(L"Unable to parse workload definition file! Exiting.");
	}
}

wpcWorkloadDefinition::~wpcWorkloadDefinition()
{

}

// Parse a Json file, returning a RapidJSON Document
// Returns false on failure, true otherwise
bool wpcWorkloadDefinition::parseJsonFile(const wchar_t * filename)
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

	wlDefinition.Parse(text.c_str());
	if (!wlDefinition.IsObject()) {
		std::wcerr << "Parser failed to parse " << filename << "\n";
		return false;
	}

	return true;
}

// Validate whether the workload definition file meets the requirements of this 
// _specific_ workload (which may require optional values not required by the spec)
// Returns false on failure, true otherwise
bool wpcWorkloadDefinition::isValidWorkload() 
{
	/*************************************
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
	if (!wlDefinition.IsObject())
		return false;

	if (!wlDefinition.HasMember(WPC_WORKLOAD_NAME_STR))
		return false;

	//	if(!doc[WPC_WORKLOAD_NAME_STR].IsString());
	//	std::cout << "Workload name is " << doc[WPC_WORKLOAD_NAME_STR].GetString() << std::endl;

	if (!wlDefinition.HasMember(WPC_WORKLOAD_REFRUN_STR))
		return false;

	if (!wlDefinition.HasMember(WPC_TESTS_STR))
		return false;

	if (!wlDefinition[WPC_TESTS_STR].IsArray())
		return false;

	bool tests_valid = wlDefinition[WPC_TESTS_STR][0].HasMember(WPC_NAME_STR);

	for (SizeType i = 0; i < wlDefinition[WPC_TESTS_STR].Size(); i++) {
		if (!wlDefinition[WPC_TESTS_STR][i].HasMember(WPC_NAME_STR))
			tests_valid = false;
		if (!wlDefinition[WPC_TESTS_STR][i].HasMember(WPC_BASELINE_STR))
			tests_valid = false;
		if (!wlDefinition[WPC_TESTS_STR][i].HasMember(WPC_COMMAND_STR))
			tests_valid = false;
		else
			numberOfTests++;
	}

	return tests_valid;
}

// returns workloadfiles from workload.json
std::vector<std::pair<std::wstring, size_t> > wpcWorkloadDefinition::getWorkloadFiles()
{
	std::vector<std::pair<std::wstring, size_t> > workloadFiles = {};
	try {
		if (wlDefinition.HasMember(WPC_WORKLOAD_FILES_STR)) {
			for (const auto& file : wlDefinition[WPC_WORKLOAD_FILES_STR].GetArray()) {
				workloadFiles.push_back(std::make_pair(file[WPC_WORKLOAD_FILENAME_STR].GetString(),
					file[WPC_WORKLOAD_FILEHASHVALUE_STR].GetUint64()));
			}
		}
		return workloadFiles;
	}
	catch (...) {
		return workloadFiles;
	}

	return workloadFiles;
}

// returns workload cleanup files from workload.json
std::vector<std::wstring> wpcWorkloadDefinition::getWorkloadCleanupFiles()
{
	std::vector<std::wstring> workloadCleanupFiles = {};
	try {
		if (wlDefinition.HasMember(WPC_WORKLOAD_CLEANUPFILES_STR)) {
			for (const auto& fileName : wlDefinition[WPC_WORKLOAD_CLEANUPFILES_STR].GetArray()) {
				workloadCleanupFiles.push_back(fileName.GetString());
			}
		}
		return workloadCleanupFiles;
	}
	catch (...) {
		return workloadCleanupFiles;
	}

	return workloadCleanupFiles;
}

// returns test cleanup files
std::vector<std::wstring> wpcWorkloadDefinition::getTestCleanupFiles(int indexNumber)
{
	std::vector<std::wstring> currTestFiles = {};
	try {
		if (wlDefinition.HasMember(WPC_TESTS_STR)) {
			if (wlDefinition[WPC_TESTS_STR][indexNumber].HasMember(WPC_WORKLOAD_CLEANUPFILES_STR)) {
				for (const auto& fileName : wlDefinition[WPC_TESTS_STR][indexNumber][WPC_WORKLOAD_CLEANUPFILES_STR].GetArray()) {
					currTestFiles.push_back(fileName.GetString());
				}
			}
			return currTestFiles;
		}
	}
	catch (...) {
		return currTestFiles;
	}

	return currTestFiles;
}

bool wpcWorkloadDefinition::generateParams()
{
	try {
		if (wlDefinition.HasMember(WPC_PARAMETERS_GENERATE) &&
			wlDefinition[WPC_PARAMETERS_GENERATE].HasMember(WPC_PARAMETERS_COMMAND_STR))
			return true;
	}
	catch (...) {
		return 0;
	}

	return 0;
}

unsigned int wpcWorkloadDefinition::getNumberOfRequirements(unsigned int testIndex)
{
	if (wlDefinition[WPC_TESTS_STR][testIndex].HasMember(WPC_REQUIRE))
		return wlDefinition[WPC_TESTS_STR][testIndex][WPC_REQUIRE].MemberCount();
	else
		return 0;
}

void wpcWorkloadDefinition::getRequirementName(unsigned int testIndex, unsigned int reqIndex, std::wstring& reqName)
{
	reqName = (wlDefinition[WPC_TESTS_STR][testIndex][WPC_REQUIRE].MemberBegin()+reqIndex)->name.GetString();
}

REQ_TYPE wpcWorkloadDefinition::getRequirementType(unsigned int testIndex, unsigned int reqIndex)
{
	WValue::ConstMemberIterator object = wlDefinition[WPC_TESTS_STR][testIndex][WPC_REQUIRE].MemberBegin();

	if ((object + reqIndex)->value.IsBool())
		return REQ_TYPE::REQ_BOOL;

	if ((object + reqIndex)->value.IsObject())
	{
		auto requirementDOM = (object + reqIndex)->value.GetObject();
		if(requirementDOM[WPC_REQUIRE_TYPE].GetString() == std::wstring(WPC_REQUIRE_MIN))
			return REQ_TYPE::REQ_MIN;

		if (requirementDOM[WPC_REQUIRE_TYPE].GetString() == std::wstring(WPC_REQUIRE_MAX))
			return REQ_TYPE::REQ_MAX;
	}

	return REQ_TYPE::REQ_UNKNOWN;
}

bool wpcWorkloadDefinition::getRequirementValueBool(unsigned int testIndex, unsigned int reqIndex)
{
	WValue::ConstMemberIterator object = wlDefinition[WPC_TESTS_STR][testIndex][WPC_REQUIRE].MemberBegin();

	return (object + reqIndex)->value.GetBool();
}

float wpcWorkloadDefinition::getRequirementValueNum(unsigned int testIndex, unsigned int reqIndex)
{
	WValue::ConstMemberIterator object = wlDefinition[WPC_TESTS_STR][testIndex][WPC_REQUIRE].MemberBegin();

	//just treat all numbers as float
	//float val;
	//if ((object + reqIndex)->value.GetObject()[L"value"].IsInt())
	//	val = (object + reqIndex)->value.GetObject()[L"value"].GetInt();
	//else
	//	val = (object + reqIndex)->value.GetObject()[L"value"].GetFloat();

	return (object + reqIndex)->value.GetObject()[WPC_REQUIRE_VALUE].GetFloat();
}