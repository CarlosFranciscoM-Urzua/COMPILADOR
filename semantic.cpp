//
// Created by CARLOS URZUA on 9/26/2025.
//

#include <iostream>
#include <memory>
#include <vector>
#include "astree.h"
#include "semantic.h"
#include <map>
#include <string>
#include <sstream>
#include "astree.h"
#include "sintaxis.h"
#include "tokens.h"

using namespace std;


//TypeTag analizarExpresion(const shared_ptr<Nodo>& nodo);
string typeTagToStr(DataType t) {
    switch(t) {
        case DataType::INT    : return "entero";
        case DataType::FLOAT  : return "flotante";
        case DataType::BOOL   : return "booleano";
        case DataType::VOID   : return "vacio";
        case DataType::CHAR   : return "caracter";
        case DataType::STRING : return "cadena";
        default: return "unknown";
    }
}

shared_ptr<Nodo> SemanticAnalyzer::simplificar(shared_ptr<Nodo> nodo) {
    if (!nodo) return nullptr;

    // Primero, simplificamos recursivamente a todos los hijos
    for (size_t i = 0; i < nodo->hijos.size(); ++i) {
        nodo->hijos[i] = simplificar(nodo->hijos[i]);
    }

    // Si el nodo no tiene hijos, no se puede simplificar más
    if (nodo->hijos.empty()) return nodo;

    // --- Reglas de simplificación ---
    // Si es un nodo estructural con un solo hijo, reemplazar por ese hijo
    if ( (nodo->valor == "E" || nodo->valor == "T" ||
     nodo->valor == "F" || nodo->valor == "A" ||
     nodo->valor == "B" || nodo->valor == "R" ||
     nodo->valor == "B" || nodo->valor == "R" ||
     nodo->valor == "E'" || nodo->valor == "T'" ||
     nodo->valor == "F'" || nodo->valor == "A'" ||
     nodo->valor == "B'" || nodo->valor == "R'" ||
     nodo->valor == "G'" || nodo->valor == "G")) {
        if (nodo->hijos.size() == 1) {
            return nodo->hijos[0];
        } else {

        }

     }

    // Si tiene hijos nulos o vacíos, limpiarlos
    vector<shared_ptr<Nodo>> hijosLimpiados;
    for (auto& h : nodo->hijos) {
        if (h) hijosLimpiados.push_back(h);
    }
    nodo->hijos = hijosLimpiados;

    return nodo;

    /*if (!nodo) return nullptr;

    // Simplificar primero todos los hijos
    vector<shared_ptr<Nodo>> nuevosHijos;
    for (auto& h : nodo->hijos) {
        auto sh = simplificar(h);
        if (sh) nuevosHijos.push_back(sh);
    }
    nodo->hijos = nuevosHijos;

    // 1️⃣ Si el nodo es no terminal y solo tiene un hijo: colapsar
    if (esNoTerminal(nodo->simbolo) && nodo->hijos.size() == 1) {
        return nodo->hijos[0];
    }

    // 2️⃣ Si el nodo es no terminal con varios hijos: aplanar hijos significativos
    if (esNoTerminal(nodo->simbolo) && nodo->hijos.size() > 1) {
        vector<shared_ptr<Nodo>> hijosFusionados;
        for (auto& h : nodo->hijos) {
            if (esNoTerminal(h->simbolo)) {
                hijosFusionados.insert(hijosFusionados.end(), h->hijos.begin(), h->hijos.end());
            } else {
                hijosFusionados.push_back(h);
            }
        }
        auto nuevo = make_shared<Nodo>(nodo->simbolo);
        nuevo->hijos = hijosFusionados;
        // Si aún queda un solo hijo, colapsar
        if (nuevo->hijos.size() == 1) return nuevo->hijos[0];
        return nuevo;
    }

    // 3️⃣ Si el nodo es un operador binario (=, +, -, *, /, etc.) y tiene más de dos hijos
    if ((nodo->simbolo == "=" || nodo->simbolo == "+" || nodo->simbolo == "-" ||
         nodo->simbolo == "*" || nodo->simbolo == "/" || nodo->simbolo == "&&" ||
         nodo->simbolo == "||" || nodo->simbolo == "<" || nodo->simbolo == ">") &&
        nodo->hijos.size() > 2) {
        auto nuevo = make_shared<Nodo>(nodo->simbolo);
        nuevo->hijos.push_back(nodo->hijos[0]);
        nuevo->hijos.push_back(nodo->hijos[1]);
        for (size_t i = 2; i < nodo->hijos.size(); ++i) {
            auto anidado = make_shared<Nodo>(nodo->simbolo);
            anidado->hijos = {nuevo->hijos.back(), nodo->hijos[i]};
            nuevo = anidado;
        }
        return nuevo;
        }

    return nodo;*/
}


