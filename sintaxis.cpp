#include "sintaxis.h"
#include "tokens.h"

#include <iostream>//ENTRADA Y SALIDA
#include <fstream>// FILE STRAM
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iomanip>//setw

#include "semantic.h"

#include "symbol_table.h"

// Constructor
Parser::Parser() {
    cargar_tabla();
	tokens = cargarTokenInfos("progfte.tok");
}

void Parser::setTokenInfo(vector<Token>& flujoTokens) {
	//tokens = flujoTokens;
	/*for (int i = 0; i < flujoTokens.size(); i++) {
		tokens.push_back(flujoTokens.at(i));
	}*/
	tokens.assign(flujoTokens.begin(), flujoTokens.end());
}

vector<Token> Parser::getTokens() {
	return tokens;
}



/*bool isOperador(string s) {
	return operadores.find(s) != operadores.end();
}*/

// -------------------------
// Utilidades de tokens
// -------------------------
string Parser::siguiente_token() {
    string token;
    if (idx < instruccion.size()) {
        token = instruccion[idx].valor_lexema;
        //if(token == "fin_principal")  return ";";
        if(token == "declaracion" || token == "fin_declaracion" || token == "principal" || token == "fin_principal") {
            idx++;
            token = instruccion[idx].valor_lexema;
            return siguiente_token();
        }

        if(instruccion[idx].tipo == TipoLexema::IDENTIFICADOR) token = "id";
        if(token == "booleano" || token == "cadena" || token == "entero" ||
           token == "flotante" || token == "caracter") token = "tipo";
        if(instruccion[idx].tipo == TipoLexema::CADENA) token = "cadena";
        if(instruccion[idx].tipo == TipoLexema::CARACTER) token = "caracter";
        if(instruccion[idx].tipo == TipoLexema::ENTERO || instruccion[idx].tipo == TipoLexema::FLOTANTE) token = "num";
        if(instruccion[idx].tipo == TipoLexema::CONSTANTE) token = "id";

    	instruccion[idx].simbolo_sintactico = token;

        idx++;
        return token;
    }
    return ";";
}

string Parser::eliminarEspacios(const string& cadena) {
    string nuevaCadena = "";
    for (char caracter : cadena) {
        if (caracter != ' ') {
            nuevaCadena += caracter;
        }
    }
    return nuevaCadena;
}

// -------------------------
// Manejo de instrucciones
// -------------------------
string Parser::imprimir_instruccion(vector<Token> instruccion) {
    string inst = "";
    string elemento;
    for (int i = 0; i < instruccion.size(); i++) {
        elemento = instruccion[i].valor_lexema;
        inst = inst + " " + elemento;
        if(instruccion[i].valor_lexema == ";") break;
    }
    return inst;
}

string Parser::imprimir_instruccion(vector<string> instruccion) {
    string inst = "";
    for (int i = 0; i < instruccion.size(); i++) {
        inst = inst + " " + instruccion[i];
        if(instruccion[i] == ";") break;
    }
    return inst;
}

vector<string> Parser::instruccion_strings(vector<Token> instruccion) {
    vector<string> cadenas;
    string valor;

    for(int i = 0; i < instruccion.size(); i++) {
        valor = instruccion[i].valor_lexema;

        if(valor == "fin_principal") break;
        else if(valor == "declaracion" || valor == "fin_declaracion" || valor == "principal" ) {
        } else {
            if(instruccion[i].tipo == TipoLexema::IDENTIFICADOR) valor = "id";
            else if(valor == "booleano" || valor == "cadena" || valor == "entero" ||
                    valor == "flotante" || valor == "caracter") valor = "tipo";
            if(instruccion[i].tipo == TipoLexema::CADENA) valor = "cadena";
            if(instruccion[i].tipo == TipoLexema::CARACTER) valor = "caracter";
            if(instruccion[i].tipo == TipoLexema::ENTERO || instruccion[i].tipo == TipoLexema::FLOTANTE) valor = "num";
            if(instruccion[i].tipo == TipoLexema::CONSTANTE) valor = "id";

        	instruccion[i].simbolo_sintactico = valor;
            cadenas.push_back(valor);
        }
    }

    return cadenas;
}

