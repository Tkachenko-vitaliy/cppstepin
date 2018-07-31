#pragma once

struct InstrSetup;

class Instrumenter
{
public:
    Instrumenter();
    virtual ~Instrumenter();
    
    bool Run(const InstrSetup& instrSetup);
private:
    const char** CreateArgv(InstrSetup& instrSetup, int& argc);
};

