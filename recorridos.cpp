//
// Created by CARLOS URZUA on 10/6/2025.
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
#include "tokens.h"

using namespace std;


// -------- RECORRIDOS --------

// Prefijo (Preorden): Raíz -> Hijos
void recorrerPrefijo(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;
    cout << nodo->valor << " "; // Procesar raíz
    for (auto& hijo : nodo->hijos) {
        recorrerPrefijo(hijo);
    }
}

// Infijo (Inorden): Hijo Izq -> Raíz -> Hijo Der
// *Generalmente pensado para arboles binarios de expresiones*
void recorrerInfijo(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;
    if (nodo->hijos.size() == 2) {
        recorrerInfijo(nodo->hijos[0]); // Izquierdo
        cout << nodo->valor << " ";   // Raíz
        recorrerInfijo(nodo->hijos[1]); // Derecho
    } else if (nodo->hijos.size() == 1) {
        recorrerInfijo(nodo->hijos[0]);
        cout << nodo->valor << " ";
    } else {
        cout << nodo->valor << " "; // Hoja
    }
}

// Postfijo (Postorden): Hijos -> Raíz
void recorrerPostfijo(const shared_ptr<Nodo>& nodo) {
    if (!nodo) return;
    for (auto& hijo : nodo->hijos) {
        recorrerPostfijo(hijo);
    }
    cout << nodo->valor << " "; // Procesar raíz
}