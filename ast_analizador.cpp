//
// Created by CARLOS URZUA on 11/23/2025.
//

#include "ast_analizador.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stack>

using namespace std;

AnalizadorAST::AnalizadorAST() : pos(0), raizAST(nullptr) {}

// --- AYUDANTES DE NAVEGACIÓN ---

Token AnalizadorAST::actual() const {
    if (pos >= tokens.size()) return {TipoLexema::FIN, "", -1, ""};
    return tokens[pos];
}

Token AnalizadorAST::anterior() const {
    if (pos - 1 >= 0) return tokens[pos - 1];
    return {TipoLexema::ERROR, "", -1, ""};
}

void AnalizadorAST::avanzar() {
    if (pos < tokens.size()) pos++;
}

// Verifica si el token actual coincide con tipo y (opcionalmente) valor
bool AnalizadorAST::coincidir(TipoLexema tipo, string val) {
    if (fin()) return false;
    if (actual().tipo == tipo) {
        if (val == "" || actual().valor_lexema == val) {
            return true;
        }
    }
    return false;
}

// Consume un token esperado y avanza. Si no está, imprime error (o lanza excepción)
Token AnalizadorAST::consumir(TipoLexema tipo, string errorMsg) {
    if (coincidir(tipo)) {
        Token t = actual();
        if (pos + 1 < tokens.size()) avanzar();
        return t;
    }
    cout << "Error Sintactico: " << errorMsg << " en linea " << actual().id
         << ". Encontrado: " << actual().valor_lexema << endl;
    return {TipoLexema::ERROR, "", -1, ""};
}

bool AnalizadorAST::fin() const {
    return pos >= tokens.size();
}

// --- CORE DEL ANÁLISIS ---

bool AnalizadorAST::analizar(const vector<Token>& tokensEntrada) {
    //cout << "\n\nIniciando analisis con " << tokensEntrada.size() << " tokens." << endl;
    if (tokensEntrada.empty()) return false;
    this->tokens = tokensEntrada;
    this->pos = 0;

    tokens.erase(
    remove_if(tokens.begin(), tokens.end(),
        [](const Token& t) {
            return t.tipo == TipoLexema::COMENTARIO;
        }),
    tokens.end()
);

    // Creamos un nodo raíz que representa el programa entero
    auto programa = make_shared<Nodo>("Programa", ParseType::Program, TipoLexema::SCOPE);

    while (!fin()) {
        // Saltamos punto y coma sueltos si los hubiera
        if (coincidir(TipoLexema::PUNTO_COMA)) {
            avanzar();
            continue;
        }

        // Parseamos una sentencia a la vez
        auto stmt = statement();
        if (stmt) {
            programa->hijos.push_back(stmt);
        } else {
            // Si retorna null y no es fin, avanzamos para evitar bucle infinito en error
            if(!fin()) avanzar();
        }
    }

    this->raizAST = programa;
    return true;
}

// El "Dispatcher": decide qué función llamar según el token actual
shared_ptr<Nodo> AnalizadorAST::statement() {
    Token t = actual();

    // 1. Bloque de código { ... }
    if (t.tipo == TipoLexema::LLAVE_ABIERTA) {
        return statementBlock();
    }

    // 2. Palabras reservadas (Control de flujo)
    if (t.tipo == TipoLexema::PALABRA_RESERVADA) {
        if (t.valor_lexema == "si") return statementIf();
        if (t.valor_lexema == "mientras") return statementWhile();
        if (t.valor_lexema == "para") return statementFor();
        if (t.valor_lexema == "imprimir") return statementPrint();
        // agregar "retornar", etc.
    }

    // 3. Si no es control, debe ser una expresión o asignación seguida de ;
    //    Ej: a = 5;  o  funcion();
    //    Aquí usamos tu lógica de expresiones, pero detectamos el límite (;)

    int inicioExpr = pos;
    // Buscamos el punto y coma
    while(pos < tokens.size() && tokens[pos].tipo != TipoLexema::PUNTO_COMA) {
        pos++;
    }
    int finExpr = pos; // El índice del punto y coma

    if (inicioExpr == finExpr) { // Sentencia vacía
        avanzar(); // consumir ;
        return nullptr;
    }

    // Construimos la expresión con el rango detectado
    auto nodoExpr = construirExpresionShuntingYard(inicioExpr, finExpr);
    consumir(TipoLexema::PUNTO_COMA, "Se esperaba ';' (1)");
    //avanzar();
    return nodoExpr;
}

