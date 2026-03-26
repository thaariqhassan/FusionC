#include "ast.h"

#include <sstream>
#include <utility>

namespace fusionc::frontend::parser
{

  namespace
  {

    std::string kindToString(AstNodeKind kind)
    {
      switch (kind)
      {
      case AstNodeKind::Program:
        return "Program";
      case AstNodeKind::Function:
        return "Function";
      case AstNodeKind::Block:
        return "Block";
      case AstNodeKind::Statement:
        return "Statement";
      case AstNodeKind::Declaration:
        return "Declaration";
      case AstNodeKind::Assignment:
        return "Assignment";
      case AstNodeKind::BinaryExpression:
        return "BinaryExpression";
      case AstNodeKind::Literal:
        return "Literal";
      case AstNodeKind::Identifier:
        return "Identifier";
      case AstNodeKind::Return:
        return "Return";
      case AstNodeKind::ExpressionStatement:
        return "ExpressionStatement";
      case AstNodeKind::WhileStatement:
        return "WhileStatement";
      case AstNodeKind::ForStatement:
        return "ForStatement";
      case AstNodeKind::Printf:
        return "Printf";
      case AstNodeKind::Scanf:
        return "Scanf";
      case AstNodeKind::Print:
        return "Print";
      default:
        return "Unknown";
      }
    }

    bool isTypeKeyword(const lexer::Token &token)
    {
      if (token.type != lexer::TokenType::Keyword)
      {
        return false;
      }

      return token.lexeme == "int" || token.lexeme == "float" || token.lexeme == "char" || token.lexeme == "void" ||
             token.lexeme == "let";
    }

  } // namespace

  Parser::Parser(std::vector<lexer::Token> tokens) : tokens_(std::move(tokens)) {}

  std::unique_ptr<AstNode> Parser::parseProgram()
  {
    auto program = std::make_unique<AstNode>();
    program->kind = AstNodeKind::Program;
    program->value = "root";

    while (!isAtEnd())
    {
      if (check(lexer::TokenType::EndOfFile))
      {
        break;
      }

      auto func = parseFunction();
      if (func)
      {
        program->children.push_back(std::move(func));
      }
      else
      {
        syncToStatementEnd();
      }
    }

    return program;
  }

  const std::vector<std::string> &Parser::errors() const
  {
    return errors_;
  }

  std::unique_ptr<AstNode> Parser::parseFunction()
  {
    std::string returnType;
    std::string functionName;

    // CustomLang style: fn main() { ... }
    if (match(lexer::TokenType::Keyword, "fn"))
    {
      returnType = "int"; // default return type for now

      if (!match(lexer::TokenType::Identifier))
      {
        addError("Expected function name after 'fn'.");
        return nullptr;
      }

      functionName = previous().lexeme;
    }
    // C style: int main() { ... }
    else if (check(lexer::TokenType::Keyword))
    {
      const auto typeToken = advance();
      returnType = typeToken.lexeme;

      if (!match(lexer::TokenType::Identifier))
      {
        addError("Expected function name after return type.");
        return nullptr;
      }

      functionName = previous().lexeme;
    }
    else
    {
      addError("Expected function declaration.");
      return nullptr;
    }

    auto func = std::make_unique<AstNode>();
    func->kind = AstNodeKind::Function;
    func->value = functionName + ":" + returnType;

    if (!match(lexer::TokenType::Punctuation, "("))
    {
      addError("Expected '(' after function name.");
      return nullptr;
    }
    if (!match(lexer::TokenType::Punctuation, ")"))
    {
      addError("Expected ')' after parameter list.");
      return nullptr;
    }

    if (!match(lexer::TokenType::Punctuation, "{"))
    {
      addError("Expected '{' to start function body.");
      return nullptr;
    }

    auto body = parseBlock();
    if (!body)
    {
      addError("Invalid function body.");
      return nullptr;
    }

    func->children.push_back(std::move(body));
    return func;
  }

  std::unique_ptr<AstNode> Parser::parseBlock()
  {
    auto block = std::make_unique<AstNode>();
    block->kind = AstNodeKind::Block;
    block->value = "block";

    while (!isAtEnd() && !check(lexer::TokenType::Punctuation, "}"))
    {
      auto stmt = parseStatement();
      if (stmt)
      {
        block->children.push_back(std::move(stmt));
      }
      else
      {
        syncToStatementEnd();
      }
    }

    if (!match(lexer::TokenType::Punctuation, "}"))
    {
      addError("Expected '}' to close block.");
      return nullptr;
    }

    return block;
  }
  

