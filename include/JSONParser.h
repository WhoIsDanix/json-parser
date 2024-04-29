#ifndef JSONPARSER_JSONPARSER_H
#define JSONPARSER_JSONPARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

namespace JSON {
    enum ValueType {
        ValueType_Object,
        ValueType_Array,
        ValueType_String,
        ValueType_Number,
        ValueType_Boolean,
        ValueType_Null,
        ValueType_Unknown
    };

    class Value {
    private:
        ValueType type;
        std::unordered_map<std::string, Value> obj;
        std::vector<Value> arr;
        std::string str;
        long double num = 0;

    public:
        Value() : type(ValueType_Unknown) {}
        Value(ValueType type) : type(type) {}
        Value(std::unordered_map<std::string, Value> obj) : obj(obj), type(ValueType_Object) {}
        Value(std::vector<Value> arr) : arr(arr), type(ValueType_Array) {}
        Value(std::string str) : str(str), type(ValueType_String) {}
        Value(long double num) : num(num), type(ValueType_Number) {}

        static Value makeObject(std::unordered_map<std::string, Value> obj = {}) {
            return Value(obj);
        }

        static Value makeArray(std::vector<Value> arr = {}) {
            return Value(arr);
        }

        static Value makeString(std::string str = "") {
            return Value(str);
        }

        static Value makeNumber(long double num = 0) {
            return Value(num);
        }

        static Value makeBoolean(bool b) {
            Value v = makeNumber(b);
            v.type = ValueType_Boolean;
            return v;
        }

        static Value makeNull() {
            return Value(ValueType_Null);
        }

        std::unordered_map<std::string, Value> toObject() {
            return obj;
        }

        std::vector<Value> toArray() {
            return arr;
        }

        std::string toString() {
            return str;
        }

        double toNumber() {
            return num;
        }

        bool toBoolean() {
            return num;
        }

        ValueType getType() {
            return type;
        }

        Value& operator[](std::string key) {
            return obj[key];
        }

        Value& operator[](int idx) {
            return arr[idx];
        }

        void pushToArray(Value value) {
            if (getType() == ValueType_Array) {
                arr.push_back(value);
            }
        }

        std::string getRepresentation(int indent = 0, int depth = 0) {
            switch (type) {
            case ValueType_Null: return "null";
            case ValueType_Boolean: return (toBoolean() ? "true" : "false");
            case ValueType_String: return '"' + toString() + '"';
            case ValueType_Number: {
                long lnum = static_cast<long>(num);
                return ((num - lnum) == 0) ? std::to_string(lnum) : std::to_string(num);
            }
            case ValueType_Array: {
                std::string repr = "[";
                std::vector<Value> arr = toArray();

                for (int i = 0; i < arr.size(); ++i) {
                    if (indent > 0) {
                        repr += "\n" + std::string(indent * (depth + 1), ' ');
                    }

                    repr += arr[i].getRepresentation(indent, depth + 1);

                    if (i != (arr.size() - 1)) {
                        repr += ",";
                    }
                }

                if (indent > 0) {
                    repr += "\n";
                }

                repr += std::string(indent * depth, ' ') + "]";
                return repr;
            }
            case ValueType_Object: {
                std::string repr = "{";
                std::unordered_map<std::string, Value> obj = toObject();

                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (indent > 0) {
                        repr += "\n" + std::string(indent * (depth + 1), ' ');
                    }

                    repr += "\"" + it->first + "\":";
                    if (indent > 0) repr += " ";
                    repr += it->second.getRepresentation(indent, depth + 1);

                    if (std::next(it) != obj.end()) {
                        repr += ",";
                    }
                }

                if (indent > 0) {
                    repr += "\n";
                }

                repr += std::string(indent * depth, ' ') + "}";
                return repr;
            }
            default: return "";
            }
        }

