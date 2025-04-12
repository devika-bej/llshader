add_library(llshaderParser Parser.cpp)

target_link_libraries(llshaderParser PRIVATE LLVMCore LLVMSupport llshaderBasic)

target_include_directories(llshaderParser PRIVATE ${LLVM_INCLUDE_DIRS})
