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

/// This file is used to create intermediate representation code for LLVM.

#include "llvm.h"
#include "ast.h"
#include "error.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <map>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::IRBuilder<>> Builder;
std::unique_ptr<llvm::Module> TheModule;
std::map<std::string, llvm::Value *> NamedValues;

std::map<std::string, llvm::FunctionType *> FunctionTypeValues;



static llvm::Value *LogErrorV(const char *Str) {
  llvm::errs() << "error: " << Str << "\n";
  return nullptr;
}


static bool functionDoesReturn;

static void add_function(ASTNode* node) {


   std::vector<llvm::Type*> list;
   ASTNode* currentArg = node->function.args;

   while (currentArg) {
      list.push_back(llvm::Type::getInt64Ty(*TheContext));
      currentArg = currentArg->list.next;
   }

   
   llvm::FunctionType *funcType;
   if (list.size() == 0) {
      funcType = llvm::FunctionType::get(
         llvm::Type::getInt64Ty(*TheContext),
         false
      );
   } else {
      funcType = llvm::FunctionType::get(
         llvm::Type::getInt64Ty(*TheContext),
         list,
         false
      );
   }

   llvm::Function *function = llvm::Function::Create(
      funcType,
      llvm::Function::ExternalLinkage,
      llvm::Twine(node->function.title),
      *TheModule
   );


   llvm::BasicBlock *entryBasicBlock = llvm::BasicBlock::Create(*TheContext, llvm::Twine(node->function.title), function);
   Builder->SetInsertPoint(entryBasicBlock);

   functionDoesReturn = false;


   // If no return statement is made, then return 0;
   if (!functionDoesReturn) {
      Builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 0));
   }

}

static void add_global_variable(ASTNode* node) {

}


static void analyze_ast() {

   bool foundMainFunction = false;
   
   for (int i = 0; i < ast_length; i++) {
      if (generated_ast[i]->type == ASTNode::_FUNCTION)
         if (strcmp(generated_ast[i]->function.title, "main") == 0)
            foundMainFunction = true;
   }

   if (!foundMainFunction) {
      fatal_error("no entry point.");
   }

}



extern "C" void generate_llvm_ir() {

   analyze_ast();

   for (int i = 0; i < ast_length; i++) {
      switch (generated_ast[i]->type) {
         case ASTNode::_FUNCTION:
            add_function(generated_ast[i]);
            break;
         case ASTNode::_GLOBAL_DECLARATION:
            add_global_variable(generated_ast[i]);
            break;
         default:
            fatal_error("unrecognized root node \"%s\"\n", ASTNodeTypeNames[generated_ast[i]->type]);
            break;
      }

   }

}
