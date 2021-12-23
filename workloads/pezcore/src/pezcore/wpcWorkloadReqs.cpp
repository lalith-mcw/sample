#include "wpcWorkloadReqs.h"

bool wpcWorkloadReqs::setParamInfo(const WDocument& requirementDom)
{
	try {
		paramInfo.CopyFrom(requirementDom, paramInfo.GetAllocator());

#ifdef DEBUGOUTPUT
		WStringBuffer buffer;
		buffer.Clear();
		Writer<WStringBuffer, UTF16<>, UTF16<>> writer(buffer);
		paramInfo.Accept(writer);
		std::wcerr << "paramInfo : " << std::wstring(buffer.GetString()) << std::endl;
#endif
	}
	catch (...)
	{
		throw L"Unable to read requirements from parameter file.";
	}

	return true;
}

bool wpcWorkloadReqs::checkBoolRequirement(std::wstring requirementName, bool val)
{
	auto requirement = paramInfo.FindMember(requirementName.c_str());
	if (requirement == paramInfo.MemberEnd())
		return false;

	std::wcerr << "Checking requirement [" << requirementName << " = " << val << "]" << std::endl;
	return requirement->value.GetBool() == val;
}

bool wpcWorkloadReqs::checkMinRequirement(std::wstring requirementName, float val)
{
	auto requirement = paramInfo.FindMember(requirementName.c_str());
	if (requirement == paramInfo.MemberEnd())
		return false;

	std::wcerr << "Checking requirement [" << requirementName << " >= " << val << "]" << std::endl;
    return requirement->value.GetFloat() >= val;
}

bool wpcWorkloadReqs::checkMaxRequirement(std::wstring requirementName, float val)
{
	auto requirement = paramInfo.FindMember(requirementName.c_str());
	if (requirement == paramInfo.MemberEnd())
		return false;

	std::wcerr << "Checking requirement [" << requirementName << " <= " << val << "]" << std::endl;
	return requirement->value.GetFloat() <= val;
}