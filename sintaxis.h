#ifndef SINTAXIS_H
#define SINTAXIS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <map>
#include <vector>
#include <string>
#include <iomanip>
#include <memory>
#include <cstdlib>
#include <unistd.h>
#include "astree.h"
#include "tokens.h"

using namespace std;

// Producción: lado derecho de una regla
typedef vector<string> Produccion;

class Parser {
private:
	// Tabla de derivaciones LL(1)
	map<string, map<string, Produccion>> tabla;

	// Tokens cargados y en uso
	vector<Token> tokens;
	vector<Token> instruccion;
	int idx = 0;
	int last_token = 0;
	vector<vector<Token>> instrucciones;

public:
	Parser();

	// Utilidades de tokens
	string siguiente_token();
	string eliminarEspacios(const string& cadena);

	// Manejo de instrucciones
	string imprimir_instruccion(vector<Token> instruccion);
	string imprimir_instruccion(vector<string> instruccion);
	vector<string> instruccion_strings(vector<Token> instruccion);
	vector<Token> cargar_siguiente_instruccion();

	// Utilidades de pila
	string printStack(stack<string> pila1);

	// Tabla LL(1)
	void cargar_tabla();

	// Entrada de tokens
	vector<Token> cargarTokenInfos(const string& nombreArchivo);
	void setTokenInfo(vector<Token>& flujoTokens);
	vector<Token> getTokens();

	// Parser con árbol
	shared_ptr<Nodo> parser_tree(const vector<string>& tokens, vector<Token> instruccion);
	void imprimir_arbol(const shared_ptr<Nodo>& nodo, int nivel = 0);
	void printAST(const shared_ptr<Nodo>& node, string prefix = "", bool isLast = true);

	// Parser con tabla LL(1)
	void parser(vector<Token> instruccion);

	// Setter para tokens globales
	void setTokens(const vector<Token>& tks) { tokens = tks; }

	shared_ptr<Nodo> execute_sintax();
	vector<vector<Token>> cargar_instrucciones();

	vector<vector<Token>> getInstrucciones();

};

#endif