// -----------------------------
// Analizador semantico
SemanticAnalyzer::SemanticAnalyzer() {
    entornos.emplace_back(); // entorno global
}

// analiza un bloque/programa (lista de SENTENCE o un único SENTENCE)
void SemanticAnalyzer::analizarPrograma(const shared_ptr<Nodo>& raiz) {
    errores.clear();
    if (!raiz) return;
    if (raiz->valor == "PROGRAMA" || raiz->valor == "PROGRAM" || raiz->valor == "BLOCK"  || raiz->valor == "BLOQUE") {
        for (auto &stmt : raiz->hijos) {
            analizarInstruccion(stmt);
        }
    } else {
        // si nos pasan una única sentencia
        analizarInstruccion(raiz);
    }

    // Print errors
    const auto& errs = getErrors();
    if (errs.empty()) {
        cout << "\nNo se encontraron errores semanticos.\n";
    } else {
        cout << "\nErrores semanticos encontrados (" << errs.size() << "):\n";
        for (auto &e : errs) {
            cout << " - " << e.msg << "\n";
        }
    }
}

// devolver errores
const vector<SemanticError>& SemanticAnalyzer::getErrors() const {
    return errores;
}

bool SemanticAnalyzer::hasErrors() const {
    return !errores.empty();
}

void SemanticAnalyzer::eraseErrors() {
    errores.clear();
}

Entorno& SemanticAnalyzer::entornoActual() {
    return entornos.back();
}

void SemanticAnalyzer::pushScope() {
    entornos.emplace_back();
}
void SemanticAnalyzer::popScope() {
    if (entornos.size() > 1) entornos.pop_back();
    else entornos.pop_back();
}

// buscar símbolo en la cadena de scopes (de inner a outer)
SymbolInfo* SemanticAnalyzer::buscarEnCadena(const string& id) {
    for (int i = (int)entornos.size() - 1; i >= 0; --i) {
        auto it = entornos[i].tabla.find(id);
        if (it != entornos[i].tabla.end()) {
            //cout <<typeTagToStr(it->second.tipo)<<"\n";
            return &it->second;
        }
    }
    return nullptr;
}

void SemanticAnalyzer::pushError(const string& msg, shared_ptr<Nodo> n) {
    errores.emplace_back(msg, n);
}

// ------------------ reconocimiento de producciones (heurísticas basadas en tus arboles) ------------------
bool SemanticAnalyzer::isSentence(const shared_ptr<Nodo>& nodo) {
    return nodo && nodo->valor == "SENTENCE";
}
bool SemanticAnalyzer::isDeclaration(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return false;
    // si SENTENCE y primer hijo es tipo como "entero" o "real"/"booleano"
    if (nodo->valor == "SENTENCE" && !nodo->hijos.empty()) {
        string s0 = nodo->hijos[0]->valor;
        if ((s0 == "entero"   ||
            s0 == "flotante" ||
            s0 == "booleano" ||
            s0 == "caracter" ||
            s0 == "cadena"   ) &&
            (nodo->hijos[1]->token == TipoLexema::IDENTIFICADOR || nodo->hijos[1]->string_token == "id")) return true;
    }
    return false;
}


bool SemanticAnalyzer::isIf(const shared_ptr<Nodo>& nodo) {
    return nodo && nodo->valor == "SENTENCE" && !nodo->hijos.empty() && nodo->hijos[0]->valor == "si";
}
bool SemanticAnalyzer::isWhile(const shared_ptr<Nodo>& nodo) {
    return nodo && nodo->valor == "SENTENCE" && !nodo->hijos.empty() && nodo->hijos[0]->valor == "mientras";
}
bool SemanticAnalyzer::isReturn(const shared_ptr<Nodo>& nodo) {
    return nodo && nodo->valor == "SENTENCE" && !nodo->hijos.empty() && nodo->hijos[0]->valor == "retornar";
}
bool SemanticAnalyzer::isFunctionDecl(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return false;
    if (nodo->valor == "SENTENCE" && nodo->hijos.size() >= 3) {
        // pattern: SENTENCE |--- funcion |--- entero |--- nombre |--- ( ...
        if (nodo->hijos[0]->valor == "funcion") return true;
    }
    return false;
}

