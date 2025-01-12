#include "SyntaxAnalysis.h"

void SyntaxAnalysis::parseLexer() {
    // 当前行分析结束，读取下一行
    if(nowLineIndex == nowLine.size()) {
        // 当读取到文件末尾后便不能继续读
        if(!std::getline(inputStream, nowLine)) {
            return;
        }
        // 行计数自增1，nowLineIndex变为0
        lineCnt++;
        nowLineIndex = 0;
    }

    std::string str;
    while(nowLineIndex < nowLine.size()) {
        char c = nowLine[nowLineIndex];
        // 当前字符为空格
        if(c == ' ') {
            if(canAddClassCode(str)) {
                str += c;
            }
            else if(!str.empty()){
                nextTokens.push_back(str);
                nowLineIndex++;
                return;
            }
        }
        // 当前字符为=、!、<、>  (此处用于特判 ==、!=、 <=、>=四种类别的单词)
        else if(c == '=' || c == '!' || c == '<' || c == '>') {
            // 当前字符串str能连接就直接连接上述四种字符(即在引号中存在上述字符)
            if(canAddClassCode(str)) {
                str += c;
            }
            else {
                // str不为空则其为一个token
                if (!str.empty()) {
                    nextTokens.push_back(str);
                    return;
                }
                // 下一个字符为=，则说明当前位置是上述的四种类别，那么将对应的token识别结果输出，并将下标+1以跳过该token
                if (nowLine[nowLineIndex + 1] == '=') {
                    nextTokens.push_back(std::string(1, c) + std::string(1, nowLine[nowLineIndex + 1]));
                    nowLineIndex += 2;
                    return;
                }
                // 下一个字符不为=，则说明当前位置不是上述四种类别，则直接将本字符识别为token
                else {
                    nextTokens.emplace_back(1, c);
                    nowLineIndex++;
                    return;
                }
            }
        }
        // 当前字符有直接对应的类别码
        else if(CLASS_CODE.count(std::string(1, c)) != 0) {
            // 当前字符串str能连接就直接连接当前字符
            if(canAddClassCode(str)) {
                str += c;
            }
            else {
                // str不为空则为一个token
                if(!str.empty()) {
                    nextTokens.push_back(str);
                    return;
                }
                // 当前字符为一个token
                nextTokens.emplace_back(1, c);
                nowLineIndex++;
                return;
            }
        }
        // 当前字符时普通字符，则直接连在str后面
        else {
            str += c;
        }
        nowLineIndex++;
    }
}

void SyntaxAnalysis::getToken() {
    // 初始时nextTokens为空，由于我们需单独缓存两个token，所以此时先读取两个token存于nextTokens中
    if(nextTokens.empty()) {
        parseLexer();
        parseLexer();
    }
    // 缓存存在token
    if(!nextTokens.empty()){
        // 移除一个token至lookahead，并将该token从缓存中移除
        lookahead = nextTokens[0];
        nextTokens.erase(nextTokens.begin());
        // 缓存一个token
        parseLexer();
    }
}

void SyntaxAnalysis::matchToken(const std::string& expected) {
    // 当匹配不成输出错误
    if(lookahead != expected) {
        Error();
    }
    // 匹配成功则调用父类LexerAnalysis的outputCharacter函数将该token对应的类别码输出到文件中
    else {
        outputCharacter(outputStream, lookahead);
        getToken();
    }
}

int SyntaxAnalysis::checkCommonCharacter(std::string& target) {
    // 如果首字符是数字，则说明该字符串为整形常量
    if(target.front() <= '9' && target.front() >= '0') {
        return INT_CONST;
    }
    // 如果首尾字符均为单引号，则说明是字符常量，删除首位引号（形参为引用类型）
    else if(target.front() == '\'' && target.back() == '\''){
        return CHAR_CONST;
    }
    // 如果首尾字符均为双引号，则说明是字符串（形参为引用类型）
    else if(target.front() == '\"' && target.back() == '\"'){
        return STRING_CONST;
    }
    // 否则便是标识符
    else if(CLASS_CODE.find(target) == CLASS_CODE.end()){
        return IDENTIFIER;
    }
    else {
        return NOT_COMMON_CHARACTER;
    }
}

// 有待优化error output
void SyntaxAnalysis::Error() const {
    // 输出出错行的内容
    std::cerr << "At line " << lineCnt << ": \n";
    for(int i = 0; i < nowLineIndex; i++) {
        std::cerr << nowLine[i];
    }
    exit(0);
}

