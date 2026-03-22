#include "cli_parser.h"

namespace fusionc::cli
{

  CliOptions CliParser::parse(int argc, char *argv[])
  {
    CliOptions options;

    for (int i = 1; i < argc; ++i)
    {
      const std::string arg = argv[i];

      if (arg == "-h" || arg == "--help")
      {
        options.showHelp = true;
      }
      else if (arg == "--dump-tokens")
      {
        options.dumpTokens = true;
      }
      else if (arg == "--dump-ast")
      {
        options.dumpAst = true;
      }
      else if (!arg.empty() && arg[0] == '-')
      {
        options.showHelp = true;
      }
      else if (options.inputFile.empty())
      {
        options.inputFile = arg;
      }
      else if (options.language.empty())
      {
        options.language = arg;
      }
      else
      {
        options.showHelp = true;
      }
    }

    if (options.inputFile.empty())
    {
      options.showHelp = true;
    }

    return options;
  }

  std::string CliParser::helpText()
  {
    return "FusionC - Intelligent Multi-Language Compiler\n"
           "Usage:\n"
           "  fusionc <source-file> <language> [--dump-tokens] [--dump-ast]\n\n"
           "Options:\n"
           "  -h, --help       Show this help\n"
           "  --dump-tokens    Print lexer tokens\n"
           "  --dump-ast       Print parser AST\n";
  }

} // namespace fusionc::cli
