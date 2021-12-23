/************************************

SPEC GWPG/WPC Workload Results
Class that manages the workload results, which is encoded as a JSON file
according to the workload architecture specification.

*************************************/

#include "wpcWorkloadResults.h"

// Given a workload definition file, copy it and start a results file record
// Throws an exception string on failure
wpcWorkloadResults::wpcWorkloadResults(const WDocument &definitionFile)
{

	try {
		wlResults.CopyFrom(definitionFile, wlResults.GetAllocator());
	}
	catch (...)
	{
		throw L"Unable to copy from workload definition file.";
	}

	// Timestamp the results
	time_t now = time(NULL);
	struct tm tbuf;
	time(&now);
	wchar_t buf[sizeof L"2011-10-08T07:07:09Z"];
	gmtime_s(&tbuf, &now);
	wcsftime(buf, sizeof buf, L"%FT%TZ", &tbuf);
	WValue dateTime;
	// Casting to SizeType to get rid of the error (the buffer size won't be larger than 32-bits can store)
	dateTime.SetString(buf, (rapidjson::SizeType)wcsnlen_s(buf, 255), wlResults.GetAllocator());
	wlResults.AddMember(L"dateTime", dateTime, wlResults.GetAllocator());

}

wpcWorkloadResults::~wpcWorkloadResults()
{

}

// Fill a string with pretty-printed JSON-formatted results
// Returns true on success, false otherwise
bool wpcWorkloadResults::getPrettyString(std::wstring &prettyString)
{
	try
	{
		WStringBuffer res_string;
		PrettyWriter<WStringBuffer, UTF16<>> writer(res_string);
		wlResults.Accept(writer);
		prettyString = res_string.GetString();
	}
	catch (...) {
		return false;
	}		

	return true;
}

// Save results to the filename provided
// Returns true on success, false otherwise
bool wpcWorkloadResults::saveResultsToFile(const wchar_t * filename)
{
	WStringBuffer res_string;

	try
	{
		PrettyWriter<WStringBuffer, UTF16<>> writer(res_string);
		wlResults.Accept(writer);	
	}
	catch (...) {
		return false;
	}

	try
	{
		std::wofstream out(filename);
		if (!out)
			return false;
		out << res_string.GetString();
		out.close();
	}
	catch (...) {
		return false;
	}

	return true;
}

void wpcWorkloadResults::addParameters(unsigned int testIndex, WValue parameters)
{
	if (!wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_PARAMETERS_PERMS))
	{
		wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_PARAMETERS_PERMS, parameters, wlResults.GetAllocator());
	}
}

void wpcWorkloadResults::swapErrors(unsigned int testIndex)
{
	if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_ERRORS_STR))
	{
		WValue resultsCopy(kArrayType);
		resultsCopy = wlResults[WPC_TESTS_STR][testIndex][WPC_ERRORS_STR];
		wlResults[WPC_TESTS_STR][testIndex].RemoveMember(WPC_ERRORS_STR);
		wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_ERRORS_STR, resultsCopy, wlResults.GetAllocator());
	}
}

// Warnings inserted into warnings array of strings
// TODO: Consider adding exception handling to log to stderr
void wpcWorkloadResults::logWarning(const wchar_t* message)
{
	if (!message)
		return;

	std::wstring sMsg = message;

	// May need just the string...
	WValue msgStr(kStringType);
	msgStr.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator());

	// ...or may need the entire array...
	WValue msgArray(kArrayType);
	msgArray.PushBack(msgStr, wlResults.GetAllocator());
	
	// If it doesn't have any warnings yet, create the array and add it as a member
	if (!wlResults.HasMember(WPC_WARNINGS_STR)) {
		wlResults.AddMember(WPC_WARNINGS_STR, msgArray, wlResults.GetAllocator());
	}
	else {
		// If it has warnings but the member isn't an array, overwrite it
		if (!wlResults[WPC_WARNINGS_STR].IsArray())
		{
			// Clear and reset it as an array
			wlResults.AddMember(WPC_WARNINGS_STR, msgArray, wlResults.GetAllocator());
		}
		else {
			// If warnings exists, and it's an array, push another value into the array
			wlResults[WPC_WARNINGS_STR].PushBack(
				rapidjson::GenericValue<rapidjson::UTF16<>>{}.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator()), wlResults.GetAllocator());
		}
	}
}