vector<Token> Parser::cargar_siguiente_instruccion() {
    vector<Token> instruccion1;
    int start = last_token;
    string valor;

    int i = start;
    for (; true; i++) {
        valor = tokens[i].valor_lexema;


    	if (valor == "{" || valor == "}") {
    		instruccion1.push_back(tokens[i]);
    		last_token = i+1;
    		break;
    	} else if(valor == "declaracion" || valor == "fin_declaracion" || valor == "principal" || valor == "fin_principal") {
        	//last_token++;
        	continue;
        } else {
            instruccion1.push_back(tokens[i]);
            //last_token++;

        	if (tokens[i+1].valor_lexema == "{" || tokens[i+1].valor_lexema == "}") {
        		last_token = i+1;
        		break;
        	}
        	if(tokens[i].valor_lexema == ";" || tokens[i].valor_lexema == " ;"  || tokens[i].valor_lexema == "PUNTO_COMA") {
        		last_token = i+1;
        		break;
        	}
        }
    }
    return instruccion1;
}

// -------------------------
// Utilidades de pila
// -------------------------
string Parser::printStack(stack<string> pila1) {
    string cadena = "";
    string elemento;
    stack<string> pila2;

    while (!pila1.empty()) {
        elemento = pila1.top();
        pila1.pop();
        pila2.push(elemento);
        cadena = cadena + elemento;
    }

    while (!pila2.empty()) {
        elemento = pila2.top();
        pila2.pop();
        pila1.push(elemento);
    }

    return cadena;
}

