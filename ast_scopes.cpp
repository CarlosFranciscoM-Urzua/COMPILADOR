//
// Created by CARLOS URZUA on 9/22/2025.
//

#import "astree.h"

string nodeTypeToString(ParseType t) {
    switch (t) {
        case ParseType::Program: return "Programa";
        case ParseType::Function: return "Principal";
        case ParseType::Statement: return "Sentencia";
        case ParseType::Conditional: return "Condicional_Scope";
        case ParseType::Loop: return "Ciclo_Scope";
    }
    return "";
}