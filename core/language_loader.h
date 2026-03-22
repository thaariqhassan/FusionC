#pragma once

#include <string>

#include "../languages/language_interface.h"

namespace fusionc::core
{

  class LanguageLoader
  {
  public:
    static languages::LanguageKind detectLanguage(const std::string &filePath, const std::string &source);
    static languages::LanguageProfile loadProfile(languages::LanguageKind kind);
    static std::string toString(languages::LanguageKind kind);
  };

} // namespace fusionc::core
