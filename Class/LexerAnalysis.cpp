#include "LexerAnalysis.h"


int LexerAnalysis::checkCommonCharacter(std::string &target) {
    // 如果首字符是数字，则说明该字符串为整形常量
    if(target.front() <= '9' && target.front() >= '0') {
        return INT_CONST;
    }
        // 如果首尾字符均为单引号，则说明是字符常量，删除首位引号（形参为引用类型）
    else if(target.front() == '\'' && target.back() == '\''){
        target.erase(target.begin());\
        target.pop_back();
        return CHAR_CONST;
    }
        // 如果首尾字符均为双引号，则说明是字符串（形参为引用类型）
    else if(target.front() == '\"' && target.back() == '\"'){
        target.erase(target.begin());
        target.pop_back();
        return STRING_CONST;
    }
        // 否则便是标识符
    else {
        return IDENTIFIER;
    }
}

void LexerAnalysis::checkAnswer() {
    if(ANSWER_FILE.empty()) {
        std::cerr << "Can't open the file " << OUTPUT_FILE << "!\n";
        return;
    }
    // 打开文件资源，分别为程序输出结果文件和正确答案文件
    std::ifstream of(OUTPUT_FILE);
    std::ifstream af(ANSWER_FILE);
    std::string my, answer;

    int lineCnt = 0;  // 行数计数
    // 逐行对比结果与答案
    while (std::getline(of, my)) {
        lineCnt++;
        std::getline(af, answer);
        // 结果与答案不同，输出行数和对比结果，终止函数
        if(my != answer) {
            std::cerr << "At lineCnt " << lineCnt << ":" << std::endl;
            std::cerr << "Your: " << my << std::endl;
            std::cerr << "Answer: " << answer << std::endl;
            return;
        }
    }

    // 关闭文件资源
    af.close();
    of.close();
    // 答案正确！
    std::cout << "Answer Right!";
}

void LexerAnalysis::outputCharacter(std::ofstream &of, const std::string &ch) {
    // 字符串为空则直接返回
    if(ch.empty())
        return;
    // 该字符串有直接对应的类别码(比如运算符等)
    if (CLASS_CODE.count(ch) != 0) {
#ifdef STD_COUT
        std::cout << CLASS_CODE[ch] << ' ' << ch << std::endl;
#endif
        of << CLASS_CODE[ch] << ' ' << ch << std::endl;
    }
    // 该字符串没有直接对应的类别码，则需要在“整形常量”“字符常量”“字符串”“标识符”四种类别中进行判断
    else {
        std::string character_copy = ch;
        int res = LexerAnalysis::checkCommonCharacter(character_copy);
#ifdef STD_COUT
        std::cout << COMMON[res] << ' ' << character_copy << std::endl;
#endif
        of << COMMON[res] << ' ' << character_copy << std::endl;
    }
}

bool LexerAnalysis::canAddClassCode(std::string ch) {
    // 如果字符串为空，则不能连接
    if(ch.empty()) {
        return false;
    }
    else {
        // 如果字符串第一个字符不为引号，则不能在后面连接
        if(ch.front() != '\'' && ch.front() != '\"') {
            return false;
        }
        else {
            // 字符串第一个字符为引号，且字符串长度为1，能连接
            if(ch.size() == 1) {
                return true;
            }
            else {
                // 字符串长度不为1，且最后一个字符为引号，不能连接
                if(ch.back() == '\'' || ch.back() == '\"') {
                    return false;
                }
                    // 字符串长度不为1，且最后一个字符不为引号，能连接
                else {
                    return true;
                }
            }
        }
    }
}

void LexerAnalysis::parseLexer(bool isCheckAnswer) {
    if(INPUT_FILE.empty() || OUTPUT_FILE.empty()) {
        return;
    }
    // 打开文件资源
    std::ifstream tf(INPUT_FILE);
    std::ofstream of(OUTPUT_FILE);
    if (!tf || !of) {
        return;
    }

    std::string line;
    while (std::getline(tf, line)) {
        std::string str;
        for(int i = 0; i < line.size(); i++) {
            char c = line[i];
            // 当前字符为空格
            if(c == ' ') {
                if(canAddClassCode(str)) {
                    str += c;
                }
                else if(!str.empty()){
                    outputCharacter(of, str);
                    str.clear();
                }
            }
            // 当前字符为=、!、<、>  (此处用于特判 ==、!=、 <=、>=四种类别的单词)
            else if(c == '=' || c == '!' || c == '<' || c == '>') {
                // 当前字符串str能连接就直接连接上述四种字符(即在引号中存在上述字符)
                if(canAddClassCode(str)) {
                    str += c;
                }
                else {
                    // 否则不能连接，则先把当前str的类别识别、输出并清空str
                    if (!str.empty()) {
                        outputCharacter(of, str);
                        str.clear();
                    }
                    // 下一个字符为=，则说明当前位置是上述的四种类别，那么将对应的单词识别结果输出，并将i+1以跳过该单词
                    if (line[i + 1] == '=') {
                        outputCharacter(of, std::string(1, c) + std::string(1, line[i + 1]));
                        i++;
                    }
                        // 下一个字符不为=，则说明当前位置不是上述四种类别，则直接将本字符识别结果输出
                    else {
                        outputCharacter(of, std::string(1, c));
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
                    // 先识别、输出并清空str
                    if(!str.empty()) {
                        outputCharacter(of, str);
                        str.clear();
                    }
                    // 输出当前字符识别结果
                    outputCharacter(of, std::string(1, c));
                }
            }
            // 当前字符时普通字符，则直接连在str后面
            else {
                str += c;
            }
        }
    }
    // 答案检查
    if(isCheckAnswer)
        checkAnswer();

    // 关闭文件资源
    of.close();
    tf.close();
}