// -------------------------
// Cargar tabla LL(1)
// -------------------------
void Parser::cargar_tabla() {
	// Inicio de tabla corregida y organizada para LL(1)

	/*// Símbolo inicial
	// S → id = E ; | E ;
	tabla["S"]["id"] = {"id", "=", "E", ";"};
	tabla["S"]["("] = {"E", ";"};
	tabla["S"]["++"] = {"E", ";"};
	tabla["S"]["--"] = {"E", ";"};
	tabla["S"]["!"] = {"E", ";"};
	tabla["S"]["num"] = {"E", ";"};

	tabla["S"]["imprimir"] = {"imprimir","G", ";"};
	tabla["S"]["tipo"] = {"tipo","id","G", ";"};
	//tabla["S"]["retornar"] = {"retornar","B", ";"};
	tabla["S"]["retornar"] = {"retornar","E", ";"};

	tabla["S"]["si"] = {"si","(","B", ")","{","S", "}"};
	tabla["}"]["si_no"] = {"}","si_no", "{", "S", "}"};
	tabla["S"]["}"] = {};*/

	//------------------------------- S --------------------------------------
	// Símbolo inicial
	//S -> tipo id G;
	tabla["S"]["tipo"] = {"tipo","id","G", ";"};
	tabla["S"]["retornar"] = {"retornar","E", ";"};

	tabla["S"]["si"] = {"si","(","B", ")"}; //{"si","(","B", ")","{","S", "}"};
	tabla["S"]["si_no"] = {"si_no"};        //{"}","si_no", "{", "S", "}"};
	tabla["S"]["}"] = {"}"};
	tabla["S"]["{"] = {"{"};                //{};

	// cuando el primer token puede ser 'id', dejamos que S derive a S'
	tabla["S"]["id"]        = {"id","S'"};

	//Otros inicios de expresiones
	// S → id = E ; | E ;
	tabla["S"]["("] = {"E", ";"};
	tabla["S"]["++"] = {"id", ";"};
	tabla["S"]["--"] = {"id", ";"};
	tabla["S"]["!"] = {"E", ";"};
	tabla["S"]["num"] = {"E", ";"};
	tabla["S"]["cadena"]    = {"E",";"};                   // expresión con cadena
	tabla["S"]["caracter"]  = {"E",";"};                   // expresión con caracter
	tabla["S"]["VERDADERO"] = {"E",";"};                   // boolean literal
	tabla["S"]["FALSO"]     = {"E",";"};                   // boolean literal



	//GRAMATICAS PARA FUNCIONES
	tabla["S"]["funcion"] = {"funcion", "tipo", "id", "(", "parametros",")"};
	tabla["parametros"][")"] = {};
	tabla["parametros"]["tipo"] = {"tipo","id","G'"};
	tabla["G'"][","] = {",", "tipo","id","G'"};
	tabla["G'"][")"] = {};
	//tabla[")"][";"] = {";"};
	//tabla["G"]["tipo"] = {"tipo","id","G"};

	// ----------------- S' -----------------
	// S' decide entre asignación, llamada, expresión simple o continuación
	tabla["S'"]["="]  = {"=","E",";"};         // asignación: id = E ;
	tabla["S'"]["("]  = {"(", "parametros", ")",";"};  // llamada: id(parametros) ;
	tabla["S'"][";"]  = {";"};                // id ;  (expresión que es solo id)
	tabla["S'"][","]  = {",","id","G",";"};    // id, id, ... ;  (si tu lenguaje permite esto)


	tabla["G"]["="] = {"=","E"};
	tabla["G"]["id"] = {"id", "G"};
	tabla["G"][","] = {",", "id", "G"};
	tabla["G"][";"] = {};
	tabla["G"]["cadena"] = {"cadena", "G"};
	tabla["G"]["+"] = {"+", "G"};

	tabla["S"]["imprimir"] = {"imprimir","concatenacion", ";"};
	tabla["cadena"]["+"] = {'+', "concatenacion"};
	tabla["concatenacion"]["+"] = {"+", "concatenacion"};
	tabla["concatenacion"]["cadena"] = {"cadena", "concatenacion"};
	tabla["+"]["id"] = {"id"};
	tabla["concatenacion"]["id"] = {"id", "concatenacion"};
	tabla["concatenacion"][";"] = {};




	// E → B
	// E como punto de entrada a booleanas
	tabla["E"]["id"] = {"B"};
	tabla["E"]["("] = {"B"};
	tabla["E"]["++"] = {"B"};
	tabla["E"]["--"] = {"B"};
	tabla["E"]["!"] = {"B"};
	tabla["E"]["num"] = {"B"};
	tabla["E"]["FALSO"] = {"B"};
	tabla["E"]["VERDADERO"] = {"B"};
	tabla["E"]["leer_entero"] = {"leer_entero"};

	tabla["E"]["caracter"] = {"caracter"};
	tabla["E"]["cadena"] = {"cadena"};

	// B → R B'|VERDADERO|FALSO
	tabla["B"]["id"] = {"R", "B'"};
	tabla["B"]["("] = {"R", "B'"};
	tabla["B"]["++"] = {"R", "B'"};
	tabla["B"]["--"] = {"R", "B'"};
	tabla["B"]["!"] = {"R", "B'"};
	tabla["B"]["num"] = {"R", "B'"};
	tabla["B"]["FALSO"] = {"FALSO"};
	tabla["B"]["VERDADERO"] = {"VERDADERO"};

	// B' → && R B' | || R B' | ^^ R B' | ε
	tabla["B'"]["&&"] = {"&&", "R", "B'"};
	tabla["B'"]["||"] = {"||", "R", "B'"};
	tabla["B'"]["^^"] = {"^^", "R", "B'"};
	tabla["B'"][";"] = {};  // ε
	tabla["B'"][")"] = {};  // ε

	// R → A R'
	tabla["R"]["id"] = {"A", "R'"};
	tabla["R"]["("] = {"A", "R'"};
	tabla["R"]["++"] = {"A", "R'"};
	tabla["R"]["--"] = {"A", "R'"};
	tabla["R"]["!"] = {"A", "R'"};
	tabla["R"]["num"] = {"A", "R'"};

	// R' → == A R' | < A R' | > A R' | <= A R' | >= A R' | ε
	tabla["R'"]["=="] = {"==", "A", "R'"};
	tabla["R'"]["<"] = {"<", "A", "R'"};
	tabla["R'"][">"] = {">", "A", "R'"};
	tabla["R'"]["<="] = {"<=", "A", "R'"};
	tabla["R'"][">="] = {">=", "A", "R'"};
	tabla["R'"]["&&"] = {};  // ε
	tabla["R'"]["||"] = {};  // ε
	tabla["R'"]["^^"] = {};  // ε
	tabla["R'"][";"] = {};   // ε
	tabla["R'"][")"] = {};   // ε

	// A → T A'
	tabla["A"]["id"] = {"T", "A'"};
	tabla["A"]["("] = {"T", "A'"};
	tabla["A"]["++"] = {"T", "A'"};
	tabla["A"]["--"] = {"T", "A'"};
	tabla["A"]["num"] = {"T", "A'"};

	// A' → + T A' | - T A' | += T A' | -= T A' | ε
	tabla["A'"]["+"] = {"+", "T", "A'"};
	tabla["A'"]["-"] = {"-", "T", "A'"};
	tabla["A'"]["+="] = {"+=", "T", "A'"};
	tabla["A'"]["-="] = {"-=", "T", "A'"};
	tabla["A'"]["=="] = {};  // ε
	tabla["A'"]["<"] = {};   // ε
	tabla["A'"][">"] = {};  // ε
	tabla["A'"]["<="] = {};  // ε
	tabla["A'"][">="] = {};  // ε
	tabla["A'"][";"] = {};   // ε
	tabla["A'"]["&&"] = {};  // ε
	tabla["A'"]["||"] = {};  // ε
	tabla["A'"]["^^"] = {};  // ε
	tabla["A'"][")"] = {};   // ε

	// T → F T'
	tabla["T"]["id"] = {"F", "T'"};
	tabla["T"]["("] = {"F", "T'"};
	tabla["T"]["++"] = {"F", "T'"};
	tabla["T"]["--"] = {"F", "T'"};
	tabla["T"]["num"] = {"F", "T'"};

	// T' → * F T' | / F T' | *= F T' | /= F T' | ε
	tabla["T'"]["*"] = {"*", "F", "T'"};
	tabla["T'"]["/"] = {"/", "F", "T'"};
	tabla["T'"]["*="] = {"*=", "F", "T'"};
	tabla["T'"]["/="] = {"/=", "F", "T'"};
	tabla["T'"]["+"] = {};  // ε
	tabla["T'"]["-"] = {};  // ε
	tabla["T'"]["+="] = {};  // ε
	tabla["T'"]["-="] = {};  // ε
	tabla["T'"]["=="] = {};  // ε
	tabla["T'"]["<"] = {};   // ε
	tabla["T'"][">"] = {};  // ε
	tabla["T'"]["<="] = {};  // ε
	tabla["T'"][">="] = {};  // ε
	tabla["T'"][";"] = {};   // ε
	tabla["T'"]["&&"] = {};  // ε
	tabla["T'"]["||"] = {};  // ε
	tabla["T'"]["^^"] = {};  // ε
	tabla["T'"][")"] = {};   // ε

	// F → id | num | ( E ) | ++ id | -- id | ! F
	tabla["F"]["id"] = {"id"};
	tabla["F"]["num"] = {"num"};
	tabla["F"]["caracter"] = {"caracter"};
	tabla["F"]["cadena"] = {"cadena"};
	tabla["F"]["("] = {"(", "E", ")"};
	tabla["F"]["++"] = {"++", "id"};
	tabla["F"]["--"] = {"--", "id"};
	tabla["F"]["!"] = {"!", "F"};
}

