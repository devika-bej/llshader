add_library(llshaderCodeGen CodeGen.cpp)

target_link_libraries(llshaderCodeGen PRIVATE LLVMCore LLVMSupport)

target_include_directories(llshaderCodeGen PRIVATE ${LLVM_INCLUDE_DIRS})