// --- IMPLEMENTACIÓN DE LOS NODOS DE CONTROL ---

// Manejo de SI (IF) y SI_NO (ELSE)
shared_ptr<Nodo> AnalizadorAST::statementIf() {
    avanzar(); // Consumir 'si'

    auto nodoIf = make_shared<Nodo>("si", ParseType::Control, TipoLexema::PALABRA_RESERVADA);

    consumir(TipoLexema::PAR_ABIERTO, "Se esperaba '(' despues de si");

    // Parsear condición (buscamos hasta el cierre de paréntesis)
    int inicioCond = pos;
    int balance = 1;
    while(pos < tokens.size() && balance > 0) {
        if(tokens[pos].tipo == TipoLexema::PAR_ABIERTO) balance++;
        if(tokens[pos].tipo == TipoLexema::PAR_CIERRE) balance--;
        if(balance > 0) pos++;
    }
    auto nodoCond = construirExpresionShuntingYard(inicioCond, pos);
    nodoIf->hijos.push_back(nodoCond); // Hijo 0: Condición

    consumir(TipoLexema::PAR_CIERRE, "Se esperaba ')'");

    // Parsear bloque 'entonces'
    auto bloqueThen = statement(); // Generalmente será un statementBlock {...}
    nodoIf->hijos.push_back(bloqueThen); // Hijo 1: Bloque True

    // Revisar si existe 'si_no'
    if (coincidir(TipoLexema::PALABRA_RESERVADA, "si_no")) {
        avanzar(); // Consumir 'si_no'
        auto bloqueElse = statement();
        nodoIf->hijos.push_back(bloqueElse); // Hijo 2: Bloque Else
    }

    return nodoIf;
}

// Manejo de MIENTRAS (WHILE)
shared_ptr<Nodo> AnalizadorAST::statementWhile() {
    avanzar(); // Consumir 'mientras'
    auto nodoWhile = make_shared<Nodo>("mientras", ParseType::Control, TipoLexema::PALABRA_RESERVADA);

    consumir(TipoLexema::PAR_ABIERTO, "Se esperaba '('");

    // Lógica para aislar la condición (igual que en IF)
    int inicioCond = pos;
    int balance = 1;
    while(pos < tokens.size() && balance > 0) {
        if(tokens[pos].tipo == TipoLexema::PAR_ABIERTO) balance++;
        if(tokens[pos].tipo == TipoLexema::PAR_CIERRE) balance--;
        if(balance > 0) pos++;
    }
    auto nodoCond = construirExpresionShuntingYard(inicioCond, pos);
    nodoWhile->hijos.push_back(nodoCond);

    consumir(TipoLexema::PAR_CIERRE, "Se esperaba ')'");

    auto bloque = statement();
    nodoWhile->hijos.push_back(bloque);

    return nodoWhile;
}

