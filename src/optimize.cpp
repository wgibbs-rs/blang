/*
   BLang
   Copyright (c) 2025 William Gibbs

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.
*/

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"

#include "llvm.h"
#include "context.h"

#include <stdio.h>


void optimize() {
   if (!ctx.optimization) return;

   // Create analysis managers
   llvm::LoopAnalysisManager LAM;
   llvm::FunctionAnalysisManager FAM;
   llvm::CGSCCAnalysisManager CGAM;
   llvm::ModuleAnalysisManager MAM;

   // Create pass builder
   llvm::PassBuilder PB;

   // Register analysis passes
   PB.registerModuleAnalyses(MAM);
   PB.registerCGSCCAnalyses(CGAM);
   PB.registerFunctionAnalyses(FAM);
   PB.registerLoopAnalyses(LAM);
   PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);


   llvm::ModulePassManager MPM;
   // Create optimization pipeline
   if (ctx.optimization == 1)       // Mild optimization 
      MPM = PB.buildPerModuleDefaultPipeline(
         llvm::OptimizationLevel::O1
      );
   else if (ctx.optimization == 2)  // Moderate optimization
      MPM = PB.buildPerModuleDefaultPipeline(
         llvm::OptimizationLevel::O2
      );
   else if (ctx.optimization == 3)  // Aggressive optimization
      MPM = PB.buildPerModuleDefaultPipeline(
         llvm::OptimizationLevel::O3
      );
   else if (ctx.optimization == 4)  // Optimize for size
      MPM = PB.buildPerModuleDefaultPipeline(
         llvm::OptimizationLevel::Os
      );
   else if (ctx.optimization == 5)  // Aggressively optimize for size
      MPM = PB.buildPerModuleDefaultPipeline(
         llvm::OptimizationLevel::Oz
      );


   MPM.run(*TheModule, MAM);

}