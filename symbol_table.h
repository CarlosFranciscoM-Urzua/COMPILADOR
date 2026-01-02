//
// Created by CARLOS URZUA on 9/21/2025.
//

#ifndef COMPILADOR_SYMBOL_TABLE_H
#define COMPILADOR_SYMBOL_TABLE_H
#include "tokens.h"
#include <vector>

class SymbolTable {
    public:
        SymbolTable();
        const vector<Token>& getFullTable() const;
        string tokenToString(TipoLexema tipo);
        void imprimirTablaTokens();
        void guardarTablaTokensEnArchivo(const string& nombreArchivo);
        void insert(Token token);
        void imprimirToken(const Token& token, int linea);
        bool isDeclared(const Token& token);
        //vector<TokenInfo> getFullTable();

    private:
        vector<Token> tablaTokens;
};//SymbolTable

#endif //COMPILADOR_SYMBOL_TABLE_H