// Test warnings inserted into the specific test object
// TODO: Consider adding exception handling to log to stderr
void wpcWorkloadResults::logTestWarning(unsigned int testIndex, std::wstring sMsg)
{
	if (sMsg.empty())
		return;

	// May need just the string...
	WValue msgStr(kStringType);
	msgStr.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator());

	// ...or may need the entire array...
	WValue msgArray(kArrayType);
	msgArray.PushBack(msgStr, wlResults.GetAllocator());

	// Test to see if the testIndex exists, is an array, and the index provided
	// is within the range
	if (!wlResults.HasMember(WPC_TESTS_STR))
		return;
	if (!wlResults[WPC_TESTS_STR].IsArray())
		return;
	if (wlResults[WPC_TESTS_STR].Size() <= testIndex)
		return;
	
	// If it doesn't have any warnings yet, create the array and add it as a member
	if (!wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_WARNINGS_STR)) {
		wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_WARNINGS_STR, msgArray, wlResults.GetAllocator());
	}
	else {
		// If it has warnings but the member isn't an array, overwrite it
		if (!wlResults[WPC_TESTS_STR][testIndex][WPC_WARNINGS_STR].IsArray())
		{
			// Clear and reset it as an array
			wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_WARNINGS_STR, msgArray, wlResults.GetAllocator());
		}
		else {
			// If warnings exists, and it's an array, push another value into the array
			wlResults[WPC_TESTS_STR][testIndex][WPC_WARNINGS_STR].PushBack(
				rapidjson::GenericValue<rapidjson::UTF16<>>{}.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator()), wlResults.GetAllocator());
		}
	}
}

// Errors inserted into errors array of strings
// TODO: Consider adding exception handling to log to stderr
void wpcWorkloadResults::logError(const wchar_t* message)
{
	if (!message)
		return;

	runtimeError = true;

	std::wstring sMsg = message;

	// May need just the string...
	WValue msgStr(kStringType);
	msgStr.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator());

	// ...or may need the entire array...
	WValue msgArray(kArrayType);
	msgArray.PushBack(msgStr, wlResults.GetAllocator());

	// If it doesn't have any warnings yet, create the array and add it as a member
	if (!wlResults.HasMember(WPC_ERRORS_STR)) {
		wlResults.AddMember(WPC_ERRORS_STR, msgArray, wlResults.GetAllocator());
	}
	else {
		// If it has warnings but the member isn't an array, overwrite it
		if (!wlResults[WPC_ERRORS_STR].IsArray())
		{
			// Clear and reset it as an array
			wlResults.AddMember(WPC_ERRORS_STR, msgArray, wlResults.GetAllocator());
		}
		else {
			// If warnings exists, and it's an array, push another value into the array
			wlResults[WPC_ERRORS_STR].PushBack(
				rapidjson::GenericValue<rapidjson::UTF16<>>{}.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator()), wlResults.GetAllocator());
		}
	}
}

// Test errors inserted into the specific test object
// TODO: Consider adding exception handling to log to stderr
void wpcWorkloadResults::logTestError(unsigned int testIndex, std::wstring sMsg)
{
	if (sMsg.empty())
		return;

	runtimeError = true;

	// May need just the string...
	WValue msgStr(kStringType);
	msgStr.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator());

	// ...or may need the entire array...
	WValue msgArray(kArrayType);
	msgArray.PushBack(msgStr, wlResults.GetAllocator());

	// Test to see if the testIndex exists, is an array, and the index provided
	// is within the range
	if (!wlResults.HasMember(WPC_TESTS_STR))
		return;
	if (!wlResults[WPC_TESTS_STR].IsArray())
		return;
	if (wlResults[WPC_TESTS_STR].Size() <= testIndex)
		return;

	// If it doesn't have any warnings yet, create the array and add it as a member
	if (!wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_ERRORS_STR)) {
		wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_ERRORS_STR, msgArray, wlResults.GetAllocator());
	}
	else {
		// If it has warnings but the member isn't an array, overwrite it
		if (!wlResults[WPC_TESTS_STR][testIndex][WPC_ERRORS_STR].IsArray())
		{
			// Clear and reset it as an array
			wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_ERRORS_STR, msgArray, wlResults.GetAllocator());
		}
		else {
			// If warnings exists, and it's an array, push another value into the array
			wlResults[WPC_TESTS_STR][testIndex][WPC_ERRORS_STR].PushBack(
				rapidjson::GenericValue<rapidjson::UTF16<>>{}.SetString(sMsg.c_str(), (rapidjson::SizeType)sMsg.length(), wlResults.GetAllocator()), wlResults.GetAllocator());
		}
	}
}

