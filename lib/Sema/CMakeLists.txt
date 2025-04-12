add_library(llshaderSema Sema.cpp)

target_link_libraries(llshaderSema PRIVATE LLVMCore LLVMSupport)

target_include_directories(llshaderSema PRIVATE ${LLVM_INCLUDE_DIRS})
