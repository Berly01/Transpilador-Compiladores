#pragma once
#include <regex>
#include <sstream>
#include <string>

class ArrayTranspiler {

    struct StringRegion {
        size_t start;
        size_t length;

        StringRegion(size_t s, size_t l) : start(s), length(l) {}
    };

private:
    // Expresión regular para capturar declaraciones de arreglos con tamaño fijo
    // Ejemplo: int arr[10], float data[5], char buffer[256]
    std::regex array_declaration_pattern{
        R"(([A-Za-z_][A-Za-z0-9_]*\s+\*?\s*)([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*(\d+)\s*\])"
    };

    // Expresión regular para declaraciones con inicialización
    // Ejemplo: int arr[5] = {1, 2, 3, 4, 5}
    std::regex array_init_pattern{
        R"(([A-Za-z_][A-Za-z0-9_]*\s+\*?\s*)([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*(\d+)\s*\]\s*=\s*(\{[^}]*\}))"
    };

    // Expresión regular para declaraciones con inicialización automática de tamaño
    // Ejemplo: int arr[] = {1, 2, 3, 4, 5}
    std::regex array_auto_init_pattern{
        R"(([A-Za-z_][A-Za-z0-9_]*\s+\*?\s*)([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*\]\s*=\s*(\{[^}]*\}))"
    };

    // Expresión regular para múltiples declaraciones en una línea
    // Ejemplo: int a[10], b[20], c[30];
    std::regex multi_array_pattern{
        R"(([A-Za-z_][A-Za-z0-9_]*\s+)([A-Za-z_][A-Za-z0-9_]*\s*\[\s*\d+\s*\](?:\s*,\s*[A-Za-z_][A-Za-z0-9_]*\s*\[\s*\d+\s*\])*)\s*;)"
    };

    // Expresión regular para detectar strings literales
    std::regex string_literal_pattern{
        R"("([^"\\]|\\.)*")"
    };

    // Expresión regular para detectar comentarios
    std::regex comment_pattern{
        R"(//.*$|/\*.*?\*/)"
    };

public:
    std::string transpileFile(const std::string& content);

private:
    std::string addArrayInclude(const std::string& content);

    std::string transpileArrayDeclarations(const std::string& content);

    std::string processArrayLine(const std::string& line);

    bool containsStringLiteralOrComment(const std::string& line);

    std::string processLineWithLiterals(const std::string& line);

    std::string processArrayDeclarations(const std::string& line);

    std::string processAutoInitArrays(const std::string& line);

    std::string processInitializedArrays(const std::string& line);

    std::string processMultipleArrays(const std::string& line);

    std::string processSimpleArrays(const std::string& line);

    std::string convertMultipleArrayDeclarations(const std::string& type, const std::string& declarations);

    int countInitializerElements(const std::string& initializer);

    bool isPartOfProcessedDeclaration(const std::string& line, size_t pos);

    std::vector<StringRegion> findProtectedRegions(const std::string& line);

    std::string trim(const std::string& str);
};

#include "ArrayTranspiler.hpp"


std::string ArrayTranspiler::transpileFile(const std::string& content) {
    std::string result = content;

    // Agregar include de <array> si es necesario
    result = addArrayInclude(result);

    // Transpilar declaraciones de arreglos
    result = transpileArrayDeclarations(result);

    return result;
}

std::string ArrayTranspiler::addArrayInclude(const std::string& content) {
    std::string result = content;

    // Verificar si ya tiene #include <array>
    if (result.find("#include <array>") == std::string::npos) {
        // Buscar la primera línea de include para insertar después
        std::regex include_pattern(R"(#include\s*[<"][^>"]*[>"])");
        std::smatch match;

        if (std::regex_search(result, match, include_pattern)) {
            size_t pos = match.position() + match.length();
            result.insert(pos, "\n#include <array>");
        }
        else {
            // Si no hay includes, agregar al inicio
            result = "#include <array>\n" + result;
        }
    }

    return result;
}

std::string ArrayTranspiler::transpileArrayDeclarations(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::string processed_content;

    while (std::getline(iss, line)) {
        std::string processed_line = processArrayLine(line);
        processed_content += processed_line + "\n";
    }

    return processed_content;
}

std::string ArrayTranspiler::processArrayLine(const std::string& line) {
    // Verificar si la línea contiene comentarios o strings literales
    if (containsStringLiteralOrComment(line)) {
        return processLineWithLiterals(line);
    }

    // Procesar la línea directamente
    return processArrayDeclarations(line);
}

bool ArrayTranspiler::containsStringLiteralOrComment(const std::string& line) {
    std::smatch match;
    return std::regex_search(line, match, string_literal_pattern) ||
        std::regex_search(line, match, comment_pattern);
}

std::string ArrayTranspiler::processLineWithLiterals(const std::string& line) {
    std::vector<StringRegion> protected_regions = findProtectedRegions(line);

    std::string processed_line;
    size_t last_pos = 0;

    for (const auto& region : protected_regions) {
        // Procesar la parte antes de la región protegida
        if (region.start > last_pos) {
            std::string segment = line.substr(last_pos, region.start - last_pos);
            segment = processArrayDeclarations(segment);
            processed_line += segment;
        }

        // Agregar la región protegida sin cambios
        processed_line += line.substr(region.start, region.length);
        last_pos = region.start + region.length;
    }

    // Procesar la parte restante
    if (last_pos < line.length()) {
        std::string segment = line.substr(last_pos);
        segment = processArrayDeclarations(segment);
        processed_line += segment;
    }

    return processed_line;
}

