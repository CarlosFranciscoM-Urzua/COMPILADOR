//
// Created by CARLOS URZUA on 11/24/2025.
//

#include "ast_analizador.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stack>

/*shared_ptr<Nodo> AnalizadorAST::construirExpresion(const vector<Token>& tokens, size_t start, size_t end) {
    stack<shared_ptr<Nodo>> operands;
    stack<string> ops;

    auto aplicarOperador = [&](const string& op){
        if (op == "u-" || op == "!") {
            if (operands.empty()) return;
            auto a = operands.top(); operands.pop();
            auto nodo = crearNodo(op, ParseType::Op, TipoLexema::OPERADOR);
            agregarHijo(nodo, a);
            operands.push(nodo);
        } else {
            if (operands.size() < 2) return;
            auto rhs = operands.top(); operands.pop();
            auto lhs = operands.top(); operands.pop();
            auto nodo = crearNodo(op, ParseType::Op, TipoLexema::OPERADOR);
            agregarHijo(nodo, lhs);
            agregarHijo(nodo, rhs);
            operands.push(nodo);
        }
    };

    for (size_t i = start; i < end; ++i) {
        const Token& tok = tokens[i];
        const string t = tok.valor_lexema;

        // --- Paréntesis ---
        if (t == "(") {
            //ops.push("(");
            //continue;
            int nivel = 1;
            size_t j = i + 1;
            while (j < end && nivel > 0) {
                if (tokens[j].tipo == TipoLexema::PAR_ABIERTO) nivel++;
                else if (tokens[j].tipo == TipoLexema::PAR_CIERRE) nivel--;
                if (nivel > 0) ++j;
            }
            if (j > i + 1) {
                auto nodoSub = construirExpresion(tokens, i + 1, j - 1);
                if (nodoSub) operands.push(nodoSub);
            }
            i = j;
            continue;
        }
        if (t == ")") {
            while (!ops.empty() && ops.top() != "(") {
                string op = ops.top(); ops.pop();
                aplicarOperador(op);
            }
            if (!ops.empty() && ops.top() == "(") ops.pop();
            continue;
        }

        // --- Literales ---
        if (tok.tipo == TipoLexema::CADENA ||
            tok.tipo == TipoLexema::CONSTANTE ||
            tok.tipo == TipoLexema::ENTERO ||
            tok.tipo == TipoLexema::FLOTANTE ||
            tok.tipo == TipoLexema::BOOLEANO ||
            tok.tipo == TipoLexema::CARACTER) {

            auto nodo = crearNodo(t, ParseType::Literal, tok.tipo);
            nodo->valor = t;
            operands.push(nodo);
            continue;
        }

        // --- Identificador ---
        if (tok.tipo == TipoLexema::IDENTIFICADOR) {
            auto nodo = crearNodo(t, ParseType::Id, TipoLexema::IDENTIFICADOR);
            nodo->valor = t;
            operands.push(nodo);
            continue;
        }

        // --- Operadores ---
        if (tok.tipo == TipoLexema::OPERADOR || esOperador(t)) {
            int p = precedencia(t);
            if (p >= 0) {
                // Detectar unario
                if (t == "-" || t == "!") {
                    bool prevIsOperand = false;
                    if (i > start) {
                        const Token& prevTok = tokens[i-1];
                        if (prevTok.tipo == TipoLexema::CADENA ||
                            prevTok.tipo == TipoLexema::CONSTANTE ||
                            prevTok.tipo == TipoLexema::IDENTIFICADOR ||
                            prevTok.valor_lexema == ")") {
                            prevIsOperand = true;
                        }
                    }
                    if (!prevIsOperand) {
                        while (!ops.empty() && precedencia(ops.top()) >= precedencia("u-")) {
                            string op = ops.top(); ops.pop();
                            aplicarOperador(op);
                        }
                        ops.push("u-");
                        continue;
                    }
                }

                // Operador binario normal
                while (!ops.empty() && ops.top() != "(" && precedencia(ops.top()) >= p) {
                    string op = ops.top(); ops.pop();
                    aplicarOperador(op);
                }
                ops.push(t);
                continue;
            }
        }
    }

    // --- Aplicar operadores restantes ---
    while (!ops.empty()) {
        string op = ops.top(); ops.pop();
        if (op == "(" || op == ")") continue;
        aplicarOperador(op);
    }

    if (!operands.empty()) return operands.top();
    return nullptr;
}*/


