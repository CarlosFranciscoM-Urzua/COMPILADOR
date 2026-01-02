//
// Created by CARLOS URZUA on 11/19/2025.
//

#ifndef COMPILADOR_ASSEMBLER_DRIVER_H
#define COMPILADOR_ASSEMBLER_DRIVER_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

using namespace std;

class AssemblerDriver {
public:

    // Escribe un archivo ASM en disco
    static bool guardarASM(const string& nombreASM, const string& codigo) {
        ofstream out(nombreASM);
        if (!out) return false;

        out << codigo;
        out.close();
        return true;
    }

    // Ejecuta un comando del sistema y muestra salida
    static int ejecutar(const string& cmd) {
        cout << "[CMD] " << cmd << "\n";
        return system(cmd.c_str());
    }

    // Ensambla usando NASM → codigo.obj
    static bool ensamblar(const string& nombreASM, const string& obj) {
        string cmd =
            ".\\.\\NASM\\nasm.exe -f win32 \"" + nombreASM + "\" -o \"" + obj + "\"";
        return ejecutar(cmd) == 0;
    }

    // LINKER
    static bool enlazar(const string& obj, const string& exe) {
        //OPTION FOR GNUL LD
        /*string cmd =
            "ld -m i386pe \"" + obj + "\" -o \"" + exe + "\"";*/
        string cmd =
            ".\\GoLink\\GoLink.exe "
            "/console "
            "/entry main "
            "programa.obj "
            "kernel32.dll "
            "msvcrt.dll "
            "user32.dll "
            "gdi32.dll "
            "COMCTL32.dll "
            "comdlg32.dll "
            "OLEAUT32.dll "
            "Hhctrl.ocx "
            "winspool.drv "
            "shell32.dll ";
        return ejecutar(cmd) == 0;
    }

    // Flujo completo: .ASM → .OBJ → .EXE
    static bool generarEXE(const string& asmFile,
                           const string& exeFile,
                           const string& codigoASM)
    {
        string objFile = asmFile.substr(0, asmFile.size() - 4) + ".obj";

        cout << "\n== GENERANDO ARCHIVO ASM ==\n";
        if (!guardarASM(asmFile, codigoASM)) {
            cerr << "ERROR: No se pudo escribir el archivo ASM.\n";
            return false;
        }

        cout << "== ENSAMBLANDO ==\n";
        if (!ensamblar(asmFile, objFile)) {
            cerr << "ERROR: NASM falló al ensamblar.\n";
            return false;
        }

        cout << "== ENLAZANDO ==\n";
        if (!enlazar(objFile, exeFile)) {
            cerr << "ERROR: El linker falló.\n";
            return false;
        }

        cout << "\n✔ EXE generado: " << exeFile << "\n";
        return true;
    }
};

#endif //COMPILADOR_ASSEMBLER_DRIVER_H