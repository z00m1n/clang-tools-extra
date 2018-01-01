// based on https://clang.llvm.org/docs/LibASTMatchersTutorial.html

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"


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


class MethodPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
    if (const CXXMethodDecl *MD = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("methodDeclaration"))
      MD->dump();
  }
};


int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, ClangOrderCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  MethodPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(MethodMatcher, &Printer);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