void SyntaxAnalysis::parseProgram() {
    // 当前token为"const"，则分析<常量说明>  例如: const int const1 = 1, const2 = -100;
    if(lookahead == "const") {
        parseDeclConst();
    }
    // 当前token为"char"或"int"且下下个token不为"("，则分析<变量说明>  例如: int change1;
    if((lookahead == "char" || lookahead == "int") && nextTokens[1] != "(") {
        parseDeclVar();
    }
    // 当前token为"char"或"int"或"void"且下个token不为"main"，则分析＜有返回值函数定义＞或＜无返回值函数定义＞
    // 这里也需要下下个token为"("，但由于上面变量分析时已经进行判断，所以这里无需再判断
    while(lookahead == "char" || lookahead == "int" || lookahead == "void" && nextTokens[0] != "main") {
        // 当前token为"int"或"char"，则分析＜有返回值函数定义＞
        if(lookahead == "int" || lookahead == "char") {
            parseDefiFunc();
        }
        // 当前token为"void"，则分析＜无返回值函数定义＞
        else if(lookahead == "void"){
            parseDefiVoidFunc();
        }
        // 输入程序的词法有误，输出错误
        else {
            Error();
        }
    }
    // 当前token为"void"且下一个token为"main"。则分析<主函数>
    if(lookahead == "void" && nextTokens[0] == "main") {
        parseMain();
    }
    // 输出当前语法成分到output.txt文件，即<程序>
    outputStream << NONE_TERMINAL["PROGRAM"] << std::endl;
}

