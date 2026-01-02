//
// Created by CARLOS URZUA on 9/21/2025.
//

#ifndef COMPILADOR_ASTREE_H
#define COMPILADOR_ASTREE_H

#include <iostream>//ENTRADA Y SALIDA
#include <fstream>// FILE STRAM
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iomanip>//setw
#include <memory>  //NECESARIA PARA LOS NODOS, INCLUYE SHARED_PTR

#include "tokens.h"

using namespace std;

enum class ParseType {
    Program,
    Function,
    Statement,
    Scope,
    Expression,
    Conditional,
    Loop,
    Id,
    Op,
    Literal,
    Gram,
    Tipo,
    Reserved,
    Assign, Control
};

//ESTRUCTURA NODO, BASE PARA PODER GENERAR EL ARBOL
struct Nodo {
    string simbolo_sintactico;
    ParseType type;
    int id;
    string tipo_dato;
    string string_token;
    TipoLexema token;
    string valor; // para literales numéricas/booleanas como "100.01" o "VERDADERO"
    vector<shared_ptr<Nodo>> hijos;
    //Nodo* father;
    weak_ptr<Nodo> father;
    Nodo(string s, ParseType t = ParseType::Statement, TipoLexema tp = TipoLexema::IDENTIFICADOR, int id2 = 0, string val = "") :
    simbolo_sintactico(s),
    type(t),
    token(tp),
    id(id2),
    valor(val){}
};

string nodeTypeToString(ParseType t);


// -----------------------------
// Funciones auxiliares
inline shared_ptr<Nodo> crearNodo(
    const string& simbolo,
    ParseType tipoNodo,
    TipoLexema token = TipoLexema::IDENTIFICADOR,
    int id = 0,
    const string& val = ""
) {
    return make_shared<Nodo>(simbolo, tipoNodo, token, id, val);
}

inline void agregarHijo(const shared_ptr<Nodo>& padre, const shared_ptr<Nodo>& hijo) {
    if (padre && hijo) {//VERIFICAR QUE NINGUNO SEA NULL
        padre->hijos.push_back(hijo);
        hijo->father = padre;
    }
}

// Impresión del árbol AST
inline void imprimirAST(const shared_ptr<Nodo>& nodo, int nivel = 0) {
    if (!nodo) return;
    cout << string(nivel * 2, ' ') << nodo->simbolo_sintactico;
    if (!nodo->valor.empty()) cout << " (" << nodo->valor << ")";
    cout << endl;
    for (auto& h : nodo->hijos)
        imprimirAST(h, nivel + 1);
}



#endif //COMPILADOR_ASTREE_H