// -------------------------
// Entrada de tokens
// -------------------------
vector<Token> Parser::cargarTokenInfos(const string& nombreArchivo) {
    cout << "CARGANDO TOKENS DESDE "<<nombreArchivo<<".\n\n";

    vector<Token> tokens1;
    ifstream archivo(nombreArchivo);
    string linea;

    while (getline(archivo, linea)) {
        size_t posID = linea.find("ID:");
        size_t posTOKEN = linea.find("TOKEN:");
        size_t posLexema = linea.find("------ ", posTOKEN);

        if (posID != string::npos && posTOKEN != string::npos && posLexema != string::npos) {
            string idStr = linea.substr(posID + 3, posTOKEN - (posID + 3));
            string tipoStr = linea.substr(posTOKEN + 6, posLexema - (posTOKEN + 6));
            string lexemaStr = linea.substr(posLexema + 7);

            idStr = eliminarEspacios(idStr);
            tipoStr = eliminarEspacios(tipoStr);

            Token t;
            t.id = stoi(idStr);
            t.tipo = TipoLexema::PUNTO_COMA;
            if (tipoStr == "CARACTER") t.tipo = TipoLexema::CARACTER;
            if (tipoStr == "CADENA") t.tipo = TipoLexema::CADENA;
            if (tipoStr == "IDENTIFICADOR") t.tipo = TipoLexema::IDENTIFICADOR;
            if (tipoStr == "ENTERO") t.tipo = TipoLexema::ENTERO;
            if (tipoStr == "FLOTANTE") t.tipo = TipoLexema::FLOTANTE;
            if (tipoStr == "CONSTANTE") t.tipo = TipoLexema::CONSTANTE;
            if (tipoStr == "PALABRA_RESERVADA") t.tipo = TipoLexema::PALABRA_RESERVADA;
            if (tipoStr == "OPERADOR") t.tipo = TipoLexema::OPERADOR;
        	if (tipoStr == "BOOLEANO" || tipoStr == "VERDADERO" || tipoStr == "FALSO" ||
        		lexemaStr == "VERDADERO" || lexemaStr == "VERDADERO" ||
        		idStr == "VERDADERO" || idStr == "VERDADERO") t.tipo = TipoLexema::BOOLEANO;
            if (tipoStr == "FUNCION") t.tipo = TipoLexema::FUNCION;

            t.valor_lexema = lexemaStr;
        	if (lexemaStr != "declaracion" && lexemaStr != "fin_declaracion" && lexemaStr != "principal" && lexemaStr != "fin_principal") {
        		tokens1.push_back(t);
        	}

        }
    }

    return tokens1;
}

