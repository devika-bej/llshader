add_library(llshaderBasic Diagnostic.cpp TokenKinds.cpp)

target_link_libraries(llshaderBasic PRIVATE LLVMCore LLVMSupport)

target_include_directories(llshaderBasic PRIVATE ${LLVM_INCLUDE_DIRS})
