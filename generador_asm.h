//
// Created by CARLOS URZUA on 10/16/2025.
//

#ifndef COMPILADOR_GENERADOR_ASM_H
#define COMPILADOR_GENERADOR_ASM_H

#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "IntermediateCoding.h" // donde está definido struct Cuadruplo

using namespace std;

class GeneradorASM {
public:
    static string generarASM(const vector<Cuadruplo>& cuadruplos);
    static void generar_archivo (string content, string nombre_archivo, string extension, string route);
};


#endif //COMPILADOR_GENERADOR_ASM_H