// based on https://clang.llvm.org/docs/LibASTMatchersTutorial.html

// https://clang.llvm.org/docs/LibASTMatchersReference.html
// https://clang.llvm.org/docs/RAVFrontendAction.html
// https://clang.llvm.org/docs/LibASTMatchersTutorial.html
// https://xaizek.github.io/2014-05-02/detecting-postfix-operators-in-for-loops

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include <iostream>
#include <string>


using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;


// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory ClangOrderCategory("clang-order options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");


DeclarationMatcher MethodMatcher = cxxMethodDecl(unless(isImplicit())).bind("methodDeclaration");


class MethodPrinter : public MatchFinder::MatchCallback
{
  public:
    virtual void run(const MatchFinder::MatchResult& result)
    {
        // TODO: use llvm::errs() ? or llvm::out() or so ?

        const CXXMethodDecl* decl = \
            result.Nodes.getNodeAs<clang::CXXMethodDecl>("methodDeclaration");
        if (decl == nullptr)
        {
            std::cout << "Can not get declaration from node" << std::endl;
            return;
        }

        // get parent declaration, i.e. the method's class
        const CXXRecordDecl* parentDecl = decl->getParent();
        if (parentDecl == nullptr)
        {
            std::cout << "Can not get parent from declaration" << std::endl;
            return;
        }

        // TODO: use isClass to verify parent is a class ?
        // https://stackoverflow.com/a/10493643

        // https://stackoverflow.com/a/22291127
        std::string parentDeclName = parentDecl->getNameAsString();

        std::cout << "Parent declaration name: " << parentDeclName << std::endl;
        return;


        // https://stackoverflow.com/a/20702776
        // SourceLocation location = decl->getLocation();
        // SourceManager& manager  = result.Context->getSourceManager();

        // NOTE: manager.isInMainFile(location) is true if location is in
        // the source file being processed; there is no isInHeaderFile(...)
        // that takes the header file matching the source file being
        // processed into account, just _any_ #included header file...

        // if ()
        // {
        //     std::cout << manager.getFilename(location).str()      << std::endl;
        //     // NOTE: in trivial test class, these three are always the same
        //     std::cout << manager.getExpansionLineNumber(location) << std::endl;
        //     std::cout << manager.getPresumedLineNumber (location) << std::endl;
        //     std::cout << manager.getSpellingLineNumber (location) << std::endl;
        //
        //     decl->dump();
        //
        //     std::cout << std::endl << std::endl;
        // }
    }
};


int main(int argc, const char** argv)
{
    CommonOptionsParser OptionsParser(argc, argv, ClangOrderCategory);
    ClangTool           Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    MethodPrinter Printer;
    MatchFinder   Finder;
    Finder.addMatcher(MethodMatcher, &Printer);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}
