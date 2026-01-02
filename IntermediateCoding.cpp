#include "IntermediateCoding.h"
#include <iomanip>

using namespace std;

string IntermediateCoding::generarDesdeAST(const shared_ptr<Nodo>& nodo) {
    cuadruplos.clear();
    tempCount = 1;
    labelCount = 1;

    if (!nodo) return "";

    // Si root es Program o Scope -> generar block
    if (nodo->type == ParseType::Program || nodo->type == ParseType::Scope) {
        generarBlock(nodo);
    } else {
        // intentar generar como bloque con un solo nodo
        generarStmt(nodo);
    }

    if (nodo->token == TipoLexema::IDENTIFICADOR || nodo->token == TipoLexema::CONSTANTE)
        return nodo->simbolo_sintactico;

    if (nodo->type == ParseType::Op) {
        string arg1 = generarDesdeAST(nodo->hijos[0]);
        string arg2 = nodo->hijos.size() > 1 ? generarDesdeAST(nodo->hijos[1]) : "_";
        string temp = generarTemporal();
        cuadruplos.push_back({ nodo->simbolo_sintactico, arg1, arg2, temp });
        return temp;
    }

    if (nodo->simbolo_sintactico == "if") {
        string L1 = generarEtiqueta();
        string L2 = generarEtiqueta();
        string cond = generarDesdeAST(nodo->hijos[0]);
        cuadruplos.push_back({ "IF_FALSE", cond, "_", L1 });
        generarDesdeAST(nodo->hijos[1]);
        cuadruplos.push_back({ "GOTO", "_", "_", L2 });
        cuadruplos.push_back({ "LABEL", "_", "_", L1 });
        cuadruplos.push_back({ "LABEL", "_", "_", L2 });
    }

    if (nodo->simbolo_sintactico == "while") {
        string L1 = generarEtiqueta();
        string L2 = generarEtiqueta();
        cuadruplos.push_back({ "LABEL", "_", "_", L1 });
        string cond = generarDesdeAST(nodo->hijos[0]);
        cuadruplos.push_back({ "IF_FALSE", cond, "_", L2 });
        generarDesdeAST(nodo->hijos[1]);
        cuadruplos.push_back({ "GOTO", "_", "_", L1 });
        cuadruplos.push_back({ "LABEL", "_", "_", L2 });
    }

    if (nodo->simbolo_sintactico == "for") {
        string L1 = generarEtiqueta();
        string L2 = generarEtiqueta();
        generarDesdeAST(nodo->hijos[0]);  // init
        cuadruplos.push_back({ "LABEL", "_", "_", L1 });
        string cond = generarDesdeAST(nodo->hijos[1]);
        cuadruplos.push_back({ "IF_FALSE", cond, "_", L2 });
        generarDesdeAST(nodo->hijos[2]);  // cuerpo
        cuadruplos.push_back({ "GOTO", "_", "_", L1 });
        cuadruplos.push_back({ "LABEL", "_", "_", L2 });
    }

    return generar(nodo);
    //return "";
   /* cuadruplos.clear();
    return generar(nodo);*/
}

void IntermediateCoding::mostrarCuadruplos() const{
    cout << "\n--- CODIGO INTERMEDIO (CUADRUPLOS) ---\n";
    for (const auto& c : cuadruplos) {
        cout << "(" << c.op << ", " << c.arg1 << ", " << c.arg2 << ", " << c.resultado << ")\n";
    }
}

void IntermediateCoding::mostrarCuadruplos(vector<Cuadruplo> cuad) const{
    cout << "\n--- CODIGO INTERMEDIO (CUADRUPLOS) ---\n";
    for (const auto& c : cuad) {
        cout << "(" << c.op << ", " << c.arg1 << ", " << c.arg2 << ", " << c.resultado << ")\n";
    }
}

string IntermediateCoding::nuevaTemporal() {
    return "t" + to_string(++tempCount);
}

