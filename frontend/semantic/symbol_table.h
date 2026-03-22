// Simple scoped symbol table and semantic analyzer.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../parser/ast.h"

namespace fusionc::frontend::semantic
{

  class SymbolTable
  {
  public:
    void pushScope();
    void popScope();
    bool declare(const std::string &name, const std::string &type);
    bool exists(const std::string &name) const;
    std::string typeOf(const std::string &name) const;

  private:
    std::vector<std::unordered_map<std::string, std::string>> scopes_{{}};
  };

  class SemanticAnalyzer
  {
  public:
    std::vector<std::string> analyze(const parser::AstNode &root);

  private:
    void analyzeFunction(const parser::AstNode &fn);
    void analyzeBlock(const parser::AstNode &block);
    void analyzeStatement(const parser::AstNode &stmt);
    std::string analyzeExpression(const parser::AstNode &expr);

    std::vector<std::string> errors_;
    SymbolTable symbols_;
    std::string currentReturnType_;
  };

} // namespace fusionc::frontend::semantic
