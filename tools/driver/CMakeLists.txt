set(LLVM_LINK_COMPONENTS Support)

add_llvm_executable(llshader LLShader.cpp)

set_target_properties(llshader PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                          ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(
  llshader PRIVATE llshaderBasic llshaderLexer llshaderParser llshaderSema
                   llshaderCodeGen)