// Versión corregida y segura de construirExpresion
shared_ptr<Nodo> AnalizadorAST::construirExpresion(const vector<Token>& tokens, size_t start, size_t end) {
    stack<shared_ptr<Nodo>> operands;
    stack<string> ops;

    // Lambda para procesar operaciones. Agregamos validaciones de seguridad.
    auto aplicarOperador = [&](const string& op){
        if (op == "u-" || op == "!") {
            if (operands.empty()) return; // EVITA CRASH si pila vacía
            auto a = operands.top(); operands.pop();
            auto nodo = crearNodo(op, ParseType::Op, TipoLexema::OPERADOR);
            // Validar que 'a' no sea null antes de agregarlo
            if(a) nodo->hijos.push_back(a);
            operands.push(nodo);
        } else {
            if (operands.size() < 2) return; // EVITA CRASH si faltan operandos
            auto rhs = operands.top(); operands.pop();
            auto lhs = operands.top(); operands.pop();
            auto nodo = crearNodo(op, ParseType::Op, TipoLexema::OPERADOR);

            // Validar hijos
            if(lhs) nodo->hijos.push_back(lhs);
            if(rhs) nodo->hijos.push_back(rhs);

            operands.push(nodo);
        }
    };

    for (size_t i = start; i < end && i < tokens.size(); ++i) {
        // PROTECCIÓN CRÍTICA: Asegurar que i está dentro del vector real
        if (i >= tokens.size()) break;

        const Token& tok = tokens[i];
        const string t = tok.valor_lexema;

        // --- Paréntesis ---
        if (t == "(") {
            int nivel = 1;
            size_t j = i + 1;
            while (j < end && nivel > 0) {
                if (j >= tokens.size()) break; // Protección bounds
                if (tokens[j].tipo == TipoLexema::PAR_ABIERTO) nivel++;
                else if (tokens[j].tipo == TipoLexema::PAR_CIERRE) nivel--;
                if (nivel > 0) ++j;
            }
            if (j > i + 1 && j <= end) { // Asegurar rango válido
                auto nodoSub = construirExpresion(tokens, i + 1, j - 1); // j-1 para ignorar ')'
                if (nodoSub) operands.push(nodoSub);
            }
            i = j; // Saltar todo el paréntesis (el for hará ++i, saltando el ')')
            continue; // El for hace ++i, así que el siguiente ciclo es j+1. Correcto si i=j apunta a ')'
        }

        if (t == ")") {
            while (!ops.empty() && ops.top() != "(") {
                string op = ops.top(); ops.pop();
                aplicarOperador(op);
            }
            if (!ops.empty() && ops.top() == "(") ops.pop();
            continue;
        }

        // --- Operandos (Literales e Identificadores) ---
        if (tok.tipo == TipoLexema::CADENA || tok.tipo == TipoLexema::CONSTANTE ||
            tok.tipo == TipoLexema::ENTERO || tok.tipo == TipoLexema::FLOTANTE ||
            tok.tipo == TipoLexema::BOOLEANO || tok.tipo == TipoLexema::CARACTER) {
            auto nodo = crearNodo(t, ParseType::Literal, tok.tipo);
            nodo->valor = t;
            operands.push(nodo);
            continue;
        }

        if (tok.tipo == TipoLexema::IDENTIFICADOR) {
            auto nodo = crearNodo(t, ParseType::Id, TipoLexema::IDENTIFICADOR);
            nodo->valor = t;
            operands.push(nodo);
            continue;
        }

        // --- Operadores ---
        if (tok.tipo == TipoLexema::OPERADOR || esOperador(t)) {
            // DETECCIÓN DE UNARIO (Revisión de límites segura)
            if (t == "-" || t == "!") {
                bool esUnario = false;
                if (i == start) { // Si es el inicio del rango, es unario
                    esUnario = true;
                } else if (i > start) {
                    // Mirar atrás de forma segura
                    const Token& prevTok = tokens[i - 1];
                    if (prevTok.tipo == TipoLexema::OPERADOR ||
                        prevTok.valor_lexema == "(" ||
                        prevTok.valor_lexema == "=" ||
                        esOperador(prevTok.valor_lexema)) {
                        esUnario = true;
                    }
                }

                if (esUnario) {
                    ops.push("u-");
                    continue;
                }
            }

            // Operador binario normal
            int p = precedencia(t);
            while (!ops.empty() && ops.top() != "(" && precedencia(ops.top()) >= p) {
                string op = ops.top(); ops.pop();
                aplicarOperador(op);
            }
            ops.push(t);
        }
    }

    // Vaciar operadores restantes
    while (!ops.empty()) {
        string op = ops.top(); ops.pop();
        if (op == "(" || op == ")") continue; // Protección contra paréntesis desbalanceados
        aplicarOperador(op);
    }

    if (!operands.empty()) return operands.top();
    return nullptr;
}