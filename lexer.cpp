//
// Created by CARLOS URZUA on 9/21/2025.
//
#include "tokens.h"
#include "lexer.h"

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


using namespace std;

AnalizadorLexico::AnalizadorLexico(const string& entrada, SymbolTable& table1) : entrada(entrada), pos(0), lineaActual(1), declarando(false), archivoTokens("progfte.tok", ios::out){

	if (!archivoTokens.is_open()) {
        cerr << "Error al abrir el archivo: progfte.tok" << endl;
    }

    table = &table1; //guardar enlace a la tabla de simbolos

	// //INSERTAR PALABRAS RESERVADAS EN LA TABLA DE TOKENS
	for (const string& id : palabrasReservadas) {
	    Token token;
	    token.tipo = TipoLexema::PALABRA_RESERVADA;
	    token.valor_lexema = id;
	    token.id = obtenerNuevoId(token.tipo);
	    //tablaTokens.push_back(token);
	    table->insert(token);
	}


	for (const string& id : operadores) {
	    Token token;
	    token.tipo = TipoLexema::OPERADOR;
	    token.valor_lexema = id;
	    token.id = obtenerNuevoId(token.tipo);
	    table->insert(token);
	}

	Token token1;
    token1.tipo = TipoLexema::PAR_ABIERTO;
    token1.valor_lexema = "(";
    token1.id = obtenerNuevoId(token1.tipo);
    table->insert(token1);

    Token token2;
    token2.tipo = TipoLexema::PAR_CIERRE;
    token2.valor_lexema = ")";
    token2.id = obtenerNuevoId(token2.tipo);
    table->insert(token2);

    Token token3;
    token3.tipo = TipoLexema::LLAVE_ABIERTA;
    token3.valor_lexema = "{";
    token3.id = obtenerNuevoId(token3.tipo);
    table->insert(token3);

    Token token4;
    token4.tipo = TipoLexema::LLAVE_CERRADA;
    token4.valor_lexema = "}";
    token4.id = obtenerNuevoId(token4.tipo);
    table->insert(token4);

    Token token5;
    token5.tipo = TipoLexema::PUNTO_COMA;
    token5.valor_lexema = ";";
    token5.id = obtenerNuevoId(token5.tipo);
    table->insert(token5);

    // Abrimos el archivo para escritura de los tokens
    ofstream archivo("progfte.tok");

    // Verificamos si se pudo abrir el archivo
    if (!archivo) {
        cerr << "No se pudo abrir el archivo para escribir." << endl;
        return;
    }
}


void AnalizadorLexico::cerrarArchivoTokens(){
	if (archivoTokens.is_open()) {
        archivoTokens.close();
    }
}

Token AnalizadorLexico::siguienteToken() {

    while (this->pos < entrada.length() && isspace(entrada[pos]) ) {
        if (entrada[pos] == '\n') lineaActual++;
        pos++;
    }


    if (pos == entrada.length()) return {TipoLexema::FIN, ""};

    char actual = entrada[pos];

    if (actual == '"') return reconocerCadena();
    if (actual == '\'') return reconocerCaracter();
    if (actual == '(') { pos++; Token token = {TipoLexema::PAR_ABIERTO, "("};   imprimirToken(token, lineaActual); return token;}
    if (actual == ')') { pos++; Token token = {TipoLexema::PAR_CIERRE, ")"};    imprimirToken(token, lineaActual); return token;}
    if (actual == '{') { pos++; Token token = {TipoLexema::LLAVE_ABIERTA, "{"}; imprimirToken(token, lineaActual); return token;}
    if (actual == '}') { pos++; Token token = {TipoLexema::LLAVE_CERRADA, "}"}; imprimirToken(token, lineaActual); return token;}
    if (actual == ';') { pos++; Token token = {TipoLexema::PUNTO_COMA, ";"};    imprimirToken(token, lineaActual); return token;}
    if (actual == ',') { pos++; Token token = {TipoLexema::COMA, ","};    imprimirToken(token, lineaActual); return token;}


    // Comentarios
    if (actual == '/' && pos + 1 < entrada.length()) {
        if (entrada[pos + 1] == '/') {
            reconocerComentarioLinea();
            return {TipoLexema::COMENTARIO, "//"};
        }
        if (entrada[pos + 1] == '*') {
            reconocerComentarioBloque();
            return {TipoLexema::COMENTARIO, "/* */"};
        }
    }

	//RECONOCER OPERADORES
    if (actual == '+' || actual == '-' || actual == '=' || actual == '*' || actual == '/') {
        return this->reconocerOperadorDoble();
    }

	if(!declarando){//SI NO SE ENCEUNTRA EN ZONA DE DECLARACION
		if (isdigit(actual) || (actual == '+' || actual == '-')) return reconocerNumero();
        if (isupper(actual)) return reconocerConstante();
        else if (isalpha(actual)) return reconocerPalabraReservadaOIdentificador();
	} else {
		if (isdigit(actual) || (actual == '+' || actual == '-')) return reconocerNumero();
        if (isupper(actual)) return reconocerConstante();
        else if (isalpha(actual)) return reconocerPalabraReservadaOIdentificador();
        if (actual == '=' && (isalnum(entrada[pos+1])
			|| entrada[pos+1] == '\'' || entrada[pos+1] == '\"' || entrada[pos+1] == ' '||
			entrada[pos+1] == '\n'|| entrada[pos+1] == '\t'|| entrada[pos+1] == '\r')) {
				pos++;
				Token token = {TipoLexema::OPERADOR, "="};
				token.id = obtenerNuevoId(token.tipo);
				imprimirToken(token, lineaActual);
				return token;
		}
		if (actual == '"') return reconocerCadena();
        if (actual == '\'') return reconocerCaracter();
        if (actual == ';') { pos++; Token token = {TipoLexema::PUNTO_COMA, ";"};    imprimirToken(token, lineaActual); return token;}
        if (actual == ',') { pos++; Token token = {TipoLexema::COMA, ","};    imprimirToken(token, lineaActual); return token;}

	}

    return {TipoLexema::ERROR, string(1, actual)};
}

