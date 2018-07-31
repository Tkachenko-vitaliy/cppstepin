#include "Instr.h"
#include "InstrSetup.h"
#include "InstrFrontend.h"

#include <clang\Tooling\CommonOptionsParser.h>
#include <clang\Rewrite\Core\Rewriter.h>

#include <algorithm>  
#include <iostream>

using namespace clang;
using namespace tooling;

Instrumenter::Instrumenter()
{
}


Instrumenter::~Instrumenter()
{
}

const char** Instrumenter::CreateArgv(InstrSetup& instrSetup, int& argc)
{
    std::for_each(instrSetup.includePaths.begin(), instrSetup.includePaths.end(), [&instrSetup](std::string& str)
    {
        str.insert(0, "-extra-arg=-I");
    }
    );

    std::for_each(instrSetup.preprocessorFlags.begin(), instrSetup.preprocessorFlags.end(), [&instrSetup](std::string& str)
    {
        str.insert(0, "-extra-arg=-D");
    }
    );

    argc = 2 + instrSetup.includePaths.size() + instrSetup.preprocessorFlags.size();
    const char** argv = new const char*[argc];
    
    unsigned int argvCounter = 0;
    argv[argvCounter] = "";
    argvCounter++;
    argv[argvCounter] = "";

    for (size_t i = 0; i < instrSetup.includePaths.size(); i++)
    {
        argvCounter++;
        argv[argvCounter] = instrSetup.includePaths[i].c_str();
    }

    for (size_t i = 0; i < instrSetup.preprocessorFlags.size(); i++)
    {
        argvCounter++;
        argv[argvCounter] = instrSetup.preprocessorFlags[i].c_str();
    }
   
    return argv;
}

bool Instrumenter::Run(const InstrSetup& instrSetup)
{
    if (instrSetup.createClock)
    {
        ClockStatement clock;
        bool res = clock.Save(instrSetup.clockFile.c_str());
        if (!res)
        {
            std::cout << "Error create the clock file" << std::endl;
        }
        return res;
    }

    llvm::cl::OptionCategory MyToolCategory("instrumenter options");

    InstrSetup setup = instrSetup;
    int argc; const char** argv = CreateArgv(setup,argc);
    std::unique_ptr<const char*> keeper(argv);

    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

    std::vector<std::string> sources;
    sources.push_back(setup.input);
    ClangTool Tool(OptionsParser.getCompilations(), sources);

    auto ptr = instrNewFrontendActionFactory(&instrSetup);

    if (ptr.get() == nullptr)
    {
        return false;
    }

    int result = Tool.run(ptr.get());

    clang::Rewriter& rewriter = ptr.get()->GetRewriter();

    if (result == 0)
    {
        std::error_code ec;
        const std::string* fileName = &instrSetup.input;
        if (!instrSetup.output.empty())
        {
            fileName = &instrSetup.output;
            llvm::raw_fd_ostream file(fileName->c_str(), ec, llvm::sys::fs::F_Text);
            rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(file);
        }
        else
        {
            rewriter.overwriteChangedFiles();
        }
    }
    else
    {
        std::cout << "Compiler error was detected" << std::endl;
    }

    return result ? false : true;
}

