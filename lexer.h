//
// Created by CARLOS URZUA on 9/21/2025.
//



#ifndef COMPILADOR_LEXER_H
#define COMPILADOR_LEXER_H

#include <iostream>//ENTRADA Y SALIDA
#include <fstream>// FILE STRAM
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iomanip>//setw

#include "symbol_table.h"
#include "tokens.h"

using namespace std;

// Clase del analizador léxico
class AnalizadorLexico {
	public:
		bool declarando;
		AnalizadorLexico(const string& entrada, SymbolTable& table1);
		void cerrarArchivoTokens();
		Token siguienteToken();

		void imprimirToken(const Token& token);
	    //const vector<TokenInfo>& getTablaTokens() const;

	 //    // Función que convierte un Token en su representación como cadena de texto
		// string tokenToString(Token tipo);
	 //    void imprimirTablaTokens();

		// Función para limpiar un archivo fuente:
		// Elimina tabulaciones, saltos de línea, comentarios de línea (//) y de bloque (/* */)
		// Luego guarda el contenido limpio en un nuevo archivo
		void limpiarArchivoFuente();

		vector<Token> flujo_tokens();

		// Guarda la tabla de tokens en un archivo
		// void guardarTablaTokensEnArchivo(const string& nombreArchivo);

		//AL MOMENTO DE CERRAR OBJETO
		~AnalizadorLexico();

	private:
		SymbolTable* table;
		ofstream archivoTokens; // archivo de salida a nivel de clase
	    string entrada;
	    size_t pos;
	    int lineaActual = 0;
		vector<Token> flujoTokens;

	    const unordered_set<string> palabrasReservadas = {
	        "entero",
			"flotante",
			"caracter",
			"cadena",
			"booleano",
			"VERDADERO",
			"FALSO",
	        "funcion",
			"fin_funcion",
			"si",
			"fin_si",
			"si_no",
			"fin_sino",
	        "para",
			"fin_para",
			"mientras",
			"fin_mientras",
			"hacer",
			"fin_hacer",
	        "principal",
			"fin_principal",
			"cortar",
			"saltar",
			"retornar",
			"imprimir",
			"declaracion",
			"fin_declaracion",
			"arc",
			"leer_cadena",
			"leer_entero",
			"leer_flotante",
			"leer_caracter",
	    	"VERDADERO",
	    	"FALSO"
	    };

	    unordered_set<string> operadores = {
			"+", "-","*","/","<",">","=","==","++","--","<=",">=","!","!=","*=","/=","^^","^"
		};

	    int idGeneral = 0;
		int idIdentificador = 100;
		int idNumero = 300;
		int idCaracter = 500;
		int idCadena = 600;

		int obtenerNuevoId(TipoLexema tipo);
	    void imprimirToken(const Token& token, int linea);
		//AUTOMATA PARA NUMEROS
	    Token reconocerNumero();
	    //REVISA SI YA ESTA EN LA TABLA
	    bool identificadorYaRegistrado(const std::string& valor);
		//AUTOMATA PARA IDENTIFICADORES Y PALABRAS RESERVADAS
	    Token reconocerPalabraReservadaOIdentificador();
		//AUTOMATA PARA CONSTANTES
	    Token reconocerConstante();
		//AUTOMATA PARA CADENAS
	    Token reconocerCadena();
		//AUTOMATA PARA CARACTERES
	    Token reconocerCaracter();
		//AUTOMATA PARA OPERADORES
	    Token reconocerOperadorDoble();
		//AUTOMATA PARA COMENTARIOS DE LINEA
	    void reconocerComentarioLinea();
		//AUTOMATA PARA COMENTARIOS DE BLOQUE
	    void reconocerComentarioBloque();
};//lexer

#endif //COMPILADOR_LEXER_H