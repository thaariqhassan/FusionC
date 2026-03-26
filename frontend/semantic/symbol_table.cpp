// Implementation of a minimal semantic analyzer.
#include "symbol_table.h"

namespace fusionc::frontend::semantic
{

  namespace
  {
    std::string normalizeType(const std::string &type)
    {
      // Treat CustomLang 'let' as int for now
      if (type == "let")
      {
        return "int";
      }
      return type;
    }
  } // namespace

  void SymbolTable::pushScope()
  {
    scopes_.push_back({});
  }

  void SymbolTable::popScope()
  {
    if (scopes_.size() > 1)
    {
      scopes_.pop_back();
    }
  }

  bool SymbolTable::declare(const std::string &name, const std::string &type)
  {
    auto &current = scopes_.back();
    if (current.find(name) != current.end())
    {
      return false;
    }
    current[name] = normalizeType(type);
    return true;
  }

  bool SymbolTable::exists(const std::string &name) const
  {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
      if (it->find(name) != it->end())
      {
        return true;
      }
    }
    return false;
  }

  std::string SymbolTable::typeOf(const std::string &name) const
  {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
      const auto found = it->find(name);
      if (found != it->end())
      {
        return found->second;
      }
    }
    return "";
  }

  std::vector<std::string> SemanticAnalyzer::analyze(const parser::AstNode &root)
  {
    errors_.clear();
    symbols_ = SymbolTable{};

    for (const auto &child : root.children)
    {
      analyzeFunction(*child);
    }

    return errors_;
  }

  void SemanticAnalyzer::analyzeFunction(const parser::AstNode &fn)
  {
    if (fn.kind != parser::AstNodeKind::Function || fn.children.empty())
    {
      errors_.push_back("Invalid function declaration.");
      return;
    }

    symbols_.pushScope();
    const auto colonPos = fn.value.find(':');
    currentReturnType_ = colonPos == std::string::npos ? "int" : normalizeType(fn.value.substr(colonPos + 1));
    analyzeBlock(*fn.children.front());
    symbols_.popScope();
  }

  void SemanticAnalyzer::analyzeBlock(const parser::AstNode &block)
  {
    symbols_.pushScope();
    for (const auto &stmt : block.children)
    {
      analyzeStatement(*stmt);
    }
    symbols_.popScope();
  }

  void SemanticAnalyzer::analyzeStatement(const parser::AstNode &stmt)
  {
    using parser::AstNodeKind;

    if (stmt.kind == AstNodeKind::Return)
    {
      if (stmt.children.empty())
      {
        if (currentReturnType_ != "void")
        {
          errors_.push_back("Non-void function must return a value.");
        }
        return;
      }

      const auto type = normalizeType(analyzeExpression(*stmt.children.front()));
      if (!currentReturnType_.empty() && currentReturnType_ != type)
      {
        errors_.push_back("Return type mismatch. Expected '" + currentReturnType_ + "' but got '" + type + "'.");
      }
      return;
    }

    if (stmt.kind == AstNodeKind::WhileStatement)
    {
      if (stmt.children.size() != 2 || !stmt.children[0] || !stmt.children[1])
      {
        errors_.push_back("Invalid while statement.");
        return;
      }

      const auto condType = normalizeType(analyzeExpression(*stmt.children[0]));
      if (!condType.empty() && condType != "int")
      {
        errors_.push_back("While condition must be of type int.");
      }

      analyzeBlock(*stmt.children[1]);
      return;
    }

    if (stmt.kind == AstNodeKind::ForStatement)
    {
      symbols_.pushScope();

      if (stmt.children.size() != 4 || !stmt.children[1] || !stmt.children[3])
      {
        errors_.push_back("Invalid for statement.");
        symbols_.popScope();
        return;
      }

      if (stmt.children[0])
      {
        analyzeExpression(*stmt.children[0]);
      }

      const auto condType = normalizeType(analyzeExpression(*stmt.children[1]));
      if (!condType.empty() && condType != "int")
      {
        errors_.push_back("For condition must be of type int.");
      }

      analyzeBlock(*stmt.children[3]);

      if (stmt.children[2])
      {
        analyzeExpression(*stmt.children[2]);
      }

      symbols_.popScope();
      return;
    }

    if (stmt.kind == AstNodeKind::ExpressionStatement && !stmt.children.empty())
    {
      analyzeExpression(*stmt.children.front());
    }
    else if (stmt.kind == AstNodeKind::Block)
    {
      analyzeBlock(stmt);
    }
  }

  std::string SemanticAnalyzer::analyzeExpression(const parser::AstNode &expr)
  {
    using parser::AstNodeKind;

    switch (expr.kind)
    {
    case AstNodeKind::Declaration:
    {
      std::string type = normalizeType(expr.value);

      if (expr.children.empty())
      {
        return type;
      }

      const auto &idNode = *expr.children.front();

      if (!symbols_.declare(idNode.value, type))
      {
        errors_.push_back("Duplicate declaration of '" + idNode.value + "'.");
      }

      if (expr.children.size() == 2)
      {
        const auto initType = normalizeType(analyzeExpression(*expr.children[1]));
        if (!initType.empty() && initType != type)
        {
          errors_.push_back("Initializer type mismatch for '" + idNode.value + "'. Expected '" + type + "' but got '" + initType + "'.");
        }
      }

      return type;
    }

    case AstNodeKind::Assignment:
    {
      if (expr.children.size() != 2)
      {
        errors_.push_back("Invalid assignment expression.");
        return "";
      }

      const auto &idNode = *expr.children.front();

      if (!symbols_.exists(idNode.value))
      {
        errors_.push_back("Use of undeclared identifier '" + idNode.value + "'.");
        analyzeExpression(*expr.children[1]);
        return "";
      }

      const auto lhsType = normalizeType(symbols_.typeOf(idNode.value));
      const auto rhsType = normalizeType(analyzeExpression(*expr.children[1]));

      if (!rhsType.empty() && lhsType != rhsType)
      {
        errors_.push_back("Assignment type mismatch for '" + idNode.value + "'. Expected '" + lhsType + "' but got '" + rhsType + "'.");
      }

      return lhsType;
    }

    case AstNodeKind::BinaryExpression:
    {
      if (expr.children.size() != 2)
      {
        errors_.push_back("Invalid binary expression.");
        return "";
      }

      const auto leftType = normalizeType(analyzeExpression(*expr.children[0]));
      const auto rightType = normalizeType(analyzeExpression(*expr.children[1]));

      if (!leftType.empty() && !rightType.empty() && leftType != rightType)
      {
        errors_.push_back("Type mismatch in binary expression. Left is '" + leftType + "', right is '" + rightType + "'.");
      }

      return "int";
    }

    case AstNodeKind::Literal:
      return "int";

    case AstNodeKind::Identifier:
      if (!symbols_.exists(expr.value))
      {
        errors_.push_back("Use of undeclared identifier '" + expr.value + "'.");
        return "";
      }
      return normalizeType(symbols_.typeOf(expr.value));

    default:
      break;
    }

    return "";
  }

} // namespace fusionc::frontend::semantic