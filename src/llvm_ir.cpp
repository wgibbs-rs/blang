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
std::map<std::string, llvm::Value *> ExtrnValues;



std::map<std::string, llvm::Function *> FunctionValues;
std::map<std::string, llvm::BasicBlock *> BasicBlockValues;


static llvm::Value *LogErrorV(const char *Str) {
  llvm::errs() << "error: " << Str << "\n";
  return nullptr;
}


static inline llvm::Value* value_of(llvm::Value* alloca) {
   return Builder->CreateLoad(llvm::Type::getInt64Ty(*TheContext), alloca, "load");
}


/** 
 * Stores whether a function being processed includes a return statement at the end.
 * If not, return(0); is added.
 */
static bool functionDoesReturn = false;

static llvm::Value* add_expression(ASTNode* node) {

   switch (node->type) {
      case ASTNode::_ADD:
         return Builder->CreateAdd(add_expression(node->factors.left), add_expression(node->factors.right), "addtmp");
         break;
      case ASTNode::_SUBTRACT:
         return Builder->CreateSub(add_expression(node->factors.left), add_expression(node->factors.right), "subtmp");
         break;
      case ASTNode::_MULTIPLY:
         return Builder->CreateMul(add_expression(node->factors.left), add_expression(node->factors.right), "multmp");
         break;
      case ASTNode::_DIVIDE:
         return Builder->CreateSDiv(add_expression(node->factors.left), add_expression(node->factors.right), "sdivtmp");
         break;
      case ASTNode::_GTEQ:
         {
            llvm::Value* cmp = Builder->CreateICmpSGE(add_expression(node->factors.left), add_expression(node->factors.right), "sgetmp");
            return Builder->CreateZExt(cmp, llvm::Type::getInt64Ty(*TheContext), "i64_bool");
            break;
         }
      case ASTNode::_LTEQ:
         {
            llvm::Value* cmp = Builder->CreateICmpSLE(add_expression(node->factors.left), add_expression(node->factors.right), "sletmp");
            return Builder->CreateZExt(cmp, llvm::Type::getInt64Ty(*TheContext), "i64_bool");
            break;
         }
      case ASTNode::_GREATER:
         {
            llvm::Value* cmp = Builder->CreateICmpSGT(add_expression(node->factors.left), add_expression(node->factors.right), "sgttmp");
            return Builder->CreateZExt(cmp, llvm::Type::getInt64Ty(*TheContext), "i64_bool");
            break;
         }
      case ASTNode::_LESS:
         {
            llvm::Value* cmp = Builder->CreateICmpSLT(add_expression(node->factors.left), add_expression(node->factors.right), "slttmp");
            return Builder->CreateZExt(cmp, llvm::Type::getInt64Ty(*TheContext), "i64_bool");
            break;
         }
      case ASTNode::_EQUALS:
         {
            llvm::Value* cmp = Builder->CreateICmpEQ(add_expression(node->factors.left), add_expression(node->factors.right), "eqtmp");
            return Builder->CreateZExt(cmp, llvm::Type::getInt64Ty(*TheContext), "i64_bool");
            break;
         }
      case ASTNode::_NEQUALS:
         {
            llvm::Value* cmp = Builder->CreateICmpNE(add_expression(node->factors.left), add_expression(node->factors.right), "netmp");
            return Builder->CreateZExt(cmp, llvm::Type::getInt64Ty(*TheContext), "i64_bool");
            break;
         }
      case ASTNode::_FUNCTION_CALL:
         break;

      case ASTNode::_NOT:
         {
            llvm::Value* not_value = Builder->CreateICmpEQ(
               add_expression(node->inner), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 0)),
               "not_tmp"
            );
            return Builder->CreateZExt(not_value, llvm::Type::getInt64Ty(*TheContext), "i64_not");
            break;
         }
      case ASTNode::_INC:
         {
            llvm::Value* inc = Builder->CreateAdd(
               value_of(NamedValues[node->string]), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 1)), 
               "inctmp");
            Builder->CreateStore(inc, NamedValues[node->string]);
            return Builder->CreateSub(inc, llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 1)), "lesser_inc");
         }
      case ASTNode::_DEC:
         {
            llvm::Value* dec = Builder->CreateSub(
               value_of(NamedValues[node->string]), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 1)), 
               "dectmp");
            Builder->CreateStore(dec, NamedValues[node->string]);
            return Builder->CreateAdd(dec, llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 1)), "greater_dec");
         }
         break;
      case ASTNode::_NUMBER:
         return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), node->integer);
         break;
      case ASTNode::_VARIABLE:
         {
            return value_of(NamedValues[node->string]);
            break;
         }
      default:
         break;
   }

   return nullptr;

}