// ------------------ analizar instruccion (dispatch) ------------------
void SemanticAnalyzer::analizarInstruccion(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;
    if (isDeclaration(nodo)) {
        cout << "INSTRUCCION DE DECLARACION DE VARIABLE\n";
        analizarDeclaracion(nodo);
    }

    if (isAssignment(nodo)) {
        cout << "INSTRUCCION DE ASIGNACION\n";
        analizarAsignacion(nodo);
    }
    else if (isIf(nodo)) {
        cout << "INSTRUCCION DE CONDICIONAL\n";
        analizarIf(nodo);
    }
    else if (isWhile(nodo)) {
        cout << "INSTRUCCION DE CICLO\n";
        analizarWhile(nodo);
    }
    else if (isReturn(nodo)) {
        cout << "INSTRUCCION DE RETORNO\n";
        analizarReturn(nodo);
    }
    else if (isFunctionDecl(nodo)) {
        cout << "INSTRUCCION DE DECLARACION DE FUNCION\n";
        analizarFuncion(nodo);
    }
    else if (nodo->hijos.size()>0 && nodo->hijos[0]->simbolo_sintactico == "imprimir") {
        cout << "INSTRUCCION DE IMPRESION\n";
    }
    else {
        if (nodo->valor == "PROGRAMA" || nodo->valor == "BLOQUE") {
            cout << "ABRIR SCOPE\n";
            pushScope();
            for (auto &s : nodo->hijos) {
                analizarInstruccion(s);
            }
            cout << "CERRAR SCOPE\n";
            popScope();
        } else {
            string t = typeTagToStr(analizarExpresion(nodo, isAssignment(nodo)));
            (void)t;
        }
    }
}

void SemanticAnalyzer::intentarSimplificado(const shared_ptr<Nodo>& nodo) {
    analizarInstruccion(simplificar(nodo));
}

// ------------------ Declaracion (maneja listas por G) ------------------
// Forma en tus arboles: SENTENCE -> hijo0 = tipo (entero/real/booleano), hijo1 = id, hijo2 = G (lista) , ultimo ;
void SemanticAnalyzer::analizarDeclaracion(const shared_ptr<Nodo>& nodo) {
    if (!nodo || nodo->hijos.size() < 2) return;
    string tipoStr = nodo->hijos[0]->valor; // "entero", "real", "booleano"
    DataType tipo = mapearTipo(tipoStr);
    if (tipo == DataType::UNKNOWN) {
        pushError("Tipo desconocido en declaracion: " + tipoStr, nodo);
        return;
    }
    // primer id es nodo->hijos[1]
    // G subtree may contain commas and further ids or assignment forms
    // We'll process nodo->hijos from index 1 onwards and collect identifiers or assignments
    for (size_t i = 1; i < nodo->hijos.size(); ++i) {
        auto child = nodo->hijos[i];
        if (!child) continue;

        // if child is '=' (initialization) or subtree with '=' inside (G -> = E)
        if (!child->hijos.empty() && (child->valor == "G" || child->valor == "G'")) {
            // traverse G structure to find ids and potential '=' nodes
            procesarSublistaDeclaracion(child, tipo);
            return;
        }
        // If child is an identifier leaf
        else if (child->token == TipoLexema::IDENTIFICADOR && child->hijos.empty()) {
            if (entornoActual().existeLocal(child->valor)) {
                pushError("Variable redeclarada en mismo scope: " + child->valor, child);
            } else {
                entornoActual().declararLocal(child->valor, tipo, false);
            }
        }

        // if it's an '=' node directly
        else if (child->valor == "=" && child->hijos.size() >= 1) {
            // left might have been the previous identifier (rare in this shape)
            // skip here: handled by G traversal

            cout<<"DECLARACION CON ASIGNACION\n";
            analizarAsignacion(nodo);
        }
        // ; or commas ignored
    }
}

