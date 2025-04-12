add_library(llshaderLexer Lexer.cpp)

target_link_libraries(llshaderLexer PRIVATE LLVMCore LLVMSupport llshaderBasic)

target_include_directories(llshaderLexer PRIVATE ${LLVM_INCLUDE_DIRS})