        bool saveToFile(std::string filename, int indent = 0) {
            std::ofstream stream(filename);

            if (!stream.is_open()) {
                std::cout << "[ERROR] Failed to open file \"" << filename << "\"\n";
                return false;
            }

            stream << getRepresentation(indent);
            stream.close();

            return true;
        }
    };

    class Parser {
    private:
        std::string content;
        int currIdx = -1;
        char currChar = '\0';
        int currLine = 1;
        bool isOk = true;

        void advance() {
            ++currIdx;
            if (currIdx < content.size()) {
                currChar = content[currIdx];
            }
        }

        bool isEOF() {
            return currIdx >= content.size();
        }

        bool isWhitespace(char c) {
            return c == '\u0020' || c == '\u000A' || c == '\u000D' || c == '\u0009';
        }

        void skipWhitespace() {
            while (!isEOF() && isWhitespace(currChar)) {
                if (currChar == '\n') {
                    ++currLine;
                }

                advance();
            }
        }

        bool validateIdentifier(std::string identifier) {
            int start = currIdx;
            int len = 0;

            while (!isEOF() && len != identifier.size()) {
                ++len;
                advance();
            }

            return content.substr(start, len) == identifier;
        }

        void reportError(std::string msg) {
            std::cout << "[ERROR] At line " << currLine << ": " << msg << "\n";
            isOk = false;
        }

        bool consume(char c, std::string msg) {
            if (currChar != c) {
                reportError(msg);
                return false;
            }

            advance();
            return true;
        }

        Value parseElement() {
            switch (currChar) {
            case '{':
                return parseObject();
            case '[':
                return parseArray();
            case '"': {
                std::string str = parseString();
                return Value::makeString(str);
            }
            case 't':
                if (validateIdentifier("true")) {
                    return Value::makeBoolean(true);
                }
                break;
            case 'f':
                if (validateIdentifier("false")) {
                    return Value::makeBoolean(false);
                }
                break;
            case 'n':
                if (validateIdentifier("null")) {
                    return Value::makeNull();
                }
                break;
            default:
                if (currChar == '+' || currChar == '-' || std::isdigit(currChar)) {
                    return parseNumber();
                }

                if (isWhitespace(currChar)) {
                    skipWhitespace();
                    break;
                }

                break;
            }

            reportError(std::string("Unexpected character: '") + currChar + "'");
            return Value();
        }

        Value parseObject() {
            if (currChar == '{') {
                advance();
            }

            skipWhitespace();

            Value obj = Value::makeObject();

            while (!isEOF() && currChar != '}') {
                skipWhitespace();
                std::string name = parseString();
                skipWhitespace();

                if (currChar != ':') {
                    reportError("Expected ':' for value assignment");
                    break;
                }

                advance();
                skipWhitespace();

                Value element = parseElement();
                skipWhitespace();

                obj[name] = element;

                if (currChar != ',') {
                    break;
                }

                advance();
            }

            consume('}', "Expected '}' for object end");
            return obj;
        }

        Value parseArray() {
            if (currChar == '[') {
                advance();
            }

            skipWhitespace();

            std::vector<Value> arr;

            while (!isEOF() && currChar != ']') {
                skipWhitespace();
                Value element = parseElement();
                arr.push_back(element);
                skipWhitespace();

                if (currChar != ',') {
                    break;
                }

                advance();
            }

            consume(']', "Expected ']' for array end");
            return Value::makeArray(arr);
        }

        std::string parseString() {
            if (currChar == '"') {
                advance();
            }

            int start = currIdx;
            int len = 0;
            
            while (!isEOF() && currChar != '"') {
                ++len;
                advance();
            }

            consume('"', "Expected '\"' for string end");
            return content.substr(start, len);
        }

        Value parseNumber() {
            int start = currIdx;
            int len = 0;

            bool floatingPoint = false;
            bool exponent = false;

            while (!isEOF() && (std::isdigit(currChar) || currChar == '+' || currChar == '-' || currChar == '.' || currChar == 'e')) {
                if (currChar == '.') {
                    if (floatingPoint) {
                        reportError("invalid number: duplicate floating point");
                        break;
                    }
                    else {
                        floatingPoint = true;
                    }
                }

                if (currChar == 'e') {
                    if (exponent) {
                        reportError("invalid number: duplicate exponent symbol");
                        break;
                    }
                    else {
                        exponent = true;
                    }
                }

                ++len;
                advance();
            }

            try {
                double num = std::stod(content.substr(start, len).c_str());
                return Value::makeNumber(num);
            }
            catch (const std::out_of_range&) {
                std::cout << "[WARNING] At line " << currLine << ": number overflow\n";
            }

            return Value::makeNumber(0);
        }

    public:
        Parser(std::string content) : content(content) {
            advance();
        }

        static Parser fromFile(std::string filename) {
            constexpr std::size_t readSize = 4096;

            std::fstream stream(filename);

            if (!stream.is_open()) {
                std::cout << "[ERROR] Failed to open file \"" << filename << "\"\n";
                Parser parser("");
                parser.isOk = false;
                return parser;
            }

            std::string out;
            std::string buf(readSize, '\0');

            while (stream.read(&buf[0], readSize)) {
                out.append(buf, 0, stream.gcount());
            }

            out.append(buf, 0, stream.gcount());
            return Parser(out);
        }

        Value parse() {
            isOk = true;
            return parseElement();
        }

        bool isOK() {
            return isOk;
        }
    };
}

#endif