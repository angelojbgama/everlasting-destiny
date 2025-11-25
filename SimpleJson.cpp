#include "SimpleJson.h"
#include <cctype>
#include <stdexcept>

SimpleJsonValue::SimpleJsonValue() : type(Type::Null), numberValue(0.0), boolValue(false) {}
SimpleJsonValue::SimpleJsonValue(double number) : type(Type::Number), numberValue(number), boolValue(false) {}
SimpleJsonValue::SimpleJsonValue(const std::string& text) : type(Type::String), numberValue(0.0), boolValue(false), stringValue(text) {}
SimpleJsonValue::SimpleJsonValue(bool boolean) : type(Type::Bool), numberValue(0.0), boolValue(boolean) {}
SimpleJsonValue::SimpleJsonValue(const Object& object) : type(Type::Object), numberValue(0.0), boolValue(false), objectValue(object) {}
SimpleJsonValue::SimpleJsonValue(const Array& array) : type(Type::Array), numberValue(0.0), boolValue(false), arrayValue(array) {}

double SimpleJsonValue::asNumber(double defaultValue) const {
    if (type == Type::Number) {
        return numberValue;
    }
    return defaultValue;
}

int SimpleJsonValue::asInt(int defaultValue) const {
    return static_cast<int>(asNumber(defaultValue));
}

bool SimpleJsonValue::asBool(bool defaultValue) const {
    if (type == Type::Bool) {
        return boolValue;
    }
    return defaultValue;
}

std::string SimpleJsonValue::asString(const std::string& defaultValue) const {
    if (type == Type::String) {
        return stringValue;
    }
    return defaultValue;
}

const SimpleJsonValue::Object& SimpleJsonValue::asObject() const {
    if (type != Type::Object) {
        throw std::runtime_error("JSON value is not an object");
    }
    return objectValue;
}

const SimpleJsonValue::Array& SimpleJsonValue::asArray() const {
    if (type != Type::Array) {
        throw std::runtime_error("JSON value is not an array");
    }
    return arrayValue;
}

bool SimpleJsonValue::hasKey(const std::string& key) const {
    if (type != Type::Object) {
        return false;
    }
    return objectValue.find(key) != objectValue.end();
}

const SimpleJsonValue& SimpleJsonValue::operator[](const std::string& key) const {
    static SimpleJsonValue nullValue;
    if (type != Type::Object) {
        return nullValue;
    }
    auto it = objectValue.find(key);
    if (it == objectValue.end()) {
        return nullValue;
    }
    return it->second;
}

SimpleJsonParser::SimpleJsonParser(const std::string& data) : input(data), index(0) {}

SimpleJsonValue SimpleJsonParser::parse() {
    skipWhitespace();
    SimpleJsonValue value = parseValue();
    return value;
}

void SimpleJsonParser::skipWhitespace() {
    while (index < input.size() && std::isspace(static_cast<unsigned char>(input[index]))) {
        index++;
    }
}

char SimpleJsonParser::peek() const {
    if (index >= input.size()) {
        return '\0';
    }
    return input[index];
}

char SimpleJsonParser::get() {
    if (index >= input.size()) {
        return '\0';
    }
    return input[index++];
}

bool SimpleJsonParser::match(char expected) {
    if (peek() == expected) {
        index++;
        return true;
    }
    return false;
}

SimpleJsonValue SimpleJsonParser::parseValue() {
    skipWhitespace();
    char current = peek();
    if (current == '{') {
        return parseObject();
    } else if (current == '[') {
        return parseArray();
    } else if (current == '"') {
        return parseString();
    } else if (std::isdigit(static_cast<unsigned char>(current)) || current == '-' ) {
        return parseNumber();
    } else if (current == 't') {
        return parseTrue();
    } else if (current == 'f') {
        return parseFalse();
    } else if (current == 'n') {
        return parseNull();
    }
    std::string token;
    if (current == '\0') {
        token = "EOF";
    } else if (std::isprint(static_cast<unsigned char>(current))) {
        token = std::string(1, current);
    } else {
        token = "\\x" + std::to_string(static_cast<unsigned char>(current));
    }
    throw std::runtime_error("Unexpected token in JSON: " + token + " at position " + std::to_string(index));
}

SimpleJsonValue SimpleJsonParser::parseObject() {
    match('{');
    skipWhitespace();
    SimpleJsonValue::Object object;

    if (peek() == '}') {
        get();
        return SimpleJsonValue(object);
    }

    while (true) {
        skipWhitespace();
        if (peek() == '}') {
            break;
        }
        SimpleJsonValue key = parseString();
        skipWhitespace();
        if (!match(':')) {
            throw std::runtime_error("Expected ':' in object");
        }
        skipWhitespace();
        SimpleJsonValue value = parseValue();
        object[key.asString()] = value;
        skipWhitespace();
        if (peek() == ',') {
            get();
            continue;
        }
        break;
    }
    skipWhitespace();
    if (!match('}')) {
        throw std::runtime_error("Expected '}'");
    }
    return SimpleJsonValue(object);
}

SimpleJsonValue SimpleJsonParser::parseArray() {
    match('[');
    skipWhitespace();
    SimpleJsonValue::Array array;
    if (peek() == ']') {
        get();
        return SimpleJsonValue(array);
    }
    while (true) {
        skipWhitespace();
        if (peek() == ']') {
            break;
        }
        SimpleJsonValue value = parseValue();
        array.push_back(value);
        skipWhitespace();
        if (peek() == ',') {
            get();
            continue;
        }
        break;
    }
    skipWhitespace();
    if (!match(']')) {
        throw std::runtime_error("Expected ']'");
    }
    return SimpleJsonValue(array);
}

SimpleJsonValue SimpleJsonParser::parseString() {
    if (!match('"')) {
        throw std::runtime_error("Expected opening quote for string");
    }
    std::string text;
    while (true) {
        char c = get();
        if (c == '"') {
            break;
        }
        if (c == '\\') {
            char next = get();
            switch (next) {
            case '"': text.push_back('"'); break;
            case '\\': text.push_back('\\'); break;
            case '/': text.push_back('/'); break;
            case 'b': text.push_back('\b'); break;
            case 'f': text.push_back('\f'); break;
            case 'n': text.push_back('\n'); break;
            case 'r': text.push_back('\r'); break;
            case 't': text.push_back('\t'); break;
            default: text.push_back(next); break;
            }
        } else {
            text.push_back(c);
        }
    }
    return SimpleJsonValue(text);
}

SimpleJsonValue SimpleJsonParser::parseNumber() {
    std::string numberStr;
    if (peek() == '-') {
        numberStr.push_back(get());
    }
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        numberStr.push_back(get());
    }
    if (peek() == '.') {
        numberStr.push_back(get());
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            numberStr.push_back(get());
        }
    }
    return SimpleJsonValue(std::stod(numberStr));
}

SimpleJsonValue SimpleJsonParser::parseTrue() {
    if (input.substr(index, 4) != "true") {
        throw std::runtime_error("Invalid literal");
    }
    index += 4;
    return SimpleJsonValue(true);
}

SimpleJsonValue SimpleJsonParser::parseFalse() {
    if (input.substr(index, 5) != "false") {
        throw std::runtime_error("Invalid literal");
    }
    index += 5;
    return SimpleJsonValue(false);
}

SimpleJsonValue SimpleJsonParser::parseNull() {
    if (input.substr(index, 4) != "null") {
        throw std::runtime_error("Invalid literal");
    }
    index += 4;
    return SimpleJsonValue();
}
