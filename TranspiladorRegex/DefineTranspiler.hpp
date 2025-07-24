#pragma once
#include <regex>
#include <sstream>

class DefineTranspiler {
private:
    // Expresion regular para capturar #define con diferentes patrones
    std::regex define_pattern{
        R"(#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.+))"
    };

    // Expresion regular para #define sin valor (solo nombre)
    std::regex define_no_value_pattern{
        R"(#define\s+([A-Za-z_][A-Za-z0-9_]*)\s*$)"
    };

    // Expresion regular para #define con parametros (macros funcion)
    std::regex define_function_pattern{
        R"(#define\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s+(.+))"
    };

public:
    std::string transpileFile(const std::string& content);

private:
    std::string transpileDefineStatements(const std::string& content);

    std::string processDefineLine(const std::string& line);

    std::string convertDefineToConstexpr(const std::string& name, const std::string& value);

    std::string convertFunctionMacro(const std::string& name, const std::string& params, const std::string& body);

    std::string deduceType(const std::string& value);

    std::string deduceReturnType(const std::string& body);

    std::string convertParameters(const std::string& params);

    std::string trim(const std::string& str);
};


std::string DefineTranspiler::transpileFile(const std::string& content)
{
    std::string result = content;

    result = transpileDefineStatements(result);

    return result;
}

std::string DefineTranspiler::transpileDefineStatements(const std::string& content)
{
    std::string result = content;
    std::smatch match;

    std::istringstream iss(content);
    std::string line;
    std::string processed_content;

    while (std::getline(iss, line)) {
        std::string processed_line = processDefineLine(line);
        processed_content += processed_line + "\n";
    }

    return processed_content;
}

std::string DefineTranspiler::processDefineLine(const std::string& line)
{
    std::smatch match;

    if (std::regex_match(line, match, define_function_pattern)) {
        std::string name = match[1].str();
        std::string params = match[2].str();
        std::string body = match[3].str();

        return convertFunctionMacro(name, params, body);
    }

    if (std::regex_match(line, match, define_no_value_pattern)) {
        std::string name = match[1].str();
        return "constexpr bool " + name + " = true; // Convertido de #define";
    }

    if (std::regex_match(line, match, define_pattern)) {
        std::string name = match[1].str();
        std::string value = match[2].str();

        return convertDefineToConstexpr(name, value);
    }

    return line;
}

std::string DefineTranspiler::convertDefineToConstexpr(const std::string& name, const std::string& value)
{
    std::string trimmed_value = trim(value);

    std::string type = deduceType(trimmed_value);

    std::string result = "constexpr " + type + " " + name + " = " + trimmed_value + ";";
    result += " // Convertido de #define";

    return result;
}

std::string DefineTranspiler::convertFunctionMacro(const std::string& name, const std::string& params, const std::string& body)
{
    std::string param_list = convertParameters(params);
    std::string return_type = deduceReturnType(body);

    std::string result = "constexpr " + return_type + " " + name + "(" + param_list + ") {";
    result += " return " + body + "; }";
    result += " // Convertido de macro #define";

    return result;
}

std::string DefineTranspiler::deduceType(const std::string& value)
{
    std::string trimmed = trim(value);

    if ((trimmed.front() == '"' && trimmed.back() == '"') ||
        (trimmed.front() == '\'' && trimmed.back() == '\'')) {
        if (trimmed.front() == '"') {
            return "const char*";
        }
        else {
            return "char";
        }
    }

    if (trimmed.find('.') != std::string::npos ||
        trimmed.find('f') != std::string::npos ||
        trimmed.find('F') != std::string::npos) {
        if (trimmed.back() == 'f' || trimmed.back() == 'F') {
            return "float";
        }
        return "double";
    }

    if (trimmed.substr(0, 2) == "0x" || trimmed.substr(0, 2) == "0X") {
        return "int";
    }

    if (trimmed.length() > 1 && trimmed[0] == '0' && std::isdigit(trimmed[1])) {
        return "int";
    }

    bool is_number = true;
    bool has_minus = false;

    for (size_t i = 0; i < trimmed.length(); ++i) {
        char c = trimmed[i];
        if (i == 0 && (c == '+' || c == '-')) {
            has_minus = (c == '-');
            continue;
        }
        if (!std::isdigit(c)) {
            is_number = false;
            break;
        }
    }

    if (is_number && !trimmed.empty()) {
        long long num = std::stoll(trimmed);
        if (num > INT_MAX || num < INT_MIN) {
            return "long long";
        }
        return "int";
    }

    return "auto";
}

std::string DefineTranspiler::deduceReturnType(const std::string& body)
{
    std::string trimmed = trim(body);

    if (trimmed.find('+') != std::string::npos ||
        trimmed.find('-') != std::string::npos ||
        trimmed.find('*') != std::string::npos ||
        trimmed.find('/') != std::string::npos) {
        return "auto"; 
    }

    return deduceType(trimmed);
}

std::string DefineTranspiler::convertParameters(const std::string& params)
{
    if (params.empty()) {
        return "";
    }

    std::vector<std::string> param_list;
    std::stringstream ss(params);
    std::string param;

    while (std::getline(ss, param, ',')) {
        param = trim(param);
        if (!param.empty()) {
            param_list.push_back("auto " + param);
        }
    }

    std::string result;
    for (size_t i = 0; i < param_list.size(); ++i) {
        if (i > 0) result += ", ";
        result += param_list[i];
    }

    return result;
}

std::string DefineTranspiler::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}