// -------------------------
// Parser con árbol
// -------------------------
shared_ptr<Nodo> Parser::parser_tree(const vector<string>& tokens, vector<Token> instruccion) {
    stack<shared_ptr<Nodo>> pila;
    pila.push(make_shared<Nodo>("S", ParseType::Program, TipoLexema::SCOPE, 0,"SENTENCE"));
    int index = 0;

    shared_ptr<Nodo> arbol = pila.top();

    while (!pila.empty()) {
        auto nodo = pila.top(); pila.pop();
        string simbolo = nodo->simbolo_sintactico;//pila.top

        if (index >= tokens.size()) {
            cout << "Error: tokens insuficientes.\n";
            return nullptr;
        }

        string token = tokens[index];

		if (token == simbolo && simbolo == ";") {
			return arbol;
		}else if (tabla.count(simbolo)) {
            if (tabla[simbolo].count(token)) {
                const auto& produccion = tabla[simbolo][token];

            	if (!produccion.empty()) {
            		for (auto it = produccion.rbegin(); it != produccion.rend(); ++it) {

            			auto hijo = make_shared<Nodo>(*it,
							ParseType::Gram,
							TipoLexema::GRAMMAR,
							index,
							*it);
            			hijo->valor = hijo->simbolo_sintactico;

            			if (isupper(hijo->simbolo_sintactico[0])) {
            				hijo->type = ParseType::Gram;
            				hijo->token = TipoLexema::GRAMMAR;
            			} else if (hijo->simbolo_sintactico == "id") {
            				hijo->type = ParseType::Id;
            				hijo->token = TipoLexema::IDENTIFICADOR;
            			} else if (hijo->simbolo_sintactico == "num") {
            				hijo->type = ParseType::Literal;
            				//hijo->token = Token::ENTERO;
            				hijo->token = instruccion[index].tipo;
            			} else if (hijo->simbolo_sintactico == "tipo") {
            				hijo->type = ParseType::Tipo;
            				hijo->token = TipoLexema::PALABRA_RESERVADA;
            			} else if (hijo->simbolo_sintactico == ";") {
            				hijo->type = ParseType::Gram;
            				hijo->token = TipoLexema::PUNTO_COMA;
            			} else if (hijo->simbolo_sintactico == "si") {
            				hijo->type = ParseType::Conditional;
            				hijo->token = TipoLexema::PALABRA_RESERVADA;
            			} else if (hijo->simbolo_sintactico == "si_no") {
            				hijo->type = ParseType::Conditional;
            				hijo->token = TipoLexema::PALABRA_RESERVADA;
            			} else if (hijo->simbolo_sintactico == "funcion") {
            				hijo->type = ParseType::Function;
            				hijo->token = TipoLexema::PALABRA_RESERVADA;
            			}

            			nodo->hijos.insert(nodo->hijos.begin(), hijo);
            			pila.push(hijo);
            		}
            	}
            } else if (token != simbolo) {
                cout << "Error de sintaxis en token '" << token << "' al esperar '" << simbolo << "'.\n";
                return nullptr;
            } else {
	            cout << "ERROR RARO DEL ANALIZADOR SINTACTICO\n";
            	//return nullptr;
            	return arbol;
            }
        } else if (simbolo == token) {
        	if (token == "num" &&
				(instruccion[index].tipo == TipoLexema::ENTERO ||
				 instruccion[index].tipo == TipoLexema::FLOTANTE)) {

        		nodo->valor = instruccion[index].valor_lexema;  // guardamos número real
        		nodo->token = instruccion[index].tipo;
				 }
        	else if (token == "id" && instruccion[index].tipo == TipoLexema::IDENTIFICADOR) {

        		nodo->valor = instruccion[index].valor_lexema;  // guardamos nombre real del id
        		nodo->token = instruccion[index].tipo;
        	}
        	else if (token == "id" && instruccion[index].tipo == TipoLexema::BOOLEANO) {
        		nodo->valor = instruccion[index].valor_lexema;  // guardamos nombre real del id
        		nodo->simbolo_sintactico = instruccion[index].valor_lexema;
        		nodo->token = TipoLexema::BOOLEANO;
        	}
        	else if (token == "tipo" && instruccion[index].tipo == TipoLexema::PALABRA_RESERVADA) {

        		nodo->valor = instruccion[index].valor_lexema;  // guardamos tipo real
        		nodo->token = instruccion[index].tipo;
        	}

        	index++;
        } else {
            cout << "Error: se esperaba '" << simbolo << "' pero se encontro '" << token << "'.\n";
            return nullptr;
        }
    }

    if (index != tokens.size()) {
        cout << "Error: tokens restantes sin consumir.\n";
        return nullptr;
    }

    return arbol;
}

