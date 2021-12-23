// pezcore.cpp : example workload using new SPECworkstation architecture

#include <windows.h>
#include <regex>
#include <filesystem>
#include <sstream>
#include <csignal>

#include "wpcWorkloadDefinition.h"
#include "wpcWorkloadResults.h"
#include "wpcWorkloadReqs.h"
#include "wpcWorkloadParams.h"

#define WORKLOAD_NAME "pezcore"
#define WORKLOAD_NAME_WS L"pezcore"


#ifndef _DEBUG
FILE* streamout;
FILE* streamerr;
#endif

template<typename T> void safe_delete(T*& p) {delete p;	p = NULL; }

std::wstring timeStamp() {
	std::wostringstream stream;
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);
	stream << "[" << std::put_time(&ltm, L"%FT%T") << "] ";
	//[2021-10-04T16:21:51]
	return stream.str();
}

void signalHandler(int signum)
{
	std::wcout << timeStamp() << "Program closed by interrupt signal" << std::endl;

#ifndef _DEBUG
	fflush(streamout);
	fflush(streamerr);
	fflush(stdout);
	fflush(stderr);

	fclose(streamout);
	fclose(streamerr);
	fclose(stdout);
	fclose(stderr);
#endif

	exit(signum);
}

// Execute a command using CreateProcess
bool execute_test(const std::wstring command, std::wstring& output, unsigned int timeOutMinutes)
{

	if (command.empty())
	{
		std::wcout << timeStamp() << "Unable to execute [" << command << "] directly. Command invalid." << std::endl;
		return false;
	}

	HANDLE hChildStdoutRd;
	HANDLE hChildStdoutWr;

	// Create security attributes to create pipe.
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe to get results from child's stdout.
	if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
	{
		return false;
	}

	// Init startup and process info including the
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = {};
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hChildStdoutWr;
	si.hStdError = hChildStdoutWr;
	si.wShowWindow = SW_HIDE;

	// CreateProcess wants a pointer to a wchar_t (and not a const *)
	wchar_t *wCmd = new wchar_t[wcslen(command.c_str()) + 1]();
	if (!wCmd)
		return false;

	wcscpy_s(wCmd, command.length() + 1, command.c_str());
	if (!wcscmp(wCmd, L""))
		return false;

	std::wcout << timeStamp() << "Executing command [" << command << "]" << std::endl;

	// Start the child process
	if (!CreateProcess(NULL, wCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		safe_delete(wCmd);
		std::wcout << timeStamp() << "CreateProcess failed with error: " << GetLastError() << std::endl;
		return false;
	}

	// Job object assignment allows child process to be killed along with parent process
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli;
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	HANDLE hJob = CreateJobObject(NULL, NULL);
	SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
	AssignProcessToJobObject(hJob, pi.hProcess);
	ResumeThread(pi.hThread);

	// Close the write end of the pipe before reading from the read end of the pipe.
	if (!CloseHandle(hChildStdoutWr))
	{
		return false;
	}

	// Wait for the child process to finish
	auto start = std::chrono::steady_clock::now();
	auto end = std::chrono::steady_clock::now();
	DWORD procEnded = WAIT_FAILED;

	DWORD dwRead;
	CHAR chBuf[4096];
	std::string testOutput;
	bool done;
	while (procEnded != WAIT_OBJECT_0)
	{
		// WARNING: This could lead to an infinite loop if the process never signals
		procEnded = WaitForSingleObject(pi.hProcess, 100);

		end = std::chrono::steady_clock::now();
		done = !ReadFile(hChildStdoutRd, chBuf, 4096, &dwRead, NULL) || dwRead == 0;
		if (done)
		{
			continue;
		}
		testOutput += std::string(chBuf, dwRead);

		if (std::chrono::duration_cast<std::chrono::minutes>(end - start).count() > timeOutMinutes) {
			std::wcout << timeStamp() << "Error: command [" << command << "] exceeded timeout of " << timeOutMinutes << " min. Terminating process." << std::endl;
			TerminateProcess(pi.hProcess, 1);
			safe_delete(wCmd);
			//TODO: whole program gets terminated when a single test terminates
			return false;
		}
	}
	output = std::wstring(testOutput.begin(), testOutput.end());

	CloseHandle(hChildStdoutRd);

	std::wcout << timeStamp() << "Execution complete. Cleaning up." << std::endl;

	// Clean up
	safe_delete(wCmd);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hJob);

	return true;
}

