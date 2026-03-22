#pragma once

#include <string>
#include <vector>

#include "../../languages/language_interface.h"

namespace fusionc::frontend::lexer
{

  enum class TokenType
  {
    Keyword,
    Identifier,
    Number,
    StringLiteral,
    Operator,
    Punctuation,
    EndOfFile,
    Unknown
  };

  struct Token
  {
    TokenType type = TokenType::Unknown;
    std::string lexeme;
    int line = 1;
    int column = 1;
  };

  class Lexer
  {
  public:
    explicit Lexer(languages::LanguageProfile profile);
    std::vector<Token> tokenize(const std::string &source) const;

  private:
    languages::LanguageProfile profile_;
  };

  std::string tokenTypeToString(TokenType type);

} // namespace fusionc::frontend::lexer
