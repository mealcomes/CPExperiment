#ifndef SYNTAX_ANALYSIS_H
#define SYNTAX_ANALYSIS_H

#include <set>
#include <vector>
#include "LexerAnalysis.h"

#define VOID_FUNCTION 0
#define FUNCTION 1

#define LEXER 0
#define SYNTAX 1

class SyntaxAnalysis : public LexerAnalysis {
private:
    std::map<std::string, std::string> NONE_TERMINAL = {
            {"OPLUS","<加法运算法>"},{"OMULT","<乘法运算法>"},{"ORELA","<关系运算符>"},{"LETTER","<字母>"},
            {"NUM","<数字>"},{"NZNUM","<非零数字>"},{"CHAR","<字符>"}, {"STRING","<字符串>"},
            { "PROGRAM","<程序>" }, { "DECLCONST","<常量说明>" }, { "DEFICONST","<常量定义>" },
            { "UINT","<无符号整数>" }, { "INT","<整数>" },
            { "IDENFR", "<标识符>" }, { "DECLHEAD", "<声明头部>" }, { "DECLVAR", "<变量说明>" },
            { "DEFIVAR", "<变量定义>" },{ "TYPEIDENFR", "<类型标识符>" }, { "DEFIFUNC", "<有返回值函数定义>" },
            { "DEFIVOIDFUNC", "<无返回值函数定义>" },{ "MULTSTATEMENTS", "<复合语句>" }, { "PARAMS", "<参数表>" },
            { "MAIN", "<主函数>" }, { "EXPRESSION", "<表达式>" }, { "ITEM", "<项>" },{"FACTOR","<因子>"},
            {"STATEMENTS","<语句>"},{"ASSIGNS","<赋值语句>"},{"CONDITIONS","<条件语句>"},{"CONDITION","<条件>"},
            {"LOOPS","<循环语句>"},{"STEPSIZE","<步长>"},{"CALLFUNCS","<有返回值函数调用语句>"},
            {"CALLVOIDFUNCS","<无返回值函数调用语句>"},{"PARAMSV","<值参数表>"},{"STATEMENTCOL","<语句列>"},
            {"READS","<读语句>"},{"WRITES","<写语句>"},{"RETURNS","<返回语句>"},
    };

    std::ifstream inputStream;
    std::ofstream outputStream;
    std::string nowLine{};
    int lineCnt{};
    std::string lookahead{}, nextToken{};
    std::vector<std::string> nextTokens;
    int nowLineIndex{};
    std::map<std::string, int> functions;

    void parseLexer();
    void getToken();
    void matchToken(const std::string& expected);
    int checkCommonCharacter(std::string& target) override;
    void Error() const;

    [[maybe_unused]] void parseLetter();
    [[maybe_unused]] void parseNum();
//    void parseDeclvarOrDefifunc();   // 变量说明或有返回值函数定义（用于解决冲突）

    void parseOPlus();
    void parseOMult();
    void parseORela();
    void parseNZNum();
    void parseChar();
    void parseString();
    void parseProgram();
    void parseDeclConst();
    void parseDefiConst();
    void parseUInt();
    void parseInt();
    void parseIdenfr();
    void parseDeclHead();
    void parseDeclVar();
    void parseDefiVar();
    void parseTypeIdenfr();
    void parseDefiFunc();
    void parseDefiVoidFunc();
    void parseMultStatements();
    void parseParams();
    void parseMain();
    void parseExpression();
    void parseItem();
    void parseFactor();
    void parseStatements();
    void parseAssignS();
    void parseConditionS();
    void parseCondition();
    void parseLoopS();
    void parseStepSize();
    void parseCallFuncS();
    void parseCallVoidFuncS();
    void parseParamsV();
    void parseStatementCol();
    void parseReadS();
    void parseWriteS();
    void parseReturnS();

public:
    SyntaxAnalysis(std::string inFile, std::string outFile, std::string ansFile = ""):
    LexerAnalysis(std::move(inFile), std::move(outFile), std::move(ansFile)) {
        inputStream.open(INPUT_FILE);
        outputStream.open(OUTPUT_FILE);
        if(!outputStream) {
            std::cerr << "Can't open the file " << OUTPUT_FILE << "!\n";
        }
        if(!inputStream) {
            std::cerr << "Can't open the file " << INPUT_FILE << "!\n";
        }
        is_eof = false;
    }
    ~SyntaxAnalysis() {
        outputStream.close();
        inputStream.close();
    }

    void parse(int select, bool isCheckAnswer = false);
};

/**
 * 判断字符串是否为标识符
 */
inline bool isLetter(std::string str) {
    if(str.empty())
        return false;
    char s = str[0];
    if(s == '_' || s >= 'a' && s <= 'z' || s >= 'A' && s <= 'Z')
        return true;
    else
        return false;
}

inline bool isNotZeroNumber(std::string str) {
    if(str.empty())
        return false;
    char s = str[0];
    if(s >= '1' && s <= '9')
        return true;
    else
        return false;
}

inline bool isUnsignedInt(const std::string& str) {
    if(str.empty())
        return false;
    if(str[0] != '0' || str == "0")
        return true;
    return false;
}

inline bool isRelationOperator(const std::string& str) {
    if(str == "<" || str == "<=" || str == ">" || str ==">=" || str == "!=" || str =="==") {
        return true;
    }
    else {
        return false;
    }
}

#endif
