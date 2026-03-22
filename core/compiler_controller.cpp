#include "compiler_controller.h"

#include <exception>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

#include "language_loader.h"
#include "../frontend/semantic/symbol_table.h"
#include "../middleend/ir/ir.h"
#include "../middleend/optimizer/optimizer.h"
#include "../backend/codegen/machine_codegen.h"

namespace fusionc::core
{

  CompilationResult CompilerController::compileToParser(const std::string &inputPath, const CompilationOptions &options) const
  {
    CompilationResult result;
    result.inputPath = inputPath;

    std::string source;
    try
    {
      source = readFile(inputPath);
    }
    catch (const std::exception &ex)
    {
      result.errors.push_back(ex.what());
      return result;
    }

    auto language = LanguageLoader::detectLanguage(inputPath, source);
    if (!options.languageHint.empty())
    {
      std::string hintLower = options.languageHint;
      for (auto &ch : hintLower)
      {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
      }
      if (hintLower == "c")
      {
        language = languages::LanguageKind::C;
      }
      else if (hintLower == "custom")
      {
        language = languages::LanguageKind::Custom;
      }
    }

    result.language = language;
    if (language == languages::LanguageKind::Unknown)
    {
      result.errors.push_back("Unable to determine source language. Use .c for C or .fsc/.fcl for custom language.");
      return result;
    }

    const auto profile = LanguageLoader::loadProfile(language);
    frontend::lexer::Lexer lexer(profile);
    result.tokens = lexer.tokenize(source);

    frontend::parser::Parser parser(result.tokens);
    result.ast = parser.parseProgram();
    result.errors = parser.errors();

    for (const auto &token : result.tokens)
    {
      if (token.type == frontend::lexer::TokenType::Unknown)
      {
        result.errors.push_back("Unknown token '" + token.lexeme + "' at " + std::to_string(token.line) + ":" + std::to_string(token.column));
      }
    }

    if (!result.errors.empty())
    {
      return result;
    }

    // Semantic analysis
    frontend::semantic::SemanticAnalyzer semantic;
    const auto semanticErrors = semantic.analyze(*result.ast);
    result.errors.insert(result.errors.end(), semanticErrors.begin(), semanticErrors.end());
    if (!result.errors.empty())
    {
      return result;
    }

    // Build and optimize IR
    try
    {
      result.ir = middleend::ir::buildProgram(*result.ast);
      result.ir = middleend::optimizer::foldConstants(result.ir);
    }
    catch (const std::exception &ex)
    {
      result.errors.push_back(ex.what());
      return result;
    }

    // Execute
    if (options.runProgram)
    {
      result.execution = backend::codegen::execute(result.ir);
      if (!result.execution.ok)
      {
        result.errors.push_back("Execution failed: " + result.execution.message);
      }
    }

    result.ok = result.errors.empty();
    return result;
  }

  std::string CompilerController::readFile(const std::string &path)
  {
    std::ifstream input(path);
    if (!input.is_open())
    {
      throw std::runtime_error("Failed to open input file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
  }

} // namespace fusionc::core