void SemanticAnalyzer::procesarSublistaDeclaracion(const shared_ptr<Nodo>& nodeG, DataType tipo) {
    if (!nodeG) return;
    // search for identifiers or '=' inside this subtree
    if (nodeG->token == TipoLexema::IDENTIFICADOR && nodeG->hijos.empty()) {
        // declare
        if (entornoActual().existeLocal(nodeG->valor)) {
            pushError("(SD) Variable redeclarada en mismo scope: " + nodeG->valor, nodeG);
        } else {
            entornoActual().declararLocal(nodeG->valor, tipo, false);
        }
        return;
    }
    // if G node has children, recurse
    for (auto &c : nodeG->hijos) {
        // assignment inside declaration: detect pattern id '=' E
        if (c && c->valor == "=" && c->hijos.size() >= 1) {
            // try to find left id - often left id is sibling; but in your trees initial assignment appears under G containing '=' and E
            // We'll try to find nearest identifier in this subtree (search siblings)
            string idname = findNearestIdentifier(nodeG);
            if (!idname.empty()) {
                // analyze RHS
                //ANALIZAR EXPRESION, ES DECLARACION=0
                DataType rhs = analizarExpresion(c->hijos[0], false);
                if (rhs == DataType::UNKNOWN) {
                    pushError("Inicializador invalido para " + idname, c);
                } else if (!tiposCompatibles(tipo, rhs)) {
                    pushError("Inicializador incompatible: " + idname + " : declared " + typeTagToStr(tipo) + " but initializer is " + typeTagToStr(rhs), c);
                } else {
                    entornoActual().declararLocal(idname, tipo, true);
                }
            }
        } else {
            procesarSublistaDeclaracion(c, tipo);
        }
    }
}

// helper to search nearest identifier in subtree (useful for G shapes)
string SemanticAnalyzer::findNearestIdentifier(const shared_ptr<Nodo>& n) {
    if (!n) return  "";
    if (n->token == TipoLexema::IDENTIFICADOR && n->hijos.empty()) return n->valor;
    for (auto &c : n->hijos) {
        string r = findNearestIdentifier(c);
        if (!r.empty()) return r;
    }
    return "";
}

void SemanticAnalyzer::analizarAsignacion(const shared_ptr<Nodo>& nodo) {
    cout<<"INICIA ANALISIS DE ASIGNACION\n";
    if (!nodo) return;

    shared_ptr<Nodo> eqNode = buscarNodoIgual(nodo);
    if (!eqNode) {
        pushError("Asignacion mal formada (no se encontro '=')", nodo);
        return;
    }

    shared_ptr<Nodo> idNode = buscarIdentificadorIzquierdo(nodo);
    if (!idNode) {
        pushError("Asignacion mal formada: no se encontro identificador antes de '='", nodo);
        return;
    }

    string idname = idNode->valor;
    SymbolInfo* sym = buscarEnCadena(idname);
    if (!sym) {
        pushError("Asignacion a variable no declarada: " + idname, nodo);
        return;
    }

    // Buscar RHS (expresión a la derecha del '=')
    shared_ptr<Nodo> rhs = nullptr;
    /*if (!eqNode->hijos.empty()) {
        rhs = eqNode->hijos[0];
    } else {
        rhs = buscarSubexpresion(eqNode);
        if (!rhs) rhs = buscarSubexpresion(nodo);
    }*/

    rhs = buscar_expresion_asignada(nodo);

    if (!rhs) {
        pushError("No se encontro expresion a la derecha del '=' para " + idname, nodo);
        return;
    }


    // Analizar tipo de la expresión
    DataType rhsType = analizarExpresion(rhs, true);
    if (rhsType == DataType::UNKNOWN) {
        pushError("Tipo inválido en la expresion de asignacion para '" + idname + "'", rhs);
        return;
    }

    // Comprobar compatibilidad
    if (!tiposCompatibles(sym->tipo, rhsType)) {
        ostringstream oss;
        oss << "Incompatibilidad de tipos: no se puede asignar "
            << typeTagToStr(rhsType) << " a variable '"
            << idname << "' de tipo " << typeTagToStr(sym->tipo);
        pushError(oss.str(), nodo);
        return;
    }

    // Marcar variable como inicializada
    sym->inicializado = true;
    buscarEnCadena(idname)->inicializado = true;
    entornoActual().tabla.at(idname).inicializado = true;
}