// Stop the timer and return the number of milliseconds since the timer was started
// Returns 0 if the timer was not running
long long wpcWorkloadResults::stopTimer()
{
	// Dangerous: if the timer wasn't running, stopping it should do nothing and inform the user
	// As is, this could create a condition where, if the timer was never started, the result was instantaneous
	if (!timerRunning)
		return 0;

	std::chrono::steady_clock::time_point tpStop = std::chrono::steady_clock::now();
	long long durationMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(tpStop - timePoint).count();

	timerRunning = false;
	return durationMilliseconds;
}

// Based on the test index provided, stop the timer and write to the results object 
// the number of seconds in the timer since the test was started
//
// IMPORTANT: This writes the time elapsed in the 'resultScore' parameter, which is overloaded
//			with other functions that manipulate the resultScore
// TODO: Consider using an array of timers to support parallelism
bool wpcWorkloadResults::recordTestTime(unsigned int testIndex)
{
	if (!timerRunning)
		return false;
	
	long long testTime = stopTimer();

	// If the timer wasn't started, we don't record a zero result
	if (!testTime)
		return false;

	return setTestTime(testTime, testIndex);
}

bool wpcWorkloadResults::setTestTime(long long testTime, unsigned int testIndex)
{
	// Note that internally the timer is in milliseconds, a long long int
	// while the results object stores it in seconds, a double

	WValue testVT;
	// Casting to SizeType to get rid of the error (the buffer size won't be larger than 32-bits can store)
	try {
		testVT.SetDouble(testTime / 1000.);
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_RESULT_SCORE_STR))
			wlResults[WPC_TESTS_STR][testIndex][WPC_RESULT_SCORE_STR].SetDouble(testTime / 1000.);
		else
			wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_RESULT_SCORE_STR, testVT, wlResults.GetAllocator());
	}
	catch (...) {
		return false;
	}
	
	return true;
}

// Get the test baseline score of the test index provided
double wpcWorkloadResults::getTestBaselineScore(unsigned int testIndex)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_BASELINE_STR))
			return wlResults[WPC_TESTS_STR][testIndex][WPC_BASELINE_STR].GetDouble();
		else
			return 0;
	}
	catch (...) {
		// Need ERROR reported  here
		return 0;
	}

	return true;
}

// Get the test weight of the test index provided
double wpcWorkloadResults::getTestWeight(unsigned int testIndex)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_WEIGHT_STR))
			return wlResults[WPC_TESTS_STR][testIndex][WPC_WEIGHT_STR].GetDouble();
		else
			return 0;
	}
	catch (...) {
		// Need ERROR reported  here
		return 0;
	}

	return 0;
}

// Set the test baseline score of the test index provided
bool wpcWorkloadResults::setTestResultsScore(unsigned int testIndex, double score)
{
	Value testScore;
	try {
		testScore.SetDouble(score);
		if(wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_RESULT_SCORE_STR))
			wlResults[WPC_TESTS_STR][testIndex][WPC_RESULT_SCORE_STR].SetDouble(score);
		else
			wlResults[WPC_TESTS_STR][testIndex].AddMember(WPC_RESULT_SCORE_STR, score, wlResults.GetAllocator());
	}
	catch (...) {
		// Need ERROR reported here
		return false;
	}

	return true;
}

// Get the test result score of the test index provided
double wpcWorkloadResults::getTestResultsScore(unsigned int testIndex)
{
	try {
		return wlResults[WPC_TESTS_STR][testIndex][WPC_RESULT_SCORE_STR].GetDouble();
	}
	catch (...) {
		// Need ERROR reported here
		return 0;
	}
}

