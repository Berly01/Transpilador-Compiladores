#pragma once
#include <regex>
#include <set>
#include <sstream>
#include <string>


class StringTranspiler {

    struct StringRegion {
        size_t start;
        size_t length;

        StringRegion(size_t s, size_t l) : start(s), length(l) {}
    };

private:
    // Expresion regular para char array con inicializacion de string literal
    // Ejemplo: char str[] = "hello", char name[50] = "world"
    std::regex char_array_init_pattern{
        R"(\bchar\s+([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*(\d*)\s*\]\s*=\s*("[^"]*"))"
    };

    // Expresion regular para char pointer con inicializacion de string literal
    // Ejemplo: char* str = "hello"
    std::regex char_pointer_init_pattern{
        R"(\bchar\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*("[^"]*"))"
    };

    // Expresion regular para strcpy calls
    // Ejemplo: strcpy(dest, "source"), strcpy(dest, src)
    std::regex strcpy_pattern{
        R"(\bstrcpy\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,\s*([^)]+)\s*\))"
    };

    // Expresion regular para strcmp calls
    // Ejemplo: strcmp(str1, str2), strcmp(str, "literal")
    std::regex strcmp_pattern{
        R"(\bstrcmp\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\))"
    };

    // Expresion regular para detectar strings literales
    std::regex string_literal_pattern{
        R"("([^"\\]|\\.)*")"
    };

    // Expresion regular para detectar comentarios
    std::regex comment_pattern{
        R"(//.*$|/\*.*?\*/)"
    };

    // Set para rastrear variables que han sido convertidas a std::string
    std::set<std::string> converted_strings;

public:
    std::string transpileFile(const std::string& content);

private:
    std::string addStringInclude(const std::string& content);

    std::string replaceStringHeader(const std::string& content);

    std::string transpileStringDeclarations(const std::string& content);

    std::string transpileStringOperations(const std::string& content);

    std::string processStringDeclarationLine(const std::string& line);

    std::string processStringOperationLine(const std::string& line);

    bool containsStringLiteralOrComment(const std::string& line);

    std::string processLineWithLiterals(const std::string& line, bool isDeclaration);

    std::string processStringDeclarations(const std::string& line);

    std::string processStringOperations(const std::string& line);

    std::string processCharArrayInit(const std::string& line);

    std::string processCharPointerInit(const std::string& line);

    std::string processStrcpyCalls(const std::string& line);

    std::string processStrcmpCalls(const std::string& line);

    std::string determineComparisonOperator(const std::string& line, size_t strcmp_pos);

    std::vector<StringRegion> findProtectedRegions(const std::string& line);

    std::string trim(const std::string& str);
};




std::string StringTranspiler::transpileFile(const std::string& content) {
    std::string result = content;

    converted_strings.clear();

    result = addStringInclude(result);

    result = replaceStringHeader(result);

    result = transpileStringDeclarations(result);

    result = transpileStringOperations(result);

    return result;
}

std::string StringTranspiler::addStringInclude(const std::string& content) {
    std::string result = content;

    if (result.find("#include <string>") == std::string::npos) {
        std::regex include_pattern(R"(#include\s*[<"][^>"]*[>"])");
        std::smatch match;

        if (std::regex_search(result, match, include_pattern)) {
            size_t pos = match.position() + match.length();
            result.insert(pos, "\n#include <string>");
        }
        else {
            result = "#include <string>\n" + result;
        }
    }

    return result;
}

std::string StringTranspiler::replaceStringHeader(const std::string& content) {
    std::string result = content;

    result = std::regex_replace(result,
        std::regex(R"(#include\s*<string\.h>)"),
        "// #include <string.h> // Reemplazado por <string>");

    return result;
}

std::string StringTranspiler::transpileStringDeclarations(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::string processed_content;

    while (std::getline(iss, line)) {
        std::string processed_line = processStringDeclarationLine(line);
        processed_content += processed_line + "\n";
    }

    return processed_content;
}

std::string StringTranspiler::transpileStringOperations(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::string processed_content;

    while (std::getline(iss, line)) {
        std::string processed_line = processStringOperationLine(line);
        processed_content += processed_line + "\n";
    }

    return processed_content;
}

std::string StringTranspiler::processStringDeclarationLine(const std::string& line) {
    if (containsStringLiteralOrComment(line)) {
        return processLineWithLiterals(line, true);
    }

    return processStringDeclarations(line);
}

std::string StringTranspiler::processStringOperationLine(const std::string& line) {
    if (containsStringLiteralOrComment(line)) {
        return processLineWithLiterals(line, false);
    }

    return processStringOperations(line);
}

bool StringTranspiler::containsStringLiteralOrComment(const std::string& line) {
    std::smatch match;
    return std::regex_search(line, match, string_literal_pattern) ||
        std::regex_search(line, match, comment_pattern);
}

