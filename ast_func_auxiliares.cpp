//
// Created by CARLOS URZUA on 11/24/2025.
//

#include "ast_analizador.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stack>


void AnalizadorAST::mostrarAST() {
    cout << "\n--- ARBOL AST ---\n";
    if (raizAST) imprimirAST(raizAST, 0);
    else cout << "AST vacio.\n";
}
//shared_ptr<Nodo> crearNodo(string valor, ParseType tipo, TipoLexema lexema);
shared_ptr<Nodo> AnalizadorAST::crearNodo(const string& valor, ParseType tipoNodo, TipoLexema lexema) {
    if (lexema == TipoLexema::PALABRA_RESERVADA) tipoNodo = ParseType::Reserved;
    else if (lexema == TipoLexema::OPERADOR) tipoNodo = ParseType::Op;
    else if (lexema == TipoLexema::IDENTIFICADOR) tipoNodo = ParseType::Id;
    else if (lexema == TipoLexema::CONSTANTE || lexema == TipoLexema::CADENA) tipoNodo = ParseType::Literal;
    return make_shared<Nodo>(valor, tipoNodo, lexema);
}

/*shared_ptr<Nodo> AnalizadorAST::crearNodo(const Token& token, ParseType tipoNodo) {
    TipoLexema tipo = token.tipo;
    string valor = token.valor_lexema;
    if (tipo == TipoLexema::PALABRA_RESERVADA) tipoNodo = ParseType::Reserved;
    else if (tipo == TipoLexema::OPERADOR) tipoNodo = ParseType::Op;
    else if (tipo == TipoLexema::IDENTIFICADOR) tipoNodo = ParseType::Id;
    else if (tipo == TipoLexema::CONSTANTE || tipo == TipoLexema::CADENA) tipoNodo = ParseType::Literal;
    return make_shared<Nodo>(valor, tipoNodo, tipo);
}*/

/*void AnalizadorAST::agregarNodoTerminal(const string& valor, TipoLexema tipo) {
    //auto nodo = crearNodo(valor, ParseType::Id, tipo);
    auto nodo = crearNodo(valor, ParseType::Id);
    // si es literal, almacenar el valor en campo valor
    if (tipo == TipoLexema::CONSTANTE || tipo == TipoLexema::CADENA) nodo->valor = valor;
    pilaAST.push(nodo);
}

void AnalizadorAST::agregarNodoTerminal(const Token& token) {
    //auto nodo = crearNodo(valor, ParseType::Id, tipo);
    auto nodo = crearNodo(token, ParseType::Id);
    // si es literal, almacenar el valor en campo valor
    if (token.tipo == TipoLexema::CONSTANTE || token.tipo == TipoLexema::CADENA) nodo->valor = token.valor_lexema;
    pilaAST.push(nodo);
}*/

int AnalizadorAST::precedencia(const string& op) {
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "==" || op == "!=") return 3;
    if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
    if (op == "+" || op == "-") return 5;
    if (op == "*" || op == "/" || op == "%") return 6;
    if (op == "=") return 0; // asignación más baja
    // asignar precedencia para unary 'u-'
    if (op == "u-") return 7;
    return -1;
}

bool AnalizadorAST::esOperador(const string& s) {
    static const vector<string> ops = {
        "+","-","*","/","%","<",">","<=",">=","==","!=","&&","||","!"
    };
    return find(ops.begin(), ops.end(), s) != ops.end();
}

void AnalizadorAST::imprimirAST(shared_ptr<Nodo> nodo, int nivel) {
    if (!nodo) return;
    cout << string(nivel * 2, ' ') << nodo->simbolo_sintactico;
    if (!nodo->valor.empty()) cout << " [" << nodo->valor << "]";
    cout << "\n";
    for (auto& h : nodo->hijos) imprimirAST(h, nivel + 1);
}

void print_instr(const vector<string>& instr) {
    for (size_t i = 0; i < instr.size(); ++i) {
        cout << instr[i] << "  ";
    }
}