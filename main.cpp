#include <iostream>
#include <iostream>//ENTRADA Y SALIDA
#include <fstream>// FILE STRAM
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iomanip>//setw
#include <memory>  //NECESARIA PARA LOS NODOS, INCLUYE SHARED_PTR
#include "tokens.h"
#include "lexer.h"
#include "symbol_table.h"
#include "astree.h"
#include "generador_asm.h"
#include "IntermediateCoding.h"
#include "scopes.h"
#include "sintaxis.h"
#include "assembler_driver.h"
#include "ast_analizador.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.

int execute_lexer(SymbolTable& table, vector<Token>& flujo_tokens) {
    //Iniciar analisis lexico
    ifstream archivo("./marco.arc");
    if (!archivo.is_open()) {
        cerr << "No se pudo abrir el archivo." << endl;
        return 1;
    }

    stringstream buffer;
    buffer << archivo.rdbuf();
    string codigo = buffer.str();
    archivo.close();

    AnalizadorLexico lexer(codigo, table);

    cout <<"\n\n\nGENERANDO ARCHIVO DE CODIGO DEPURADO\n";
    lexer.limpiarArchivoFuente();

    Token token;
    cout <<"\n\n\nANALISIS LEXICO DE LA ENTRADA\n\n";
    do {
        token = lexer.siguienteToken();

        if(token.tipo == TipoLexema::FIN || token.tipo == TipoLexema::ERROR){

            if(token.tipo == TipoLexema::FIN) cout << "Fin de la entrada.\n" << endl;
            else cout << "Error en Analizador Lexico... Proceso finalizado.\n" << endl;
            table.imprimirTablaTokens();

            cout <<"\n\n\nGUARDANDO LA TABLA DE TOKENS" <<endl;

            lexer.cerrarArchivoTokens();
            cout << "La tabla de tokens se ha guardado exitosamente en 'progfte.tab'." << endl;

            cout <<"\n\n\nGUARDANDO LA LISTA DE TOKENS" << endl;
            table.guardarTablaTokensEnArchivo("progfte");
        } else {
            flujo_tokens.push_back(token);
        }
    } while (token.tipo != TipoLexema::FIN && token.tipo != TipoLexema::ERROR);

    /*for (int i = 0; i < flujo_tokens.size(); ++i) {
        lexer.imprimirToken(flujo_tokens[i]);
    }*/

    //getchar();
    return 0;
}//execute_lexer



int main() {
    SymbolTable table;
    SymbolTable *pointer_table = &table;
    //vector<Token> *pointer_flujo_tokens = &flujo_tokens;
    vector<Token> flujo_tokens;


    execute_lexer(*pointer_table, flujo_tokens);
    Parser parser;

    //parser.setTokenInfo(*pointer_flujo_tokens);
    /*cout<<flujo_tokens.size();
    getchar();
    cout<<parser.getTokens().size();
    getchar();*/
    /*vector<vector<TokenInfo>> instrucciones = parser.cargar_instrucciones();
    //stack<scope> scopes;
    vector<scope> scopes;

    scopes.push_back(scope());
    stack<int> indexes;
    indexes.push(0);
    for (int i = 0; i < instrucciones.size(); i++) {
        if (instrucciones[i].end()->simbolo == "{") {
            scopes.push_back(scope());
            indexes.push(scopes.size()-1);
        } else if (instrucciones[i].end()->simbolo == "}") {
            indexes.pop();
        } else {
            scopes.at(indexes.top()).instructios.push_back(instrucciones[i]);
        }
    }

    if (indexes.top() != 0) {
        cout<<"LLAVES DESBALANCEADAS\n\n";
    }
    */

    //EXECUTING PARSING (SINTAX ANALYSIS) AND SEMANTIC SEMANTIC
    parser.execute_sintax();

    //EXECUTING INTERMEDIATE CODING GENERATION
    cout << "\n\n\nINICIO DE GENERACION DE CODIGO INTERMEDIO";
    /*vector<vector<Token>> instrucciones = parser.getInstrucciones();
    vector<vector<string>> instrucciones_str;

    for (int i = 0; i < instrucciones.size(); i++) {
        instrucciones_str.push_back(vector<string>());
        for (int j = 0; j < instrucciones[i].size(); j++) {
            instrucciones_str[i].push_back(instrucciones[i][j].valor_lexema);
        }
    }*/


    AnalizadorAST ast;
    IntermediateCoding inter_cod;
    vector<Cuadruplo> cuadrupos;
   /* for (int i = 0; i < instrucciones_str.size(); i++) {
        //ast.raizAST = nullptr;

        cout<< "\n\nSIGUIENTE INSTRUCCION: ";
        print_instr(instrucciones_str[i]);
        cout << endl;
        ast.analizar(instrucciones_str[i]);
        ast.mostrarAST();
        inter_cod.generarDesdeAST(ast.raizAST);
        for (int i = 0; i < inter_cod.getCuadruplos().size(); ++i) {
            cuadrupos.push_back(inter_cod.getCuadruplos()[i]);
        }
        inter_cod.mostrarCuadruplos();

    }*/

    /*vector<string> flujo_instr_str;
    for (int i = 0; i < flujo_tokens.size(); i++) {
        flujo_instr_str.push_back(flujo_tokens[i].valor_lexema);
    }*/
    ast.analizar(flujo_tokens);
    ast.mostrarAST();
    inter_cod.generarDesdeAST(ast.raizAST);
    inter_cod.mostrarCuadruplos();


    //EXECUTING ASM GENERATION
    string nombre_archivo = "marco";
    cout<<"\n\n";
    string codigoASM = GeneradorASM::generarASM(inter_cod.getCuadruplos());
    cout << codigoASM << endl;
    GeneradorASM::generar_archivo(codigoASM, nombre_archivo, ".asm", "./");

    //ASSEMBLING CODE //GENERATING FILE .OBJ
    AssemblerDriver::generarEXE("programa.asm", "programa.exe", codigoASM);

    return 0;
}