static void add_statement(ASTNode* node) {

   switch (node->type) {
      case ASTNode::STOP: return;
      case ASTNode::_AUTO:
      case ASTNode::_EXTRN:
         add_statement(node->list.next);
         add_statement(node->successor);
         break;
      case ASTNode::_VARIABLE:
         {
            if (node->list.variableType == VariableType::VAR_AUTO) {
               NamedValues[node->list.title] = Builder->CreateAlloca(llvm::Type::getInt64Ty(*TheContext), nullptr, node->list.title);
               add_statement(node->list.next);
            }
            else if (node->list.variableType == VariableType::VAR_EXTRN) {
               ExtrnValues[node->list.title] = new llvm::GlobalVariable(
                  *TheModule,
                  llvm::Type::getInt64Ty(*TheContext),
                  false,
                  llvm::GlobalValue::ExternalLinkage,
                  nullptr,
                  node->list.title
               );
            }
            break;
         }
      case ASTNode::_ASSIGNMENT:
         {
            Builder->CreateStore(add_expression(node->list.next), NamedValues[node->list.title]);
            add_statement(node->successor);
            break;
         }
      case ASTNode::_WHILE_LOOP:
         { 
            llvm::BasicBlock *Then = llvm::BasicBlock::Create(*TheContext, "while_then", Builder->GetInsertBlock()->getParent());
            llvm::BasicBlock *Merge = llvm::BasicBlock::Create(*TheContext, "while_merge", Builder->GetInsertBlock()->getParent());

            llvm::Value* cond_i1 = Builder->CreateICmpNE(
               add_expression(node->list.inner), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 0)), 
               "while_cond_i1_1" 
            );
            Builder->CreateCondBr(cond_i1, Then, Merge);

            Builder->SetInsertPoint(Then);
            add_statement(node->list.next);
            cond_i1 = Builder->CreateICmpNE(
               add_expression(node->list.inner), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 0)), 
               "while_cond_i1_2" 
            );
            Builder->CreateCondBr(cond_i1, Then, Merge);

            Builder->SetInsertPoint(Merge);
            add_statement(node->successor);
            break;
         }
      case ASTNode::_IF:
         {

            llvm::BasicBlock *Then = llvm::BasicBlock::Create(*TheContext, "if_then", Builder->GetInsertBlock()->getParent());
            llvm::BasicBlock *Merge = llvm::BasicBlock::Create(*TheContext, "if_merge", Builder->GetInsertBlock()->getParent());

            // To set the i64 conditional (if_t.cond) to i1, we perform if_t.cond != 0.
            llvm::Value* cond_i1 = Builder->CreateICmpNE(
               add_expression(node->if_t.cond), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 0)), 
               "cond_i1" 
            );
            
            // Test if statement includes an "else" section
            if (node->if_t.else_t->type != ASTNode::STOP) {
               llvm::BasicBlock *Else = llvm::BasicBlock::Create(*TheContext, "if_else", Builder->GetInsertBlock()->getParent());

               Builder->CreateCondBr(cond_i1, Then, Else);

               // Write "else" code
               Builder->SetInsertPoint(Else);
               add_statement(node->if_t.else_t);
               if (!Else->getTerminator()) Builder->CreateBr(Merge);
            
            }
            else {
               Builder->CreateCondBr(cond_i1, Then, Merge);
            }

            // Write "then" code.
            Builder->SetInsertPoint(Then);
            add_statement(node->if_t.statements);
            if (!Then->getTerminator()) Builder->CreateBr(Merge);

            Builder->SetInsertPoint(Merge);
            add_statement(node->successor);

            break;
         }
      case ASTNode::_LABEL:
         BasicBlockValues[node->string] = llvm::BasicBlock::Create(*TheContext, node->string, Builder->GetInsertBlock()->getParent());
         Builder->CreateBr(BasicBlockValues[node->string]);
         Builder->SetInsertPoint(BasicBlockValues[node->string]);
         add_statement(node->successor);
         break;
      case ASTNode::_GOTO:
         Builder->CreateBr(BasicBlockValues[node->string]);
         add_statement(node->successor);
         break;
      case ASTNode::_RETURN:
         Builder->CreateRet(add_expression(node->list.next));
         functionDoesReturn = true;
         break;
      case ASTNode::_INC:
         {
            llvm::Value* inc = Builder->CreateAdd(
               value_of(NamedValues[node->string]), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 1)), 
               "inctmp");
            Builder->CreateStore(inc, NamedValues[node->string]);
            add_statement(node->successor);
            break;
         }
      case ASTNode::_DEC:
         {
            llvm::Value* dec = Builder->CreateSub(
               value_of(NamedValues[node->string]), 
               llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 1)), 
               "dectmp");
            Builder->CreateStore(dec, NamedValues[node->string]);
            add_statement(node->successor);
            break;
         }
      default:
         fatal_error("unknown statement.");
         break;
   }
}



static void add_function(ASTNode* node) {


   // Create an array of Int64Ty's. Does not store their information.
   std::vector<llvm::Type*> list;
   for ( ASTNode* currentArg = node->function.args; 
         currentArg; 
         currentArg = currentArg->list.next
   ) list.push_back(llvm::Type::getInt64Ty(*TheContext));

   // Create the function type, including arguments if any.
   llvm::FunctionType *funcType;
   if (list.size() == 0 ) 
   {
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

   functionDoesReturn = false; // False, unless proven otherwise.

   Builder->SetInsertPoint(llvm::BasicBlock::Create(*TheContext, "entry", function));

   add_statement(node->function.statements); // Begin adding statements to module.

   // If no return statement is made, then return undefined information.
   if (!functionDoesReturn) {
      llvm::Value* undefined_value = llvm::UndefValue::get(Builder->getInt64Ty());
      Builder->CreateRet(undefined_value);
   }
}

static void add_global_variable(ASTNode* node) {
   // TODO
}

static void analyze_ast() {

   bool foundMainFunction = false;
   
   for (int i = 0; i < ast_length; i++)
      if (generated_ast[i]->type == ASTNode::_FUNCTION)
         if (strcmp(generated_ast[i]->function.title, "main") == 0)
            foundMainFunction = true;

   if (!foundMainFunction) fatal_error("no entry point.");

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
            fatal_error("unrecognized root type \"%s\"\n", ASTNodeTypeNames[generated_ast[i]->type]);
            break;
      }

   }

}
