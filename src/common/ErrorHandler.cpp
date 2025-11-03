#include "ErrorHandler.h"
#include "ParserRuleContext.h"
#include "Token.h"
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
void ErrorHandler::conditionErrorReg(bool cond, std::string name, int error_class, antlr4::ParserRuleContext *ctx){
    if(cond) errorReg(name, error_class, ctx);
}

void ErrorHandler::printError(Error error){
    if(error.errorClass == 0) std::cout << "[Error]: ";
    else if(error.errorClass == 1) std::cout << "[Warn]: ";
    else if(error.errorClass == 2) std::cout << "[Info]: ";
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