// Función para limpiar un archivo fuente:
// Elimina tabulaciones, saltos de línea, comentarios de línea (//) y de bloque (/* */)
// Luego guarda el contenido limpio en un nuevo archivo
void AnalizadorLexico::limpiarArchivoFuente() {
    //ifstream entrada(fichero + ".arc");    // Abrimos el archivo de entrada
    ofstream salida("progfte.dep");     // Abrimos el archivo de salida

    // Verificamos si se pudo abrir el archivo de entrada
    /*if (!entrada.is_open()) {
        cerr << "No se pudo abrir el archivo de entrada." << endl;
        return;
    }*/

    // Verificamos si se pudo abrir el archivo de salida
    if (!salida.is_open()) {
        cerr << "No se pudo abrir el archivo de salida." << endl;
        return;
    }

    /*stringstream buffer;
    buffer << entrada.rdbuf();          // Leemos todo el contenido del archivo de entrada
    string contenido = buffer.str();    // Lo almacenamos en un string
	*/
    string limpio;                      // Aquí construiremos el contenido limpio
    bool enComentarioLinea = false;     // Flag para saber si estamos dentro de un comentario de línea (//)
    bool enComentarioBloque = false;    // Flag para saber si estamos dentro de un comentario de bloque (/* */)

    // Recorremos todo el contenido carácter por carácter
    for (size_t i = 0; i < entrada.length(); ++i) {

        // Si estamos en un comentario de línea, ignoramos hasta el siguiente salto de línea
        if (enComentarioLinea) {
            if (entrada[i] == '\n') {
                enComentarioLinea = false;  // Final del comentario de línea
            }
            continue;  // Saltamos el carácter actual
        }

        // Si estamos en un comentario de bloque, ignoramos hasta encontrar */
        if (enComentarioBloque) {
            if (entrada[i] == '*' && i + 1 < entrada.length() && entrada[i + 1] == '/') {
                enComentarioBloque = false; // Fin del comentario de bloque
                i++;  // Avanzamos un carácter extra para saltar el '/'
            }
            continue;  // Saltamos el carácter actual
        }

        // Detectar inicio de comentario de línea
        if (entrada[i] == '/' && i + 1 < entrada.length() && entrada[i + 1] == '/') {
            enComentarioLinea = true;  // Activamos el flag
            i++;  // Avanzamos un carácter extra para saltar el segundo '/'
            continue;
        }

        // Detectar inicio de comentario de bloque
        if (entrada[i] == '/' && i + 1 < entrada.length() && entrada[i + 1] == '*') {
            enComentarioBloque = true; // Activamos el flag
            i++;  // Avanzamos un carácter extra para saltar el '*'
            continue;
        }

        // Ignorar tabulaciones y saltos de línea
        if (entrada[i] == '\t' || entrada[i] == '\n' || entrada[i] == '\r') {
            continue;
        }

        if (i < entrada.length() -1 && entrada[i] == ' ' && (entrada[i + 1] == ' ' || entrada[i] == '\t' || entrada[i] == '\n' || entrada[i] == '\r')) {
            i++;
			continue;
        }

        // Si no estamos en comentarios ni espacios, agregamos el carácter al resultado limpio
        limpio += entrada[i];
    }

    salida << limpio;           // Guardamos el contenido limpio en el archivo de salida

    //entrada.close();            // Cerramos el archivo de entrada
    salida.close();             // Cerramos el archivo de salida

    cout << "Archivo limpio guardado como 'progfte.dep'" << endl;
}