void validateJsonFiles(std::wstring schemaPath, const WDocument& inputJsonDoc) {

	// 1. get  json schema to validate

	WDocument wlSchema;
	std::wstring lineSchema, textSchema;
	std::wifstream inSc(schemaPath);

	if (!inSc.is_open()) {
		std::wcerr << "Unable to open filename " << schemaPath << "\n";
		return;
	}

	while (std::getline(inSc, lineSchema))
	{
		textSchema += lineSchema + L"\n";
	}

	if (wlSchema.Parse(textSchema.c_str()).HasParseError()) {
		std::cerr << "Error in parsing workload json schema" << std::endl;
	}

	typedef GenericSchemaDocument<WDocument::ValueType> SchemaDoc;
	typedef GenericSchemaValidator<SchemaDoc> SchemaValid;
	typedef GenericStringBuffer<UTF16<> > StringBufferW;

	// 2. validate workload.json with json schema
	SchemaDoc schema{ wlSchema };
	SchemaValid validator{ schema };

	if (!inputJsonDoc.Accept(validator)) {
		GenericStringBuffer<UTF16<> > gStringbuffer16;
		std::wcerr << timeStamp() << "Failed validation, in following object " << std::endl;
		validator.GetInvalidSchemaPointer().StringifyUriFragment(gStringbuffer16);
		std::wcerr << "Invalid schema-" << gStringbuffer16.GetString() << std::endl;
		std::wcerr << "Invalid Keyword-" << validator.GetInvalidSchemaKeyword() << std::endl;
		gStringbuffer16.Clear();
		validator.GetInvalidDocumentPointer().StringifyUriFragment(gStringbuffer16);
		std::wcerr << "Invalid document-" << gStringbuffer16.GetString() << std::endl;
		gStringbuffer16.Clear();

		StringBufferW stringbuffer16;
		PrettyWriter<StringBufferW, UTF16<>> w16(stringbuffer16);
		validator.GetError().Accept(w16);
		std::wcerr << "Error report " << stringbuffer16.GetString() << std::endl;
		std::wcout << timeStamp() << " Failed validation of " << schemaPath <<" json with schema "<< std::endl;

		// exit porgram
		fflush(stdout);
		fflush(stderr);
		fclose(stdout);
		fclose(stderr);
		exit(EXIT_FAILURE);

		return;

	}

	std::wcout << timeStamp() << L"Successfull validation with " << schemaPath << std::endl;

}


void CleanUp(std::wstring path) {
	std::wstring delCmd = std::filesystem::path(path).has_filename() ? L"del /q " : L"rmdir /s /q ";
	delCmd += path;

	if (_wsystem(delCmd.c_str()) != 0)
	{
		std::wcerr << timeStamp() << L"Error deleting file/folder: " + path << std::endl;
	}
}