std::string StringTranspiler::processLineWithLiterals(const std::string& line, bool isDeclaration) {
    std::vector<StringRegion> protected_regions = findProtectedRegions(line);

    std::string processed_line;
    size_t last_pos = 0;

    for (const auto& region : protected_regions) {
        if (region.start > last_pos) {
            std::string segment = line.substr(last_pos, region.start - last_pos);
            if (isDeclaration) {
                segment = processStringDeclarations(segment);
            }
            else {
                segment = processStringOperations(segment);
            }
            processed_line += segment;
        }

        processed_line += line.substr(region.start, region.length);
        last_pos = region.start + region.length;
    }

    if (last_pos < line.length()) {
        std::string segment = line.substr(last_pos);
        if (isDeclaration) {
            segment = processStringDeclarations(segment);
        }
        else {
            segment = processStringOperations(segment);
        }
        processed_line += segment;
    }

    return processed_line;
}

std::string StringTranspiler::processStringDeclarations(const std::string& line) {
    std::string result = line;

    result = processCharArrayInit(result);

    result = processCharPointerInit(result);

    return result;
}

std::string StringTranspiler::processStringOperations(const std::string& line) {
    std::string result = line;

    result = processStrcpyCalls(result);

    result = processStrcmpCalls(result);

    return result;
}

std::string StringTranspiler::processCharArrayInit(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, char_array_init_pattern)) {
        std::string var_name = trim(match[1].str());
        std::string array_size = trim(match[2].str());
        std::string string_literal = match[3].str();

        converted_strings.insert(var_name);

        std::string replacement = "std::string " + var_name + " = " + string_literal +
            "; // Convertido de char array";

        size_t pos = match.position();
        size_t len = match.length();
        result.replace(pos, len, replacement);
    }

    return result;
}

std::string StringTranspiler::processCharPointerInit(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, char_pointer_init_pattern)) {
        std::string var_name = trim(match[1].str());
        std::string string_literal = match[2].str();

        converted_strings.insert(var_name);

        std::string replacement = "std::string " + var_name + " = " + string_literal +
            "; // Convertido de char*";

        size_t pos = match.position();
        size_t len = match.length();
        result.replace(pos, len, replacement);
    }

    return result;
}

std::string StringTranspiler::processStrcpyCalls(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, strcpy_pattern)) {
        std::string dest = trim(match[1].str());
        std::string source = trim(match[2].str());

        if (converted_strings.find(dest) != converted_strings.end()) {
            std::string replacement = dest + " = " + source + "; // Convertido de strcpy";

            size_t pos = match.position();
            size_t len = match.length();
            result.replace(pos, len, replacement);
        }
    }

    return result;
}

std::string StringTranspiler::processStrcmpCalls(const std::string& line) {
    std::string result = line;
    std::smatch match;

    while (std::regex_search(result, match, strcmp_pattern)) {
        std::string str1 = trim(match[1].str());
        std::string str2 = trim(match[2].str());

        bool str1_converted = converted_strings.find(str1) != converted_strings.end();
        bool str2_converted = converted_strings.find(str2) != converted_strings.end();

        if (str1_converted || str2_converted) {
            std::string comparison_op = determineComparisonOperator(result, match.position());
            std::string replacement = "(" + str1 + " " + comparison_op + " " + str2 + ")";

            size_t pos = match.position();
            size_t len = match.length();
            result.replace(pos, len, replacement + " /* Convertido de strcmp */");
        }
    }

    return result;
}

std::string StringTranspiler::determineComparisonOperator(const std::string& line, size_t strcmp_pos) {
    if (strcmp_pos > 0) {
        std::string before = line.substr(0, strcmp_pos);

        if (before.find("== 0") != std::string::npos ||
            before.find("!= 0") != std::string::npos) {
            return "=="; 
        }
        if (before.find("< 0") != std::string::npos) {
            return "<";  
        }
        if (before.find("> 0") != std::string::npos) {
            return ">";  
        }
        if (before.find("<=") != std::string::npos) {
            return "<=";
        }
        if (before.find(">=") != std::string::npos) {
            return ">=";
        }
    }

    size_t after_pos = strcmp_pos + 6; 
    if (after_pos < line.length()) {
        std::string after = line.substr(after_pos);
        if (after.find("== 0") != std::string::npos) {
            return "==";
        }
        if (after.find("!= 0") != std::string::npos) {
            return "!=";
        }
        if (after.find("< 0") != std::string::npos) {
            return "<";
        }
        if (after.find("> 0") != std::string::npos) {
            return ">";
        }
    }

    return "==";
}

std::vector<StringTranspiler::StringRegion> StringTranspiler::findProtectedRegions(const std::string& line) {
    std::vector<StringRegion> regions;

    std::sregex_iterator iter(line.begin(), line.end(), comment_pattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        const std::smatch& match = *iter;
        regions.emplace_back(match.position(), match.length());
    }

    std::sort(regions.begin(), regions.end(),
        [](const StringRegion& a, const StringRegion& b) {
            return a.start < b.start;
        });

    return regions;
}

std::string StringTranspiler::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
