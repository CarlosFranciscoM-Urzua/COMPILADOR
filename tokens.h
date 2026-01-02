//
// Created by CARLOS URZUA on 9/21/2025.
//

#ifndef COMPILADOR_TOKENS_H
#define COMPILADOR_TOKENS_H

#include <string>


using namespace std;

// -----------------------------
// Tipos y símbolos
enum class DataType { INT, FLOAT, BOOL, VOID, UNKNOWN, CHAR, STRING };

// Tipos de tokens reconocidos
enum class TipoLexema {
    ENTERO,
    FLOTANTE,
    IDENTIFICADOR,
    CONSTANTE,
    OPERADOR,
    CADENA,
    CARACTER,
    FUNCION,
    PALABRA_RESERVADA,
    PAR_ABIERTO,
    PAR_CIERRE,
    LLAVE_ABIERTA,
    LLAVE_CERRADA,
    PUNTO_COMA,
    COMENTARIO,
    BOOLEANO,
    FIN,
    ERROR,
    COMA,
    SCOPE,
    GRAMMAR
};

// Información de cada token, ahora con línea incluida
struct Token {
    TipoLexema tipo;
    string valor_lexema;
    int id;
    string simbolo_sintactico;
};

/*unordered_set<string> operadores = {
    "+", "-","*","/","<",">","=","==","++","--","<=",">=","!","!=","*=","/=","^^","^"
};*/

#endif //COMPILADOR_TOKENS_H