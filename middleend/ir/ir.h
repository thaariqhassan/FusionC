// Simple three-address code representation used by FusionC.
#pragma once

#include <string>
#include <vector>

#include "../../frontend/parser/ast.h"

namespace fusionc::middleend::ir
{

  struct Instruction
  {
    // op can be: const, copy, add, sub, mul, div, ret, label, jmp, jz, lt, gt, eq, print
    std::string op;
    std::string dst;
    std::string arg1;
    std::string arg2;
  };

  using Program = std::vector<Instruction>;

  // Build TAC from the parsed AST.
  Program buildProgram(const fusionc::frontend::parser::AstNode &root);

} // namespace fusionc::middleend::ir