  std::unique_ptr<AstNode> Parser::parseStatement()
  {
    if (match(lexer::TokenType::Punctuation, "{"))
    {
      return parseBlock();
    }
    if (match(lexer::TokenType::Keyword, "printf"))
    {
    return parsePrintf();
    }
    if (match(lexer::TokenType::Keyword, "scanf"))
    {
    return parseScanf();
    }
    if (match(lexer::TokenType::Keyword, "print"))
    {
    return parsePrint();
    }

    if (match(lexer::TokenType::Keyword, "while"))
    {
      auto whileNode = std::make_unique<AstNode>();
      whileNode->kind = AstNodeKind::WhileStatement;
      whileNode->value = "while";

      if (!match(lexer::TokenType::Punctuation, "("))
      {
        addError("Expected '(' after while.");
        return nullptr;
      }

      auto condition = parseExpression();
      if (!condition)
      {
        addError("Expected condition in while statement.");
        return nullptr;
      }

      if (!match(lexer::TokenType::Punctuation, ")"))
      {
        addError("Expected ')' after while condition.");
        return nullptr;
      }

      if (!match(lexer::TokenType::Punctuation, "{"))
      {
        addError("Expected '{' to start while body.");
        return nullptr;
      }

      auto body = parseBlock();
      if (!body)
      {
        addError("Invalid while body.");
        return nullptr;
      }

      whileNode->children.push_back(std::move(condition));
      whileNode->children.push_back(std::move(body));
      return whileNode;
    }

    if (match(lexer::TokenType::Keyword, "for"))
    {
      auto forNode = std::make_unique<AstNode>();
      forNode->kind = AstNodeKind::ForStatement;
      forNode->value = "for";

      if (!match(lexer::TokenType::Punctuation, "("))
      {
        addError("Expected '(' after for.");
        return nullptr;
      }

      std::unique_ptr<AstNode> initializer;
      if (!check(lexer::TokenType::Punctuation, ";"))
      {
        initializer = parseDeclarationOrAssignment();
        if (!initializer)
        {
          addError("Invalid initializer in for statement.");
          return nullptr;
        }
      }

      if (!match(lexer::TokenType::Punctuation, ";"))
      {
        addError("Expected ';' after for initializer.");
        return nullptr;
      }

      std::unique_ptr<AstNode> condition;
      if (!check(lexer::TokenType::Punctuation, ";"))
      {
        condition = parseExpression();
        if (!condition)
        {
          addError("Invalid condition in for statement.");
          return nullptr;
        }
      }
      else
      {
        auto literal = std::make_unique<AstNode>();
        literal->kind = AstNodeKind::Literal;
        literal->value = "1";
        condition = std::move(literal);
      }

      if (!match(lexer::TokenType::Punctuation, ";"))
      {
        addError("Expected ';' after for condition.");
        return nullptr;
      }

      std::unique_ptr<AstNode> increment;
      if (!check(lexer::TokenType::Punctuation, ")"))
      {
        increment = parseDeclarationOrAssignment();
        if (!increment)
        {
          addError("Invalid increment in for statement.");
          return nullptr;
        }
      }

      if (!match(lexer::TokenType::Punctuation, ")"))
      {
        addError("Expected ')' after for clauses.");
        return nullptr;
      }

      if (!match(lexer::TokenType::Punctuation, "{"))
      {
        addError("Expected '{' to start for body.");
        return nullptr;
      }

      auto body = parseBlock();
      if (!body)
      {
        addError("Invalid for body.");
        return nullptr;
      }

      forNode->children.push_back(std::move(initializer));
      forNode->children.push_back(std::move(condition));
      forNode->children.push_back(std::move(increment));
      forNode->children.push_back(std::move(body));
      return forNode;
    }

    if (match(lexer::TokenType::Keyword, "return"))
    {
      auto retNode = std::make_unique<AstNode>();
      retNode->kind = AstNodeKind::Return;
      retNode->value = "return";

      auto expr = parseExpression();
      if (expr)
      {
        retNode->children.push_back(std::move(expr));
      }

      if (!match(lexer::TokenType::Punctuation, ";"))
      {
        addError("Expected ';' after return statement.");
        return nullptr;
      }
      return retNode;
    }

    auto parsed = parseDeclarationOrAssignment();
    if (!parsed)
    {
      return nullptr;
    }

    if (!match(lexer::TokenType::Punctuation, ";"))
    {
      addError("Expected ';' at end of statement.");
      return nullptr;
    }

    auto statement = std::make_unique<AstNode>();
    statement->kind = AstNodeKind::ExpressionStatement;
    statement->value = "stmt";
    statement->children.push_back(std::move(parsed));
    return statement;
  }