//AL MOMENTO DE CERRAR OBJETO
AnalizadorLexico::~AnalizadorLexico() {
    if (archivoTokens.is_open()) {
        archivoTokens.close();
    }
}

int AnalizadorLexico::obtenerNuevoId(TipoLexema tipo) {
    switch (tipo) {
        case TipoLexema::IDENTIFICADOR: return idIdentificador++;
        case TipoLexema::ENTERO:        return idNumero++;
        case TipoLexema::CARACTER:      return idCaracter++;
        case TipoLexema::CADENA:        return idCadena++;
        default:                   return idGeneral++;
    }
}

vector<Token> AnalizadorLexico::flujo_tokens() {
    return flujoTokens;
}

void AnalizadorLexico::imprimirToken(const Token& token) {
    // Convertir el enum a texto legible
    string tipoTexto;
    tipoTexto = table->tokenToString(token.tipo);

    // Imprimir en pantalla y escribit en archivo de tokens
    cout<<"ID: " <<setw(4)<< token.id << " ----- TOKEN: "<< tipoTexto << " ------ " << token.valor_lexema << endl;
}


//TAMBIEN APARECCE EN SYMBOL TABLE
void AnalizadorLexico::imprimirToken(const Token& token, int linea) {
    // Convertir el enum a texto legible
    string tipoTexto;
    tipoTexto = table->tokenToString(token.tipo);

    // Imprimir en pantalla y escribit en archivo de tokens
    cout          << "Linea " <<setw(4)<< linea << " - " << "ID: " <<setw(4)<< token.id << " ----- TOKEN: "<< tipoTexto << " ------ " << token.valor_lexema << endl;
    archivoTokens << "Linea " <<setw(4)<< linea << " - " << "ID: " <<setw(4)<< token.id << " ----- TOKEN: "<< tipoTexto << " ------ " << token.valor_lexema << endl;
}

//AUTOMATA PARA NUMEROS
Token AnalizadorLexico::reconocerNumero() {
    int inicio = pos;
    bool isFlotante = false;

    //[0-9]+
    while (pos < entrada.length() && isdigit(entrada[pos])) pos++;
    //.
	if(pos < entrada.length() && entrada[pos] == '.'){
        isFlotante = true;
        pos++;
	}
	//[0-9]+
	while (pos < entrada.length() && isdigit(entrada[pos])) pos++;

    Token token;
    if(!isFlotante) token = Token{TipoLexema::ENTERO, entrada.substr(inicio, pos - inicio)};
    else token = Token{TipoLexema::FLOTANTE, entrada.substr(inicio, pos - inicio)};
	//AGREGAR A LA TABLA
	//tablaTokens.push_back(token);
    table->insert(token);
	imprimirToken(token, lineaActual);
    return token;
}


//REVISA SI YA ESTA EN LA TABLA
bool AnalizadorLexico::identificadorYaRegistrado(const std::string& valor) {
    for (const auto& token : table->getFullTable()) {
        if (token.tipo == TipoLexema::IDENTIFICADOR && token.valor_lexema == valor) {
            return true;
        }
    }
    return false;
}

//AUTOMATA PARA IDENTIFICADORES Y PALABRAS RESERVADAS
Token AnalizadorLexico::reconocerPalabraReservadaOIdentificador() {
    size_t inicio = pos;
    bool hasErrors = false;
    string palabra;

	if(declarando){
		while (pos < entrada.length() && (entrada[pos] != ',' && entrada[pos] != ';' && entrada[pos] != ' ' && entrada[pos] != '\n' && entrada[pos] != '\t' && entrada[pos] != '\r')) {

	        if (!(isalnum(entrada[pos]) || entrada[pos] != '_')){
		        hasErrors = true;
			}
			pos++;
		}

		palabra = entrada.substr(inicio, pos - inicio);
	} else {
		while (pos < entrada.length() && (isalnum(entrada[pos]) || entrada[pos] == '_')) pos++;

		palabra = entrada.substr(inicio, pos - inicio);
	}

    TipoLexema tipo = (palabrasReservadas.count(palabra)) ? TipoLexema::PALABRA_RESERVADA : TipoLexema::IDENTIFICADOR;
    Token token = {tipo, palabra};
	token.id = obtenerNuevoId(token.tipo);
    imprimirToken(token, lineaActual);

    if (hasErrors && token.tipo == TipoLexema::IDENTIFICADOR) {
        cout<<"\nIDENTIFICADOR ENCONTRADO: "<< palabra <<" ES INVALIDO.\n";
		return {TipoLexema::ERROR, palabra};
	}

    if (palabra == "declaracion") {
        declarando = true;
		//cout << "INICIA ZONA DE DELCARACION \n";
	}
    else if (palabra == "fin_declaracion") declarando = false;

    if(token.tipo == TipoLexema::IDENTIFICADOR && declarando) {
        if (!identificadorYaRegistrado(palabra)) {
            //tablaTokens.push_back(token);
            table->insert(token);
        }
	}

	if(!declarando){
		if (!identificadorYaRegistrado(palabra) && !palabrasReservadas.count(palabra)) {
            cout << "NO HA SIDO DECLARADO EL IDENTIFICADO\n";
        }
	}
    return token;
}

