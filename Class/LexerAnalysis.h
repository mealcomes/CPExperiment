#ifndef LEXER_ANALYSIS_H
#define LEXER_ANALYSIS_H

#include <string>
#include <map>
#include <iostream>
#include <ostream>
#include <fstream>
#include <utility>

#define IDENTIFIER 1    // 标识符
#define INT_CONST 2     // 整形常量
#define CHAR_CONST 3    // 字符常量
#define STRING_CONST 4  // 字符串
#define NOT_COMMON_CHARACTER 0


class LexerAnalysis {
protected:
    std::map<std::string, std::string> CLASS_CODE = {
            {"const", "CONSTTK"},{"int", "INTTK"},{"char", "CHARTK"},{"void", "VOIDTK"},
            {"main", "MAINTK"},{"if", "IFTK"},{"else", "ELSETK"},{"do", "DOTK"},
            {"while", "WHILETK"},{"for", "FORTK"},{"scanf", "SCANFTK"},{"printf", "PRINTFTK"},
            {"return", "RETURNTK"},{"+", "PLUS"},{"-", "MINU"},{"*", "MULT"},
            {"/", "DIV"},{"<", "LSS"},{"<=", "LEQ"},{">", "GRE"},
            {">=", "GEQ"},{"==", "EQL"},{"!=", "NEQ"},{"=", "ASSIGN"},
            {";", "SEMICN"},{",", "COMMA"},{"(", "LPARENT"},{")", "RPARENT"},
            {"[", "LBRACK"},{"]", "RBRACK"},{"{", "LBRACE"},{"}", "RBRACE"}
    };
    std::map<int, std::string> COMMON = {
            {1, "IDENFR"},{2, "INTCON"},{3, "CHARCON"},{4, "STRCON"}
    };
    void outputCharacter(std::ofstream& of, const std::string& ch);
    std::string INPUT_FILE{};
    std::string OUTPUT_FILE{};
    std::string ANSWER_FILE{};

    void checkAnswer();
    virtual int checkCommonCharacter(std::string& target);
    bool canAddClassCode(std::string ch);

public:
    LexerAnalysis(std::string inFile, std::string outFile, std::string ansFile = "") {
        INPUT_FILE = std::move(inFile);
        OUTPUT_FILE = std::move(outFile);
        ANSWER_FILE = std::move(ansFile);
    }

    void parseLexer(bool isCheckAnswer = false);
    void setInputFile(std::string fileName) {
        INPUT_FILE = std::move(fileName);
    }
    void setOutputFile(std::string fileName) {
        OUTPUT_FILE = std::move(fileName);
    }
};
#endif