// Set the total amount of time to run all tests
bool wpcWorkloadResults::setResultsRunTime(unsigned int runTime)
{
	WValue vRT;
	try {
		vRT.SetInt(runTime);
		if (wlResults.HasMember(WPC_RESULTS_RUNTIME_STR))
			wlResults[WPC_RESULTS_RUNTIME_STR].SetInt(runTime);
		else
			wlResults.AddMember(WPC_RESULTS_RUNTIME_STR, vRT, wlResults.GetAllocator());
	}
	catch (...) {
		// Need ERROR reported here
		return false;
	}

	return true;
}

// Set the total score for the entire workload (accounting for all test scores)
bool wpcWorkloadResults::setResultsScore(double score)
{
	if (runtimeError)
		return false;

	WValue vScore;
	try {
		vScore.SetDouble(score);
		if (wlResults.HasMember(WPC_RESULT_SCORE_STR))
			wlResults[WPC_RESULT_SCORE_STR].SetDouble(score);
		else
			wlResults.AddMember(WPC_RESULT_SCORE_STR, vScore, wlResults.GetAllocator());
	}
	catch (...) {
		// Need ERROR reported here
		return false;
	}

	return true;
}

// Set the total score for the entire workload (accounting for all test scores)
double wpcWorkloadResults::getResultsScore()
{
	try {
		if (wlResults.HasMember(WPC_RESULT_SCORE_STR))
			return wlResults[WPC_RESULT_SCORE_STR].GetDouble();
		else
			return 0;
	}
	catch (...) {
		// Need ERROR reported here
		return 0;
	}
}

// Compare the time to complete each test with the baseline score and compute the final score using 
//	a blend of the other scores(e.g. weight geometric mean) in the results
bool wpcWorkloadResults::computeWeightedGeoMean()
{
	//		Math of this: https://en.wikipedia.org/wiki/Weighted_geometric_mean
	double testWeight = 0.;
	double baselineScore = 0.;
	double testScore = 0.;
	std::vector <std::pair<double, double>> weights_scores;
	double sumWlnX = 0.;
	double sumW = 0.;
	double finalScore = 0.;
	bool lowerIsBetter = false;

	try {
		// Start by scoring all the tests and saving those
		for (unsigned int i = 0; i < wlResults[WPC_TESTS_STR].Size(); i++) {

			//Check that all values necessary to calculate geomean are present
			if (!wlResults[WPC_TESTS_STR][i].HasMember(WPC_BASELINE_STR)
				|| !wlResults[WPC_TESTS_STR][i].HasMember(WPC_RESULT_SCORE_STR)
				|| !wlResults[WPC_TESTS_STR][i].HasMember(WPC_WEIGHT_STR)
				|| !wlResults[WPC_TESTS_STR][i].HasMember(WPC_METHODS_STR)) {
				throw(L"Test " + std::to_wstring(i) + L" is missing data for geomean calculation.");
			}

			// Gather all weights and scores for each test
			baselineScore = wlResults[WPC_TESTS_STR][i][WPC_BASELINE_STR].GetDouble();
			testScore = wlResults[WPC_TESTS_STR][i][WPC_RESULT_SCORE_STR].GetDouble();
			testWeight = wlResults[WPC_TESTS_STR][i][WPC_WEIGHT_STR].GetDouble();

			if (baselineScore == 0.0) {
				throw(L"Test " + std::to_wstring(i) + L" has a baseline of zero.");
			}

			if (testScore == 0.0) {
				throw(L"Test " + std::to_wstring(i) + L" returned a score of zero.");
			}

			// Determine scoring method
			std::wstring method = wlResults[WPC_TESTS_STR][i][WPC_METHODS_STR].GetString();
			std::wstring lower = WPC_METHOD_LOWER_STR;
			if (!method.compare(WPC_METHOD_LOWER_STR))
				lowerIsBetter = true;

			// Re-compute score relative to baseline
			// Lower is better for cases of "time to complete"
			if (lowerIsBetter) {
				testScore = baselineScore / testScore;
			}
			// Otherwise assumes throughput or bandwidth or similar where higher is better
			else {
				testScore = testScore / baselineScore;
			}

			// Add the weights and scores found into the vector
			weights_scores.push_back(std::make_pair(testWeight, testScore));
		}
	}
	catch (...) {
		// Need ERROR reported here
		return false;
	}
	
	try {
		// Now that we have a vector full of pairs of weights and scores, 
		// compute the weighted geometric mean
		// In the pair, weight is first, score is second
		for (auto &it : weights_scores) {
			sumWlnX += it.first * (log(it.second));
			sumW += it.first;
		}

		finalScore = exp(sumWlnX / sumW);	
		
		// Save the final score
		return setResultsScore(finalScore);
	}
	catch (...) {
		// Need ERROR reported here
		return false;
	}

	// We should have returned true when setting the results score above, so
	// fail if we reach here
	return false;	
}