//AUTOMATA PARA CONSTANTES
Token AnalizadorLexico::reconocerConstante() {
    size_t inicio = pos;
    while (pos < entrada.length() && ((isupper(entrada[pos]) || isdigit(entrada[pos]))|| entrada[pos] == '_')) pos++;
    string token_s = entrada.substr(inicio, pos - inicio);
    Token token;
    if (token_s == "VERDADERO" || token_s == "FALSO") {
        token = {TipoLexema::BOOLEANO, token_s};
    } else {
        token = {TipoLexema::CONSTANTE, entrada.substr(inicio, pos - inicio)};
        token.id = obtenerNuevoId(token.tipo);
        //tablaTokens.push_back(token);
    }

    table->insert(token);
    imprimirToken(token, lineaActual);
    return token;
}

//AUTOMATA PARA CADENAS
Token AnalizadorLexico::reconocerCadena() {
    size_t inicio = pos++;
    while (pos < entrada.length() && entrada[pos] != '"') {
        if (entrada[pos] == '\n') lineaActual++;
        pos++;
    }
    if (pos < entrada.length()) pos++; // cerrar comillas
    Token token = {TipoLexema::CADENA, entrada.substr(inicio, pos - inicio)};
    token.id = obtenerNuevoId(token.tipo);
	//tablaTokens.push_back(token);
    table->insert(token);

	imprimirToken(token, lineaActual);
    return token;
}

//AUTOMATA PARA CARACTERES
Token AnalizadorLexico::reconocerCaracter() {
    size_t inicio = pos++;
    while (pos < entrada.length() && entrada[pos] != '\'') {
        if (entrada[pos] == '\n') lineaActual++;
        pos++;
    }
    if (pos < entrada.length()) pos++; // cerrar comillas
    Token token = {TipoLexema::CARACTER, entrada.substr(inicio, pos - inicio)};
    token.id = obtenerNuevoId(token.tipo);
	//tablaTokens.push_back(token);
    table->insert(token);
    imprimirToken(token, lineaActual);
	return token;
}

//AUTOMATA PARA OPERADORES
Token AnalizadorLexico::reconocerOperadorDoble() {
    char actual = entrada[pos];
    char siguiente = (pos + 1 < entrada.length()) ? entrada[pos + 1] : '\0';

    if ((actual == '+' && siguiente == '+') || (actual == '-' && siguiente == '-') ||
        (actual == '=' && siguiente == '=') || (actual == '!' && siguiente == '=') ||
        (actual == '+' && siguiente == '=') || (actual == '-' && siguiente == '=') ||
        (actual == '*' && siguiente == '=') || (actual == '/' && siguiente == '=')) {
        pos += 2;
        Token token = {TipoLexema::OPERADOR, string(1, actual) + siguiente};
        token.id = obtenerNuevoId(token.tipo);
		imprimirToken(token, lineaActual);
		return token;
    }

    pos++;
    Token token = {TipoLexema::OPERADOR, string(1, actual)};
    //tablaTokens.push_back(token);
    imprimirToken(token, lineaActual);
    return token;
}

//AUTOMATA PARA COMENTARIOS DE LINEA
void AnalizadorLexico::reconocerComentarioLinea() {
    while (pos < entrada.length() && entrada[pos] != '\n') pos++;
}

//AUTOMATA PARA COMENTARIOS DE BLOQUE
void AnalizadorLexico::reconocerComentarioBloque() {
    pos += 2;
    while (pos < entrada.length() && !(entrada[pos] == '*' && entrada[pos + 1] == '/')) {
        if (entrada[pos] == '\n') lineaActual++;
        pos++;
    }
    if (pos + 1 < entrada.length()) pos += 2;
}
