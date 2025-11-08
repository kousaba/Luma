#pragma once
#include "ParserRuleContext.h"
#include "ErrorDef.h"
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
    Language currentLang = Language::EN;
public:
    void errorReg(ErrorCode code, const std::vector<std::string>& args, int line = 0);
    void warnReg(WarnCode code, const std::vector<std::string>& args, int line = 0);
    void infoReg(InfoCode code, const std::vector<std::string>& args, int line = 0);
    void compilerErrorReg(CompilerErrorCode code, const std::vector<std::string>& args, int line = 0);
    void errorReg(std::string name, int errorClass, antlr4::ParserRuleContext *ctx = nullptr);
    void conditionErrorReg(bool cond, std::string name, int errorClass, antlr4::ParserRuleContext *ctx = nullptr);
    void printError(Error error);
    void printAllErrors();
    bool hasError();
    void setLang(Language lang);
};

extern ErrorHandler errorHandler;