string IntermediateCoding::generar(shared_ptr<Nodo> nodo) {
    if (!nodo) return "";

    // Si es literal o identificador → devuelve su valor
    if (nodo->token == TipoLexema::IDENTIFICADOR ||
        nodo->token == TipoLexema::CADENA ||
        nodo->token == TipoLexema::ENTERO ||
        nodo->token == TipoLexema::FLOTANTE ||
        nodo->token == TipoLexema::CARACTER ||
        nodo->token == TipoLexema::BOOLEANO)
        return nodo->valor;

    // Casos base
    if (nodo->type == ParseType::Id || nodo->type == ParseType::Literal)
        return nodo->simbolo_sintactico;

    // IF
    if (nodo->simbolo_sintactico == "if" || nodo->type == ParseType::Conditional) {
        string cond = generar(nodo->hijos[0]);
        /*cuadruplos.push_back({"IF_FALSE", cond, "", "L1"});
        cuadruplos.push_back({"GOTO", "", "", "L2"});
        return "";*/
        string etiquetaFalsa = "L" + to_string(tempCount + 1);
        cuadruplos.push_back({"IF_FALSE", cond, "", etiquetaFalsa});
        generar(nodo->hijos[1]); // bloque verdadero
        cuadruplos.push_back({"LABEL", "", "", etiquetaFalsa});
        return "";
    }

    // WHILE
    if (nodo->type == ParseType::Loop && nodo->simbolo_sintactico == "while") {
        /*string cond = generar(nodo->hijos[0]);
        cuadruplos.push_back({"LABEL", "", "", "L1"});
        cuadruplos.push_back({"IF_FALSE", cond, "", "L2"});
        cuadruplos.push_back({"GOTO", "", "", "L1"});
        return "";*/
        string etiquetaInicio = "L" + to_string(tempCount + 1);
        string etiquetaFin = "L" + to_string(tempCount + 2);
        cuadruplos.push_back({"LABEL", "", "", etiquetaInicio});
        string cond = generar(nodo->hijos[0]);
        cuadruplos.push_back({"IF_FALSE", cond, "", etiquetaFin});
        generar(nodo->hijos[1]);
        cuadruplos.push_back({"GOTO", "", "", etiquetaInicio});
        cuadruplos.push_back({"LABEL", "", "", etiquetaFin});
        return "";
    }

    // FOR
    if (nodo->simbolo_sintactico == "for") {
        /*string inicio = generar(nodo->hijos[0]);
        string cond = generar(nodo->hijos[1]);
        string inc = generar(nodo->hijos[2]);
        cuadruplos.push_back({"FOR", inicio, cond, inc});
        return "";*/
        string etiquetaInicio = "L" + to_string(tempCount + 1);
        string etiquetaFin = "L" + to_string(tempCount + 2);
        generar(nodo->hijos[0]); // inicialización
        cuadruplos.push_back({"LABEL", "", "", etiquetaInicio});
        string cond = generar(nodo->hijos[1]); // condición
        cuadruplos.push_back({"IF_FALSE", cond, "", etiquetaFin});
        generar(nodo->hijos[3]); // cuerpo
        generar(nodo->hijos[2]); // incremento
        cuadruplos.push_back({"GOTO", "", "", etiquetaInicio});
        cuadruplos.push_back({"LABEL", "", "", etiquetaFin});
        return "";
    }

    // ------------------------------
    // 📌 Asignaciones: a = b + c;
    // ------------------------------
    if (nodo->type == ParseType::Assign && nodo->hijos.size() == 2) {
        string lhs = generar(nodo->hijos[0]);
        string rhs = generar(nodo->hijos[1]);
        cuadruplos.push_back({"=", rhs, "", lhs});
        return lhs;
    }

    // ------------------------------
    // 📌 Expresiones binarias
    // ------------------------------
    if (nodo->type == ParseType::Expression && nodo->hijos.size() == 3) {
        string arg1 = generar(nodo->hijos[0]);
        string op = nodo->hijos[1]->valor;
        string arg2 = generar(nodo->hijos[2]);
        string temp = nuevaTemporal();
        cuadruplos.push_back({op, arg1, arg2, temp});
        return temp;
    }


    // Operadores
    if (nodo->token == TipoLexema::OPERADOR && nodo->hijos.size() >= 2) {
        string izq = generar(nodo->hijos[0]);
        string der = generar(nodo->hijos[1]);
        string temp = nuevaTemporal();
        cuadruplos.push_back({nodo->simbolo_sintactico, izq, der, temp});
        return temp;
    }

    // ------------------------------
    // 📌 IMPRIMIR
    // ------------------------------
    /*if (nodo->type == NodeType::Function && nodo->simbolo == "imprimir") {
        string arg = generar(nodo->hijos[0]);
        cuadruplos.push_back({"PRINT", arg, "", ""});
        return "";
    }*/

    // --------------------------------------------
    // 📘 FUNCIÓN IMPRIMIR
    // --------------------------------------------
    if (nodo->simbolo_sintactico == "imprimir" || nodo->valor == "imprimir") {
        //cout<<"entra\n";
        if (!nodo->hijos.empty()) {
            string arg = generar(nodo->hijos[0]);  // puede ser literal o expresión
            cuadruplos.push_back({"PRINT", arg, "", ""});
        } else {
            cuadruplos.push_back({"PRINT", "\"\"", "", ""}); // imprimir vacío
        }
        return "";
    }

    // ------------------------------
    // 📌 Recursión genérica
    // ------------------------------
    for (auto& h : nodo->hijos)
        generar(h);

    return "";
}

