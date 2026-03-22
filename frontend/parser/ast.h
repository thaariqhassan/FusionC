#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../lexer/token.h"

namespace fusionc::frontend::parser
{

  enum class AstNodeKind
  {
    Program,
    Function,
    Block,
    Statement,
    Declaration,
    Assignment,
    BinaryExpression,
    Literal,
    Identifier,
    Return,
    ExpressionStatement,
    Unknown
  };

  struct AstNode
  {
    AstNodeKind kind = AstNodeKind::Unknown;
    std::string value;
    std::vector<std::unique_ptr<AstNode>> children;
  };

  class Parser
  {
  public:
    explicit Parser(std::vector<lexer::Token> tokens);

    std::unique_ptr<AstNode> parseProgram();
    const std::vector<std::string> &errors() const;

  private:
    std::unique_ptr<AstNode> parseFunction();
    std::unique_ptr<AstNode> parseBlock();
    std::unique_ptr<AstNode> parseStatement();
    std::unique_ptr<AstNode> parseDeclarationOrAssignment();
    std::unique_ptr<AstNode> parseExpression();
    std::unique_ptr<AstNode> parseTerm();
    std::unique_ptr<AstNode> parseFactor();

    bool match(lexer::TokenType type, const std::string &lexeme = "");
    bool check(lexer::TokenType type, const std::string &lexeme = "") const;
    const lexer::Token &advance();
    const lexer::Token &peek() const;
    const lexer::Token &previous() const;
    bool isAtEnd() const;
    void syncToStatementEnd();
    void addError(const std::string &message);

    std::vector<lexer::Token> tokens_;
    std::size_t current_ = 0;
    std::vector<std::string> errors_;
  };

  std::string astToString(const AstNode &node, int indent = 0);

} // namespace fusionc::frontend::parser
