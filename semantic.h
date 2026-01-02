//
// Created by CARLOS URZUA on 9/26/2025.
//

#ifndef COMPILADOR_SEMANTIC_H
#define COMPILADOR_SEMANTIC_H
#include "astree.h"
#include <map>
#include <stack>

#include "symbol_table.h"

struct SymbolInfo {
    DataType tipo;
    //string symbol;
    bool inicializado;

    SymbolInfo(DataType _tipo = DataType::UNKNOWN, bool _inicializado = false) : tipo(_tipo), inicializado(_inicializado){}
    //SymbolInfo();
};

// Entorno (scope)
struct Entorno {
    map<string, SymbolInfo> tabla;
    void declararLocal(const string& id, DataType tipo, bool init=false) {
        tabla[id] = SymbolInfo(tipo, init);
    }
    bool existeLocal(const string& id) const {
        //return tabla.find(id) != tabla.end();
        return tabla.count(id) > 0;
    }
};

struct SemanticError {
    string msg;
    shared_ptr<Nodo> nodo;
    SemanticError(const string& m, shared_ptr<Nodo> n = nullptr) : msg(m), nodo(n) {}
};


void recorrerPrefijo(const shared_ptr<Nodo>& nodo);
void recorrerInfijo(const shared_ptr<Nodo>& nodo);
void recorrerPostfijo(const shared_ptr<Nodo>& nodo);
string typeTagToStr(DataType t);

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    // analiza un bloque/programa (lista de SENTENCE o un único SENTENCE)
    void analizarPrograma(const shared_ptr<Nodo>& raiz);
    const vector<SemanticError>& getErrors() const;
    bool hasErrors() const;
    void exec_semantic(const shared_ptr<Nodo>& raiz);
    void eraseErrors();
    //void addError(const SemanticError& error);
    void pushError(const string& msg, shared_ptr<Nodo> n = nullptr);
    void imprimirTablaGlobal();
    shared_ptr<Nodo> simplificar(shared_ptr<Nodo> nodo);
private:
    vector<Entorno> entornos;

    vector<SemanticError> errores;

    Entorno& entornoActual();
    void pushScope();
    void popScope();
    SymbolInfo* buscarEnCadena(const string& id);
    // void pushError(const string& msg, shared_ptr<Nodo> n = nullptr);
    bool isAssignment(const shared_ptr<Nodo>& nodo);
    bool isDeclaration(const shared_ptr<Nodo>& nodo);
    bool isSentence(const shared_ptr<Nodo>& nodo);
    bool isIf(const shared_ptr<Nodo>& nodo);
    bool isWhile(const shared_ptr<Nodo>& nodo);
    bool isReturn(const shared_ptr<Nodo>& nodo);
    bool isFunctionDecl(const shared_ptr<Nodo>& nodo);
    void analizarInstruccion(const shared_ptr<Nodo>& nodo);

    void intentarSimplificado(const shared_ptr<Nodo> &nodo);

    void analizarDeclaracion(const shared_ptr<Nodo>& nodo);
    void procesarSublistaDeclaracion(const shared_ptr<Nodo>& nodeG, DataType tipo);
    string findNearestIdentifier(const shared_ptr<Nodo>& n);
    void analizarAsignacion(const shared_ptr<Nodo>& nodo);
    // ------------------ If / While: chequear condición y cuerpo en scope nuevo ------------------
    void analizarIf(const shared_ptr<Nodo>& nodo);
    void analizarWhile(const shared_ptr<Nodo>& nodo);
    void analizarReturn(const shared_ptr<Nodo>& nodo);
    void analizarFuncion(const shared_ptr<Nodo>& nodo);
    DataType analizarExpresion(const shared_ptr<Nodo>& nodo, bool isAsignacion);
    bool isOperator(const string& s);
    DataType tipoResultadoOperator(const string& op, DataType L, DataType R, shared_ptr<Nodo> node);
    DataType mapearTipo(const string& s);
    bool tiposCompatibles(DataType left, DataType right);
    shared_ptr<Nodo> buscarSubexpresion(const shared_ptr<Nodo>& nodo);
    shared_ptr<Nodo> buscarIdentificadorIzquierdo(const shared_ptr<Nodo>& nodo);
    shared_ptr<Nodo> buscarNodoIgual(const shared_ptr<Nodo>& nodo);

    shared_ptr<Nodo> buscar_expresion_asignada(const shared_ptr<Nodo> &nodo);

    map<string, SymbolInfo> tabla;
    void declararLocal(const string& id, DataType tipo, bool init=false);
    bool existeLocal(const string& id) const ;



};

#endif //COMPILADOR_SEMANTIC_H