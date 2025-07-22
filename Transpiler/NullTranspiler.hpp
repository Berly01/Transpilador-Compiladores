#pragma once
#include <regex>
#include <string>
#include <sstream>

class NullTranspiler {

    struct StringRegion {
        size_t start;
        size_t length;

        StringRegion(size_t s, size_t l) : start(s), length(l) {}
    };

private:
    // Expresi�n regular para capturar NULL como palabra completa
    // Evita reemplazar NULL dentro de strings o como parte de otras palabras
    std::regex null_pattern{
        R"(\bNULL\b)"
    };

    // Expresi�n regular para detectar strings literales (para evitar reemplazar NULL dentro de ellos)
    std::regex string_literal_pattern{
        R"("([^"\\]|\\.)*")"
    };

    // Expresi�n regular para detectar comentarios de l�nea
    std::regex line_comment_pattern{
        R"(//.*$)"
    };

    // Expresi�n regular para detectar comentarios de bloque
    std::regex block_comment_pattern{
        R"(/\*.*?\*/)"
    };

public:
    std::string transpileFile(const std::string& content);

private:
    std::string transpileNullStatements(const std::string& content);

    std::string processNullLine(const std::string& line);

    bool containsStringLiteralOrComment(const std::string& line);

    std::string processLineWithLiterals(const std::string& line);

    std::vector<NullTranspiler::StringRegion> findProtectedRegions(const std::string& line);

    std::string trim(const std::string& str);
};



std::string NullTranspiler::transpileFile(const std::string& content) {
    std::string result = content;

    // Transpilar NULL statements
    result = transpileNullStatements(result);

    return result;
}

std::string NullTranspiler::transpileNullStatements(const std::string& content)
{
    // Procesar l�nea por l�nea para mejor control
    std::istringstream iss(content);
    std::string line;
    std::string processed_content;

    while (std::getline(iss, line)) {
        std::string processed_line = processNullLine(line);
        processed_content += processed_line + "\n";
    }

    return processed_content;
}

std::string NullTranspiler::processNullLine(const std::string& line)
{
    // Verificar si la l�nea contiene comentarios o strings literales
    if (containsStringLiteralOrComment(line)) {
        return processLineWithLiterals(line);
    }

    // Si no hay strings literales ni comentarios, procesar directamente
    return std::regex_replace(line, null_pattern, "nullptr");
}

bool NullTranspiler::containsStringLiteralOrComment(const std::string& line)
{
    std::smatch match;

    // Verificar si contiene string literales
    if (std::regex_search(line, match, string_literal_pattern)) {
        return true;
    }

    // Verificar si contiene comentarios
    if (std::regex_search(line, match, line_comment_pattern) ||
        std::regex_search(line, match, block_comment_pattern)) {
        return true;
    }

    return false;
}

std::string NullTranspiler::processLineWithLiterals(const std::string& line)
{
    std::string result = line;
    std::vector<NullTranspiler::StringRegion> protected_regions = findProtectedRegions(line);

    // Procesar la l�nea evitando las regiones protegidas
    std::string processed_line;
    size_t last_pos = 0;

    for (const auto& region : protected_regions) {
        // Procesar la parte antes de la regi�n protegida
        if (region.start > last_pos) {
            std::string segment = line.substr(last_pos, region.start - last_pos);
            segment = std::regex_replace(segment, null_pattern, "nullptr");
            processed_line += segment;
        }

        // Agregar la regi�n protegida sin cambios
        processed_line += line.substr(region.start, region.length);
        last_pos = region.start + region.length;
    }

    // Procesar la parte restante
    if (last_pos < line.length()) {
        std::string segment = line.substr(last_pos);
        segment = std::regex_replace(segment, null_pattern, "nullptr");
        processed_line += segment;
    }

    return processed_line;
}

std::vector<NullTranspiler::StringRegion> NullTranspiler::findProtectedRegions(const std::string& line)
{
    std::vector<StringRegion> regions;

    // Encontrar strings literales
    std::sregex_iterator string_iter(line.begin(), line.end(), string_literal_pattern);
    std::sregex_iterator string_end;

    for (; string_iter != string_end; ++string_iter) {
        const std::smatch& match = *string_iter;
        regions.emplace_back(match.position(), match.length());
    }

    // Encontrar comentarios de l�nea
    std::sregex_iterator comment_iter(line.begin(), line.end(), line_comment_pattern);
    std::sregex_iterator comment_end;

    for (; comment_iter != comment_end; ++comment_iter) {
        const std::smatch& match = *comment_iter;
        regions.emplace_back(match.position(), match.length());
    }

    // Encontrar comentarios de bloque
    std::sregex_iterator block_iter(line.begin(), line.end(), block_comment_pattern);
    std::sregex_iterator block_end;

    for (; block_iter != block_end; ++block_iter) {
        const std::smatch& match = *block_iter;
        regions.emplace_back(match.position(), match.length());
    }

    // Ordenar regiones por posici�n
    std::sort(regions.begin(), regions.end(),
        [](const NullTranspiler::StringRegion& a, const NullTranspiler::StringRegion& b) {
            return a.start < b.start;
        });

    return regions;
}

std::string NullTranspiler::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