// Check if a test has an error field
bool wpcWorkloadResults::testHasError(unsigned int testIndex)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_ERRORS_STR)) {
			return true;
		}
		// if a specific parameters object has a error return true
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_PARAMETERS_PERMS)) {
			for (const auto& param : wlResults[WPC_TESTS_STR][testIndex][WPC_PARAMETERS_PERMS].GetArray()) {
				if (param.HasMember(WPC_ERRORS_STR)) {
					if (param[WPC_ERRORS_STR].GetArray().Size() >= 1) {
						return true;
					}
				}
			}
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

// Return workload name
bool wpcWorkloadResults::getWorkloadName(std::wstring& workloadName)
{
	try {
		workloadName = wlResults[WPC_WORKLOAD_NAME_STR].GetString(); //must exist for valid workload
		return true;
	}
	catch (...) {
		return false;
	}

	return false;
}

// Populate a string with command for generating parameters
// Returns true on success, false otherwise
bool wpcWorkloadResults::getParamCommand(std::wstring& command)
{
	try {
		if (wlResults[WPC_PARAMETERS_GENERATE].HasMember(WPC_PARAMETERS_COMMAND_STR)) {
			command = wlResults[WPC_PARAMETERS_GENERATE][WPC_PARAMETERS_COMMAND_STR].GetString();
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

// Based on the index provided, populate a string with testName
// Returns true on success, false otherwise
bool wpcWorkloadResults::getTestNameByIndex(unsigned int testIndex, std::wstring& testName)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_NAME_STR)) {
			testName = wlResults[WPC_TESTS_STR][testIndex][WPC_NAME_STR].GetString();
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

// Based on the index provided, populate a string with command for that test
// Returns true on success, false otherwise
bool wpcWorkloadResults::getTestCommandByIndex(unsigned int testIndex, std::wstring& command)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_COMMAND_STR)) {
			command = wlResults[WPC_TESTS_STR][testIndex][WPC_COMMAND_STR].GetString();
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

// Based on the index provided, populate a string with the score regex string for that test
// Returns true on success, false otherwise
bool wpcWorkloadResults::getTestScoreRegexByIndex(unsigned int testIndex, std::wstring& scoreRegex)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_SCORE_REGEX)) {
			scoreRegex = wlResults[WPC_TESTS_STR][testIndex][WPC_SCORE_REGEX].GetString();
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

// Based on the index provided, populate a string with the validate regex string for that test
// Returns true on success, false otherwise
bool wpcWorkloadResults::getTestValidateRegexByIndex(unsigned int testIndex, std::wstring& validateRegex)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_VALIDATE_REGEX)) {
			validateRegex = wlResults[WPC_TESTS_STR][testIndex][WPC_VALIDATE_REGEX].GetString();
			return true;
		}
	}
	catch (...) {
		return false;
	}

	return false;
}

//Makes a copy of the test and appends it at the end of the test array
void wpcWorkloadResults::copyTestDom(unsigned int testIndex)
{
	WValue copy(wlResults[WPC_TESTS_STR][testIndex], wlResults.GetAllocator());
	wlResults[WPC_TESTS_STR].PushBack(copy, wlResults.GetAllocator());
	numberOfTests++;
}

void wpcWorkloadResults::deleteTest(unsigned int testIndex)
{
	WValue::ConstValueIterator lItr = wlResults[WPC_TESTS_STR].Begin();
	wlResults[WPC_TESTS_STR].Erase(lItr+testIndex);
	numberOfTests--;
}

