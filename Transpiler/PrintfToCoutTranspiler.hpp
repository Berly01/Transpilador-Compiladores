#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <algorithm>

class PrintfTranspiler {
private:
    // Expresión regular para capturar printf con formato y argumentos
    std::regex printf_pattern{
        "printf\\s*\\(\\s*\"([^\"]*)\"\\s*(?:,\\s*([^)]*))?\\s*\\)"
    };

    // Mapeo de especificadores de formato C a manipuladores C++
    std::vector<std::pair<std::regex, std::string>> format_replacements{
        {std::regex(R"(%d)"), ""},           // %d -> directo
        {std::regex(R"(%i)"), ""},           // %i -> directo
        {std::regex(R"(%u)"), ""},           // %u -> directo
        {std::regex(R"(%ld)"), ""},          // %ld -> directo
        {std::regex(R"(%lu)"), ""},          // %lu -> directo
        {std::regex(R"(%f)"), ""},           // %f -> directo
        {std::regex(R"(%lf)"), ""},          // %lf -> directo
        {std::regex(R"(%g)"), ""},           // %g -> directo
        {std::regex(R"(%c)"), ""},           // %c -> directo
        {std::regex(R"(%s)"), ""},           // %s -> directo
        {std::regex(R"(%x)"), " << std::hex << "}, // %x -> hex
        {std::regex(R"(%X)"), " << std::hex << std::uppercase << "}, // %X -> HEX
        {std::regex(R"(%o)"), " << std::oct << "}, // %o -> octal
        {std::regex(R"(\\n)"), "\\n"},       // \n se mantiene
        {std::regex(R"(\\t)"), "\\t"},       // \t se mantiene
    };

public:
    std::string transpileFile(const std::string& content);

private:
    std::string addIncludes(const std::string& content);

    std::string transpilePrintfStatements(const std::string& content);

    std::string convertToCout(const std::string& format, const std::string& args);

    std::vector<std::string> splitArguments(const std::string& args);

    std::string processFormatString(
        const std::string& format,
        const std::vector<std::string>& args);
};



std::string PrintfTranspiler::transpileFile(const std::string& content)
{
    // Agregar includes necesarios al inicio
    std::string result = addIncludes(content);

    // Transpilar printf statements
    result = transpilePrintfStatements(result);

    return result;
}

std::string PrintfTranspiler::addIncludes(const std::string& content)
{
    std::string result = content;

    // Verificar si ya tiene #include <iostream>
    if (result.find("#include <iostream>") == std::string::npos) {
        // Buscar la primera línea de include para insertar después
        std::regex include_pattern(R"(#include\s*[<"][^>"]*[>"])");
        std::smatch match;

        if (std::regex_search(result, match, include_pattern)) {
            size_t pos = match.position() + match.length();
            result.insert(pos, "\n#include <iostream>");
        }
        else {
            // Si no hay includes, agregar al inicio
            result = "#include <iostream>\n" + result;
        }
    }

    // Reemplazar #include <stdio.h> si existe
    result = std::regex_replace(result,
        std::regex(R"(#include\s*<stdio\.h>)"),
        "// #include <stdio.h> // Reemplazado por <iostream>");

    return result;
}

std::string PrintfTranspiler::transpilePrintfStatements(const std::string& content)
{
    std::string result = content;
    std::smatch match;

    while (std::regex_search(result, match, printf_pattern)) {
        std::string format_string = match[1].str();
        std::string arguments = match.length() > 2 ? match[2].str() : "";

        std::string cout_statement = convertToCout(format_string, arguments);

        // Reemplazar la primera ocurrencia encontrada
        size_t pos = match.position();
        size_t len = match.length();
        result.replace(pos, len, cout_statement);
    }

    return result;
}

std::string PrintfTranspiler::convertToCout(const std::string& format, const std::string& args)
{
    std::string result = "std::cout";

    if (format.empty()) {
        return result + ";";
    }

    // Dividir argumentos por comas (simplificado)
    std::vector<std::string> arguments = splitArguments(args);

    // Procesar el string de formato
    std::string processed_format = processFormatString(format, arguments);

    if (!processed_format.empty()) {
        result += " << " + processed_format;
    }

    return result /*+ ";"*/;
}

std::vector<std::string> PrintfTranspiler::splitArguments(const std::string& args)
{
    std::vector<std::string> result;
    if (args.empty()) return result;

    std::string trimmed = args;
    // Eliminar espacios al inicio y final
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t") + 1);

    // División simple por comas (no maneja comas dentro de strings)
    std::stringstream ss(trimmed);
    std::string item;

    while (std::getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        if (!item.empty()) {
            result.push_back(item);
        }
    }

    return result;
}

std::string PrintfTranspiler::processFormatString(const std::string& format, const std::vector<std::string>& args)
{
    std::string result;
    std::string current_format = format;
    size_t arg_index = 0;

    // Buscar especificadores de formato
    std::regex format_spec(R"(%([-+ #0]*)([0-9]*)(\.?[0-9]*)([diouxXfFeEgGcs]))");
    std::smatch match;

    size_t last_pos = 0;

    while (std::regex_search(current_format, match, format_spec)) {
        // Agregar texto literal antes del especificador
        std::string literal = current_format.substr(last_pos, match.position());
        if (!literal.empty()) {
            if (!result.empty()) result += " << ";
            result += "\"" + literal + "\"";
        }

        // Agregar el argumento correspondiente
        if (arg_index < args.size()) {
            if (!result.empty()) result += " << ";

            char spec = match[4].str()[0];
            if (spec == 'x') {
                result += "std::hex << " + args[arg_index] + " << std::dec";
            }
            else if (spec == 'X') {
                result += "std::hex << std::uppercase << " + args[arg_index]
                    + " << std::nouppercase << std::dec";
            }
            else if (spec == 'o') {
                result += "std::oct << " + args[arg_index] + " << std::dec";
            }
            else {
                result += args[arg_index];
            }
            arg_index++;
        }

        last_pos = match.position() + match.length();
        current_format = current_format.substr(last_pos);
        last_pos = 0;
    }

    // Agregar texto restante
    if (!current_format.empty()) {
        if (!result.empty()) result += " << ";
        result += "\"" + current_format + "\"";
    }

    return result;
}
