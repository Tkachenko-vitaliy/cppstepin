#pragma once

#include <clang\Frontend\FrontendAction.h>
#include <clang\Tooling\Tooling.h>
#include <clang\Rewrite\Core\Rewriter.h>

#include "ClockStatement.h"

class InstrAST;
struct InstrSetup;

class InstrASTConsumer : public clang::ASTConsumer
{

public:
    explicit InstrASTConsumer(clang::CompilerInstance *CI, clang::Rewriter& rewriter, ClockStatement& clock);
    void HandleTranslationUnit(clang::ASTContext &Context) override;
    InstrAST* GetVisitor();

private:
    InstrAST *visitor;
};

class InstrFrontendAction : public clang::ASTFrontendAction
{
public:
    InstrFrontendAction(const InstrSetup* instrSetup, clang::Rewriter& rewriter, ClockStatement& clock);
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, StringRef file) override;
private:
    const InstrSetup* instrSetup;
    clang::Rewriter& rewriter;
    ClockStatement& clock;

};

class InstrFrontendActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    InstrFrontendActionFactory(const InstrSetup* instrSetup);

    clang::FrontendAction *create() override;
    clang::Rewriter& GetRewriter();
private:
    const InstrSetup* instrSetup;
    clang::Rewriter rewriter;
    ClockStatement clock;
};

//We use custom FrontendActionFactory instead of newFrontendActionFactory declared in tooling.h, because we have to pass setup parameters to the instrumenter AST
std::unique_ptr <InstrFrontendActionFactory> instrNewFrontendActionFactory(const InstrSetup* instrSetup);
