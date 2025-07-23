#include <iostream>
#include <fstream>
#include "PrintfToCoutTranspiler.hpp"
#include "DefineTranspiler.hpp"
#include "NullTranspiler.hpp"
#include "ArrayTranspiler.hpp"
#include "StringTranspiler.hpp"

std::string test_input();

int main(int argc, char** argv) {

    std::string input_file;
    std::string output_ile;
    std::string content;

    if (argc == 3) {
        input_file = argv[1];
        output_ile = argv[2];

        std::ifstream inFile(input_file);
        if (!inFile.is_open()) {
            std::cerr << "No se pudo abrir el archivo: " << input_file << "\n";
            return 1;
        }

        std::string aux_content((std::istreambuf_iterator<char>(inFile)),
            std::istreambuf_iterator<char>());
        inFile.close();

        content = aux_content;
    }
    else {
        std::cerr << "No se ingreso un archivo de entrada y salida, se utilizara un archivo de prueba\n";
        content = test_input();
        output_ile = "test-output.cpp";
    }

    std::ofstream outFile(output_ile, std::ofstream::trunc);

    try {
        DefineTranspiler defineTranspiler;
        NullTranspiler nullTranspiler;
        ArrayTranspiler arrayTranspiler;
        StringTranspiler stringTranspiler;
        PrintfTranspiler printfTranspiler;

        std::string result = defineTranspiler.transpileFile(content);
        result = nullTranspiler.transpileFile(result);
        result = arrayTranspiler.transpileFile(result);
        result = stringTranspiler.transpileFile(result);
        result = printfTranspiler.transpileFile(result);

        outFile << result;

        std::cout << "Transpilacion completada\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

	return 0;
}

std::string test_input() {

    std::string test_input = R"(#include <stdio.h>

#define PI 3.1416

int main() {

	char* firstname = "mike";

	char lastname[] = "perez";

	int money = 2000;

	double radio = PI * 100;

	char c[] = { 'a', 'b', 'c', 'd', 'e' };

	double nums[] = { 1.2, 1.3, 1.4, 1.5, 1.6 };

	int i[10];

	int* x = (int*)malloc(sizeof(int) * 10);

	printf("hola %s %s, tienes %d soles?", firstname, lastname, money);

	int* ptr = NULL;

	return 0;
})";

    std::ofstream out("test_input.c", std::ostream::trunc);
    out << test_input;
    out.close();

    return test_input;
}