shared_ptr<Nodo> SemanticAnalyzer::buscarNodoIgual(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return nullptr;
    if (nodo->hijos.size() == 0) return nullptr;


    if (nodo->valor == "=" || nodo->simbolo_sintactico == "=")
        return nodo;

    for (const auto& hijo : nodo->hijos) {
        if (hijo->valor == "=") return hijo;
    }

    for (const auto& hijo : nodo->hijos) {
        shared_ptr<Nodo> res = buscarNodoIgual(hijo);
        if (res) return res;
    }
    return nullptr;
}


shared_ptr<Nodo> SemanticAnalyzer::buscar_expresion_asignada(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return nullptr;
    if (nodo->hijos.size() < 1) return nullptr;

    int i = 0;
    for (; i < nodo->hijos.size() - 1; i++) {
        if (nodo->hijos[i]->valor == "=" || nodo->hijos[i]->simbolo_sintactico == "=") {
            return nodo->hijos[i+1];
        }
    }

    for (const auto& hijo : nodo->hijos) {
        shared_ptr<Nodo> res = buscar_expresion_asignada(hijo);
        if (res) return res;
    }

    return nullptr;
}

shared_ptr<Nodo> SemanticAnalyzer::buscarIdentificadorIzquierdo(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return nullptr;
    if (nodo->token == TipoLexema::IDENTIFICADOR)
        return nodo;

    for (const auto& hijo : nodo->hijos) {
        shared_ptr<Nodo> id = buscarIdentificadorIzquierdo(hijo);
        if (id) return id;
    }
    return nullptr;
}

shared_ptr<Nodo> SemanticAnalyzer::buscarSubexpresion(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return nullptr;
    if (nodo->valor == "E" || nodo->valor == "B" || nodo->valor == "R" ||
        nodo->valor == "A" || nodo->valor == "T" || nodo->valor == "F") {
        return nodo;
        }

    for (const auto& hijo : nodo->hijos) {
        shared_ptr<Nodo> expr = buscarSubexpresion(hijo);
        if (expr) return expr;
    }
    return nullptr;
}


bool SemanticAnalyzer::isAssignment(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return false;

    // Un árbol de asignación siempre tiene símbolo raíz SENTENCE
    if (nodo->valor != "SENTENCE") return false;

    // Caso 1: asignación simple (id = ...)
    if (!nodo->hijos.empty() && nodo->hijos[0]->token == TipoLexema::IDENTIFICADOR) {
        // Buscar '=' en cualquier hijo o nieto inmediato
        for (const auto& hijo : nodo->hijos) {
            if (hijo->valor == "=" || hijo->simbolo_sintactico == "=")
                return true;
            for (const auto& sub : hijo->hijos) {
                if (sub->valor == "=" || sub->simbolo_sintactico == "=")
                    return true;
            }
        }
    }

    // Caso 2: declaración con asignación (entero id = ...)
    if (!nodo->hijos.empty() && nodo->hijos[0]->valor == "entero") {
        for (const auto& hijo : nodo->hijos) {
            if (hijo->valor == "=" || hijo->simbolo_sintactico == "=")
                return true;
            for (const auto& sub : hijo->hijos) {
                if (sub->valor == "=" || sub->simbolo_sintactico == "=")
                    return true;
            }
        }
    }

    return false;
}


// ------------------ If / While: chequear condicion y cuerpo en scope nuevo ------------------
void SemanticAnalyzer::analizarIf(const shared_ptr<Nodo>& nodo) {
    // Esperamos estructura: SENTENCE |--- si |--- ( |--- B (cond) |--- ) |--- (body)
    // find cond subtree (B) - typically it's at index 2 in your tree
    shared_ptr<Nodo> cond = nullptr;
    if (nodo->hijos.size() >= 3) cond = nodo->hijos[2];
    else {
        // search for a B subtree
        for (auto &c : nodo->hijos) if (c && (c->valor == "B" || c->valor == "VERDADERO" || c->valor == "FALSO")) { cond = c; break; }
    }
    if (!cond) {
        pushError("Condicion de 'si' no encontrada", nodo);
    } else {
        DataType t = analizarExpresion(cond, true);
        if (t != DataType::BOOL) pushError("Condicion del 'si' debe ser boolean (found " + typeTagToStr(t) + ")", cond);
    }
    // body: might be absent or next sibling; if exists, analyze in new scope
    if (nodo->hijos.size() >= 4) {
        pushScope();
        analizarInstruccion(nodo->hijos[3]);
        popScope();
    }
}

