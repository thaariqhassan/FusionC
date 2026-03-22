#include <iostream>

#include "core/compiler_controller.h"
#include "core/language_loader.h"
#include "frontend/lexer/token.h"
#include "frontend/parser/ast.h"
#include "fusionc_cli/cli_parser.h"

int main(int argc, char *argv[])
{
  using fusionc::cli::CliParser;

  const auto options = CliParser::parse(argc, argv);
  if (options.showHelp)
  {
    std::cout << CliParser::helpText();
    return 0;
  }

  fusionc::core::CompilerController controller;
  fusionc::core::CompilationOptions compilationOptions;
  compilationOptions.dumpTokens = options.dumpTokens;
  compilationOptions.dumpAst = options.dumpAst;
  compilationOptions.languageHint = options.language;

  const auto result = controller.compileToParser(options.inputFile, compilationOptions);

  std::cout << "Input: " << result.inputPath << "\n";
  std::cout << "Detected language: " << fusionc::core::LanguageLoader::toString(result.language) << "\n";

  if (options.dumpTokens)
  {
    std::cout << "\nTokens:\n";
    for (const auto &token : result.tokens)
    {
      std::cout << "  [" << fusionc::frontend::lexer::tokenTypeToString(token.type) << "] '"
                << token.lexeme << "' @ " << token.line << ":" << token.column << "\n";
    }
  }

  if (options.dumpAst && result.ast)
  {
    std::cout << "\nAST:\n";
    std::cout << fusionc::frontend::parser::astToString(*result.ast);
  }

  if (!result.errors.empty())
  {
    std::cerr << "\nErrors:\n";
    for (const auto &error : result.errors)
    {
      std::cerr << "  - " << error << "\n";
    }
    return 1;
  }

  std::cout << "\nCompilation pipeline completed successfully.\n";

  if (result.execution.ok)
  {
    std::cout << "Program exit code: " << result.execution.exitCode << "\n";
  }
  return 0;
}