std::string ArrayTranspiler::processArrayDeclarations(const std::string& line) {
    std::string result = line;

    // Procesar declaraciones con inicialización automática de tamaño primero
    result = processAutoInitArrays(result);

    // Procesar declaraciones con inicialización explícita
    result = processInitializedArrays(result);

    // Procesar múltiples declaraciones en una línea
    result = processMultipleArrays(result);

    // Procesar declaraciones simples
    result = processSimpleArrays(result);

    return result;
}

std::string ArrayTranspiler::processAutoInitArrays(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, array_auto_init_pattern)) {
        std::string type = trim(match[1].str());
        std::string name = trim(match[2].str());
        std::string initializer = match[3].str();

        // Contar elementos en el inicializador
        int size = countInitializerElements(initializer);

        std::string replacement = "std::array<" + type + ", " + std::to_string(size) + "> " +
            name + " = " + initializer + "; // Convertido de arreglo C";

        size_t pos = match.position();
        size_t len = match.length();
        result.replace(pos, len, replacement);
    }

    return result;
}

std::string ArrayTranspiler::processInitializedArrays(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, array_init_pattern)) {
        std::string type = trim(match[1].str());
        std::string name = trim(match[2].str());
        std::string size = match[3].str();
        std::string initializer = match[4].str();

        std::string replacement = "std::array<" + type + ", " + size + "> " +
            name + " = " + initializer + "; // Convertido de arreglo C";

        size_t pos = match.position();
        size_t len = match.length();
        result.replace(pos, len, replacement);
    }

    return result;
}

std::string ArrayTranspiler::processMultipleArrays(const std::string& line) {
    std::string result = line;
    std::smatch match;

    if (std::regex_search(result, match, multi_array_pattern)) {
        std::string type = trim(match[1].str());
        std::string declarations = match[2].str();

        std::string converted = convertMultipleArrayDeclarations(type, declarations);

        size_t pos = match.position();
        size_t len = match.length();
        result.replace(pos, len, converted);
    }

    return result;
}

std::string ArrayTranspiler::processSimpleArrays(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, array_declaration_pattern)) {
        std::string type = trim(match[1].str());
        std::string name = trim(match[2].str());
        std::string size = match[3].str();

        // Verificar que no sea parte de una declaración ya procesada
        if (!isPartOfProcessedDeclaration(result, match.position())) {
            std::string replacement = "std::array<" + type + ", " + size + "> " +
                name + "; // Convertido de arreglo C";

            size_t pos = match.position();
            size_t len = match.length();
            result.replace(pos, len, replacement);
        }
    }

    return result;
}

std::string ArrayTranspiler::convertMultipleArrayDeclarations(const std::string& type, const std::string& declarations) {
    std::string result;
    std::regex single_decl_pattern{ R"(([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*(\d+)\s*\])" };
    std::sregex_iterator iter(declarations.begin(), declarations.end(), single_decl_pattern);
    std::sregex_iterator end;

    bool first = true;
    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        std::string name = trim(match[1].str());
        std::string size = match[2].str();

        if (!first) {
            result += "\n";
        }
        first = false;

        result += "std::array<" + type + ", " + size + "> " + name + "; // Convertido de arreglo C";
    }

    return result;
}

int ArrayTranspiler::countInitializerElements(const std::string& initializer) {
    if (initializer.length() < 2) return 0;

    std::string content = initializer.substr(1, initializer.length() - 2); // Remover { }
    if (content.empty()) return 0;

    int count = 1; // Al menos un elemento si no está vacío
    for (char c : content) {
        if (c == ',') count++;
    }

    return count;
}

bool ArrayTranspiler::isPartOfProcessedDeclaration(const std::string& line, size_t pos) {
    // Buscar hacia atrás para ver si ya hay "std::array" antes de esta posición
    if (pos > 0) {
        std::string before = line.substr(0, pos);
        return before.find("std::array") != std::string::npos;
    }
    return false;
}

std::vector<ArrayTranspiler::StringRegion> ArrayTranspiler::findProtectedRegions(const std::string& line) {
    std::vector<ArrayTranspiler::StringRegion> regions;

    // Encontrar strings literales y comentarios
    std::sregex_iterator iter(line.begin(), line.end(), string_literal_pattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        regions.emplace_back(match.position(), match.length());
    }

    std::sregex_iterator comment_iter(line.begin(), line.end(), comment_pattern);
    for (; comment_iter != end; ++comment_iter) {
        const std::smatch& match = *comment_iter;
        regions.emplace_back(match.position(), match.length());
    }

    // Ordenar regiones por posición
    std::sort(regions.begin(), regions.end(),
        [](const StringRegion& a, const StringRegion& b) {
            return a.start < b.start;
        });

    return regions;
}

std::string ArrayTranspiler::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