void Parser::imprimir_arbol(const shared_ptr<Nodo>& nodo, int nivel) {
    //for (int i = 0; i < nivel; ++i) cout << "    ";
    cout << nodo->valor << "\n";
	//cout << nodo->simbolo << "\n";
	for (int i = 0; i < nodo->hijos.size(); i++) {
		auto hijo = nodo->hijos[i];
		for (int j = 0; j < nivel; ++j) {
			cout << "|    ";
			//cout << "    ";
		}
		//cout << "|    ";
		if (i == nodo->hijos.size() - 1) cout << "o--- ";
		else if (i <= nodo->hijos.size() - 1) cout << "|--- ";
		else cout << "     ";
		imprimir_arbol(hijo, nivel + 1);
	}
}

// -------------------------
// Parser LL(1) paso a paso
// -------------------------
void Parser::parser(vector<Token> instruccion) {
    stack<string> pila;
    pila.push("S");

    int count = 0;
    idx = 0;
    string instruccionS = imprimir_instruccion(instruccion);

    cout<<"  PASO"
        <<setw(40)<<"PILA"
        <<setw(50)<<"DERIVACION\n";

    string a = siguiente_token();//token

    while (!pila.empty()) {
        count++;
    	cout<<"\n";
        cout<<setw(6)<<count;
        cout<<setw(40)<<instruccionS;
        cout<<setw(20)<<printStack(pila);
        cout<<setw(10)<<" ";

        string X = pila.top();
        pila.pop();

        if (X == a && X == ";") {
            cout << "Derivacion: ; --> ; (fin de analisis)\n\n";
            //break;
        	return;
        } else if (X == a) {
            cout << "TokenInfo coincidente: " << a;
            a = siguiente_token();
        } else if (tabla.count(X) == 0) {
            cout << "Error: simbolo inesperado '" << X;
            return;
        } else if (tabla[X].count(a) == 0) {
            cout << "\n\nError de sintaxis en token '" << a << "' al esperar '" << X << "'\n";
            return;
        } else {
            Produccion prod = tabla[X][a];
            if (!prod.empty()) {
                cout << X << " --> ";
                for (string s : prod) cout << s << "   ";
                //cout << endl;

                for (auto it = prod.rbegin(); it != prod.rend(); ++it) {
                    pila.push(*it);
                }
            }
        }
    }
}

vector<vector<Token>> Parser::cargar_instrucciones() {
	last_token = 0;
	//vector<vector<TokenInfo>> instrucciones;
	while (last_token < tokens.size()) {
		instrucciones.push_back(cargar_siguiente_instruccion());
	}
	return instrucciones;
}