string IntermediateCoding::newTemp() {
    return "t" + to_string(tempCount++);
}

string IntermediateCoding::newLabel() {
    return "L" + to_string(labelCount++);
}

// Genera y devuelve el nombre (temp o id) de la expresión
string IntermediateCoding::generarExpr(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return "";

    // Identificador o literal: devolvemos su nombre/valor
    if (nodo->type == ParseType::Id || nodo->type == ParseType::Literal) {
        if (!nodo->valor.empty()) return nodo->valor;
        return nodo->simbolo_sintactico;
    }

    // Si es un statement que contiene la expresión dentro
    if (nodo->type == ParseType::Statement && !nodo->hijos.empty()) {
        return generarExpr(nodo->hijos[0]);
    }

    // Si es un bloque/Scope con primer hijo expresión -> delegar
    if (nodo->type == ParseType::Scope || nodo->type == ParseType::Program) {
        if (!nodo->hijos.empty()) return generarExpr(nodo->hijos[0]);
    }

    // Si es operador
    if (nodo->type == ParseType::Op || nodo->type == ParseType::Assign || nodo->type == ParseType::Expression) {
        string op = nodo->simbolo_sintactico;

        // Asignación especial: lhs es identificador, rhs es expresión
        if (op == "=" && nodo->hijos.size() >= 2) {
            string lhs;
            // lhs puede ser nodo Id
            if (nodo->hijos[0]) {
                if (!nodo->hijos[0]->valor.empty()) lhs = nodo->hijos[0]->valor;
                else lhs = nodo->hijos[0]->simbolo_sintactico;
            }

            string rhs = generarExpr(nodo->hijos[1]);
            // Generar cuadruplo de asignación: (=, rhs, _, lhs)
            cuadruplos.push_back({ "=", rhs, "_", lhs });
            return lhs;
        }

        // Unario
        if (nodo->hijos.size() == 1) {
            string a = generarExpr(nodo->hijos[0]);
            string t = newTemp();
            cuadruplos.push_back({ op, a, "_", t });
            return t;
        }

        // Binario
        if (nodo->hijos.size() >= 2) {
            string a = generarExpr(nodo->hijos[0]);
            string b = generarExpr(nodo->hijos[1]);
            string t = newTemp();
            cuadruplos.push_back({ op, a, b, t });
            return t;
        }
    }

    // Conditional / Loop / For nodes used as expressions? improbable, tratar como error silencioso
    return "";
}

