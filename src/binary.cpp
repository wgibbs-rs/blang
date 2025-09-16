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

#include <iostream>

#include "llvm.h"
#include "context.h"
#include "error.h"
#include "opt.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"

#include <llvm/TargetParser/Host.h>

extern "C" void initialize_llvm() {
   TheContext = std::make_unique<llvm::LLVMContext>();
   Builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*TheContext));
   TheModule = std::make_unique<llvm::Module>(ctx.inputFile, *TheContext);

   // Initialize all targets (only do this once in your program)
   LLVMInitializeX86TargetInfo();
   LLVMInitializeX86Target();
   LLVMInitializeX86TargetMC();
   LLVMInitializeX86AsmParser();
   LLVMInitializeX86AsmPrinter();
   
   LLVMInitializeAArch64TargetInfo();
   LLVMInitializeAArch64Target();
   LLVMInitializeAArch64TargetMC();
   LLVMInitializeAArch64AsmParser();
   LLVMInitializeAArch64AsmPrinter();
}

extern "C" void export_asm() {
   // Construct a Triple from the default target triple string
   llvm::Triple triple(llvm::sys::getDefaultTargetTriple());

   // Set the module's target triple (takes llvm::Triple now)
   TheModule->setTargetTriple(triple);

   // Look up the target with the Triple
   std::string error;
   auto target = llvm::TargetRegistry::lookupTarget(triple, error);
   if (!target) {
      llvm::errs() << "Failed to lookup target: " << error << "\n";
      return;
   }

   // Create the TargetMachine using the Triple (string overload is deprecated)
   llvm::TargetOptions opt;
   auto RM = std::optional<llvm::Reloc::Model>();
   auto targetMachine = target->createTargetMachine(triple, "generic", "", opt, RM);

   // Update the module data layout
   TheModule->setDataLayout(targetMachine->createDataLayout());

   // Prepare output file
   std::error_code EC;
   llvm::raw_fd_ostream dest(ctx.outputFilename, EC, llvm::sys::fs::OF_None);

   if (EC) {
      llvm::errs() << "Could not open file: " << EC.message() << "\n";
      return;
   }

   // Create a pass manager to emit machine code
   llvm::legacy::PassManager pass;

   // Set file type: object file (.o)
#if defined(__APPLE__)
    llvm::CodeGenFileType fileType = llvm::CodeGenFileType::AssemblyFile;
#else
    llvm::CodeGenFileType fileType = llvm::CGFT_AssemblyFile;
#endif

   if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
      llvm::errs() << "TargetMachine can't emit a file of this type\n";
      return;
   }

   pass.run(*TheModule);
   dest.flush();
}

extern "C" void export_ir() {
   std::error_code EC;
   llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);

   if (EC)
      fatal_error("Could not open file: %s", EC.message().c_str());
   else
      TheModule->print(dest, nullptr); // Print IR to file
}

extern "C" void export_bin() {
   // Construct a Triple from the default target triple string
   llvm::Triple triple(llvm::sys::getDefaultTargetTriple());

   // Set the module's target triple (takes llvm::Triple now)
   TheModule->setTargetTriple(triple);

   // Look up the target with the Triple
   std::string error;
   auto target = llvm::TargetRegistry::lookupTarget(triple, error);
   if (!target) {
      llvm::errs() << "Failed to lookup target: " << error << "\n";
      return;
   }

   // Create the TargetMachine using the Triple (string overload is deprecated)
   llvm::TargetOptions opt;
   auto RM = std::optional<llvm::Reloc::Model>();
   auto targetMachine = target->createTargetMachine(triple, "generic", "", opt, RM);

   // Update the module data layout
   TheModule->setDataLayout(targetMachine->createDataLayout());

   // Prepare output file
   std::error_code EC;
   llvm::raw_fd_ostream dest(ctx.outputFilename, EC, llvm::sys::fs::OF_None);

   if (EC) {
      llvm::errs() << "Could not open file: " << EC.message() << "\n";
      return;
   }

   // Create a pass manager to emit machine code
   llvm::legacy::PassManager pass;

   // Set file type: object file (.o)
#if defined(__APPLE__)
   llvm::CodeGenFileType fileType = llvm::CodeGenFileType::ObjectFile;
#else
   llvm::CodeGenFileType fileType = llvm::CGFT_ObjectFile;
#endif

   if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
      llvm::errs() << "TargetMachine can't emit a file of this type\n";
      return;
   }

   pass.run(*TheModule);
   dest.flush();
}

extern "C" void optimize() {
   if (GCC_LIKELY(!ctx.optimization)) return;

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
      MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1);
   else if (ctx.optimization == 2)  // Moderate optimization
      MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
   else if (ctx.optimization == 3)  // Aggressive optimization
      MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
   else if (ctx.optimization == 4)  // Optimize for size
      MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::Os);
   else if (ctx.optimization == 5)  // Aggressively optimize for size
      MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::Oz);

   MPM.run(*TheModule, MAM);
}