shared_ptr<Nodo> Parser::execute_sintax() {

	SemanticAnalyzer semantico;

	vector<string> string_instr;
	shared_ptr<Nodo> tree;
	// Build program root
	auto program = make_shared<Nodo>("PROGRAMA", ParseType::Gram,TipoLexema::SCOPE,0,"PROGRAMA");
	stack<shared_ptr<Nodo>> blocks;
	blocks.push(program);
	shared_ptr<Nodo> last_tree = nullptr;

	cout<<"\n\n\nINICIANDO PROCESO  DE ANALISIS SINTACTICO\n";


	/*last_token = 0;
	//vector<vector<TokenInfo>> instrucciones;
	while (last_token < tokens.size()) {
		instrucciones.push_back(cargar_siguiente_instruccion());
	}*/

	cargar_instrucciones();
	//IntermediateCoding cod_int;

	/*while(last_token < tokens.size()){
		instruccion = cargar_siguiente_instruccion();
		string_instr = instruccion_strings(instruccion);
		cout<<"\n\nANALISIS DE LA EXPRESION: "<<imprimir_instruccion(instruccion)<<"\n";
		parser(instruccion);
		tree = parser_tree(string_instr);
		if (tree) {
			cout<<"\nARBOL DE DERIVACIONES DE LA INSTRUCCION\n";
			imprimir_arbol(tree);
		} else {
			cout << "Analisis sintactico fallido para esta instruccion.\n";
		}
	}*/

	for (int i = 0; i < static_cast<int>(instrucciones.size()); i++){
		instruccion = instrucciones[i];
		string_instr = instruccion_strings(instruccion);
		cout<<"\n\nANALISIS DE LA EXPRESION: "<<imprimir_instruccion(instruccion)<<"\n";
		parser(instruccion);

		cout<<"\nAGENERACION DE ARBOL DE LA INSTRUCCION\n";
		tree = parser_tree(string_instr, instruccion);

		if (tree) {
			cout<<"\nARBOL DE DERIVACIONES DE LA INSTRUCCION\n";
			imprimir_arbol(tree);
			cout<<"\n";
			cout<<"\nARBOL SIMPLIFICADO DE LA INSTRUCCION\n";
			imprimir_arbol(semantico.simplificar(tree));
			cout<<"\n";
			recorrerPrefijo(tree);
			semantico.exec_semantic(tree);

			if (tree->hijos[0]->simbolo_sintactico == "si") {
				blocks.top()->hijos.push_back(tree);
				blocks.push(tree);
				last_tree = tree;
			} else if (tree->hijos[0]->simbolo_sintactico == "{") {
				auto block = make_shared<Nodo>("BLOCK", ParseType::Scope,TipoLexema::LLAVE_ABIERTA,1,"BLOCK");
				blocks.top()->hijos.push_back(block);
				blocks.push(block);
			} else if (tree->hijos[0]->simbolo_sintactico == "si_no") {
				if (last_tree->hijos[0]->simbolo_sintactico == "si") {
					last_tree->hijos.push_back(tree);
				} else {
					semantico.pushError("si_no Solo puede usarse desPues de una estructura si(condicion){}", tree);
				}
			} else if (tree->hijos[0]->simbolo_sintactico == "}") {
				blocks.pop();
			} else {
				blocks.top()->hijos.push_back(tree);
				last_tree = tree;
			}

			/*cod_int.generar(tree);
			cod_int.imprimir();*/
		} else {
			cout << "Analisis sintactico fallido para esta instruccion.\n";
		}
	}

	cout<<"\n\n\nARBOL SINTACTICO TOTAL DEL PROGRAMA\n";
	imprimir_arbol(program);

	cout<<"\n\n\nANALISIS SEMANTICO TOTAL DEL PROGRAMA\n";
	//semantico.getErrors();
	SemanticAnalyzer sem2;
	sem2.analizarPrograma(program);

	semantico.imprimirTablaGlobal();


	cout<<"\n\n\nGENERACION DE CODIGO INTERMEDIO DEL PROGRAMA\n";

	// cod_int.generar(program);
	// cod_int.imprimir();

	return program;
}

vector<vector<Token> > Parser::getInstrucciones() {
	if (instrucciones.size() == 0) {
		cargar_instrucciones();
	}

	return instrucciones;
}
