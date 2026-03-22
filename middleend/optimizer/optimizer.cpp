// Constant folding optimizer for the TAC program.
#include "optimizer.h"

#include <cstdlib>
#include <optional>

namespace fusionc::middleend::optimizer
{

  namespace
  {
    std::optional<int> toInt(const std::string &s)
    {
      char *end = nullptr;
      const long val = std::strtol(s.c_str(), &end, 10);
      if (end && *end == '\0')
      {
        return static_cast<int>(val);
      }
      return std::nullopt;
    }

    std::string eval(const std::string &op, const std::string &a, const std::string &b)
    {
      const auto lhs = toInt(a);
      const auto rhs = toInt(b);
      if (!lhs || !rhs)
      {
        return {};
      }

      int result = 0;
      if (op == "add")
      {
        result = *lhs + *rhs;
      }
      else if (op == "sub")
      {
        result = *lhs - *rhs;
      }
      else if (op == "mul")
      {
        result = *lhs * *rhs;
      }
      else if (op == "div" && *rhs != 0)
      {
        result = *lhs / *rhs;
      }
      else
      {
        return {};
      }

      return std::to_string(result);
    }
  } // namespace

  middleend::ir::Program foldConstants(const middleend::ir::Program &program)
  {
    middleend::ir::Program optimized;
    optimized.reserve(program.size());

    for (const auto &ins : program)
    {
      if ((ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "div"))
      {
        const auto folded = eval(ins.op, ins.arg1, ins.arg2);
        if (!folded.empty())
        {
          optimized.push_back({"const", ins.dst, folded, ""});
          continue;
        }
      }

      optimized.push_back(ins);
    }

    return optimized;
  }

} // namespace fusionc::middleend::optimizer
