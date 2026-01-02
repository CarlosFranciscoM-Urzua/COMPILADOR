//
// Created by CARLOS URZUA on 11/20/2025.
//

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <sstream>
#include <iostream>
#include <cctype>
#include <iomanip>
#include "generador_asm.h" // Define Cuadruplo { string op, arg1, arg2, resultado; }

string GeneradorASM::generarASM(const vector<Cuadruplo>& cuadruplos) {
    ostringstream out;
    out << "; ==============================================\n";
    out << ";  CODIGO ENSAMBLADOR NASM 32-BIT GENERADO POR EL COMPILADOR\n";
    out << ";  Versión corregida: declara todas las variables y soporta concatenación (+ con strings)\n";
    out << "; ==============================================\n\n";

    out << "global main\n";
    out << "extern printf\n";
    out << "extern sprintf\n\n";

    // Tipos simplificados
    enum Tipo { T_INT, T_FLOAT, T_STRING, T_UNKNOWN };

    // Recolectar símbolos: temporales, variables, literales
    unordered_set<string> temporales;
    unordered_set<string> variables; // nombres que no son temporales pero aparecen
    unordered_map<string,string> stringLiterals; // literal->label
    unordered_map<string,string> floatLiterals;  // float literal -> label
    int nextStrId = 1, nextFloatId = 1;

    auto esTemporal = [](const string &s){ return !s.empty() && s[0]=='t'; };
    auto esCadena = [](const string &s){ return s.size()>=2 && s.front()=='"' && s.back()=='"'; };
    auto esCharLiteral = [](const string &s){ return s.size()>=3 && s.front()=='\'' && s.back()=='\''; };
    auto esNumeroEntero = [](const string &s)->bool {
        if (s.empty()) return false;
        size_t i=0; if ((s[0]=='+'||s[0]=='-') && s.size()>1) i=1;
        for (; i<s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
        return true;
    };
    auto esFloatLiteral = [](const string &s)->bool {
        if (s.empty()) return false;
        bool dot=false; size_t i=0; if ((s[0]=='+'||s[0]=='-') && s.size()>1) i=1;
        bool hasDigit=false;
        for (; i<s.size(); ++i) {
            if (s[i]=='.') { if (dot) return false; dot=true; continue;}
            if (!isdigit((unsigned char)s[i])) return false;
            hasDigit=true;
        }
        return dot && hasDigit;
    };
    auto esBooleano = [](const string &s)->bool {
        if (s.empty()) return false;
        string u=s; for(auto &c:u) c=toupper((unsigned char)c);
        return u=="FALSO" || u=="VERDADERO";
    };
    auto upper = [](string s){ for(auto &c:s) c=toupper((unsigned char)c); return s; };

    // Primera pasada: detectar variables (aparecen en arg1/arg2/resultado) y literales
    for (const auto &q : cuadruplos) {
        // resultado
        if (!q.resultado.empty()) {
            if (esTemporal(q.resultado)) temporales.insert(q.resultado);
            else variables.insert(q.resultado);
        }
        // arg1
        if (!q.arg1.empty()) {
            if (esCadena(q.arg1)) {
                if (!stringLiterals.count(q.arg1)) {
                    stringLiterals[q.arg1] = "str_" + to_string(nextStrId++);
                }
            } else if (esFloatLiteral(q.arg1)) {
                if (!floatLiterals.count(q.arg1)) {
                    floatLiterals[q.arg1] = "fconst_" + to_string(nextFloatId++);
                }
            } else if (!esNumeroEntero(q.arg1) && !esCharLiteral(q.arg1) && !esBooleano(q.arg1)) {
                // nombre de variable/identificador
                if (!esTemporal(q.arg1)) variables.insert(q.arg1);
            }
        }
        // arg2
        if (!q.arg2.empty()) {
            if (esCadena(q.arg2)) {
                if (!stringLiterals.count(q.arg2)) {
                    stringLiterals[q.arg2] = "str_" + to_string(nextStrId++);
                }
            } else if (esFloatLiteral(q.arg2)) {
                if (!floatLiterals.count(q.arg2)) {
                    floatLiterals[q.arg2] = "fconst_" + to_string(nextFloatId++);
                }
            } else if (!esNumeroEntero(q.arg2) && !esCharLiteral(q.arg2) && !esBooleano(q.arg2)) {
                if (!esTemporal(q.arg2)) variables.insert(q.arg2);
            }
        }
    }

    // Map de tipos (simple): por defecto int, si aparece float literal -> float para esa variable/temporal
    unordered_map<string,Tipo> tipo;
    for (auto &t : temporales) tipo[t]=T_INT;
    for (auto &v : variables) tipo[v]=T_INT;
    for (auto &p : floatLiterals) {
        // si un literal ya corresponde a nombre no aplica; sólo marcar variables que tengan literal en asign
        // pero también si una variable tiene mismo texto (raro) no lo tocamos.
    }

    // SECTION .data
    out << "section .data\n";
    // declarar temporales y variables (tipo dd o dq según heurística simple: si nombre contiene 't' luego float detection will change)
    for (auto &p : tipo) {
        if (p.second == T_FLOAT) out << p.first << " dq 0.0\n";
        else out << p.first << " dd 0\n";
    }

    // literales string
    for (auto &kv : stringLiterals) {
        string lit = kv.first.substr(1, kv.first.size()-2); // quitar comillas externas
        out << kv.second << " db " << "\"" << lit << "\"" << ",0\n";
    }
    // literales float
    for (auto &kv : floatLiterals) {
        out << kv.second << " dq " << kv.first << "\n";
    }

    // buffers y formatos se crearán según se necesiten (los declaramos aquí si surgen)
    // formateadores básicos
    out << "fmt_d db \"%d\",10,0\n";
    out << "fmt_f db \"%f\",10,0\n";
    out << "fmt_s db \"%s\",10,0\n";
    out << "fmt_c db \"%c\",10,0\n\n";

    // Prealocar mapas para buffers/formatos creados dinámicamente (se llenan en segunda pasada)
    unordered_map<string,string> bufferForResult;   // resultado -> buffer label
    unordered_map<string,string> fmtForConcat;      // resultado -> fmt label

    // Segunda pasada: generar .data adicional para buffers/formats necesarios por concatenación
    // detectamos + con cadena y pre-creamos buffer y fmt
    int nextBufId = 1;
    string section_bss = "";
    for (const auto &q : cuadruplos) {
        if (q.op == "+") {
            // si alguno es string literal -> concatenación/formateo
            bool aIsStr = esCadena(q.arg1);
            bool bIsStr = esCadena(q.arg2);
            if (aIsStr || bIsStr) {
                string res = q.resultado;
                if (res.empty()) continue; // no dónde guardar
                string bufLabel = "buf_" + to_string(nextBufId++);
                section_bss = section_bss + bufLabel  + " resb 300\n";
                string fmtLabel = "fmt_concat_" + to_string(nextBufId++);
                bufferForResult[res] = bufLabel;
                // decidir formato
                string fmt;
                // posibles combinaciones: str+int, str+float, int+str, float+str, str+str
                // analizamos arg1,arg2 tipos superficiales:
                auto tipoOp = [&](const string &op)->string{
                    if (esCadena(op)) return "STR";
                    if (esFloatLiteral(op)) return "FLOAT";
                    if (esNumeroEntero(op)) return "INT";
                    if (esCharLiteral(op)) return "CHAR";
                    if (esBooleano(op)) return "BOOL";
                    // si es variable y en tipo map puede ser float, intentar adivinar
                    if (tipo.find(op)!=tipo.end()) {
                        if (tipo[op]==T_FLOAT) return "FLOAT";
                        if (tipo[op]==T_INT) return "INT";
                    }
                    return "INT";
                };
                string ta = tipoOp(q.arg1);
                string tb = tipoOp(q.arg2);
                if ((ta=="STR" && tb=="STR")) fmt = "%s%s";
                else if ((ta=="STR" && tb=="FLOAT") || (ta=="FLOAT" && tb=="STR")) fmt = "%s%f";
                else if ((ta=="STR" && tb=="INT") || (ta=="INT" && tb=="STR")) fmt = "%s%d";
                else fmt = "%s%d";
                // declarar buffer y fmt en .data
                //out << bufLabel << " resb 300\n"; // buffer temporal para sprintf
                out << fmtLabel << " db \"" << fmt << "\",0\n";
                fmtForConcat[res] = fmtLabel;
            }
        }
    }

    //SECTION .bss
    out << "\nsection .bss\n";
    //out << bufLabel << " resb 300\n"; // buffer temporal para sprintf
    out << section_bss;
    // SECTION .text
    out << "\nsection .text\n";
    out << "main:\n\n";

    // Generación de código (tercera pasada)
    for (const auto &q : cuadruplos) {
        out << "    ; --- (" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.resultado << ") ---\n";

        // helpers locales (simples)
        auto cargarEnteroEnEAX = [&](const string &op) {
            if (op.empty()) { out << "    ; operando vacío\n"; return; }
            if (esNumeroEntero(op)) out << "    mov eax, " << op << "\n";
            else if (esCharLiteral(op)) { string ch = op.substr(1, op.size()-2); out << "    mov eax, '"<<ch<<"'\n"; }
            else if (esBooleano(op)) { string u=upper(op); out << "    mov eax, " << (u=="VERDADERO"?"1":"0") << "\n"; }
            else out << "    mov eax, [" << op << "]\n";
        };

        // Caso: concatenación/format si + y alguna operando es cadena literal
        if (q.op == "+") {
            bool aStr = esCadena(q.arg1);
            bool bStr = esCadena(q.arg2);
            if (aStr || bStr) {
                // usar sprintf para construir la cadena en bufferForResult[q.resultado]
                string res = q.resultado;
                string buf = bufferForResult.count(res) ? bufferForResult[res] : "";
                string fmt = fmtForConcat.count(res) ? fmtForConcat[res] : "";
                if (buf.empty() || fmt.empty()) {
                    out << "    ; error: buffer o fmt no generado para concatenación\n\n";
                    continue;
                }

                // push args right-to-left for sprintf(buf, fmt, arg1, arg2)
                // calcular qué tipo tiene cada arg y empujar adecuadamente
                // orden: push argN ... push arg1 push fmt push buf
                // NOTA: Para double (float) hay que empujar 8 bytes (high then low)
                auto pushArg = [&](const string &op) {
                    if (op.empty()) {
                        out << "    push 0\n";
                    } else if (esCadena(op)) {
                        string lbl = stringLiterals[op];
                        out << "    push " << lbl << "\n";
                    } else if (esFloatLiteral(op) || (tipo.find(op)!=tipo.end() && tipo[op]==T_FLOAT)) {
                        // push high dword then low dword
                        if (esFloatLiteral(op)) {
                            string lbl = floatLiterals[op];
                            out << "    push dword [" << lbl << " + 4]\n";
                            out << "    push dword [" << lbl << "]\n";
                        } else {
                            // variable double
                            out << "    push dword [" << op << " + 4]\n";
                            out << "    push dword [" << op << "]\n";
                        }
                    } else if (esNumeroEntero(op) || esCharLiteral(op) || esBooleano(op) || (tipo.find(op)!=tipo.end() && tipo[op]==T_INT)) {
                        if (esNumeroEntero(op)) out << "    push " << op << "\n";
                        else if (esCharLiteral(op)) { string ch=op.substr(1,op.size()-2); out << "    push '"<<ch<<"'\n"; }
                        else if (esBooleano(op)) { string u=upper(op); out << "    push " << (u=="VERDADERO"?"1":"0") << "\n"; }
                        else out << "    push dword [" << op << "]\n";
                    } else {
                        // fallback: treat as dword value
                        out << "    push dword [" << op << "]\n";
                    }
                };

                // push second arg then first arg (right-to-left)
                // determine order: sprintf(buf, fmt, arg1, arg2)
                // so we push arg2 then arg1, then fmt, then buf
                pushArg(q.arg2);
                pushArg(q.arg1);
                out << "    push " << fmt << "\n";
                out << "    push " << buf << "\n";
                out << "    call sprintf\n";
                // limpiar pila: args size depends: for each integer arg -> 4 bytes; float arg -> 8 bytes
                // compute popBytes:
                int popBytes = 8; // fmt + buf (each 4)
                // arg1
                auto argBytes = [&](const string &op)->int {
                    if (op.empty()) return 4;
                    if (esFloatLiteral(op) || (tipo.find(op)!=tipo.end() && tipo[op]==T_FLOAT)) return 8;
                    return 4;
                };
                popBytes += argBytes(q.arg1) + argBytes(q.arg2);
                out << "    add esp, " << popBytes << "\n";
                // finalmente almacenar la dirección del buffer en el resultado (dd pointer)
                out << "    mov dword [" << res << "], " << buf << "\n\n";
                continue; // siguiente cuadruplo
            }
        }

        // ARITMETICA con floats / ints (similar a versión anterior)
        if (q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/") {
            // decidir si float: si alguno literal float o tipo marcado float
            bool usarFloat = esFloatLiteral(q.arg1) || esFloatLiteral(q.arg2) ||
                             (tipo.find(q.arg1)!=tipo.end() && tipo[q.arg1]==T_FLOAT) ||
                             (tipo.find(q.arg2)!=tipo.end() && tipo[q.arg2]==T_FLOAT) ||
                             (tipo.find(q.resultado)!=tipo.end() && tipo[q.resultado]==T_FLOAT);
            if (usarFloat) {
                // Ruta float con FPU
                string a = q.arg1.empty() ? q.arg2 : q.arg1;
                string b = q.arg1.empty() ? string() : q.arg2; // si arg1 estaba vacio, no hay b
                if (esFloatLiteral(a)) out << "    fld qword [" << floatLiterals[a] << "]\n";
                else out << "    fld qword [" << a << "]\n";
                if (!b.empty()) {
                    if (esFloatLiteral(b)) out << "    ";
                    // elegir operación
                    if (q.op == "+") {
                        if (esFloatLiteral(b)) out << "fadd qword [" << floatLiterals[b] << "]\n";
                        else out << "    fadd qword [" << b << "]\n";
                    } else if (q.op == "-") {
                        if (esFloatLiteral(b)) out << "fsub qword [" << floatLiterals[b] << "]\n";
                        else out << "    fsub qword [" << b << "]\n";
                    } else if (q.op == "*") {
                        if (esFloatLiteral(b)) out << "fmul qword [" << floatLiterals[b] << "]\n";
                        else out << "    fmul qword [" << b << "]\n";
                    } else if (q.op == "/") {
                        if (esFloatLiteral(b)) out << "fdiv qword [" << floatLiterals[b] << "]\n";
                        else out << "    fdiv qword [" << b << "]\n";
                    }
                }
                if (!q.resultado.empty()) out << "    fstp qword [" << q.resultado << "]\n";
                else out << "    fstp st0\n";
            } else {
                // integer path
                string a = q.arg1.empty() ? q.arg2 : q.arg1;
                string b = q.arg1.empty() ? string() : q.arg2;
                if (!a.empty()) cargarEnteroEnEAX(a);
                if (!b.empty()) {
                    if (esNumeroEntero(b)) {
                        if (q.op=="+") out << "    add eax, " << b << "\n";
                        else if (q.op=="-") out << "    sub eax, " << b << "\n";
                        else if (q.op=="*") out << "    imul eax, " << b << "\n";
                        else if (q.op=="/") { out << "    cdq\n    mov ebx, " << b << "\n    idiv ebx\n"; }
                    } else if (esCharLiteral(b)) {
                        string ch = b.substr(1,b.size()-2);
                        if (q.op=="+") out << "    add eax, '"<<ch<<"'\n";
                        else if (q.op=="-") out << "    sub eax, '"<<ch<<"'\n";
                        else if (q.op=="*") out << "    imul eax, '"<<ch<<"'\n";
                        else if (q.op=="/") { out << "    cdq\n    mov ebx, '"<<ch<<"'\n    idiv ebx\n"; }
                    } else {
                        if (q.op=="+") out << "    add eax, [" << b << "]\n";
                        else if (q.op=="-") out << "    sub eax, [" << b << "]\n";
                        else if (q.op=="*") { out << "    mov ebx, ["<<b<<"]\n    imul eax, ebx\n"; }
                        else if (q.op=="/") { out << "    cdq\n    mov ebx, ["<<b<<"]\n    idiv ebx\n"; }
                    }
                }
                if (!q.resultado.empty()) out << "    mov [" << q.resultado << "], eax\n";
            }
            out << "\n";
            continue;
        }

        else if (q.op == "=") {
            string val = q.arg1.empty() ? q.arg2 : q.arg1;
            if (val.empty()) { out << "    ; asignación vacía, omitir\n\n"; continue; }
            // si destino es float o fuente float -> usar FPU path
            bool destFloat = (tipo.find(q.resultado)!=tipo.end() && tipo[q.resultado]==T_FLOAT);
            bool valFloat = esFloatLiteral(val) || (tipo.find(val)!=tipo.end() && tipo[val]==T_FLOAT);
            if (destFloat || valFloat) {
                if (esFloatLiteral(val)) out << "    fld qword [" << floatLiterals[val] << "]\n";
                else if (esNumeroEntero(val)) {
                    out << "    mov eax, " << val << "\n";
                    out << "    push eax\n";
                    out << "    fild dword [esp]\n";
                    out << "    add esp, 4\n";
                } else if (esCharLiteral(val)) {
                    string ch=val.substr(1,val.size()-2);
                    out << "    mov eax, '"<<ch<<"'\n";
                    out << "    push eax\n";
                    out << "    fild dword [esp]\n";
                    out << "    add esp, 4\n";
                } else out << "    fld qword [" << val << "]\n";
                if (!q.resultado.empty()) out << "    fstp qword [" << q.resultado << "]\n";
                else out << "    fstp st0\n";
            } else if (esCadena(val)) {
                // asignar direccion de la cadena
                out << "    mov dword [" << q.resultado << "], " << stringLiterals[val] << "\n";
            } else {
                // entero normal
                cargarEnteroEnEAX(val);
                if (!q.resultado.empty()) out << "    mov [" << q.resultado << "], eax\n";
            }
            out << "\n";
            continue;
        }

        else if (q.op == "PRINT") {
            string what = q.arg1;
            if (what.empty()) {
                out << "    push 0\n    push fmt_s\n    call printf\n    add esp, 8\n\n";
                continue;
            }
            if (esCadena(what)) {
                out << "    push " << stringLiterals[what] << "\n";
                out << "    push fmt_s\n";
                out << "    call printf\n";
                out << "    add esp, 8\n\n";
                continue;
            }
            // float?
            if (esFloatLiteral(what) || (tipo.find(what)!=tipo.end() && tipo[what]==T_FLOAT)) {
                if (esFloatLiteral(what)) {
                    out << "    push dword [" << floatLiterals[what] << " + 4]\n";
                    out << "    push dword [" << floatLiterals[what] << "]\n";
                } else {
                    out << "    push dword [" << what << " + 4]\n";
                    out << "    push dword [" << what << "]\n";
                }
                out << "    push fmt_f\n    call printf\n    add esp, 12\n\n";
                continue;
            }
            // entero / variable
            if (esNumeroEntero(what)) out << "    push " << what << "\n";
            else if (esCharLiteral(what)) { string ch=what.substr(1,what.size()-2); out << "    push '"<<ch<<"'\n"; }
            else if (esBooleano(what)) { string u=upper(what); out << "    push " << (u=="VERDADERO"?"1":"0") << "\n"; }
            else out << "    push dword [" << what << "]\n";
            out << "    push fmt_d\n    call printf\n    add esp, 8\n\n";
            continue;
        }

        else if (q.op == "LABEL") {
            out << q.resultado << ":\n\n";
            continue;
        }
        else if (q.op == "GOTO") {
            out << "    jmp " << q.resultado << "\n\n";
            continue;
        }
        else if (q.op=="==" || q.op=="!=" || q.op=="<" || q.op==">" || q.op=="<=" || q.op==">=") {
            // comparación: simple ruta entera o float con FPU
            string left = q.arg1.empty() ? q.arg2 : q.arg1;
            string right = (!q.arg1.empty() && !q.arg2.empty()) ? q.arg2 : "";
            bool compFloat = esFloatLiteral(left) || esFloatLiteral(right) ||
                             (tipo.find(left)!=tipo.end() && tipo[left]==T_FLOAT) ||
                             (tipo.find(right)!=tipo.end() && tipo[right]==T_FLOAT);
            if (compFloat) {
                if (esFloatLiteral(left)) out << "    fld qword [" << floatLiterals[left] << "]\n";
                else out << "    fld qword [" << left << "]\n";
                if (esFloatLiteral(right)) out << "    fld qword [" << floatLiterals[right] << "]\n";
                else out << "    fld qword [" << right << "]\n";
                out << "    fcomip st1, st0\n";
                out << "    fstp st0\n";
                if (q.op=="==") out << "    je " << q.resultado << "\n";
                else if (q.op=="!=") out << "    jne " << q.resultado << "\n";
                else if (q.op=="<") out << "    jb " << q.resultado << "\n";
                else if (q.op==">") out << "    ja " << q.resultado << "\n";
                else if (q.op=="<=") out << "    jbe " << q.resultado << "\n";
                else if (q.op==">=") out << "    jae " << q.resultado << "\n";
            } else {
                // entero
                cargarEnteroEnEAX(left);
                if (right.empty()) out << "    cmp eax, 0\n";
                else if (esNumeroEntero(right)) out << "    cmp eax, " << right << "\n";
                else if (esCharLiteral(right)) { string ch=right.substr(1,right.size()-2); out << "    cmp eax, '"<<ch<<"'\n"; }
                else if (esBooleano(right)) { string u=upper(right); out << "    cmp eax, " << (u=="VERDADERO"?"1":"0") << "\n"; }
                else out << "    cmp eax, [" << right << "]\n";
                if (q.op=="==") out << "    je " << q.resultado << "\n";
                else if (q.op=="!=") out << "    jne " << q.resultado << "\n";
                else if (q.op=="<") out << "    jl " << q.resultado << "\n";
                else if (q.op==">") out << "    jg " << q.resultado << "\n";
                else if (q.op=="<=") out << "    jle " << q.resultado << "\n";
                else if (q.op==">=") out << "    jge " << q.resultado << "\n";
            }
            out << "\n";
            continue;
        }

        else {
            out << "    ; operación no soportada: " << q.op << "\n\n";
            continue;
        }
    } // end for cuadruplos

    // finalizar main: return 0
    out << "    mov eax, 0\n";
    out << "    ret\n";

    return out.str();
}

void GeneradorASM::generar_archivo(string content, string nombre_archivo, string extension, string route) {
    // Create and open a text file
    ofstream file(route + nombre_archivo + extension); // archivo de salida

    // Write to the file
    file << content;

    //closing the file
    file.close();

    cout << "ARCHIVO " << nombre_archivo << extension <<  " CREADO CON EXITO.\n\n"<< endl;
}
