#pragma once

#include <string>
#include <unordered_set>

namespace fusionc::languages
{

  enum class LanguageKind
  {
    C,
    Custom,
    Unknown
  };

  struct LanguageProfile
  {
    LanguageKind kind = LanguageKind::Unknown;
    std::string displayName;
    std::unordered_set<std::string> keywords;
  };

} // namespace fusionc::languages
