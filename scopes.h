//
// Created by CARLOS URZUA on 9/24/2025.
//

#ifndef COMPILADOR_SCOPES_H
#define COMPILADOR_SCOPES_H
#include <string>
#include <vector>
#include "tokens.h"
#include "astree.h"

struct scope {
    int id;
    string name;
    vector<vector<Token>> instructios;
    vector<shared_ptr<Nodo>> trees;
    vector<Token> controladora;
};

#endif //COMPILADOR_SCOPES_H