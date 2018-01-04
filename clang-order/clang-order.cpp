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

        // TODO: remove
        // decl->dump();

        // get parent declaration, i.e. the method's class
        const CXXRecordDecl* parentDecl = decl->getParent();
        if (parentDecl == nullptr)
        {
            std::cout << "Can not get parent from declaration" << std::endl;
            return;
        }

        // TODO: use isClass to verify parent is a class ?
        // https://stackoverflow.com/a/10493643

        std::string parentName = parentDecl->getNameAsString();
        // TODO: remove this once development is complete
        // std::cout << "Parent decl name: " << parentName << std::endl;

        // https://stackoverflow.com/a/20702776
        SourceLocation location = decl->getLocation();
        SourceManager& manager  = result.Context->getSourceManager();

        std::string filename = manager.getFilename(location).str();

        // sample output:
        // filename        : <some path>/extra/clang-order/TestClass_01.h
        // parentName      : TestClass_01
        // parentName + .h : TestClass_01.h
        // std::cout << "filename        : " << filename          << std::endl;
        // std::cout << "parentName      : " << parentName        << std::endl;
        // std::cout << "parentName + .h : " << parentName + ".h" << std::endl;

        // remove path and leave filename only
        // https://stackoverflow.com/a/10364927
        filename = filename.substr(filename.find_last_of("/") + 1,
                                   filename.length());

        // NOTE: manager.isInMainFile(location) is true if location is in
        // the source file being processed; there is no isInHeaderFile(...)
        // that takes the header file matching the source file being
        // processed into account, just _any_ #included header file...

        // TODO: in trivial test class, these three are always the same
        // std::cout << manager.getExpansionLineNumber(location) << std::endl;
        // std::cout << manager.getPresumedLineNumber (location) << std::endl;
        // std::cout << manager.getSpellingLineNumber (location) << std::endl;

        // TODO: this relies on class name == filename.[cpp|h]

        // https://stackoverflow.com/a/46296671
        if (filename == parentName + ".h")
        {
            int lineNumber = manager.getSpellingLineNumber(location);

            // https://stackoverflow.com/a/22291127
            DeclarationNameInfo nameInfo = decl->getNameInfo();
            std::string methodName = nameInfo.getName().getAsString();

            std::cout << "Method declaration in header file:" << std::endl;
            std::cout << "  method name : " << methodName << std::endl;
            std::cout << "  filename    : " << filename   << std::endl;
            std::cout << "  line number : " << lineNumber << std::endl;
            std::cout << std::endl;
        }
        else
        if (filename == parentName + ".cpp")
        {
            int lineNumber = manager.getSpellingLineNumber(location);

            DeclarationNameInfo nameInfo = decl->getNameInfo();
            std::string methodName = nameInfo.getName().getAsString();

            std::cout << "Method definition in source file:" << std::endl;
            std::cout << "  method name : " << methodName << std::endl;
            std::cout << "  filename    : " << filename   << std::endl;
            std::cout << "  line number : " << lineNumber << std::endl;
            std::cout << std::endl;
        }
    }
};


int main(int argc, const char** argv)
{
    CommonOptionsParser OptionsParser(argc, argv, ClangOrderCategory);
    ClangTool           Tool(OptionsParser.getCompilations(),
                             OptionsParser.getSourcePathList());

    MethodPrinter Printer;
    MatchFinder   Finder;
    Finder.addMatcher(MethodMatcher, &Printer);

    // TODO: should this tool support processing multiple files at once ?
    // TODO: how to access Tool or sourcePaths from within run(...) ?

    ArrayRef<std::string> sourcePaths = Tool.getSourcePaths();
    std::cout << "file list size: " << sourcePaths.size() << std::endl;
    std::cout << "first filename: " << sourcePaths[0]     << std::endl;

    return Tool.run(newFrontendActionFactory(&Finder).get());
}
