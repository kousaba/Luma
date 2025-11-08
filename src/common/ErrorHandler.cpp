#include "ErrorHandler.h"
#include "ParserRuleContext.h"
#include "Token.h"
#include "common/ErrorDef.h"
#include <iostream>

ErrorHandler errorHandler;

void ErrorHandler::errorReg(std::string name, int error_class, antlr4::ParserRuleContext *ctx){
    Error error;
    error.name = name;
    error.errorClass = error_class;
    if(error_class == 0){
        errorCount++;
    }else if(error_class == 1){
        warnCount++;
    }else if(error_class == 2){
        infoCount++;
    }else if(error_class == -1){
        // コンパイラエラーのときの処理
    }else{
        errorReg("Unknown Error Happend in errorReg.", 0, ctx);
    }
    if(ctx){
        if(auto token = ctx->getStart()){
            error.userCodeLine = token->getLine();
            error.userCodeColumn = token->getCharPositionInLine();
            error.userCodeStr = token->getText();
        }
    }
    errors.push_back(Error{error_class, name});
}
void ErrorHandler::errorReg(ErrorCode code, const std::vector<std::string>& args, int line){
    auto it = errorMessages.find(code);
    if(it == errorMessages.end()){
        errorReg("Unknown error code.", -1);
        return;
    }
    std::string format;
    if(currentLang == Language::JA){
        format = it->second.ja;
    }else{
        format = it->second.en;
    }
    std::string message = formatErrorMessage(format, args);
    errorReg(message, 0);
}
void ErrorHandler::warnReg(WarnCode code, const std::vector<std::string>& args, int line){
    auto it = warnMessages.find(code);
    if(it == warnMessages.end()){
        errorReg("Unknown warn code.", -1);
        return;
    }
    std::string format;
    if(currentLang == Language::JA){
        format = it->second.ja;
    }else{
        format = it->second.en;
    }
    std::string message = formatErrorMessage(format, args);
    errorReg(message, 1);
}
void ErrorHandler::infoReg(InfoCode code, const std::vector<std::string>& args, int line){
    auto it = infoMessages.find(code);
    if(it == infoMessages.end()){
        errorReg("Unknown info code.", -1);
        return;
    }
    std::string format;
    if(currentLang == Language::JA){
        format = it->second.ja;
    }else{
        format = it->second.en;
    }
    std::string message = formatErrorMessage(format, args);
    errorReg(message, 2);
}
void ErrorHandler::compilerErrorReg(CompilerErrorCode code, const std::vector<std::string>& args, int line){
    auto it = compilerErrorMessages.find(code);
    if(it == compilerErrorMessages.end()){
        errorReg("Unknown compiler error code.", -1);
        return;
    }
    std::string format;
    if(currentLang == Language::JA){
        format = it->second.ja;
    }else{
        format = it->second.en;
    }
    std::string message = formatErrorMessage(format, args);
    errorReg(message, -1);
}

void ErrorHandler::conditionErrorReg(bool cond, std::string name, int error_class, antlr4::ParserRuleContext *ctx){
    if(cond) errorReg(name, error_class, ctx);
}

void ErrorHandler::printError(Error error){
    if(error.errorClass == 0) std::cout << "[Error]: ";
    else if(error.errorClass == 1) std::cout << "[Warn]: ";
    else if(error.errorClass == 2) std::cout << "[Info]: ";
    else if(error.errorClass == -1) std::cout << "[Compiler Error]: ";
    else{
        std::cout << "[Unknown Error]: \n\t";
    }
    std::cout << error.name << "\n";
    if(error.userCodeStr != "")std::cout << "\t\n" << "at Line: " << error.userCodeLine << " Col: " << error.userCodeColumn << " text: " << error.userCodeStr << "\n";
}

void ErrorHandler::printAllErrors(){
    for(auto error: errors){
        printError(error);
    }
}

bool ErrorHandler::hasError() {
    return errorCount > 0;
}

void ErrorHandler::setLang(Language lang){
    currentLang = lang;
}