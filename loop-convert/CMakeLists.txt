# taken from https://clang.llvm.org/docs/LibASTMatchersTutorial.html

set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert
  LoopConvert.cpp
  )

# NOTE: without PRIVATE, cmake fails with this error message:
#   CMake Error at tools/clang/tools/extra/loop-convert/CMakeLists.txt:7 ...
#    ... (target_link_libraries):
#   The keyword signature for target_link_libraries has already been used with
#   the target "loop-convert".  All uses of target_link_libraries with a target
#   must be either all-keyword or all-plain.
#
#   The uses of the keyword signature are here:
#
#    * cmake/modules/LLVM-Config.cmake:105 (target_link_libraries)
#
# --> The official LLVM docs are buggy :-(
# see https://stackoverflow.com/a/47738097
target_link_libraries(loop-convert PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )
