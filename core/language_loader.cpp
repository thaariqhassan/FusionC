#include "language_loader.h"

#include <algorithm>
#include <cctype>

namespace fusionc::core
{

  namespace
  {

    bool contains(const std::string &text, const std::string &needle)
    {
      return text.find(needle) != std::string::npos;
    }

    std::string extensionOf(const std::string &filePath)
    {
      const auto pos = filePath.find_last_of('.');
      if (pos == std::string::npos || pos + 1 >= filePath.size())
      {
        return "";
      }

      std::string ext = filePath.substr(pos + 1);
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c)
                     { return static_cast<char>(std::tolower(c)); });
      return ext;
    }

  } // namespace

  languages::LanguageKind LanguageLoader::detectLanguage(const std::string &filePath, const std::string &source)
  {
    const std::string ext = extensionOf(filePath);
    if (ext == "c" || ext == "h")
    {
      return languages::LanguageKind::C;
    }
    if (ext == "fsc" || ext == "fcl" || ext == "fusion")
    {
      return languages::LanguageKind::Custom;
    }

    if (contains(source, "#include") || contains(source, "int main("))
    {
      return languages::LanguageKind::C;
    }
    if (contains(source, "fn ") || contains(source, "let "))
    {
      return languages::LanguageKind::Custom;
    }

    return languages::LanguageKind::Unknown;
  }

  languages::LanguageProfile LanguageLoader::loadProfile(languages::LanguageKind kind)
  {
    using languages::LanguageKind;
    using languages::LanguageProfile;

    if (kind == LanguageKind::C)
    {
      return LanguageProfile{
          LanguageKind::C,
          "C",
          {"int", "float", "char", "void", "if", "else", "while", "for", "return", "struct", "typedef", "printf"}};
    }

    if (kind == LanguageKind::Custom)
    {
      return LanguageProfile{
          LanguageKind::Custom,
          "CustomLang",
          {"fn", "let", "mut", "if", "else", "while", "return", "print", "true", "false"}};
    }

    return LanguageProfile{LanguageKind::Unknown, "Unknown", {}};
  }

  std::string LanguageLoader::toString(languages::LanguageKind kind)
  {
    switch (kind)
    {
    case languages::LanguageKind::C:
      return "C";
    case languages::LanguageKind::Custom:
      return "CustomLang";
    default:
      return "Unknown";
    }
  }

} // namespace fusionc::core