void SemanticAnalyzer::analizarWhile(const shared_ptr<Nodo>& nodo) {
    // similar a if
    shared_ptr<Nodo> cond = nullptr;
    if (nodo->hijos.size() >= 3) cond = nodo->hijos[2];
    if (!cond) {
        for (auto &c : nodo->hijos) if (c && c->valor == "B") { cond = c; break; }
    }
    if (!cond) {
        pushError("Condicion de 'mientras' no encontrada", nodo);
    } else {
        DataType t = analizarExpresion(cond, true);
        if (t != DataType::BOOL) pushError("Condicion de 'mientras' debe ser boolean (found " + typeTagToStr(t) + ")", cond);
    }
    if (nodo->hijos.size() >= 4) {
        pushScope();
        analizarInstruccion(nodo->hijos[3]);
        popScope();
    }
}

// ------------------ Return: chequear expresion (si la hay) ------------------
void SemanticAnalyzer::analizarReturn(const shared_ptr<Nodo>& nodo) {
    // find expression subtree after 'retornar'
    if (nodo->hijos.size() >= 2) {
        shared_ptr<Nodo> expr = nodo->hijos[1];
        DataType t = analizarExpresion(expr, true);
        (void)t;
    } else {
        // return without expr — in this language maybe allowed or not; we warn
        pushError("Return sin expresion (verificar si se permite)", nodo);
    }
}

// ------------------ Function declaration ------------------
// Detecta "funcion entero sumar ( parametros )" con body opcional. Comprueba la existencia de 'retornar' en body si tipo != void
void SemanticAnalyzer::analizarFuncion(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;
    // forma esperada: SENTENCE |--- funcion |--- tipo |--- nombre |--- ( |--- parametros |--- ) ...
    if (nodo->hijos.size() < 3) {
        pushError("Declaracion de funcion mal formada", nodo);
        return;
    }
    string tipoStr = nodo->hijos[1]->valor;
    DataType tipoRet = mapearTipo(tipoStr);
    string nombre = nodo->hijos[2]->valor;
    // registrar funcion en entorno actual (como símbolo con tipo de retorno)
    if (entornoActual().existeLocal(nombre)) {
        pushError("Funcion/identificador redeclarado en mismo scope: " + nombre, nodo);
        return;
    }
    // declarar funcion (marcar inicializada true)
    entornoActual().declararLocal(nombre, tipoRet, true);
    // crear nuevo scope para parametros y cuerpo
    pushScope();
    // parametros (if present) at index 4 maybe
    shared_ptr<Nodo> params = nullptr;
    shared_ptr<Nodo> body = nullptr;
    for (auto &c : nodo->hijos) {
        if (!c) continue;
        if (c->valor == "parametros") params = c;
        if (c->valor == "BLOCK" || c->valor == "BLOQUE") body = c;
    }
    // register params
    if (params) {
        // params structure: parametros -> tipo, id, G'
        for (auto &p : params->hijos) {
            if (p->hijos.size() >= 2) {
                string pTipo = p->hijos[0]->valor;
                string pName = p->hijos[1]->valor;
                DataType tp = mapearTipo(pTipo);
                if (tp == DataType::UNKNOWN && pTipo != ",") pushError("Tipo de parametro desconocido: " + pTipo, p);
                else entornoActual().declararLocal(pName, tp, true);
            } else if (p->token == TipoLexema::IDENTIFICADOR) {
                // fallback: param with only id -> default float? we'll mark unknown
                entornoActual().declararLocal(p->valor, DataType::UNKNOWN, true);
            }
        }
    }
    // analyze body and check for return
    bool foundReturn = false;
    if (body) {
        // body->hijos are statements
        for (auto &stmt : body->hijos) {
            if (stmt && stmt->valor == "SENTENCE" && !stmt->hijos.empty() && stmt->hijos[0]->valor == "retornar")
                foundReturn = true;
            analizarInstruccion(stmt);
        }
    } else {
        pushError("Funcion sin cuerpo: " + nombre, nodo);
    }
    if (!foundReturn && tipoRet != DataType::VOID) {
        pushError("Funcion '" + nombre + "' sin 'retornar' (tipo esperado: " + typeTagToStr(tipoRet) + ")", nodo);
    }
    popScope();
}

