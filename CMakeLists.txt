cmake_minimum_required(VERSION 4.0)
project(COMPILADOR)

set(CMAKE_CXX_STANDARD 17)

add_executable(COMPILADOR main.cpp
        lexer.cpp
        tokens.h
        lexer.h
        symbol_table.h
        symbol_table.cpp
        sintaxis.cpp
        sintaxis.h
        astree.h
        ast_scopes.cpp
        scopes.h
        semantic.h
        semantic.cpp
        recorridos.cpp
        ast_analizador.cpp
        ast_analizador.h
        IntermediateCoding.cpp
        IntermediateCoding.h
        generador_asm.h
        assembler_driver.h
        generador_asm.cpp
        ast_construir_exp.cpp
        ast_func_auxiliares.cpp

)
