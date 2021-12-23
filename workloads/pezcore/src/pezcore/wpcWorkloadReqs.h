/************************************

SPEC GWPG/WPC Requirement Check
Class that checks if the system meets requirements
for a workload to run

*************************************/

#pragma once

#include "wpcWorkloads.h"
typedef GenericDocument<UTF16<> > WDocument;
typedef GenericValue<UTF16<> > WValue;
typedef GenericStringBuffer<UTF16<> > WStringBuffer;

class wpcWorkloadReqs
{
public:
	wpcWorkloadReqs() {};//(const WDocument& definitionFile); //TODO: expect JSON of system info dump
	~wpcWorkloadReqs();

	bool setParamInfo(const WDocument& requirementDom);

	//functions to check requirements from workload parameters
	bool checkBoolRequirement(std::wstring requirementName, bool val);
	bool checkMinRequirement(std::wstring requirementName, float val);
	bool checkMaxRequirement(std::wstring requirementName, float val);

private:
	WDocument sysInfo; //JSON with system dump info
	WDocument paramInfo; //JSON with workload parameter requirement info
};

