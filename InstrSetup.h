#pragma once

#include <vector>
#include <string>

struct InstrSetup
{
    unsigned int operationCount = 1;
    unsigned int statementCount = 1;
    std::string input;
    std::string output;
    std::string clockFunction = "CLK";
    std::string clockFile;
    std::vector<std::string> includePaths;
    std::vector<std::string> preprocessorFlags;
    std::string addInclude;
    std::string addExtern;
    bool includeStd = false;
	bool createClock = false;
};