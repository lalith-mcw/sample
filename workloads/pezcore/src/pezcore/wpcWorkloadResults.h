/************************************

SPEC GWPG/WPC Workload Results
Class that manages the workload results, which is encoded as a JSON file
according to the workload architecture specification.

*************************************/

#include "wpcWorkloads.h"
typedef GenericDocument<UTF16<> > WDocument;
typedef GenericValue<UTF16<> > WValue;
typedef GenericStringBuffer<UTF16<> > WStringBuffer;

class wpcWorkloadResults {
public:
	wpcWorkloadResults(const WDocument &definitionFile);
	~wpcWorkloadResults();

	const WDocument & getDoc() { return wlResults; }
	bool getPrettyString(std::wstring &prettyString);
	bool saveResultsToFile(const wchar_t* filename);

	//To Add the parameters into the result JSON as a copy
	void addParameters(unsigned int testIndex, WValue parameters);
	//To copy the Errors after parameters
	void swapErrors(unsigned int testIndex);

	// Timers use std::chrono::steady_clock to track time
	// Time points that don't use native steady_clock data types use long long int (e.g. duration in milliseconds)
	void startTimer() { timePoint = std::chrono::steady_clock::now(); timerRunning = true; };
	long long stopTimer();
	bool recordTestTime(unsigned int testIndex);
	bool setTestTime(long long testTime, unsigned int testIndex);

	bool getWorkloadName(std::wstring& workloadName);
	bool getParamCommand(std::wstring& command);
	bool getTestNameByIndex(unsigned int textIndex, std::wstring& testName);
	bool getTestCommandByIndex(unsigned int textIndex, std::wstring& command);
	bool getTestScoreRegexByIndex(unsigned int textIndex, std::wstring& scoreRegex);
	bool getTestValidateRegexByIndex(unsigned int textIndex, std::wstring& validateRegex);
	void copyTestDom(unsigned int testIndex);
	void deleteTest(unsigned int testIndex);
	std::vector<std::wstring>  getResCurrTestCleanupFiles(int testIndex);
	bool isMetaDataAvailable(int testIndex);
	bool getTestMetaData(int testIndex, std::vector<std::pair<std::wstring, std::wstring>>& metaData);
	bool setMetaDataResult(int testIndex, std::wstring label, std::wstring value);

	void incNumberOfTests() { numberOfTests++;  }
	void setNumberOfTests(const unsigned int val) { numberOfTests = val; }
	unsigned int getNumberOfTests() { return numberOfTests; }

	// Scores and weights stored in double precision
	// Durations saved to the result object are saved in double precision (e.g. duration in seconds)
	double getTestBaselineScore(unsigned int testIndex);
	double getTestWeight(unsigned int testIndex);
	bool setTestResultsScore(unsigned int testIndex, double score);
	double getTestResultsScore(unsigned int testIndex);
	bool setResultsRunTime(unsigned int runTime);
	bool setResultsScore(double score);
	double getResultsScore();

	// Warnings inserted into warnings array of strings
	void logWarning(const wchar_t* message);
	// Test warnings inserted into the specific test object
	void logTestWarning(unsigned int testIndex, std::wstring sMsg);
	// Errors inserted into errors array of strings
	void logError(const wchar_t* message);
	// Test errors inserted into the specific test object
	void logTestError(unsigned int testIndex, std::wstring sMsg);

	bool computeWeightedGeoMean();

	bool testHasError(unsigned int testIndex);
	void updateCommandbyIndex(unsigned int testIndex, std::wstring param, std::wstring val);
	void appendTestNameByIndex(unsigned int testIndex, std::wstring param, std::wstring description);
	unsigned int getTestTimeOutMinutes(unsigned int testIndex);
private:
	bool runtimeError = false;
	WDocument wlResults;
	std::chrono::steady_clock::time_point timePoint;
	bool timerRunning = false;
	unsigned int numberOfTests = 0;
};
