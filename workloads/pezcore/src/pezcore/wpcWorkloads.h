#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <tchar.h>
#include <exception>
#include <functional>
#include <process.h>
#include <vector>

#include "include\rapidjson\document.h"
#include "include\rapidjson\prettywriter.h"
#include "include\rapidjson\stringbuffer.h"
#include "include\rapidjson\schema.h"
#include "include\rapidjson\filereadstream.h"
using namespace rapidjson;

#define WPC_WORKLOAD_DEFINITION_FILENAME L"workload.json"
#define WPC_PARAMETERS_FILENAME L"query.json"
#define WPC_PARAMETERS_GENERATE L"workloadSetup"
#define WPC_PARAMETERS_COMMAND_STR L"command"
#define WPC_PARAMETERS_FILE_STR L"output"
#define WPC_PARAMETERS_REQS L"requirements"
#define WPC_PARAMETERS_PERMS L"parameters"
#define WPC_PARAMETERS_PNAME L"parameter"
#define WPC_PARAMETERS_VALUE L"value"
#define WPC_PARAMETERS_DESC L"description"
#define WPC_WORKLOAD_NAME_STR L"workloadName"
#define WPC_WORKLOAD_REFRUN_STR L"referenceRunTimeSeconds"
#define WPC_WORKLOAD_FILES_STR L"workloadFiles"
#define WPC_WORKLOAD_FILENAME_STR L"fileName"
#define WPC_WORKLOAD_FILEHASHVALUE_STR L"hashValue"
#define WPC_WORKLOAD_CLEANUPFILES_STR L"cleanupFiles"
#define WPC_ERRORS_STR L"errors"
#define WPC_WARNINGS_STR L"warnings"
#define WPC_NAME_STR L"name"
#define WPC_COMMAND_STR L"command"
#define WPC_VALIDATE_REGEX L"validateRegex"
#define WPC_SCORE_REGEX L"scoreRegex"
#define WPC_BASELINE_STR L"baselineScore"
#define WPC_RESULT_SCORE_STR L"resultScore"
#define WPC_RESULTS_RUNTIME_STR L"resultsRunTimeSeconds"
#define WPC_WEIGHT_STR L"weight"
#define WPC_TESTS_STR L"tests"
#define WPC_METHODS_STR L"scoringMethod"
#define WPC_METHOD_LOWER_STR L"lower"
#define WPC_MAXIMUM_TEST_TIME_MINS 60
#define WPC_TIMEOUT_MINUTES L"timeOutMinutes"
#define WPC_METADATA L"metadata"

#define WPC_PARAMETER_START_DELIMITER L"["
#define WPC_PARAMETER_END_DELIMITER L"]"

//Strings for defining system requirements
#define WPC_REQUIRE L"require"
#define WPC_REQUIRE_OPENCLGPU L"openclgpu"
#define WPC_REQIURE_OPENGLGPU L"openglgpu"
#define WPC_REQUIRE_DISKSPACE L"diskspace"
#define WPC_REQUIRE_VALUE L"value"
#define WPC_REQUIRE_TYPE L"type"
#define WPC_REQUIRE_MAX L"max"
#define WPC_REQUIRE_MIN L"min"
enum class REQ_TYPE {
	REQ_BOOL,
	REQ_MIN,
	REQ_MAX,
	REQ_UNKNOWN
};