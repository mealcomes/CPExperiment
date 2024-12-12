#include <iostream>
#include "Class/SyntaxAnalysis.h"


#define TEST_FILE "testfile.txt"
#define OUTPUT_FILE "output.txt"
#define ANSWER_FILE "answer.txt"

#define SELECT 1       // 0-parse lexer  1-parse syntax

int main() {
    SyntaxAnalysis syntaxAnalysis(TEST_FILE, OUTPUT_FILE, ANSWER_FILE);
    syntaxAnalysis.parse(SELECT);

    return 0;
}