void SyntaxAnalysis::parseDeclConst() {
    if(lookahead == "const") {
        matchToken("const");
        parseDefiConst();
        matchToken(";");
        while(lookahead == "const") {
            matchToken("const");
            parseDefiConst();
            matchToken(";");
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DECLCONST"] << std::endl;
}

void SyntaxAnalysis::parseDefiConst() {
    if(lookahead == "int") {
        matchToken("int");
        parseIdenfr();
        matchToken("=");
        parseInt();
        while(lookahead == ",") {
            matchToken(",");
            parseIdenfr();
            matchToken("=");
            parseInt();
        }
    }
    else if(lookahead == "char"){
        matchToken("char");
        parseIdenfr();
        matchToken("=");
        parseChar();
        while(lookahead == ",") {
            matchToken(",");
            parseIdenfr();
            matchToken("=");
            parseChar();
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DEFICONST"] << std::endl;
}

void SyntaxAnalysis::parseDeclVar() {
    if(lookahead == "int" || lookahead == "char") {
        parseDefiVar();
        matchToken(";");
        while((lookahead == "int" || lookahead == "char") && nextTokens[1] != "(") {
            parseDefiVar();
            matchToken(";");
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DECLVAR"] << std::endl;
}

void SyntaxAnalysis::parseDefiVar() {
    if(lookahead == "int" || lookahead == "char") {
        parseTypeIdenfr();
        parseIdenfr();
        if(lookahead == "[") {
            matchToken("[");
            parseUInt();
            matchToken("]");
        }
        while(lookahead == ",") {
            matchToken(",");
            parseIdenfr();
            if(lookahead == "[") {
                matchToken("[");
                parseUInt();
                matchToken("]");
            }
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DEFIVAR"] << std::endl;
}

void SyntaxAnalysis::parseDefiFunc() {
    if(lookahead == "int" || lookahead == "char") {
        parseDeclHead();
        matchToken("(");
        parseParams();
        matchToken(")");
        matchToken("{");
        parseMultStatements();
        matchToken("}");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DEFIFUNC"] << std::endl;
}

void SyntaxAnalysis::parseDefiVoidFunc() {
    if(lookahead == "void") {
        matchToken("void");
        functions[lookahead] = VOID_FUNCTION;
        parseIdenfr();
        matchToken("(");
        parseParams();
        matchToken(")");
        matchToken("{");
        parseMultStatements();
        matchToken("}");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DEFIVOIDFUNC"] << std::endl;
}

void SyntaxAnalysis::parseMain() {
    if(lookahead == "void") {
        matchToken("void");
        matchToken("main");
        matchToken("(");
        matchToken(")");
        matchToken("{");
        parseMultStatements();
        matchToken("}");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["MAIN"] << std::endl;
}

void SyntaxAnalysis::parseDeclHead() {
    if(lookahead == "int" || lookahead == "char") {
        parseTypeIdenfr();
        parseIdenfr();
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["DECLHEAD"] << std::endl;
}

void SyntaxAnalysis::parseParams() {
    if(lookahead == "int" || lookahead == "char") {
        parseTypeIdenfr();
        parseIdenfr();
        while(lookahead == ",") {
            matchToken(",");
            parseTypeIdenfr();
            parseIdenfr();
        }
    }
    else if(lookahead == ")") {
        outputStream << NONE_TERMINAL["PARAMS"] << std::endl;
        return;
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["PARAMS"] << std::endl;
}

void SyntaxAnalysis::parseMultStatements() {
    if(lookahead == "const") {
        parseDeclConst();
    }
    if(lookahead == "char" || lookahead == "int") {
        parseDeclVar();
    }
    if(lookahead == "if" || lookahead == "while"
       || lookahead == "do" || lookahead == "for"
       || lookahead == "{" || lookahead == "scanf" || lookahead == "printf"
       || lookahead == "return" || isLetter(lookahead)) {
        parseStatementCol();
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["MULTSTATEMENTS"] << std::endl;
}

void SyntaxAnalysis::parseIdenfr() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        matchToken(lookahead);
    }
    else {
        Error();
    }
//    if(isLetter(lookahead)) {
//        parseLetter();
//        while(isLetter(lookahead) || isNumber(lookahead)) {
//            if(isLetter(lookahead)) {
//                parseLetter();
//            }
//            if(isNumber(lookahead)) {
//                parseNum();
//            }
//        }
//    }
//    else {
//        Error();
//    }
}

void SyntaxAnalysis::parseUInt() {
    if(isUnsignedInt(lookahead)) {
        matchToken(lookahead);
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["UINT"] << std::endl;
//    if(lookahead == "0") {
//        matchToken("0");
//    }
//    else if(isNumber(lookahead)) {
//        parseNZNum();
//        while(isNumber(lookahead)) {
//            parseNum();
//        }
//    }
//    else {
//        Error();
//    }
}

void SyntaxAnalysis::parseInt() {
    if(lookahead == "+") {
        matchToken("+");
    }
    if(lookahead == "-") {
        matchToken("-");
    }
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == INT_CONST) {
        parseUInt();
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["INT"] << std::endl;
}

void SyntaxAnalysis::parseOPlus() {
    if(lookahead == "+") {
        matchToken("+");
    }
    else if(lookahead == "-") {
        matchToken("-");
    }
    else {
        Error();
    }
}

void SyntaxAnalysis::parseOMult() {
    if(lookahead == "*") {
        matchToken("*");
    }
    else if(lookahead == "/") {
        matchToken("/");
    }
    else {
        Error();
    }
}

void SyntaxAnalysis::parseORela() {
    if(lookahead == "<") {
        matchToken("<");
    }
    else if(lookahead == "<=") {
        matchToken("<=");
    }
    else if(lookahead == ">") {
        matchToken(">");
    }
    else if(lookahead ==">=") {
        matchToken(">=");
    }
    else if(lookahead == "!=") {
        matchToken("!=");
    }
    else if(lookahead =="==") {
        matchToken("==");
    }
    else {
        Error();
    }
}

[[maybe_unused]] void SyntaxAnalysis::parseLetter() {
    if(isLetter(lookahead)) {
        matchToken(lookahead);
    }
    else {
        Error();
    }
}

[[maybe_unused]] void SyntaxAnalysis::parseNum() {
    if(lookahead == "0") {
        matchToken("0");
    }
    else if(isNotZeroNumber(lookahead)){
        parseNZNum();
    }
    else {
        Error();
    }
}

[[maybe_unused]] void SyntaxAnalysis::parseNZNum() {
    if(isNotZeroNumber(lookahead)) {
        matchToken(lookahead);
    }
    else {
        Error();
    }
}

void SyntaxAnalysis::parseChar() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == CHAR_CONST) {
        matchToken(lookahead);
    }
    else {
        Error();
    }
//    if(lookahead == "'") {
//        matchToken("'");
//        if(lookahead == "+" || lookahead == "-") {
//            parseOPlus();
//        }
//        else if(lookahead == "*" || lookahead == "/") {
//            parseOMult();
//        }
//        else if(isLetter(lookahead)) {
//            parseLetter();
//        }
//        else if(isNumber(lookahead)) {
//            parseNum();
//        }
//        else {
//            Error();
//        }
//        matchToken("'");
//    }
//    else {
//        Error();
//    }
}

void SyntaxAnalysis::parseString() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == STRING_CONST) {
        matchToken(lookahead);
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["STRING"] << std::endl;
//    if(lookahead == "\"") {
//        matchToken("\"");
//        for(auto c: lookahead) {
//            if(!(c == 32 || c == 33 || c >= 35 && c <= 126)) {
//                Error();
//            }
//        }
//        matchToken(lookahead);
//        matchToken("\"");
//    }
//    else {
//        Error();
//    }
}

void SyntaxAnalysis::parseTypeIdenfr() {
    if(lookahead == "int") {
        matchToken("int");
    }
    else if(lookahead == "char") {
        matchToken("char");
    }
    else {
        Error();
    }
}

void SyntaxAnalysis::parseExpression() {
    if(lookahead == "+") {
        matchToken("+");
    }
    else if(lookahead == "-") {
        matchToken("-");
    }
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER
    || lookahead == "(" || lookahead == "+" || lookahead == "-"
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == CHAR_CONST
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == INT_CONST) {
        parseItem();
        while(lookahead == "+" || lookahead == "-") {
            parseOPlus();
            parseItem();
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["EXPRESSION"] << std::endl;
}

void SyntaxAnalysis::parseItem() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER
    || lookahead == "(" || lookahead == "+" || lookahead == "-"
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == CHAR_CONST
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == INT_CONST) {
        parseFactor();
        while(lookahead == "*" || lookahead == "/") {
            parseOMult();
            parseFactor();
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["ITEM"] << std::endl;
}

void SyntaxAnalysis::parseFactor() {
    if(lookahead == "(") {
        matchToken("(");
        parseExpression();
        matchToken(")");
    }
    else if(lookahead == "+" || lookahead == "-" || SyntaxAnalysis::checkCommonCharacter(lookahead) == INT_CONST) {
        parseInt();
    }
    else if(SyntaxAnalysis::checkCommonCharacter(lookahead) == CHAR_CONST) {
        parseChar();
    }
    else if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        if(nextTokens[0] == "(") {
            parseCallFuncS();
        }
        else {
            parseIdenfr();
            if(lookahead == "[") {
                matchToken("[");
                parseExpression();
                matchToken("]");
            }
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["FACTOR"] << std::endl;
}

void SyntaxAnalysis::parseStatements() {
    if(lookahead == "if") {
        parseConditionS();
    }
    else if(lookahead == "while" || lookahead == "do" || lookahead == "for") {
        parseLoopS();
    }
    else if(lookahead == "{") {
        matchToken("{");
        parseStatementCol();
        matchToken("}");
    }
    else if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        if(nextTokens[0] == "=" || nextTokens[0] == "[") {
            parseAssignS();
            matchToken(";");
        }
        else if(functions.find(lookahead) != functions.end()
        && functions[lookahead] == FUNCTION) {
            parseCallFuncS();
            matchToken(";");
        }
        else if(functions.find(lookahead) != functions.end()
        && functions[lookahead] == VOID_FUNCTION) {
            parseCallVoidFuncS();
            matchToken(";");
        }
    }
    else if(lookahead == "scanf") {
        parseReadS();
        matchToken(";");
    }
    else if(lookahead == "printf") {
        parseWriteS();
        matchToken(";");
    }
    else if(lookahead == "return") {
        parseReturnS();
        matchToken(";");
    }
    else if(lookahead == ";"){
        matchToken(";");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["STATEMENTS"] << std::endl;
}

void SyntaxAnalysis::parseAssignS() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        parseIdenfr();
        if(lookahead == "[") {
            matchToken("[");
            parseExpression();
            matchToken("]");
        }
        matchToken("=");
        parseExpression();
    }
    outputStream << NONE_TERMINAL["ASSIGNS"] << std::endl;
}

void SyntaxAnalysis::parseConditionS() {
    if(lookahead == "if") {
        matchToken("if");
        matchToken("(");
        parseCondition();
        matchToken(")");
        parseStatements();
        if(lookahead == "else") {
            matchToken("else");
            parseStatements();
        }
    }
    outputStream << NONE_TERMINAL["CONDITIONS"] << std::endl;
}

void SyntaxAnalysis::parseCondition() {
    if(lookahead == "+" || lookahead == "-" || SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        parseExpression();
        // <条件> ::= <表达式><关系运算符><表达式>
        if(isRelationOperator(lookahead)) {
            parseORela();
            parseExpression();
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["CONDITION"] << std::endl;
}

void SyntaxAnalysis::parseLoopS() {
    if(lookahead == "while") {
            matchToken("while");
            matchToken("(");
            parseCondition();
            matchToken(")");
            parseStatements();
    }
    else if(lookahead == "do") {
            matchToken("do");
            parseStatements();
            matchToken("while");
            matchToken("(");
            parseCondition();
            matchToken(")");
    }
    else if(lookahead == "for") {
        matchToken("for");
        matchToken("(");
        parseIdenfr();
        matchToken("=");
        parseExpression();
        matchToken(";");
        parseCondition();
        matchToken(";");
        parseIdenfr();
        matchToken("=");
        parseIdenfr();
        if (lookahead == "+" || lookahead == "-") {
            matchToken(lookahead);
        }
        parseStepSize();
        matchToken(")");
        parseStatements();
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["LOOPS"] << std::endl;
}

void SyntaxAnalysis::parseStepSize() {
    if(isUnsignedInt(lookahead)) {
        parseUInt();
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["STEPSIZE"] << std::endl;
}

void SyntaxAnalysis::parseCallFuncS() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        parseIdenfr();
        matchToken("(");
        parseParamsV();
        matchToken(")");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["CALLFUNCS"] << std::endl;
}

void SyntaxAnalysis::parseCallVoidFuncS() {
    if(SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER) {
        parseIdenfr();
        matchToken("(");
        parseParamsV();
        matchToken(")");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["CALLVOIDFUNCS"] << std::endl;
}

void SyntaxAnalysis::parseParamsV() {
    if(lookahead == "+" || lookahead == "-"
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER
    || lookahead == "(" || lookahead == "+" || lookahead == "-"
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == CHAR_CONST
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == INT_CONST) {
        parseExpression();
        while(lookahead == ",") {
            matchToken(",");
            parseExpression();
        }
    }
    else if(lookahead == ")"){
        outputStream << NONE_TERMINAL["PARAMSV"] << std::endl;
        return;
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["PARAMSV"] << std::endl;
}

void SyntaxAnalysis::parseStatementCol() {
    while(lookahead == "if" || lookahead == "while"
    || lookahead == "do" || lookahead == "for" || lookahead == "{"
    || SyntaxAnalysis::checkCommonCharacter(lookahead) == IDENTIFIER
    || lookahead == "scanf" || lookahead == "printf"
    || lookahead == "return" || lookahead == ";") {
        parseStatements();
    }
    outputStream << NONE_TERMINAL["STATEMENTCOL"] << std::endl;
}

void SyntaxAnalysis::parseReadS() {
    if(lookahead == "scanf") {
        matchToken("scanf");
        matchToken("(");
        parseIdenfr();
        while(lookahead == ",") {
            matchToken(",");
            parseIdenfr();
        }
        matchToken(")");
    }
    outputStream << NONE_TERMINAL["READS"] << std::endl;
}

void SyntaxAnalysis::parseWriteS() {
    if(lookahead == "printf") {
        matchToken("printf");
        matchToken("(");
        if(checkCommonCharacter(lookahead) == STRING_CONST) {
            parseString();
            if(lookahead == ",") {
                matchToken(",");
                parseExpression();
            }
        }
        else {
            parseExpression();
        }
        matchToken(")");
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["WRITES"] << std::endl;
}

void SyntaxAnalysis::parseReturnS() {
    if(lookahead == "return") {
        matchToken("return");
        if(lookahead == "(") {
            matchToken("(");
            parseExpression();
            matchToken(")");
        }
    }
    else {
        Error();
    }
    outputStream << NONE_TERMINAL["RETURNS"] << std::endl;
}

/** 解决冲突 （为了能通过实验系统的测试，现已废弃）
 *          <变量说明>             <有返回值函数定义>
 *            /  \                  /        \
 *           /    \                /          \
 *     <变量定义>   ;         <声明头部>         (
 *     /      \               /    \
 *    /        \             /      \
 * <类型标识符>  <标识符>   <类型标识符>  <标识符>
 *                     ||
 *                     ||
 *                    \||/
 *                     \/
 *          <变量说明或有返回值函数定义>
 *            /                 \
 *           /                   \
 *      <类型标识符>        <变量说明>|<有返回值函数定义>
 */
/*void SyntaxAnalysis::parseDeclvarOrDefifunc() {
    if(lookahead == "char" || lookahead == "int") {
        parseTypeIdenfr();
        if(nextToken == "("){
            parseDefiFunc();
        }
        else {
            parseDeclVar();
        }
    }
}*/

void SyntaxAnalysis::parse(int select, bool isCheckAnswer) {
    // 进行程词法分析
    if(select == LEXER) {
        LexerAnalysis::parseLexer();
    }
    // 进行语法分析
    else if(select == SYNTAX) {
        if (!inputStream) {
            std::cerr << "Can't open the file " << OUTPUT_FILE << "!\n";
        }
        // 现获取token
        getToken();
        // 开始分析程序
        parseProgram();
        // 调用父类函数核查答案
        if (isCheckAnswer) {
            LexerAnalysis::checkAnswer();
        }
    }
}
