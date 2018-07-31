cmake_minimum_required(VERSION 3.8)

set(project_name cppstepin)
set(header_patterns *.h *.hpp)
set(source_patterns *.c *.cc *.cpp *.cxx)
set(configuration Debug)

#Root directory where LLVM is installed
set(LLVM D:\\llvm)

#Directory where LLVM libraries are located
set(LLVM_lib "${LLVM}\\build\\$(Configuration)\\lib")

set(LLVM_include ${LLVM}\\include ${LLVM}\\tools\\clang\\include ${LLVM}\\build\\include ${LLVM}\\build\\tools\\clang\\include)

set(link_lib LLVMBinaryFormat LLVMSupport LLVMOption LLVMBitReader LLVMMC LLVMDebugInfoPDB LLVMMCParser LLVMProfileData LLVMCore clangTooling clangFrontend clangAST clangBasic clangDriver clangLex clangSema clangSerialization clangParse clangEdit clangAnalysis clangRewrite version) 

project(${project_name}) 


list(APPEND source_patterns ${header_patterns} ${source_patterns})

macro(enumerate_files_for_humans root_dir patterns output)
  foreach(pattern ${patterns})
    file(GLOB files "${pattern}")
    list(APPEND ${output} ${files})
  endforeach()
endmacro()

enumerate_files_for_humans($(project_name) "${source_patterns}" source_files)
enumerate_files_for_humans($(project_name) "${header_patterns}" header_files)

include_directories(${LLVM_include})
link_directories(${LLVM_lib})

add_executable(${project_name} ${source_files} ${header_files})
target_link_libraries(${project_name} ${link_lib})
