#include "CmdLineParser.h"
#include "InstrSetup.h"
#include "Instr.h"

#include <iostream>

int main(int argc, char* argv[])
{
    InstrSetup setup;
    CmdLineParser parser({ "/", "-" });
    parser.BindParam("Input", setup.input, CmdLineParser::CN_MANDATORY | CmdLineParser::CN_NO_DUPLICATE);
	parser.BindParam("Output", setup.output, CmdLineParser::CN_NO_DUPLICATE);
    parser.BindParam("I", CmdLineParser::callback_string_t(
        [&setup](const char* paramName, const char* paramValue) {setup.includePaths.push_back(paramValue); }
    ));
    parser.BindParam("D", CmdLineParser::callback_string_t(
        [&setup](const char* paramName, const char* paramValue) {setup.preprocessorFlags.push_back(paramValue); }
    ));
    parser.BindParam("Function", setup.clockFunction, CmdLineParser::CN_NO_DUPLICATE);
    parser.BindParam("Clock", setup.clockFile, CmdLineParser::CN_NO_DUPLICATE);
    parser.BindParamIsSet("Create", setup.createClock);
    parser.BindParam("include", setup.addInclude, CmdLineParser::CN_NO_DUPLICATE);
    parser.BindParam("extern", setup.addExtern, CmdLineParser::CN_NO_DUPLICATE);
    parser.BindParamIsSet("includeStd", setup.includeStd);
	parser.BindParam("step", setup.operationCount, CmdLineParser::CN_NO_DUPLICATE);
	parser.BindParam("statement", setup.statementCount, CmdLineParser::CN_NO_DUPLICATE);

    try
    {
        parser.Parse(argc, argv, 1);
    }
    catch (CmdLineParser::CmdLineParseException& e)
    {
        std::cout << e.what() << std::endl;
        return e.GetErrorCode();
    }

    Instrumenter instr;
    bool res = instr.Run(setup);

    return res ? 1 : 0;
}