void wpcWorkloadResults::updateCommandbyIndex(unsigned int testIndex, std::wstring param, std::wstring val) {
	std::wstring cmd;
	getTestCommandByIndex(testIndex, cmd);

	std::wstring fullParameter = WPC_PARAMETER_START_DELIMITER + param + WPC_PARAMETER_END_DELIMITER;

	//std::wcerr << "Parameter parser replacing [" << param << "] with [" << val << "] in [" << cmd << "]" << std::endl;
	std::wstring::size_type pos = cmd.find(fullParameter);
	while (pos != std::string::npos)
	{
		cmd.replace(pos, fullParameter.length(), val);
		pos = cmd.find(fullParameter);
	}

	wlResults[WPC_TESTS_STR][testIndex][WPC_COMMAND_STR].SetString(cmd.c_str(), wlResults.GetAllocator());
}

void wpcWorkloadResults::appendTestNameByIndex(unsigned int testIndex, std::wstring param, std::wstring description) {
	std::wstring testName;
	getTestNameByIndex(testIndex, testName);

	testName += L" [" + param + L":" + description + L"]";

	wlResults[WPC_TESTS_STR][testIndex][WPC_NAME_STR].SetString(testName.c_str(), wlResults.GetAllocator());
}

// Get the test timeout minutes of the test index provided
unsigned int wpcWorkloadResults::getTestTimeOutMinutes(unsigned int testIndex)
{
	try {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_TIMEOUT_MINUTES)) {
			return wlResults[WPC_TESTS_STR][testIndex][WPC_TIMEOUT_MINUTES].GetDouble();
		}
		else {
			return WPC_MAXIMUM_TEST_TIME_MINS;
		}
	}
	catch (...) {
		// Need ERROR reported  here
		return 0;
	}
}

// returns current test cleanup files from result object
std::vector<std::wstring> wpcWorkloadResults::getResCurrTestCleanupFiles(int testIndex)
{
	std::vector<std::wstring> resTestCleanUpFiles = {};
	try {
		if (wlResults.HasMember(WPC_TESTS_STR)) {
			if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_WORKLOAD_CLEANUPFILES_STR)) {
				for (const auto& fileName : wlResults[WPC_TESTS_STR][testIndex][WPC_WORKLOAD_CLEANUPFILES_STR].GetArray()) {
					resTestCleanUpFiles.push_back(fileName.GetString());
				}
				return resTestCleanUpFiles;
			}
		}
	}
	catch (...) {
		std::cerr << "Error occured in fetching test cleanup files" << std::endl;
		return resTestCleanUpFiles;
	}

	return resTestCleanUpFiles;
}

bool wpcWorkloadResults::isMetaDataAvailable(int testIndex)
{
	try {
		if (wlResults.HasMember(WPC_TESTS_STR)) {
			if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_METADATA)) {
				if (wlResults[WPC_TESTS_STR][testIndex][WPC_METADATA].MemberCount() >= 1) {
					return true;
				}
			}
		}
	}
	catch (...) {
		std::cerr << "Error checking for presence of meta data" << std::endl;
		return false;
	}
	return false;
}

bool wpcWorkloadResults::getTestMetaData(int testIndex, std::vector<std::pair<std::wstring, std::wstring>>& metaData)
{
	try {
		if (wlResults.HasMember(WPC_TESTS_STR)) {
			if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_METADATA)) {
				for (auto& data : wlResults[WPC_TESTS_STR][testIndex][WPC_METADATA].GetObject()) {
					metaData.push_back(std::make_pair(data.name.GetString(), data.value.GetString()));
				}

				return true;
			}
		}
	}
	catch (...) {
		std::cerr << "Error occured in fetching meta data" << std::endl;
		return false;
	}
	return false;
}

bool wpcWorkloadResults::setMetaDataResult(int testIndex, std::wstring label, std::wstring value)
{
	if (wlResults.HasMember(WPC_TESTS_STR)) {
		if (wlResults[WPC_TESTS_STR][testIndex].HasMember(WPC_METADATA)) {
			wlResults[WPC_TESTS_STR][testIndex][WPC_METADATA][label.c_str()].SetString(value.c_str(), wlResults.GetAllocator());
			return true;
		}
	}
	return false;
}