  std::unique_ptr<AstNode> Parser::parsePrintf()
  {
    auto printfNode = std::make_unique<AstNode>();
    printfNode->kind = AstNodeKind::Printf;
    printfNode->value = "printf";

    if (!match(lexer::TokenType::Punctuation, "("))
    {
      addError("Expected '(' after printf.");
      return nullptr;
    }

    if (!match(lexer::TokenType::StringLiteral))
    {
      addError("Expected string literal in printf.");
      return nullptr;
    }

    auto stringLit = std::make_unique<AstNode>();
    stringLit->kind = AstNodeKind::Literal;
    stringLit->value = previous().lexeme;
    printfNode->children.push_back(std::move(stringLit));

    // Optional additional arguments
    while (match(lexer::TokenType::Punctuation, ","))
    {
      auto expr = parseExpression();
      if (!expr)
      {
        addError("Expected expression in printf arguments.");
        return nullptr;
      }
      printfNode->children.push_back(std::move(expr));
    }

    if (!match(lexer::TokenType::Punctuation, ")"))
    {
      addError("Expected ')' after printf arguments.");
      return nullptr;
    }

    if (!match(lexer::TokenType::Punctuation, ";"))
    {
      addError("Expected ';' after printf statement.");
      return nullptr;
    }

    return printfNode;
  }

  std::unique_ptr<AstNode> Parser::parseScanf()
  {
    auto scanfNode = std::make_unique<AstNode>();
    scanfNode->kind = AstNodeKind::Scanf;
    scanfNode->value = "scanf";

    if (!match(lexer::TokenType::Punctuation, "("))
    {
      addError("Expected '(' after scanf.");
      return nullptr;
    }

    if (!match(lexer::TokenType::StringLiteral))
    {
      addError("Expected format string in scanf.");
      return nullptr;
    }

    auto formatLit = std::make_unique<AstNode>();
    formatLit->kind = AstNodeKind::Literal;
    formatLit->value = previous().lexeme;
    scanfNode->children.push_back(std::move(formatLit));

    if (!match(lexer::TokenType::Punctuation, ","))
    {
      addError("Expected ',' after format string in scanf.");
      return nullptr;
    }

    if (!match(lexer::TokenType::Identifier))
    {
      addError("Expected identifier in scanf.");
      return nullptr;
    }

    auto varNode = std::make_unique<AstNode>();
    varNode->kind = AstNodeKind::Identifier;
    varNode->value = previous().lexeme;
    scanfNode->children.push_back(std::move(varNode));

    if (!match(lexer::TokenType::Punctuation, ")"))
    {
      addError("Expected ')' after scanf arguments.");
      return nullptr;
    }

    if (!match(lexer::TokenType::Punctuation, ";"))
    {
      addError("Expected ';' after scanf statement.");
      return nullptr;
    }

    return scanfNode;
  }

  std::unique_ptr<AstNode> Parser::parsePrint()
  {
    auto printNode = std::make_unique<AstNode>();
    printNode->kind = AstNodeKind::Print;
    printNode->value = "print";

    if (check(lexer::TokenType::StringLiteral))
    {
      advance();
      auto stringLit = std::make_unique<AstNode>();
      stringLit->kind = AstNodeKind::Literal;
      stringLit->value = previous().lexeme;
      printNode->children.push_back(std::move(stringLit));
    }
    else
    {
      auto expr = parseExpression();
      if (!expr)
      {
        addError("Expected string or expression in print statement.");
        return nullptr;
      }
      printNode->children.push_back(std::move(expr));
    }

    if (!match(lexer::TokenType::Punctuation, ";"))
    {
      addError("Expected ';' after print statement.");
      return nullptr;
    }

    return printNode;
  }

  std::unique_ptr<AstNode> Parser::parseDeclarationOrAssignment()
  {
    if (isTypeKeyword(peek()))
    {
      const auto &typeToken = advance();
      if (!match(lexer::TokenType::Identifier))
      {
        addError("Expected identifier after type keyword '" + typeToken.lexeme + "'.");
        return nullptr;
      }

      auto node = std::make_unique<AstNode>();
      node->kind = AstNodeKind::Declaration;
      node->value = typeToken.lexeme;

      auto identifier = std::make_unique<AstNode>();
      identifier->kind = AstNodeKind::Identifier;
      identifier->value = previous().lexeme;
      node->children.push_back(std::move(identifier));

      if (match(lexer::TokenType::Operator, "="))
      {
        auto expr = parseExpression();
        if (!expr)
        {
          addError("Expected expression in declaration initializer.");
          return nullptr;
        }
        node->children.push_back(std::move(expr));
      }

      return node;
    }

    if (check(lexer::TokenType::Identifier) && current_ + 1 < tokens_.size() &&
        tokens_[current_ + 1].type == lexer::TokenType::Operator && tokens_[current_ + 1].lexeme == "=")
    {
      const auto name = advance().lexeme;
      advance();

      auto expr = parseExpression();
      if (!expr)
      {
        addError("Expected expression on right-hand side of assignment.");
        return nullptr;
      }

      auto assign = std::make_unique<AstNode>();
      assign->kind = AstNodeKind::Assignment;
      assign->value = "=";

      auto identifier = std::make_unique<AstNode>();
      identifier->kind = AstNodeKind::Identifier;
      identifier->value = name;
      assign->children.push_back(std::move(identifier));
      assign->children.push_back(std::move(expr));
      return assign;
    }

    return parseExpression();
  }