// ------------------ ANALIZAR EXPRESION (deducir tipo) ------------------
// Esta funcion es robusta frente a la forma de tu AST (nodos intermedios A,R,B,T...),
// busca operadores conocidos y evalúa recursivamente.
DataType SemanticAnalyzer::analizarExpresion(const shared_ptr<Nodo>& nodo, bool isAsignacion) {
    if (!nodo) return DataType::UNKNOWN;

    if (nodo->token == TipoLexema::ENTERO) {
        return DataType::INT;
    }

    if (nodo->token == TipoLexema::FLOTANTE) {
        return DataType::FLOAT;
    }

    // caso hoja: boolean literal (you used "VERDADERO" etc)
    if (nodo->token == TipoLexema::BOOLEANO
        || nodo->valor == "VERDADERO"
        || nodo->valor == "FALSO"
        || nodo->simbolo_sintactico == "FALSO"
        || nodo->string_token == "FALSO") {
        return DataType::BOOL;
    }
    // caso hoja: identificador
    if (nodo->token == TipoLexema::IDENTIFICADOR && nodo->hijos.empty()) {
        SymbolInfo* s = buscarEnCadena(nodo->valor);
        if (!s) {
            pushError("(AE) Uso de variable no declarada: " + nodo->valor, nodo);
            return DataType::UNKNOWN;
        }
        if (!s->inicializado && isAsignacion) {
            pushError("(AE) Uso de variable no inicializada: " + nodo->valor, nodo);
            // devolvemos su tipo para continuar el analisis (evitar cascada)
            return s->tipo;
        }
        return s->tipo;
    }

    // Si el nodo es un operador conocido (y tiene 2 hijos) -> evaluar binario
    if (isOperator(nodo->valor) && nodo->hijos.size() >= 2) {
        DataType L = analizarExpresion(nodo->hijos[0], true);
        DataType R = analizarExpresion(nodo->hijos[1], true);
        return tipoResultadoOperator(nodo->valor, L, R, nodo);
    }

    // Si no es operador pero tiene hijos: buscar dentro de los hijos (algunos nodos A,R,B... contiene operadores como hijos)
    // Priorizar hijos que son operadores
    for (auto &c : nodo->hijos) {
        if (!c) continue;
        if (isOperator(c->valor)) {
            DataType res = analizarExpresion(c, isAsignacion);
            if (res != DataType::UNKNOWN) return res;
        }
    }
    // si no se encontro operador directamente, procesar hijos y devolver el primer tipo valido
    for (auto &c : nodo->hijos) {
        if (!c) continue;
        DataType t = analizarExpresion(c, isAsignacion);
        if (t != DataType::UNKNOWN) return t;
    }
    // si nada
    return DataType::UNKNOWN;
}

bool SemanticAnalyzer::isOperator(const string& s) {
    static const string ops[] = { "+","-","*","/","<",">","<=",">=","==","!=","&&","||" };
    for (auto &op : ops) if (s == op) return true;
    return false;
}

