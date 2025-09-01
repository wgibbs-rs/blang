
cmake_minimum_required(VERSION 3.10)
project(blang C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


set(CMAKE_BUILD_TYPE Debug)

if (APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/Flex")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/Bison")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/llvm")
endif()


find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_DIR}")

# Generate Bison parser
BISON_TARGET(Parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)

# Generate Flex lexer, using Bison's generated header
FLEX_TARGET(Lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

# Make Flex depend on Bison (so y.tab.h / parser.h is generated first)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

if (APPLE)
    llvm_map_components_to_libnames(llvm_libs
        Core Support Target
        ARM AVR Hexagon MSP430 Mips PowerPC RISCV WebAssembly X86 XCore VE
        SystemZ Sparc SPIRV NVPTX LoongArch Lanai BPF AArch64 AMDGPU
    )
else()
    llvm_map_components_to_libnames(llvm_libs
        Core Support Target
        ARM AVR Hexagon MSP430 Mips PowerPC RISCV WebAssembly X86 XCore VE
        SystemZ Sparc NVPTX LoongArch Lanai BPF AArch64 AMDGPU
    )
endif()

# Include directories for generated headers
include_directories(${CMAKE_CURRENT_BINARY_DIR} include)

# Your other source files (if any)
file(
    GLOB_RECURSE
    SOURCES 
    "src/main.c"
    "src/error.c"
    "src/ast.c"
    "src/binary.cpp"
    "src/llvm_ir.cpp"
    "src/optimize.cpp"
)

# Remove raw lexer and parser source files if they're listed by GLOB
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/lexing.l")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y")

# Add executable BEFORE applying target-specific commands
add_executable(blang
    ${SOURCES}
    ${FLEX_Lexer_OUTPUTS}
    ${BISON_Parser_OUTPUTS}
)

# Now target-specific commands
target_include_directories(blang PRIVATE ${LLVM_INCLUDE_DIRS})
target_include_directories(blang PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(blang PRIVATE ${LLVM_DEFINITIONS})
target_compile_options(blang PRIVATE -fno-rtti)
target_link_libraries(blang PRIVATE ${llvm_libs} ${FLEX_LIBRARIES})