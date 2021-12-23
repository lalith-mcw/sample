/************************************

SPEC GWPG/WPC Workload Definition 
Class that manages the workload definition, which is encoded as a JSON file
according to the workload architecture specification.

Provides useful functions for fetching relevant information from the 
workload definition file and invoking workloads as defined by the file.

*************************************/
#include "wpcWorkloads.h"
typedef GenericDocument<UTF16<> > WDocument;
typedef GenericValue<UTF16<> > WValue;

class wpcWorkloadDefinition {

public:
	wpcWorkloadDefinition(); // Defaults to workload.json in the same directory
	wpcWorkloadDefinition(const wchar_t * workload_definition_file);
	~wpcWorkloadDefinition();

	std::vector<std::pair<std::wstring, size_t> > getWorkloadFiles();
	std::vector<std::wstring> getWorkloadCleanupFiles();
	std::vector<std::wstring> getTestCleanupFiles(int indexNumber);

	const WDocument & getDoc() { return wlDefinition; }
	unsigned int getNumberOfTests() { return numberOfTests; }
	bool generateParams();
	unsigned int getNumberOfRequirements(unsigned int testIndex);
	void getRequirementName(unsigned int testIndex, unsigned int reqIndex, std::wstring& reqName);
	REQ_TYPE getRequirementType(unsigned int testIndex, unsigned int reqIndex);
	bool getRequirementValueBool(unsigned int testIndex, unsigned int reqIndex);
	float getRequirementValueNum(unsigned int testIndex, unsigned int reqIndex);

private:
	WDocument wlDefinition; // The rapidjson DOM version of the JSON workload definition
	unsigned int numberOfTests = 0;
	std::vector< std::vector<std::wstring>> testCleanupFiles = {};

	template<typename T> void safe_delete(T*& p) { delete p;	p = NULL; }
	bool parseJsonFile(const wchar_t * filename);
	bool isValidWorkload();
};