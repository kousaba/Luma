#pragma once
#include "ParserRuleContext.h"
#include <string>
#include <vector>

struct Error{
    int errorClass;
    std::string name;
    size_t userCodeLine = 0;
    size_t userCodeColumn = 0;
    std::string userCodeStr;
};

class ErrorHandler{
private:
    std::vector<Error> errors;
    size_t errorCount, warnCount, infoCount;
public:
    void errorReg(std::string name, int errorClass, antlr4::ParserRuleContext *ctx = nullptr);
    void conditionErrorReg(bool cond, std::string name, int errorClass, antlr4::ParserRuleContext *ctx = nullptr);
    void printError(Error error);
    void printAllErrors();
};

extern ErrorHandler errorHandler;