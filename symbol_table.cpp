//
// Created by CARLOS URZUA on 9/21/2025.
//

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

SymbolTable::SymbolTable() {
}


const vector<Token>& SymbolTable::getFullTable() const {
    return tablaTokens;
}

// Función que convierte un Token en su representación como cadena de texto
string SymbolTable::tokenToString(TipoLexema tipo) {
    switch (tipo) {
        case TipoLexema::ENTERO:            return "ENTERO           ";
        case TipoLexema::FLOTANTE:          return "FLOTANTE         ";
        case TipoLexema::IDENTIFICADOR:     return "IDENTIFICADOR    ";
        case TipoLexema::CONSTANTE:         return "CONSTANTE        ";
        case TipoLexema::OPERADOR:          return "OPERADOR         ";
        case TipoLexema::CADENA:            return "CADENA           ";
        case TipoLexema::CARACTER:          return "CARACTER         ";
        case TipoLexema::FUNCION:           return "FUNCION          ";
        case TipoLexema::PALABRA_RESERVADA: return "PALABRA_RESERVADA";
        case TipoLexema::PAR_ABIERTO:       return "PAR_ABRE         ";
        case TipoLexema::PAR_CIERRE:        return "PAR_CIERRE       ";
        case TipoLexema::LLAVE_ABIERTA:     return "LLAVE_ABIERTA    ";
        case TipoLexema::LLAVE_CERRADA:     return "LLAVE_CERRADA    ";
        case TipoLexema::PUNTO_COMA:        return "PUNTO_COMA       ";
        case TipoLexema::COMENTARIO:        return "COMENTARIO       ";
        case TipoLexema::FIN:               return "FIN              ";
        case TipoLexema::ERROR:             return "ERROR            ";
        case TipoLexema::BOOLEANO:          return "BOOLEANO         ";
        default:                       return "DESCONOCIDO      "; // En caso de que no se reconozca el tipo
    }
}

void SymbolTable::insert(Token token) {
    tablaTokens.push_back(token);
}


void SymbolTable::imprimirTablaTokens() {
    cout << "\n\nTABLA DE TOKENS\n";
    int i = 0;
    string tipoTexto;
    for (const auto& token : tablaTokens) {
        tipoTexto = SymbolTable::tokenToString(token.tipo);
        cout << "id: "<<setw(3)<<token.id<<" | Tipo: " << tipoTexto << " | Valor: " << token.valor_lexema << endl;
        i++;
    }
}

void SymbolTable::imprimirToken(const Token& token, int linea) {
    // Convertir el enum a texto legible
    string tipoTexto;
    tipoTexto = tokenToString(token.tipo);

    cout<< "Linea " <<setw(4)<< linea << " - " << "ID: " <<setw(4)<< token.id << " ----- TOKEN: "<< tipoTexto << " ------ " << token.valor_lexema << endl;
    //archivoTokens << "Linea " <<setw(4)<< linea << " - " << "ID: " <<setw(4)<< token.id << " ----- TOKEN: "<< tipoTexto << " ------ " << token.valor << endl;
}

bool SymbolTable::isDeclared(const Token &token) {
    for (int i = 0; i < static_cast<int>(tablaTokens.size()); i++) {
        if (tablaTokens[i].valor_lexema == token.valor_lexema) {
            return true;
        }
    }

    return false;
}

// Guarda la tabla de tokens en un archivo
void SymbolTable::guardarTablaTokensEnArchivo(const string& nombreArchivo) {
    ofstream archivo(nombreArchivo + ".tab");
    if (!archivo.is_open()) {
        cerr << "Error al abrir " << nombreArchivo << ".tab para escritura." << endl;
        return;
    }

    for (const auto& token : tablaTokens) {
        archivo << "id: " << setw(4)<< token.id
                << " | Tipo: " << tokenToString(token.tipo)
                << " | Valor: " << token.valor_lexema << endl;
    }

    archivo.close();
    cout << "Tabla de tokens guardada en '" << nombreArchivo << ".tab correctamente." << endl;
}

