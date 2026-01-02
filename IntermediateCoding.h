//
// Created by CARLOS URZUA on 10/12/2025.
//

#ifndef COMPILADOR_INTERMEDIATECODING_H
#define COMPILADOR_INTERMEDIATECODING_H

#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "astree.h"
#include "tokens.h"

using namespace std;

struct Cuadruplo {
    string op;
    string arg1;
    string arg2;
    string resultado;
};

class IntermediateCoding {
private:
    vector<Cuadruplo> cuadruplos;
    int tempCount = 1;
    int labelCount = 1;

    string generar(shared_ptr<Nodo> nodo);
    string nuevaTemporal();
    string generarTemporal() { return "t" + to_string(tempCount++); }
    string generarEtiqueta() { return "L" + to_string(labelCount++); }
    string newTemp();
    string newLabel();
    // auxiliares recursivos
    string generarExpr(const shared_ptr<Nodo>& nodo);   // devuelve temporal o nombre
    void generarStmt(const shared_ptr<Nodo>& nodo);     // genera cuádruplos para statements (no retorna)
    void generarBlock(const shared_ptr<Nodo>& nodo);    // genera secuencia de statements
    void imprimirCuadruplos();

public:
    IntermediateCoding() = default;
    string generarDesdeAST(const shared_ptr<Nodo>& nodo);
    const vector<Cuadruplo>& getCuadruplos() const { return cuadruplos; }
    void mostrarCuadruplos() const;

    void mostrarCuadruplos(vector<Cuadruplo> cuad) const;

    void reset() { cuadruplos.clear(); tempCount = 1; labelCount = 1; }
};

#endif //COMPILADOR_INTERMEDIATECODING_H