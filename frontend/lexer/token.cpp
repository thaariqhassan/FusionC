#include "token.h"

#include <cctype>
#include <utility>

namespace fusionc::frontend::lexer
{

  namespace
  {

    bool isIdentifierStart(char c)
    {
      return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    }

    bool isIdentifierPart(char c)
    {
      return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    bool isOperatorChar(char c)
    {
      switch (c)
      {
      case '+':
      case '-':
      case '*':
      case '/':
      case '%':
      case '=':
      case '!':
      case '<':
      case '>':
      case '&':
      case '|':
        return true;
      default:
        return false;
      }
    }

    bool isPunctuationChar(char c)
    {
      switch (c)
      {
      case '(':
      case ')':
      case '{':
      case '}':
      case '[':
      case ']':
      case ';':
      case ',':
      case ':':
        return true;
      default:
        return false;
      }
    }

  } // namespace

  Lexer::Lexer(languages::LanguageProfile profile) : profile_(std::move(profile)) {}

  std::vector<Token> Lexer::tokenize(const std::string &source) const
  {
    std::vector<Token> tokens;

    std::size_t index = 0;
    int line = 1;
    int col = 1;

    auto pushToken = [&](TokenType type, std::string lexeme, int tokenLine, int tokenCol)
    {
      tokens.push_back(Token{type, std::move(lexeme), tokenLine, tokenCol});
    };

    while (index < source.size())
    {
      const char c = source[index];

      if (c == ' ' || c == '\t' || c == '\r')
      {
        ++index;
        ++col;
        continue;
      }

      if (c == '\n')
      {
        ++index;
        ++line;
        col = 1;
        continue;
      }

      if (c == '/' && index + 1 < source.size() && source[index + 1] == '/')
      {
        while (index < source.size() && source[index] != '\n')
        {
          ++index;
          ++col;
        }
        continue;
      }

      const int tokenLine = line;
      const int tokenCol = col;

      if (isIdentifierStart(c))
      {
        std::string lexeme;
        while (index < source.size() && isIdentifierPart(source[index]))
        {
          lexeme.push_back(source[index]);
          ++index;
          ++col;
        }

        const TokenType type = profile_.keywords.find(lexeme) != profile_.keywords.end()
                                   ? TokenType::Keyword
                                   : TokenType::Identifier;
        pushToken(type, std::move(lexeme), tokenLine, tokenCol);
        continue;
      }

      if (std::isdigit(static_cast<unsigned char>(c)))
      {
        std::string lexeme;
        bool seenDot = false;
        while (index < source.size())
        {
          const char d = source[index];
          if (std::isdigit(static_cast<unsigned char>(d)))
          {
            lexeme.push_back(d);
            ++index;
            ++col;
          }
          else if (d == '.' && !seenDot)
          {
            seenDot = true;
            lexeme.push_back(d);
            ++index;
            ++col;
          }
          else
          {
            break;
          }
        }
        pushToken(TokenType::Number, std::move(lexeme), tokenLine, tokenCol);
        continue;
      }

      if (c == '"')
      {
        std::string lexeme;
        lexeme.push_back(c);
        ++index;
        ++col;

        while (index < source.size() && source[index] != '"')
        {
          if (source[index] == '\n')
          {
            ++line;
            col = 1;
          }
          lexeme.push_back(source[index]);
          ++index;
          ++col;
        }

        if (index < source.size() && source[index] == '"')
        {
          lexeme.push_back('"');
          ++index;
          ++col;
          pushToken(TokenType::StringLiteral, std::move(lexeme), tokenLine, tokenCol);
        }
        else
        {
          pushToken(TokenType::Unknown, std::move(lexeme), tokenLine, tokenCol);
        }
        continue;
      }

      if (isOperatorChar(c))
      {
        std::string lexeme;
        lexeme.push_back(c);

        if (index + 1 < source.size())
        {
          const char n = source[index + 1];
          const bool twoChar =
              (c == '=' && n == '=') || (c == '!' && n == '=') || (c == '<' && n == '=') || (c == '>' && n == '=') ||
              (c == '&' && n == '&') || (c == '|' && n == '|');

          if (twoChar)
          {
            lexeme.push_back(n);
            ++index;
            ++col;
          }
        }

        ++index;
        ++col;
        pushToken(TokenType::Operator, std::move(lexeme), tokenLine, tokenCol);
        continue;
      }

      if (isPunctuationChar(c))
      {
        std::string lexeme(1, c);
        ++index;
        ++col;
        pushToken(TokenType::Punctuation, std::move(lexeme), tokenLine, tokenCol);
        continue;
      }

      pushToken(TokenType::Unknown, std::string(1, c), tokenLine, tokenCol);
      ++index;
      ++col;
    }

    pushToken(TokenType::EndOfFile, "<eof>", line, col);
    return tokens;
  }

  std::string tokenTypeToString(TokenType type)
  {
    switch (type)
    {
    case TokenType::Keyword:
      return "Keyword";
    case TokenType::Identifier:
      return "Identifier";
    case TokenType::Number:
      return "Number";
    case TokenType::StringLiteral:
      return "StringLiteral";
    case TokenType::Operator:
      return "Operator";
    case TokenType::Punctuation:
      return "Punctuation";
    case TokenType::EndOfFile:
      return "EndOfFile";
    default:
      return "Unknown";
    }
  }

} // namespace fusionc::frontend::lexer
