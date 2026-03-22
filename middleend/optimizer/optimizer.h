// Simple optimizer pass for TAC.
#pragma once

#include "../ir/ir.h"

namespace fusionc::middleend::optimizer
{

  // Performs constant folding on TAC instructions.
  middleend::ir::Program foldConstants(const middleend::ir::Program &program);

} // namespace fusionc::middleend::optimizer