  std::unique_ptr<AstNode> Parser::parseExpression()
  {
    auto left = parseTerm();
    if (!left)
    {
      return nullptr;
    }

    while (match(lexer::TokenType::Operator, "+") || match(lexer::TokenType::Operator, "-"))
    {
      const std::string op = previous().lexeme;
      auto right = parseTerm();
      if (!right)
      {
        addError("Expected expression after operator '" + op + "'.");
        return nullptr;
      }

      auto binary = std::make_unique<AstNode>();
      binary->kind = AstNodeKind::BinaryExpression;
      binary->value = op;
      binary->children.push_back(std::move(left));
      binary->children.push_back(std::move(right));
      left = std::move(binary);
    }

    return left;
  }

  std::unique_ptr<AstNode> Parser::parseTerm()
  {
    auto left = parseFactor();
    if (!left)
    {
      return nullptr;
    }

    while (match(lexer::TokenType::Operator, "*") || match(lexer::TokenType::Operator, "/"))
    {
      const std::string op = previous().lexeme;
      auto right = parseFactor();
      if (!right)
      {
        addError("Expected factor after operator '" + op + "'.");
        return nullptr;
      }

      auto binary = std::make_unique<AstNode>();
      binary->kind = AstNodeKind::BinaryExpression;
      binary->value = op;
      binary->children.push_back(std::move(left));
      binary->children.push_back(std::move(right));
      left = std::move(binary);
    }

    return left;
  }

  std::unique_ptr<AstNode> Parser::parseFactor()
  {
    if (match(lexer::TokenType::Number) || match(lexer::TokenType::StringLiteral))
    {
      auto literal = std::make_unique<AstNode>();
      literal->kind = AstNodeKind::Literal;
      literal->value = previous().lexeme;
      return literal;
    }

    if (match(lexer::TokenType::Identifier))
    {
      auto identifier = std::make_unique<AstNode>();
      identifier->kind = AstNodeKind::Identifier;
      identifier->value = previous().lexeme;
      return identifier;
    }

    if (match(lexer::TokenType::Punctuation, "("))
    {
      auto expr = parseExpression();
      if (!match(lexer::TokenType::Punctuation, ")"))
      {
        addError("Expected ')' after grouped expression.");
        return nullptr;
      }
      return expr;
    }

    addError("Unexpected token '" + peek().lexeme + "'.");
    return nullptr;
  }

  bool Parser::match(lexer::TokenType type, const std::string &lexeme)
  {
    if (!check(type, lexeme))
    {
      return false;
    }

    advance();
    return true;
  }

  bool Parser::check(lexer::TokenType type, const std::string &lexeme) const
  {
    if (isAtEnd() && type != lexer::TokenType::EndOfFile)
    {
      return false;
    }

    const auto &token = peek();
    if (token.type != type)
    {
      return false;
    }

    return lexeme.empty() || token.lexeme == lexeme;
  }

  const lexer::Token &Parser::advance()
  {
    if (!isAtEnd())
    {
      ++current_;
    }
    return previous();
  }

  const lexer::Token &Parser::peek() const
  {
    return tokens_[current_];
  }

  const lexer::Token &Parser::previous() const
  {
    return tokens_[current_ - 1];
  }

  bool Parser::isAtEnd() const
  {
    return current_ >= tokens_.size() || tokens_[current_].type == lexer::TokenType::EndOfFile;
  }

  void Parser::syncToStatementEnd()
  {
    while (!isAtEnd())
    {
      if (match(lexer::TokenType::Punctuation, ";"))
      {
        return;
      }
      if (check(lexer::TokenType::Punctuation, "}"))
      {
        return;
      }
      advance();
    }
  }

  void Parser::addError(const std::string &message)
  {
    errors_.push_back(message);
  }

  std::string astToString(const AstNode &node, int indent)
  {
    std::ostringstream out;
    out << std::string(static_cast<std::size_t>(indent), ' ') << kindToString(node.kind);
    if (!node.value.empty())
    {
      out << "(" << node.value << ")";
    }
    out << '\n';

    for (const auto &child : node.children)
    {
      if (!child)
      {
        out << std::string(static_cast<std::size_t>(indent + 2), ' ') << "<null>" << '\n';
        continue;
      }
      out << astToString(*child, indent + 2);
    }

    return out.str();
  }

} // namespace fusionc::frontend::parser