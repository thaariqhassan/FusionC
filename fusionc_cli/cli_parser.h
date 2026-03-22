#pragma once

#include <string>
#include <vector>

namespace fusionc::cli
{

  struct CliOptions
  {
    bool showHelp = false;
    bool dumpTokens = false;
    bool dumpAst = false;
    std::string inputFile;
    std::string language;
  };

  class CliParser
  {
  public:
    static CliOptions parse(int argc, char *argv[]);
    static std::string helpText();
  };

} // namespace fusionc::cli