DataType SemanticAnalyzer::tipoResultadoOperator(const string& op, DataType L, DataType R, shared_ptr<Nodo> node) {
    // if either unknown, propagate unknown
    if (L == DataType::UNKNOWN || R == DataType::UNKNOWN) {
        // but still may report invalid combos
        // don't duplicate errors
    }
    // aritméticos
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (L == DataType::BOOL || R == DataType::BOOL) {
            pushError("Operacion aritmética con booleano", node);
            return DataType::UNKNOWN;
        }
        if (L == DataType::FLOAT || R == DataType::FLOAT) return DataType::FLOAT;
        if (L == DataType::INT && R == DataType::INT) return DataType::INT;
        // if one is unknown but the other int/float: assume numeric -> propagate numeric type if possible
        if ((L == DataType::INT || L == DataType::FLOAT) && R == DataType::UNKNOWN) return L;
        if (L == DataType::UNKNOWN && (R == DataType::INT || R == DataType::FLOAT)) return R;
        return DataType::UNKNOWN;
    }
    // relacionales -> bool (permitir int/float combos)
    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if ((L == DataType::INT || L == DataType::FLOAT) && (R == DataType::INT || R == DataType::FLOAT)) return DataType::BOOL;
        pushError("Comparacion entre tipos incompatibles: " + typeTagToStr(L) + " vs " + typeTagToStr(R), node);
        return DataType::UNKNOWN;
    }
    if (op == "==" || op == "!=") {
        // allow equality across same categories (numbers together, bool with bool)
        if ((L == DataType::INT || L == DataType::FLOAT) && (R == DataType::INT || R == DataType::FLOAT)) return DataType::BOOL;
        if (L == DataType::BOOL && R == DataType::BOOL) return DataType::BOOL;
        pushError("Operador ==/!= entre tipos incompatibles", node);
        return DataType::UNKNOWN;
    }
    // logicos
    if (op == "&&" || op == "||") {
        if (L != DataType::BOOL || R != DataType::BOOL) {
            pushError("Operador logico con operandos no booleanos", node);
            return DataType::UNKNOWN;
        }
        return DataType::BOOL;
    }
    // unknown operator
    pushError("Operador no soportado en analisis semantico: " + op, node);
    return DataType::UNKNOWN;
}

// ------------------ utilitarios ------------------
DataType SemanticAnalyzer::mapearTipo(const string& s) {
    if (s == "entero") return DataType::INT;
    if (s == "flotante") return DataType::FLOAT;
    if (s == "booleano") return DataType::BOOL;
    if (s == "cadena") return DataType::STRING;
    if (s == "caracter") return DataType::CHAR;
    if (s == "vacio") return DataType::VOID;
    return DataType::UNKNOWN;
}

bool SemanticAnalyzer::tiposCompatibles(DataType left, DataType right) {
    if (left == right) return true;
    // allow int -> float
    if (left == DataType::FLOAT && right == DataType::INT) return true;
    if (left == DataType::STRING && right == DataType::CHAR) return true;
    return false;
}

void SemanticAnalyzer::exec_semantic(const shared_ptr<Nodo>& raiz) {
    cout << "\n\n === ANALISIS SEMANTICO DE LA EXPRESION \n";

    //pushScope();
    if (entornos.size() < 1) pushScope();

    /*if (raiz->hijos.size() > 0 && raiz->hijos[0]->valor == "{") {
        pushScope();
    }

    if (raiz->hijos.size() > 0 && raiz->hijos[0]->valor == "}") {
        popScope();
    }*/

    analizarInstruccion(raiz);

    // Print errors
    const auto& errs = getErrors();
    if (errs.empty()) {
        cout << "\nNo se encontraron errores semanticos.\n";
    } else {
        cout << "\nErrores semanticos encontrados (" << errs.size() << "):\n";
        for (auto &e : errs) {
            cout << " - " << e.msg << "\n";
        }
    }

    eraseErrors();
}


void SemanticAnalyzer::declararLocal(const string& id, DataType tipo, bool init) {
    tabla[id] = SymbolInfo(tipo, init);
}
bool SemanticAnalyzer::existeLocal(const string& id) const {
    //return tabla.find(id) != tabla.end();
    return tabla.count(id) > 0;
}

void SemanticAnalyzer::imprimirTablaGlobal() {
    for (int i = (int)entornos.size() - 1; i >= 0; --i) {
        auto tabla = entornos[i].tabla;
        cout << "----- TABLA DE SIMBOLOS -----" << endl;
        cout<<"IDENTIFICADOR              TIPO       INICIALIZADO\n";
        for (auto it = tabla.begin(); it != tabla.end(); ++it) {
            cout <<setw(20)<< it->first <<"     ";
            switch (it->second.tipo) {
                case DataType::INT:      cout << "entero  "; break;
                case DataType::FLOAT:    cout << "flotante"; break;
                case DataType::STRING:   cout << "cadena  "; break;
                case DataType::BOOL:     cout << "booleano"; break;
                case DataType::CHAR:     cout << "caracter"; break;
            }
            cout << "        " << (it->second.inicializado ? "SI" : "NO") << endl;
        }
        cout << "------------------------------" << endl;
    }
}