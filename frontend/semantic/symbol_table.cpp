// Implementation of a minimal semantic analyzer.
#include "symbol_table.h"

namespace fusionc::frontend::semantic
{

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
    current[name] = type;
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
    currentReturnType_ = colonPos == std::string::npos ? "int" : fn.value.substr(colonPos + 1);
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
      const auto type = analyzeExpression(*stmt.children.front());
      if (!currentReturnType_.empty() && currentReturnType_ != type)
      {
        errors_.push_back("Return type mismatch. Expected '" + currentReturnType_ + "' but got '" + type + "'.");
      }
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
      const auto type = expr.value;
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
        analyzeExpression(*expr.children[1]);
      }
      return type;
    }
    case AstNodeKind::Assignment:
    {
      const auto &idNode = *expr.children.front();
      if (!symbols_.exists(idNode.value))
      {
        errors_.push_back("Use of undeclared identifier '" + idNode.value + "'.");
      }
      analyzeExpression(*expr.children[1]);
      return symbols_.typeOf(idNode.value);
    }
    case AstNodeKind::BinaryExpression:
      analyzeExpression(*expr.children[0]);
      analyzeExpression(*expr.children[1]);
      return "int";
    case AstNodeKind::Literal:
      return "int";
    case AstNodeKind::Identifier:
      if (!symbols_.exists(expr.value))
      {
        errors_.push_back("Use of undeclared identifier '" + expr.value + "'.");
      }
      return symbols_.typeOf(expr.value);
    default:
      break;
    }
    return "";
  }

} // namespace fusionc::frontend::semantic
