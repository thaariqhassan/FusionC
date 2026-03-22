cmake_minimum_required(VERSION 3.16)
project(FusionC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(fusionc
	main.cpp
	fusionc_cli/cli_parser.cpp
	core/language_loader.cpp
	core/compiler_controller.cpp
	frontend/lexer/token.cpp
	frontend/parser/ast.cpp
	frontend/semantic/symbol_table.cpp
	middleend/ir/three_address_code.cpp
	middleend/optimizer/optimizer.cpp
	backend/codegen/machine_codegen.cpp
)

target_include_directories(fusionc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
