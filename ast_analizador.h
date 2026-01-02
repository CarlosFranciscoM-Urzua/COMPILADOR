//
// Created by CARLOS URZUA on 11/23/2025.
//

#ifndef COMPILADOR_ANALIZADORAST_H
#define COMPILADOR_ANALIZADORAST_H

#pragma once
#include <vector>
#include <string>
#include <memory>
#include "astree.h"

using namespace std;

class AnalizadorAST {
private:
    vector<Token> tokens;
    int pos; // Posición actual en el vector

    // --- Ayudantes de Navegación ---
    Token actual() const;           // Retorna token actual
    Token anterior() const;         // Retorna token anterior
    bool coincidir(TipoLexema tipo, string val = ""); // Verifica si el actual es X
    Token consumir(TipoLexema tipo, string errorMsg); // Espera X y avanza, o lanza error
    void avanzar();                 // Simplemente avanza id++
    bool fin() const;

    // --- Funciones de Análisis (Gramática) ---
    shared_ptr<Nodo> statement();       // Decide qué tipo de sentencia es (if, while, block, expr)
    shared_ptr<Nodo> statementIf();     // Maneja si / si_no
    shared_ptr<Nodo> statementWhile();  // Maneja mientras
    shared_ptr<Nodo> statementFor();    // Maneja para
    shared_ptr<Nodo> statementBlock();  // Maneja { ... }
    shared_ptr<Nodo> statementPrint();  // Maneja imprimir

    // Tu función de expresiones existente (ligeramente adaptada)
    shared_ptr<Nodo> parseExpression();

    // Métodos auxiliares de creación
    //shared_ptr<Nodo> crearNodo(string valor, ParseType tipoNodo, TipoLexema lexema);
    shared_ptr<Nodo> crearNodo(const string& valor, ParseType tipoNodo, TipoLexema lexema);

    // Métodos privados de tu lógica de precedencia (Shunting yard)
    int precedencia(const string& op);
    bool esOperador(const string &s);
    shared_ptr<Nodo> construirExpresionShuntingYard(int start, int end);

public:
    shared_ptr<Nodo> raizAST;
    AnalizadorAST();
    bool analizar(const vector<Token>& tokensEntrada);
    void mostrarAST();
    void imprimirAST(shared_ptr<Nodo> nodo, int nivel);
    shared_ptr<Nodo> construirExpresion(const vector<Token> &tokens, size_t start, size_t end);
};

#endif //COMPILADOR_ANALIZADORAST_H