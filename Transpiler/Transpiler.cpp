#include <iostream>
#include "PrintfToCoutTranspiler.hpp"
#include "DefineTranspiler.hpp"
#include "NullTranspiler.hpp"
#include "ArrayTranspiler.hpp"
#include "StringTranspiler.hpp"


int main(int argc, char** argv)
{

    std::string input;
    std::string output;

    if (argc == 3) {
        input = argv[1];
        output = argv[2];
    }
    else {
        return 1;
    }

    std::ifstream inFile(input);
    if (!inFile.is_open()) {
        exit(1);
    }
    std::string content((std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    std::ofstream outFile(output, std::ofstream::trunc);

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
