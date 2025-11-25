#ifndef SIMPLEJSON_H
#define SIMPLEJSON_H

#include <string>
#include <unordered_map>
#include <vector>

class SimpleJsonValue {
public:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Object,
        Array
    };

    using Object = std::unordered_map<std::string, SimpleJsonValue>;
    using Array = std::vector<SimpleJsonValue>;

    SimpleJsonValue();
    explicit SimpleJsonValue(double number);
    explicit SimpleJsonValue(const std::string& text);
    explicit SimpleJsonValue(bool boolean);
    explicit SimpleJsonValue(const Object& object);
    explicit SimpleJsonValue(const Array& array);

    Type getType() const { return type; }
    double asNumber(double defaultValue = 0.0) const;
    int asInt(int defaultValue = 0) const;
    bool asBool(bool defaultValue = false) const;
    std::string asString(const std::string& defaultValue = "") const;
    const Object& asObject() const;
    const Array& asArray() const;

    bool hasKey(const std::string& key) const;
    const SimpleJsonValue& operator[](const std::string& key) const;

private:
    Type type;
    double numberValue;
    bool boolValue;
    std::string stringValue;
    Object objectValue;
    Array arrayValue;
};

class SimpleJsonParser {
public:
    explicit SimpleJsonParser(const std::string& data);
    SimpleJsonValue parse();

private:
    std::string input;
    size_t index;

    void skipWhitespace();
    char peek() const;
    char get();
    bool match(char expected);

    SimpleJsonValue parseValue();
    SimpleJsonValue parseObject();
    SimpleJsonValue parseArray();
    SimpleJsonValue parseString();
    SimpleJsonValue parseNumber();
    SimpleJsonValue parseTrue();
    SimpleJsonValue parseFalse();
    SimpleJsonValue parseNull();
};

#endif
