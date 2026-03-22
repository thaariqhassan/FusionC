#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../frontend/lexer/token.h"
#include "../frontend/parser/ast.h"
#include "../languages/language_interface.h"
#include "../middleend/ir/ir.h"
#include "../backend/codegen/machine_codegen.h"

namespace fusionc::core
{

  struct CompilationOptions
  {
    bool dumpTokens = false;
    bool dumpAst = false;
    bool runProgram = true;
    std::string languageHint;
  };

  struct CompilationResult
  {
    bool ok = false;
    std::string inputPath;
    languages::LanguageKind language = languages::LanguageKind::Unknown;
    std::vector<frontend::lexer::Token> tokens;
    std::unique_ptr<frontend::parser::AstNode> ast;
    middleend::ir::Program ir;
    backend::codegen::ExecutionResult execution;
    std::vector<std::string> errors;
  };

  class CompilerController
  {
  public:
    CompilationResult compileToParser(const std::string &inputPath, const CompilationOptions &options) const;

  private:
    static std::string readFile(const std::string &path);
  };

} // namespace fusionc::core