// Genera cuádruplos para un statement (incluye control flow)
void IntermediateCoding::generarStmt(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;

    // Si es un nodo Program o Scope con multiples hijos -> tratar cada hijo como stmt
    if (nodo->type == ParseType::Program || nodo->type == ParseType::Scope) {
        generarBlock(nodo);
        return;
    }

    // Si es Statement que contiene una expresión u operador
    if (nodo->type == ParseType::Statement) {
        if (!nodo->hijos.empty()) {
            // El primer hijo es la expresión/operador real
            auto child = nodo->hijos[0];
            // Si child es un control flow, tratarlo abajo
            if (child->simbolo_sintactico == "if") {
                // if cond then [else]
                // formato esperado: hijos = { cond, thenBlock, [elseBlock] }
                string Lfalse = newLabel();
                string Lend = newLabel();

                string condTmp = generarExpr(child->hijos[0]);
                // if false -> goto Lfalse
                cuadruplos.push_back({ "IF_FALSE", condTmp, "_", Lfalse });

                // then
                generarStmt(child->hijos[1]);
                // after then -> goto end
                cuadruplos.push_back({ "GOTO", "_", "_", Lend });

                // else label
                cuadruplos.push_back({ "LABEL", "_", "_", Lfalse });
                if (child->hijos.size() >= 3 && child->hijos[2]) {
                    generarStmt(child->hijos[2]);
                }

                // end label
                cuadruplos.push_back({ "LABEL", "_", "_", Lend });
                return;
            }
            else if (child->simbolo_sintactico == "if-else") {
                // alternativa si tu parser creó label distinto
                // manejar igual que 'if'
            }
            else if (child->simbolo_sintactico == "while") {
                // while cond cuerpo
                // hijos: { cond, body }
                string Lstart = newLabel();
                string Lfalse = newLabel();

                cuadruplos.push_back({ "LABEL", "_", "_", Lstart });
                string condTmp = generarExpr(child->hijos[0]);
                cuadruplos.push_back({ "IF_FALSE", condTmp, "_", Lfalse });

                // cuerpo
                generarStmt(child->hijos[1]);

                // goto inicio
                cuadruplos.push_back({ "GOTO", "_", "_", Lstart });
                cuadruplos.push_back({ "LABEL", "_", "_", Lfalse });
                return;
            }
            else if (child->simbolo_sintactico == "for") {
                // for init; cond; inc { body }
                // hijos esperados: { init, cond, inc, body } o { init, cond, body } (sin inc)
                string Lstart = newLabel();
                string Lfalse = newLabel();

                // init
                if (child->hijos.size() >= 1 && child->hijos[0]) generarStmt(child->hijos[0]);

                // start label
                cuadruplos.push_back({ "LABEL", "_", "_", Lstart });

                // check cond
                if (child->hijos.size() >= 2 && child->hijos[1]) {
                    string condTmp = generarExpr(child->hijos[1]);
                    cuadruplos.push_back({ "IF_FALSE", condTmp, "_", Lfalse });
                }

                // body
                if (child->hijos.size() >= 4 && child->hijos[3]) {
                    generarStmt(child->hijos[3]); // cuerpo
                } else if (child->hijos.size() == 3 && child->hijos[2]) {
                    // ambiguous layout: init, cond, body OR init, cond, inc?
                    // try treat as body if simbolo indicates block
                    generarStmt(child->hijos[2]);
                }

                // inc
                if (child->hijos.size() >= 3) {
                    // common layout: init, cond, inc, body OR init, cond, body (no inc)
                    // if there's an inc node and it's not the body
                    if (child->hijos.size() == 4 && child->hijos[2]) {
                        generarStmt(child->hijos[2]); // incremento
                    } else if (child->hijos.size() == 3) {
                        // uncertain: we already consumed child[2] as body above, skip
                    }
                }

                // goto start
                cuadruplos.push_back({ "GOTO", "_", "_", Lstart });
                cuadruplos.push_back({ "LABEL", "_", "_", Lfalse });
                return;
            }
            else {
                // No es control flow -> puede ser una expresion (asignacion u operacion)
                generarExpr(child);
                return;
            }
        }
        return;
    }

    // Si el nodo directamente es control flow (no envuelto en Statement)
    if (nodo->simbolo_sintactico == "if") {
        // manejar igual que arriba pero para nodo directo
        string Lfalse = newLabel();
        string Lend = newLabel();

        string condTmp = generarExpr(nodo->hijos[0]);
        cuadruplos.push_back({ "IF_FALSE", condTmp, "_", Lfalse });

        // then
        generarStmt(nodo->hijos[1]);
        cuadruplos.push_back({ "GOTO", "_", "_", Lend });

        cuadruplos.push_back({ "LABEL", "_", "_", Lfalse });
        if (nodo->hijos.size() >= 3 && nodo->hijos[2]) generarStmt(nodo->hijos[2]);
        cuadruplos.push_back({ "LABEL", "_", "_", Lend });
        return;
    }

    if (nodo->simbolo_sintactico == "while") {
        string Lstart = newLabel();
        string Lfalse = newLabel();
        cuadruplos.push_back({ "LABEL", "_", "_", Lstart });
        string condTmp = generarExpr(nodo->hijos[0]);
        cuadruplos.push_back({ "IF_FALSE", condTmp, "_", Lfalse });
        generarStmt(nodo->hijos[1]);
        cuadruplos.push_back({ "GOTO", "_", "_", Lstart });
        cuadruplos.push_back({ "LABEL", "_", "_", Lfalse });
        return;
    }

    if (nodo->simbolo_sintactico == "for") {
        // same as above direct node
        string Lstart = newLabel();
        string Lfalse = newLabel();

        if (nodo->hijos.size() >= 1 && nodo->hijos[0]) generarStmt(nodo->hijos[0]); // init
        cuadruplos.push_back({ "LABEL", "_", "_", Lstart });
        if (nodo->hijos.size() >= 2 && nodo->hijos[1]) {
            string condTmp = generarExpr(nodo->hijos[1]);
            cuadruplos.push_back({ "IF_FALSE", condTmp, "_", Lfalse });
        }
        if (nodo->hijos.size() >= 4 && nodo->hijos[3]) generarStmt(nodo->hijos[3]); // body
        else if (nodo->hijos.size() == 3 && nodo->hijos[2]) generarStmt(nodo->hijos[2]); // body maybe here

        if (nodo->hijos.size() >= 3 && nodo->hijos[2]) {
            // inc could be hijo[2] in layout init, cond, inc, body OR init, cond, body(where child[2] is body)
            // attempt to detect by token content: if child[2] is an operator or assign we treat as inc
            auto possibleInc = nodo->hijos[2];
            if (possibleInc && (possibleInc->type == ParseType::Op || possibleInc->simbolo_sintactico == "=")) {
                generarStmt(possibleInc);
            }
        }

        cuadruplos.push_back({ "GOTO", "_", "_", Lstart });
        cuadruplos.push_back({ "LABEL", "_", "_", Lfalse });
        return;
    }

    // default: try expression
    generarExpr(nodo);
}

// Genera secuencia de statements (si nodo tiene varios hijos)
void IntermediateCoding::generarBlock(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;
    for (auto &h : nodo->hijos) {
        generarStmt(h);
    }
}


// ------------------------------
// 🖨️ Imprimir cuádruplos
// ------------------------------
void IntermediateCoding::imprimirCuadruplos() {
    cout << "\n=== CÓDIGO INTERMEDIO (CUÁDRUPLOS) ===\n";
    cout << left << setw(10) << "OP" << setw(15) << "ARG1" << setw(15)
         << "ARG2" << setw(15) << "RESULT" << "\n";
    cout << string(55, '-') << "\n";
    for (auto& c : cuadruplos) {
        cout << setw(10) << c.op << setw(15) << c.arg1 << setw(15)
             << c.arg2 << setw(15) << c.resultado << "\n";
    }
}