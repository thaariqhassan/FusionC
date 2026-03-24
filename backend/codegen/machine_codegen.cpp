#include "machine_codegen.h"

#include <iostream>
#include <unordered_map>
#include <stdexcept>

namespace fusionc::backend::codegen
{

  ExecutionResult execute(const middleend::ir::Program &program)
  {
    std::unordered_map<std::string, int> slots;
    std::unordered_map<std::string, std::size_t> labels;

    ExecutionResult result;

    // First pass: collect labels
    for (std::size_t i = 0; i < program.size(); ++i)
    {
      if (program[i].op == "label")
      {
        labels[program[i].dst] = i;
      }
    }

    auto read = [&](const std::string &name) -> int
    {
      const auto it = slots.find(name);
      if (it != slots.end())
      {
        return it->second;
      }
      return std::stoi(name);
    };

    // Execution with program counter
    for (std::size_t pc = 0; pc < program.size(); ++pc)
    {
      const auto &ins = program[pc];

      if (ins.op == "label")
      {
        continue;
      }
      else if (ins.op == "const")
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
      else if (ins.op == "jmp")
      {
        pc = labels.at(ins.dst) - 1;
      }
      else if (ins.op == "jz")
      {
        if (read(ins.arg1) == 0)
        {
          pc = labels.at(ins.dst) - 1;
        }
      }
      else if (ins.op == "print")
      {
        std::cout << ins.dst << std::endl;
      }
      else if (ins.op == "ret")
      {
        result.ok = true;
        result.exitCode = read(ins.dst);
        result.message = "Program executed";
        return result;
      }
      else
      {
        throw std::runtime_error("Unknown instruction: " + ins.op);
      }
    }

    result.ok = true;
    result.exitCode = 0;
    result.message = "Program executed without explicit return";
    return result;
  }

} // namespace fusionc::backend::codegen