void CleanUpFiles(std::vector<std::wstring> cleanupFileList)
{
	for (auto& file : cleanupFileList) {
		CleanUp(file);
	}
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

bool CheckFileHash(std::wstring filename, size_t hash)
{
	std::wifstream infile(filename, std::ifstream::binary);
	if (!infile.is_open()) throw (L"File not found.");

	const int BUFSIZE = 4096;
	std::wstring buffer(BUFSIZE, '\0');
	size_t seed_hash = 438234;

	do {
		infile.read(&buffer[0], BUFSIZE);
		hash_combine(seed_hash, buffer);
	} while (infile.gcount() == BUFSIZE);

	if (seed_hash == hash) {
		return true;
	}
	else {
		std::wcerr << timeStamp() << "HASH MISMATCH: [" << filename << "] = [" << seed_hash << "]" << std::endl;
		return false;
	}
}

// Validate the test output
bool ValidateTestOutput(std::wstring output, std::wregex validationString)
{
	std::wsmatch smatch;
	return std::regex_search(output, smatch, validationString);
}

// Parse the test output to get the score
double ParseTestScore(std::wstring output, std::wregex scoreString)
{
	std::wsmatch smatch;
	if (std::regex_search(output, smatch, scoreString) && smatch.size() > 1)
	{
		return std::stod(smatch.str(1), NULL);
	}
	else
	{
		throw (L"Score parsing failed.");
	}
}

// Parse the test output to get the score
std::wstring ParseMetadata(std::wstring output, std::wregex scoreString)
{
	std::wsmatch smatch;
	if (std::regex_search(output, smatch, scoreString) && smatch.size() > 1)
	{
		return smatch.str();
	}
	else
	{
		throw (L"Meta data parsing failed.");
	}
}

// Recursive directory creation (useful because CreateDirectory function won't create intermediate directories)
void CreateDirectoryDeep(std::wstring &dir)
{
	std::error_code err;
	std::filesystem::create_directories(dir, err);
	if (err)
	{
		std::wcerr << timeStamp() << L"Creating directory failed: " << dir << std::endl;
		throw(L"Creating directory failed: " + dir);
	}
}

// Usage Message
void ShowUsage(std::wstring testExe)
{
	std::wcout << "Usage: " << testExe << " <option(s)>\n"
		<< "Options:\n"
		<< "\t -tests <input.json>\n"
		<< "\t -result <output.json>\n"
		<< "\t -output <output log>\n"
		<< "\t -error <error log>\n"
		<< std::endl;
	std::wcout << "Sample command:\n\t"
		<< testExe << " -tests workload.json -result result.json"
		<< " -output stdout.log -error stderr.log"
		<< std::endl;
	exit(-1);
}

int _tmain(int argc, TCHAR *argv[])
{
	signal(SIGINT, signalHandler);

	if (argc == 2 && ((std::wcscmp(argv[1], L"-help") == 0) || (std::wcscmp(argv[1], L"-h") == 0)))
	{
		ShowUsage(argv[0]);
	}

	// Environmental setup...need to know several paths:
	//		1. Module (executable) location
	//		2. Results directory
	//		3. stdout and stderr file locations

	WCHAR dir[MAX_PATH];
	GetModuleFileName(NULL, dir, MAX_PATH);
	WCHAR driveLetter = dir[0];
	std::wstring workloadDirectory = std::filesystem::path(dir).parent_path();
	workloadDirectory += L"\\"; //add trailing slash

	std::wstring testJson, resultJson, paramJson, errorLog, outputLog, validTestsJson, workloadSchemaJson, querySchemaJson;
	// Default options
	testJson = workloadDirectory + WPC_WORKLOAD_DEFINITION_FILENAME;
	paramJson = workloadDirectory + WPC_PARAMETERS_FILENAME;
	workloadSchemaJson = workloadDirectory + L"pezcore\\schema\\reference_schema_files\\workload_schema.json";
	querySchemaJson = workloadDirectory + L"pezcore\\schema\\reference_schema_files\\query_schema.json";

	int index;
	bool queryflag;
	for (int i = 1; i < argc; i += 2)
	{
		index = (std::wcscmp(argv[i], L"-tests") == 0 ? 1 : (std::wcscmp(argv[i], L"-result") == 0 ? 2 :
				(std::wcscmp(argv[i], L"-output") == 0 ? 3 : (std::wcscmp(argv[i], L"-error") == 0 ? 4 : (std::wcscmp(argv[i], L"--query") == 0 ? 5 : 0)))));
		switch (index)
		{
		case 1:
		{
			if ((argc == i + 1) || (argv[i + 1][0] == '-'))
			{
				std::wcerr << timeStamp() << "Test argument missing!" << std::endl;
				ShowUsage(argv[0]);
			}
			testJson = argv[i + 1];
			break;
		}
		case 2:
		{
			if ((argc == i + 1) || (argv[i + 1][0] == '-'))
			{
				std::wcerr << timeStamp() << "Result argument missing!" << std::endl;
				ShowUsage(argv[0]);
			}
			resultJson = argv[i + 1];
			break;
		}
		case 3:
			if ((argc == i + 1) || (argv[i + 1][0] == '-'))
			{
				std::wcerr << timeStamp() << "Output argument missing!" << std::endl;
				ShowUsage(argv[0]);
			}
			outputLog = argv[i + 1];
			break;
		case 4:
			if ((argc == i + 1) || (argv[i + 1][0] == '-'))
			{
				std::wcerr << timeStamp() << "Error argument missing!" << std::endl;
				ShowUsage(argv[0]);
			}
			errorLog = argv[i + 1];
			break;
		case 5:
			queryflag = true;
			break;
		default:
			ShowUsage(argv[0]);
			break;
		}
	}
	std::wstring workloadFolderPath = std::filesystem::absolute(testJson.c_str()).parent_path();
	paramJson = workloadFolderPath + paramJson.substr(paramJson.find_last_of('\\'), paramJson.length());
	resultJson = workloadFolderPath + L"\\" + L"result.json";
	errorLog = workloadFolderPath + L"\\" + L"stderr.log";
	outputLog = workloadFolderPath + L"\\" + L"stdout.log";
	validTestsJson = workloadFolderPath + L"\\" + L"validTests.json";

	// Get absolute paths for input/output files
	testJson = std::filesystem::absolute(testJson);
	paramJson = workloadFolderPath + paramJson.substr(paramJson.find_last_of('\\'), paramJson.length());
	resultJson = workloadFolderPath + L"\\" + L"result.json";
	errorLog = workloadFolderPath + L"\\" + L"stderr.log";
	outputLog = workloadFolderPath + L"\\" + L"stdout.log";
	validTestsJson = workloadFolderPath + L"\\" + L"validTests.json";

	// The results directory must exist, so do that now
	std::wstring resultDirectory = std::filesystem::path(resultJson).parent_path();
	std::wstring outLogDirectory = std::filesystem::path(outputLog).parent_path();
	std::wstring errLogDirectory = std::filesystem::path(errorLog).parent_path();
	std::wstring validTestsDirectory = std::filesystem::path(validTestsJson).parent_path();

	std::wcout << "Starting workload. See results directory for more information." << std::endl;
#ifndef _DEBUG
	std::wcout << "All further output will be redirected to the log file." << std::endl;
#endif
	std::wcout << "Running..." << std::endl;
	std::wcout << "Attempting to create results directory at " << resultDirectory << std::endl;

	try {
		CreateDirectoryDeep(resultDirectory);
		CreateDirectoryDeep(outLogDirectory);
		CreateDirectoryDeep(errLogDirectory);
		CreateDirectoryDeep(validTestsDirectory);
	}
	catch (...) {
		std::wcerr << timeStamp() << "Directory creation failed - exiting." << std::endl;
		return -1;
	}

#ifndef _DEBUG
	// Redirect stdout and stderr to a file
	FILE* streamout;
	FILE* streamerr;
	if (_wfreopen_s(&streamout, outputLog.c_str(), L"w", stdout) != 0)
	{
		std::wcerr << timeStamp() << "Unable to open output file for writing: " << outputLog << std::endl;
		return -1;
	}
	if (_wfreopen_s(&streamerr, errorLog.c_str(), L"w", stderr) != 0)
	{
		std::wcerr << timeStamp() << "Unable to open error file for writing: " << errorLog << std::endl;
		return -1;
	}
#endif

	// Set working directory to specific workload directory
	SetCurrentDirectory(workloadFolderPath.c_str());

	// Buffer for console output of tested workload
	std::wstring testOutput;

	/********************************
	Pezcore style!
	Maintaining a 'black box' approach to workload execution

	Order of operations:
	1. Initialize the workload definition
	2. Copy the workload definition to become a results file
	2.5. Check requirements and user params, enumerate available tests
	3. Execute each of the tests in the workload.json file, recording the time in the results document
	4. Validate that each test produced expected output
	5. Compute scores
	6. Save the result file
	7. Clean up
	*********************************/

	// 1. Initialize the workload definition
	wpcWorkloadDefinition *pDef = NULL;
	try {
		pDef = new wpcWorkloadDefinition(testJson.c_str());
		validateJsonFiles(workloadSchemaJson, pDef->getDoc());
	}
	catch (const wchar_t* err) {
		std::wcout << timeStamp() << err << std::endl;
		return -1;
	}

	// cleaning up files if any, from last run
	CleanUpFiles(pDef->getWorkloadCleanupFiles());
	for (int index = 0; index < pDef->getNumberOfTests() ; index++)
	{
		CleanUpFiles(pDef->getTestCleanupFiles(index));
	}

	// 2. Copy the workload definition to become a results file
	wpcWorkloadResults *pRes = NULL;
	try {
		pRes = new wpcWorkloadResults(pDef->getDoc());
		pRes->setNumberOfTests(pDef->getNumberOfTests());
	}
	catch (const wchar_t* err) {
		std::wcout << timeStamp() << err << std::endl;
		return -1;
	}

	try {
		bool hashPass = true;
		for (std::pair<std::wstring, size_t> fileHash : pDef->getWorkloadFiles()) {
			if (!CheckFileHash(fileHash.first, fileHash.second)) {
				hashPass = false;
			}
		}
		if (!hashPass) pRes->logWarning(L"Some file(s) did not match the expected hash.");
	}
	catch (...) {
		std::wcerr << timeStamp() << "Unable to verify benchmark files." << std::endl;
		pRes->logWarning(L"Unable to verify benchmark files.");
	}

	std::wstring workloadName;
	pRes->getWorkloadName(workloadName);

	// 2.5 Check requirements and user params, enumerate available tests
	/*
	1. Determine which checks are requested from JSON
	2. Calls checks on per-test basis
	3. If fail, record error in test result
	4. If pass, leave test in run list
	5. If test has user params, add test to run list with each possible output
	6. Output final test list including those that failed
	7. If passed --enumerate parameter, output JSON DOM into availableTests.json and exit
	*/

	std::wcout << timeStamp() << "Workload [" << workloadName << "] : Checking requirements" << std::endl;

	// Create requirement check class -- TODO: convert to singleton
	wpcWorkloadReqs* pReq = new wpcWorkloadReqs();

	//Check if workload needs parameters generated
	if (pDef->generateParams())
	{
		std::wcout << timeStamp() << "Workload [" << workloadName << "] : Generating parameters" << std::endl;

		std::wstring param_cmd;
		if (!pRes->getParamCommand(param_cmd))
		{
			std::wcerr << timeStamp() << "Unable to parse parameter generation command" << std::endl;
			return -1;
		}
		else if (!execute_test(param_cmd.c_str(), testOutput, WPC_MAXIMUM_TEST_TIME_MINS))
		{
			std::wcerr << timeStamp() << "Unable to execute parameter generation command [" << param_cmd << "]" << std::endl;
			return -1;
		}

		wpcWorkloadParams* pPar = NULL;
		try {
			pPar = new wpcWorkloadParams(paramJson.c_str());
			validateJsonFiles(querySchemaJson, pPar->getDoc());
		}
		catch (const wchar_t* err) {
			std::wcout << timeStamp() << err << std::endl;
		}

		if (pPar != NULL) {
			//Check if custom requirements are defined in parameters JSON
			WDocument requirementDOM;
			if (pPar->getCustomRequirements(requirementDOM))
			{
				try {
					pReq->setParamInfo(requirementDOM);
				}
				catch (const wchar_t* err) {
					std::wcout << timeStamp() << err << std::endl;
					return -1;
				}

				for (unsigned int i = 0, offset = 0; i < pDef->getNumberOfTests(); i++)
				{
					std::wstring test_name;
					pRes->getTestNameByIndex(i, test_name);
					//for each test
					//1. loop through requirement list, skip any built-in tests
					for (unsigned int j = 0; j < pDef->getNumberOfRequirements(i); j++)
					{
						std::wstring requirementName;
						pDef->getRequirementName(i, j, requirementName);
						//TODO: skip if requirement matches built-in requirement
						//if (pRes->isBuiltInRequirement(requirementName)) continue;

						bool requirementMet = false;

						switch (pDef->getRequirementType(i, j)) {
						case REQ_TYPE::REQ_BOOL:
							requirementMet = pReq->checkBoolRequirement(requirementName, pDef->getRequirementValueBool(i, j));
							break;
						case REQ_TYPE::REQ_MIN:
							requirementMet = pReq->checkMinRequirement(requirementName, pDef->getRequirementValueNum(i, j));
							break;
						case REQ_TYPE::REQ_MAX:
							requirementMet = pReq->checkMaxRequirement(requirementName, pDef->getRequirementValueNum(i, j));
							break;
						default:
							std::wcerr << timeStamp() << "Test [" << test_name << "], Requirement [" << requirementName << "] : Unknown Type" << std::endl;
							pRes->logTestError(i, std::wstring(L"Unknown requirement type for: ") + requirementName);
						}

						if (requirementMet)
						{
							std::wcout << timeStamp() << "Test [" << test_name << "], Requirement [" << requirementName << "] : PASS" << std::endl;
						}
						else
						{
							std::wcout << timeStamp() << "Test [" << test_name << "], Requirement [" << requirementName << "] : FAIL" << std::endl;
							pRes->logTestError(i, std::wstring(L"Requirement not met: ") + requirementName);
						}

					}

				}
			}

			//Ensure parameter JSON has same number of tests as definition JSON
			//At this step the definition, result, and param DOMs should all have the same number of tests
			//TODO: replace with name matching for tests and parameters so tests without parameters do not need to be listed in parameter JSON
			if (pPar->getNumberOfTests() != pDef->getNumberOfTests())
			{
				std::wcerr << timeStamp() << "Parameter file does not have the same number of tests as workload definition, aborting!" << std::endl;
				return -1;
			}

			for (unsigned int i = 0, offset = 0; i < pPar->getNumberOfTests(); i++)
			{
				unsigned int i_indx = i - offset;
				//check if test has error, if so, log an error and continue to next test
				if (pPar->testHasError(i))
				{
					pRes->logTestError(i_indx, L"Parameters could not be generated for this test.");
					continue;
				}

				for (unsigned int j = 0; j < pPar->getNumberOfPermutations(i); j++)
				{
					pRes->copyTestDom(i_indx);

					for (unsigned int k = 0; k < pPar->getNumberOfParamsInPermutation(i, j); k++)
					{
						pRes->updateCommandbyIndex(pRes->getNumberOfTests() - 1, pPar->getParameterString(i, j, k), pPar->getParameterValue(i, j, k));
					}

					pRes->addParameters(pRes->getNumberOfTests() - 1, pPar->getParameterObject(i, j));
					pRes->swapErrors(pRes->getNumberOfTests() - 1);
				}

				//delete the original parameterized test case
				pRes->deleteTest(i_indx);
				offset++;
			}
		}
	}

	//DEBUG BLOCK
	//printout all cmds found
	/*
	for (unsigned int i = 0; i < pRes->getNumberOfTests(); i++)
	{
		std::wstring test_name, test_cmd;
		pRes->getTestNameByIndex(i, test_name);
		pRes->getTestCommandByIndex(i, test_cmd);

		std::wcerr << "NAME = " << test_name << " | COMMAND = " << test_cmd << std::endl;
	}
	*/

	if (queryflag == true)
	{
		pRes->saveResultsToFile(validTestsJson.c_str());
		safe_delete(pDef);
		safe_delete(pRes);
		fclose(streamout);
		fclose(streamerr);
		fclose(stdout);
		fclose(stderr);
		exit(0);
	}

	std::wcout << timeStamp() << "Workload [" << workloadName << "] : Executing workload" << std::endl;
	// 3. Execute each of the tests in the workload definition
	for (unsigned int i = 0; i < pRes->getNumberOfTests(); i++)
	{
		std::wstring test_name, test_cmd, validate_regex, score_regex;
		if (!pRes->getTestNameByIndex(i, test_name) ||
		    !pRes->getTestCommandByIndex(i, test_cmd) ||
		    !pRes->getTestValidateRegexByIndex(i, validate_regex))
			return -1;

		//check if test has error, if so, pass
		if (pRes->testHasError(i)) {
			std::wcout << timeStamp() << "Skipping test [" << test_name << "], errors detected" << std::endl;
				continue;
		}

		//If score regex defined for this test, use it instead of timer to calculate score
		bool scoreRegex = pRes->getTestScoreRegexByIndex(i, score_regex);

		pRes->startTimer();
		unsigned int timeOutMinutes = pRes->getTestTimeOutMinutes(i);

		// Execute a test command
		std::wcout << timeStamp() << "Starting test [" << test_name << "]" << std::endl;
		if (!execute_test(test_cmd.c_str(), testOutput, timeOutMinutes))
		{
			pRes->logTestError(i, L"Unable to execute test. Results for this test are invalid.");
			std::wcerr << timeStamp() << "Unable to execute test [" << test_name << "]" << std::endl;
		}

		if (!pRes->recordTestTime(i))
		{
			pRes->logTestError(i, L"Unable to record test time.");
			std::wcerr << timeStamp() << "Unable to record test time for test [" << test_name << "]" << std::endl;
		}

		// 4. Validate the output of each test
		std::wregex validateRegex(validate_regex);
		if (!ValidateTestOutput(testOutput, validateRegex))
		{
			std::wcout << timeStamp() << "Validation failed for test [" << test_name << "]" << std::endl;
			pRes->logTestWarning(i, L"Validation failed.");
		}

		// Parse and Save test result score if score regex defined
		if (scoreRegex) {
			try {
				std::wregex scoreRegex(score_regex);
				double resultScore = ParseTestScore(testOutput, scoreRegex);
				pRes->setTestResultsScore(i, resultScore);
			}
			catch (const wchar_t* err)
			{
				std::wcerr << timeStamp() << err << std::endl;
				pRes->logTestError(i, err);
			}
		}

		// Parse Meta data details with given regex
		if (pRes->isMetaDataAvailable(i))
		{
			std::vector<std::pair<std::wstring, std::wstring>> metaData;
			pRes->getTestMetaData(i, metaData);
			for (auto& data : metaData)
			{
				try {
					std::wregex metaRegex(data.second.c_str());
					std::wstring matchData = ParseMetadata(testOutput, metaRegex);
					pRes->setMetaDataResult(i, data.first, matchData);
				}
				catch (...)
				{
					pRes->setMetaDataResult(i, data.first, L"");
					pRes->logTestError(i, L"Metadata["+ data.first + L"] not found");
					std::wcerr << timeStamp() << "Error occured while capturing meta data" << std::endl;
				}
			}
		}

		//dump output to stdout and clear string
		std::wcout << "-------- TEST OUTPUT START --------" << std::endl;
		std::wcout << testOutput;
		std::wcout << "-------- TEST OUTPUT END   --------" << std::endl;
		testOutput.clear();

		// clear current test cleanupfiles
		CleanUpFiles(pRes->getResCurrTestCleanupFiles(i));

	}
	std::wcout << timeStamp() << "Workload [" << workloadName << "] : Execution complete" << std::endl;

	// 5. Compare the time to complete each test with the baseline score and compute the final score using
	//		a blend of the other scores(e.g. weight geometric mean) in the results document
	//TODO: update score calculation to use highest of tests with the same name but different parameters
	std::wcout << timeStamp() << "Workload [" << workloadName << "] : Computing score" << std::endl;

	if (!pRes->computeWeightedGeoMean())
	{
		pRes->logError(L"Unable to compute the weighted geometric mean. Results for this workload are invalid.");
		std::wcerr << timeStamp() << "Unable to compute weighted geometric mean." << std::endl;
	}

	// 6. Save the result file
	std::wcout << timeStamp() << "Workload [" << workloadName << "] : Saving results" << std::endl;
	if (!pRes->saveResultsToFile(resultJson.c_str()))
		std::wcerr << timeStamp() << "Unable to save results to file." << std::endl;

	// 7. Clean up any intermediate files, state information, etc. to prepare for any subsequent run
	CleanUpFiles(pDef->getWorkloadCleanupFiles());

	safe_delete(pDef);
	safe_delete(pRes);

	std::wcout << timeStamp() << "Complete." << std::endl;

#ifndef _DEBUG
	fflush(streamout);
	fflush(streamerr);
	fflush(stdout);
	fflush(stderr);

	fclose(streamout);
	fclose(streamerr);
	fclose(stdout);
	fclose(stderr);
#endif

	return 0;
}
