// #include "llshader/CodeGen/CodeGen.h"
// #include "llvm/ADT/StringMap.h"
// #include "llvm/IR/IRBuilder.h"
// #include "llvm/IR/LLVMContext.h"
// #include "llvm/IR/Module.h"
// #include "llvm/Support/raw_ostream.h"

// using namespace llvm;

// namespace
// {
// class ToIRVisitor : public ASTVisitor
// {
//     Module *M;
//     IRBuilder<> Builder;
//     Type *VoidTy;
//     Type *Int32Ty;
//     PointerType *PtrTy;
//     Constant *Int32Zero;
//     Value *V;
//     StringMap<Value *> nameMap;

//   public:
//     ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
//     {
//         VoidTy = Type::getVoidTy(M->getContext());
//         Int32Ty = Type::getInt32Ty(M->getContext());
//         PtrTy = PointerType::getUnqual(M->getContext());
//         Int32Zero = ConstantInt::get(Int32Ty, 0, true);
//     }

//     void run(AST *Tree)
//     {
//         FunctionType *MainFty =
//             FunctionType::get(Int32Ty, {Int32Ty, PtrTy}, false);
//         Function *MainFn =
//             Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);
//         BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
//         Builder.SetInsertPoint(BB);
//         Tree->accept(*this);

//         FunctionType *WriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);
//         Function *WriteFn = Function::Create(
//             WriteFnTy, GlobalValue::ExternalLinkage, "write_int", M);
//         Builder.CreateCall(WriteFnTy, WriteFn, {V});
//         Builder.CreateRet(Int32Zero);
//     }

//     virtual void visit(Program &Node) override
//     {
//         if (Node.getSL())
//         {
//             for (auto &stmt : *Node.getSL())
//             {
//                 stmt->accept(*this);
//             }
//         }
//     };

//     void visit(Statement &Node) override
//     {
//         if (llvm::isa<Declaration>(Node))
//         {
//             Node.accept(*this);
//             return;
//         }
//         if (llvm::isa<Scoped>(Node))
//         {
//             Node.accept(*this);
//             return;
//         }
//         if (llvm::isa<Assignment>(Node))
//         {
//             Node.accept(*this);
//             return;
//         }
//     }

//     void visit(Scoped &Node) override
//     {
//         if (Node.getSL())
//         {
//             for (auto &stmt : *Node.getSL())
//             {
//                 stmt->accept(*this);
//             }
//         }
//     }

//     void visit(Declaration &Node) override {}
// };
// }; // namespace

// void CodeGen::compile(AST *Tree)
// {
//     ToIRVisitor ToIR(M);
//     ToIR.run(Tree);
// }