// Manejo de PARA (FOR)
// Estructura: para ( inicio ; condicion ; incremento ) { cuerpo }
shared_ptr<Nodo> AnalizadorAST::statementFor() {
    avanzar(); // Consumir 'para'
    auto nodoFor = make_shared<Nodo>("para", ParseType::Control, TipoLexema::PALABRA_RESERVADA);

    consumir(TipoLexema::PAR_ABIERTO, "Se esperaba '('");

    // 1. Inicialización (ej: int i = 0)
    // Buscamos hasta el primer punto y coma
    int inicioInit = pos;
    while(pos < tokens.size() && tokens[pos].tipo != TipoLexema::PUNTO_COMA) pos++;
    auto nodoInit = construirExpresionShuntingYard(inicioInit, pos);
    if (nodoInit) nodoFor->hijos.push_back(nodoInit);

    consumir(TipoLexema::PUNTO_COMA, "Se esperaba ';'");

    // 2. Condición (ej: i < 10)
    int inicioCond = pos;
    while(pos < tokens.size() && tokens[pos].tipo != TipoLexema::PUNTO_COMA) pos++;
    auto nodoCond = construirExpresionShuntingYard(inicioCond, pos);
    nodoFor->hijos.push_back(nodoCond);

    consumir(TipoLexema::PUNTO_COMA, "Se esperaba ';' (2)");

    // 3. Incremento (ej: i = i + 1) NO termina en punto y coma, termina en ')'
    int inicioInc = pos;
    int balance = 1; // Ya estamos dentro de un paréntesis del for
    while(pos < tokens.size()) {
        if(tokens[pos].tipo == TipoLexema::PAR_ABIERTO) balance++;
        if(tokens[pos].tipo == TipoLexema::PAR_CIERRE) {
            balance--;
            if(balance == 0) break; // Encontramos el cierre del for
        }
        pos++;
    }
    auto nodoInc = construirExpresionShuntingYard(inicioInc, pos);
    nodoFor->hijos.push_back(nodoInc);

    consumir(TipoLexema::PAR_CIERRE, "Se esperaba ')' al final del for");

    // 4. Cuerpo del bucle
    auto cuerpo = statement();
    nodoFor->hijos.push_back(cuerpo);

    return nodoFor;
}

// Manejo de BLOQUES { ... }
shared_ptr<Nodo> AnalizadorAST::statementBlock() {
    consumir(TipoLexema::LLAVE_ABIERTA, "Se esperaba '{'");

    auto nodoBloque = make_shared<Nodo>("Bloque", ParseType::Scope, TipoLexema::SCOPE);

    // Mientras no encontremos la llave de cierre o el fin del archivo
    while (!fin() && actual().tipo != TipoLexema::LLAVE_CERRADA) {
        auto stmt = statement(); // Llamada recursiva
        if(stmt) {
            nodoBloque->hijos.push_back(stmt);
        }
    }

    consumir(TipoLexema::LLAVE_CERRADA, "Se esperaba '}'");
    return nodoBloque;
}

// Manejo de IMPRIMIR
shared_ptr<Nodo> AnalizadorAST::statementPrint() {
    //cout<<"\nSTATEMENT PRINT\n\n";
    avanzar(); // consumir 'imprimir'
    auto nodoPrint = make_shared<Nodo>("imprimir", ParseType::Function, TipoLexema::PALABRA_RESERVADA);

    // Permitir paréntesis opcionales o expresión directa
    bool tienePar = false;
    if(coincidir(TipoLexema::PAR_ABIERTO)) {
        avanzar();
        tienePar = true;
    }

    // Buscamos fin de expresión (un ; o un ) )
    int inicioExpr = pos;
    while(pos < tokens.size() &&
          tokens[pos].tipo != TipoLexema::PUNTO_COMA &&
          tokens[pos].tipo != TipoLexema::PAR_CIERRE) {
        pos++;
    }

    auto expr = construirExpresionShuntingYard(inicioExpr, pos);
    nodoPrint->hijos.push_back(expr);

    if(tienePar) consumir(TipoLexema::PAR_CIERRE, "Se esperaba ')'");
    consumir(TipoLexema::PUNTO_COMA, "Se esperaba ';' (3)");

    return nodoPrint;
}

// Reutilizamos tu lógica de Shunting Yard, pero encapsulada
//METODO ENVOLTORIO WRAPPER
shared_ptr<Nodo> AnalizadorAST::construirExpresionShuntingYard(int start, int end) {
    if (start >= end) return nullptr; // Protección contra rangos vacíos
    // Aquí copias tu lógica anterior de 'construirExpresion'.
    // Es importante que esa lógica NO use 'pos' global, sino los índices 'start' y 'end' pasados.
    // ... Tu implementación existente de pilas operands/ops ...

    try {
        return construirExpresion(this->tokens, start, end); // Llamada a tu función original
    } catch (exception e) {
        cout<<"ERROR IDENTIFICADO EN METODO construirExpresionShutingYard\n";
        cout<<e.what()<<"\n";
    }

    return nullptr;

}
