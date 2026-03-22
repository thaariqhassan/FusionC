// Simple interpreter backend for TAC.
#pragma once

#include <string>

#include "../../middleend/ir/ir.h"

namespace fusionc::backend::codegen
{

  struct ExecutionResult
  {
    bool ok = false;
    int exitCode = 0;
    std::string message;
  };

  // Executes TAC program and returns the exit code of main.
  ExecutionResult execute(const middleend::ir::Program &program);

} // namespace fusionc::backend::codegen
