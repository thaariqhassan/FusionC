// Executes the TAC program produced by the middle-end.
#include "machine_codegen.h"

#include <unordered_map>

namespace fusionc::backend::codegen
{

  ExecutionResult execute(const middleend::ir::Program &program)
  {
    std::unordered_map<std::string, int> slots;
    ExecutionResult result;

    auto read = [&](const std::string &name) -> int
    {
      const auto it = slots.find(name);
      if (it != slots.end())
      {
        return it->second;
      }
      return std::stoi(name); // fall back to literal
    };

    for (const auto &ins : program)
    {
      if (ins.op == "const")
      {
        slots[ins.dst] = std::stoi(ins.arg1);
      }
      else if (ins.op == "copy")
      {
        slots[ins.dst] = read(ins.arg1);
      }
      else if (ins.op == "add")
      {
        slots[ins.dst] = read(ins.arg1) + read(ins.arg2);
      }
      else if (ins.op == "sub")
      {
        slots[ins.dst] = read(ins.arg1) - read(ins.arg2);
      }
      else if (ins.op == "mul")
      {
        slots[ins.dst] = read(ins.arg1) * read(ins.arg2);
      }
      else if (ins.op == "div")
      {
        slots[ins.dst] = read(ins.arg1) / read(ins.arg2);
      }
      else if (ins.op == "ret")
      {
        result.ok = true;
        result.exitCode = read(ins.dst);
        result.message = "Program executed";
        return result;
      }
    }

    result.ok = true;
    result.message = "Program executed without explicit return; exit code 0";
    result.exitCode = 0;
    return result;
  }

} // namespace fusionc::backend::codegen
