// Build a minimal three-address code (TAC) program from the parsed AST.
#include "ir.h"

#include <sstream>
#include <stdexcept>

#include "../../frontend/parser/ast.h"

namespace fusionc::middleend::ir
{

  namespace
  {
    class TacBuilder
    {
    public:
      Program build(const frontend::parser::AstNode &root)
      {
        Program prog;
        tempCounter_ = 0;
        labelCounter_ = 0;

        for (const auto &child : root.children)
        {
          if (child->kind == frontend::parser::AstNodeKind::Function)
          {
            emitFunction(*child, prog);
          }
        }

        return prog;
      }

    private:
      std::string newTemp()
      {
        std::ostringstream oss;
        oss << "t" << tempCounter_++;
        return oss.str();
      }

      std::string newLabel()
      {
        std::ostringstream oss;
        oss << "L" << labelCounter_++;
        return oss.str();
      }

      void emitFunction(const frontend::parser::AstNode &fn, Program &prog)
      {
        if (fn.children.empty())
        {
          return;
        }

        emitBlock(*fn.children.front(), prog);
      }

      void emitBlock(const frontend::parser::AstNode &block, Program &prog)
      {
        for (const auto &stmt : block.children)
        {
          emitStatement(*stmt, prog);
        }
      }

      void emitStatement(const frontend::parser::AstNode &stmt, Program &prog)
      {
        using frontend::parser::AstNodeKind;

        if (stmt.kind == AstNodeKind::ExpressionStatement)
        {
          if (!stmt.children.empty())
          {
            emitExpr(*stmt.children.front(), prog);
          }
        }
        else if (stmt.kind == AstNodeKind::Return)
        {
          std::string value = stmt.children.empty() ? "0" : emitExpr(*stmt.children.front(), prog);
          prog.push_back(Instruction{"ret", value, "", ""});
        }
        else if (stmt.kind == AstNodeKind::Block)
        {
          emitBlock(stmt, prog);
        }
        else if (stmt.kind == AstNodeKind::WhileStatement)
        {
          if (stmt.children.size() != 2)
          {
            throw std::runtime_error("Invalid while statement in TAC builder.");
          }

          std::string startLabel = newLabel();
          std::string endLabel = newLabel();

          prog.push_back(Instruction{"label", startLabel, "", ""});

          std::string cond = emitExpr(*stmt.children[0], prog);
          prog.push_back(Instruction{"jz", endLabel, cond, ""});

          emitBlock(*stmt.children[1], prog);

          prog.push_back(Instruction{"jmp", startLabel, "", ""});
          prog.push_back(Instruction{"label", endLabel, "", ""});
        }
        else if (stmt.kind == AstNodeKind::Printf)
        {
          if (!stmt.children.empty())
          {
            std::string msg = stmt.children.front()->value;
            prog.push_back(Instruction{"print", msg, "", ""});
          }
        }
      }

      std::string emitExpr(const frontend::parser::AstNode &expr, Program &prog)
      {
        using frontend::parser::AstNodeKind;

        switch (expr.kind)
        {
        case AstNodeKind::Declaration:
          if (!expr.children.empty())
          {
            const auto &idNode = *expr.children.front();
            if (expr.children.size() == 2)
            {
              std::string rhs = emitExpr(*expr.children[1], prog);
              prog.push_back(Instruction{"copy", idNode.value, rhs, ""});
            }
            return idNode.value;
          }
          break;

        case AstNodeKind::Assignment:
        {
          if (expr.children.size() != 2)
          {
            break;
          }

          std::string rhs = emitExpr(*expr.children[1], prog);
          const auto &idNode = *expr.children[0];
          prog.push_back(Instruction{"copy", idNode.value, rhs, ""});
          return idNode.value;
        }

        case AstNodeKind::BinaryExpression:
        {
          std::string left = emitExpr(*expr.children[0], prog);
          std::string right = emitExpr(*expr.children[1], prog);
          std::string dst = newTemp();

          std::string op;
          if (expr.value == "+")
          {
            op = "add";
          }
          else if (expr.value == "-")
          {
            op = "sub";
          }
          else if (expr.value == "*")
          {
            op = "mul";
          }
          else if (expr.value == "/")
          {
            op = "div";
          }
          else
          {
            throw std::runtime_error("Unsupported binary operator: " + expr.value);
          }

          prog.push_back(Instruction{op, dst, left, right});
          return dst;
        }

        case AstNodeKind::Literal:
          return expr.value;

        case AstNodeKind::Identifier:
          return expr.value;

        default:
          break;
        }

        throw std::runtime_error("Unsupported expression in TAC builder.");
      }

      int tempCounter_ = 0;
      int labelCounter_ = 0;
    };
  } // namespace

  Program buildProgram(const frontend::parser::AstNode &root)
  {
    TacBuilder builder;
    return builder.build(root);
  }

} // namespace fusionc::middleend::ir