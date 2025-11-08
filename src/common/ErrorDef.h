#pragma once
#include <string>
#include <vector>
#include <map>

enum class Language{
    EN,
    JA
};

enum class ErrorCode{
    VARDECL_SYMBOL_ALREADY_DEFINED,
    VARDECL_CANNOT_DECLARE_VOID,
    VARDECL_TYPE_NOT_DEFINED,
    VARDECL_INIT_TYPE_MISMATCH,
    VARDECL_NO_TYPE_AND_INIT,
    VARDECL_CANNOT_DETERMINE_TYPE,
    BINARYOP_OPERAND_MISMATCH,
    FUNCCALL_NOT_DEFINED,
    FUNCCALL_NOT_FUNC_CALL,
    FUNCCALL_ARG_SIZE_MISMATCH,
    FUNCCALL_ARG_TYPE_MISMATCH,
    ASSIGNMENT_NOT_DEFINED,
    ASSIGNMENT_NOT_VARIABLE,
    ASSIGNMENT_TYPE_MISMATCH,
    IF_NOT_BOOL,
    FOR_NOT_BOOL,
    LEAVESCOPE_WITH_EMPTY_SYMBOLTABLE,
    ADDSYMBOL_ALREADY_CONTAINS,
    VARREF_NOT_DEFINED,
    VARREF_NOT_VARIABLE,
    CAST_TO_NON_BASIC,
    CAST_INVALID_TYPE
};

struct ErrorTemplate{
    std::string en;
    std::string ja;
};

const std::map<ErrorCode, ErrorTemplate> errorMessages = {
    // VARDECL_SYMBOL_ALREADY_DEFIND
    {
        ErrorCode::VARDECL_SYMBOL_ALREADY_DEFINED,
        {
            "Symbol '%0 is already defined in this scope.",
            "シンボル '%0' はこのスコープですでに定義されています。"
        }
    },
    // VARDECL_CANNOT_DECLARE_VOID
    {
        ErrorCode::VARDECL_CANNOT_DECLARE_VOID,
        {
            "Cannot declare variable '%0' of type 'void'.",
            "'void' 型の変数 '%0' を宣言することはできません。"
        }
    },
    // VARDECL_TYPE_NOT_DEFINED
    {
        ErrorCode::VARDECL_TYPE_NOT_DEFINED,
        {
            "Type '%0' used for variable '%1' is not defined.",
            "変数 '%1' に使われている型 '%0' は定義されていません。"
        }
    },
    // VARDECL_INIT_TYPE_MISMATCH
    {
        ErrorCode::VARDECL_INIT_TYPE_MISMATCH,
        {
            "Cannot initialize variable '%0' (type: %1) with a value of type '%2.",
            "変数 '%0' (%1 型) を %2 型の値で初期化することはできません。"
        }
    },
    // VARDECL_NO_TYPE_AND_INIT
    {
        ErrorCode::VARDECL_NO_TYPE_AND_INIT,
        {
            "Variable '%0' is declared without an initialization expression or a type annotation.",
            "変数 '%0' が初期化式、型注釈のどちらもない状態で宣言されています。"
        }
    },
    // VARDECL_CANNOT_DETERMINE_TYPE
    {
        ErrorCode::VARDECL_CANNOT_DETERMINE_TYPE,
        {
            "Could not determine type for variable '%0'.",
            "変数 '%0' の型を判別できませんでした。"
        }
    },
    // BINARYOP_OPERAND_MISMATCH
    {
        ErrorCode::BINARYOP_OPERAND_MISMATCH,
        {
            "The '%0' operator cannot be used with different types ('%1', '%2').",
            "演算子 '%0' は異なる型('%1', '%2') で行うことはできません。"
        }
    },
    // FUNCCALL_NOT_DEFINED
    {
        ErrorCode::FUNCCALL_NOT_DEFINED,
        {
            "Function '%0' is not defined.",
            "関数 '%0' は定義されていません。"
        }
    },
    // FUNCCALL_NOT_FUNC_CALL
    {
        ErrorCode::FUNCCALL_NOT_FUNC_CALL,
        {
            "'%0' is not a callable function.",
            "'%0' は呼び出し可能な関数ではありません。"
        }
    },
    // FUNCCALL_ARG_SIZE_MISMATCH
    {
        ErrorCode::FUNCCALL_ARG_SIZE_MISMATCH,
        {
            "Function '%0' expects %1 arguments, but %2 were provided.",
            "関数 '%0' は %1 個の引数を期待していますが、%2 個が提供されました。"
        }
    },
    // FUNCCALL_ARG_TYPE_MISMATCH
    {
        ErrorCode::FUNCCALL_ARG_TYPE_MISMATCH,
        {
            "Argument %0 of function '%1' has an incorrect type. Expected '%2', but got '%3'.",
            "関数 '%1' の引数 %0 の型が不正です。'%2' 型が期待されましたが、'%3' 型が渡されました。"
        }
    },
    // ASSIGNMENT_NOT_DEFINED
    {
        ErrorCode::ASSIGNMENT_NOT_DEFINED,
        {
            "Symbol '%0' is not defined and cannot be assigned to.",
            "シンボル '%0' は定義されておらず、代入できません。"
        }
    },
    // ASSIGNMENT_NOT_VARIABLE
    {
        ErrorCode::ASSIGNMENT_NOT_VARIABLE,
        {
            "Cannot assign a value to '%0' because it is not a modifiable variable.",
            "'%0' は変更可能な変数ではないため、値を代入できません。"
        }
    },
    // ASSIGNMENT_TYPE_MISMATCH
    {
        ErrorCode::ASSIGNMENT_TYPE_MISMATCH,
        {
            "Cannot assign value of type '%0' to variable '%1' of type '%2'.",
            "'%2' 型の変数 '%1' に '%0' 型の値を代入することはできません。"
        }
    },
    // IF_NOT_BOOL
    {
        ErrorCode::IF_NOT_BOOL,
        {
            "The condition for 'if' statement must be of type 'bool', but got '%0'.",
            "'if' 文の条件は 'bool' 型である必要がありますが、'%0' 型が渡されました。"
        }
    },
    // FOR_NOT_BOOL
    {
        ErrorCode::FOR_NOT_BOOL,
        {
            "The condition in 'for' loop must be of type 'bool', but got '%0'.",
            "'for' ループの条件は 'bool' 型である必要がありますが、'%0' 型が渡されました。"
        }
    },
    // ADDSYMBOL_ALREADY_CONTAINS
    {
        ErrorCode::ADDSYMBOL_ALREADY_CONTAINS,
        {
            "A symbol with the same name already exists in one scope.",
            "同じ名前のシンボルが既に一つのスコープ内に存在します。"
        }
    },
    // VARREF_NOT_DEFINED
    {
        ErrorCode::VARREF_NOT_DEFINED,
        {
            "Undeclared variable '%0'.",
            "未定義の変数 '%0' です。"
        }
    },
    // VARREF_NOT_VARIABLE
    {
        ErrorCode::VARREF_NOT_VARIABLE,
        {
            "'%0' is not a variable.",
            "'%0' は変数ではありません。"
        }
    },
    // CAST_TO_NON_BASIC
    {
        ErrorCode::CAST_TO_NON_BASIC,
        {
            "Non-basic types cannot be cast.",
            "基本型でない型へのキャストはできません。"
        }
    },
    // CAST_INVALID_TYPE
    {
        ErrorCode::CAST_INVALID_TYPE,
        {
            "Type is not valid when using CastNode.",
            "キャストで指定された型は無効です。"
        }
    }
};

/*
// ErrorCode
{
    ErrorCode,
    {
        "EN_ERROR_MSG",
        "JA_ERROR_MSG"
    }
},
*/

enum class WarnCode{
    EXPR_STMT_NO_EXPR
};

const std::map<WarnCode, ErrorTemplate> warnMessages = {
    {
        WarnCode::EXPR_STMT_NO_EXPR,
        {
            "There is no expression in the expression statement.",
            "式文に式がありません。"
        }
    }
};

enum class InfoCode{

};

const std::map<InfoCode, ErrorTemplate> infoMessages = {
    
};

enum class CompilerErrorCode{
    LEAVESCOPE_WITH_EMPTY_SYMBOLTABLE,
    ADDSYMBOL_WITH_NO_SCOPE,
    EXPR_VISIT_COULDNOT_CAST,
    STMT_VISIT_COULDNOT_CAST, // ★追加
    CAST_NODE_TYPE_NULL // ★追加
};

const std::map<CompilerErrorCode, ErrorTemplate> compilerErrorMessages = {
    // LEAVESCOPE_WITH_EMPTY_SYMBOLTABLE
    {
        CompilerErrorCode::LEAVESCOPE_WITH_EMPTY_SYMBOLTABLE,
        {
            "The symbol table was empty when exiting the scope.",
            "シンボルテーブルが空の状態でスコープを抜けました。"
        }
    },
    // ADDSYMBOL_WITH_NO_SCOPE
    {
        CompilerErrorCode::ADDSYMBOL_WITH_NO_SCOPE,
        {
            "attempting to add a symbol without a scope.",
            "スコープが存在しない状態で記号を追加しようとしています。"
        }
    },
    // EXPR_VISIT_COULDNOT_CAST
    {
        CompilerErrorCode::EXPR_VISIT_COULDNOT_CAST,
        {
            "SemanticAnalysis::visit(ExprNode *node) failed to find correct type '%0'.",
            "SemanticAnalysis::visit(ExprNode *node) で型 '%0' から正しい型が見つかりませんでした。"
        }
    },
    // STMT_VISIT_COULDNOT_CAST
    {
        CompilerErrorCode::STMT_VISIT_COULDNOT_CAST,
        {
            "SemanticAnalysis::visit(StatementNode *node) failed to find correct type '%0'.",
            "SemanticAnalysis::visit(StatementNode *node) で型 '%0' から正しい型が見つかりませんでした。"
        }
    },
    // CAST_NODE_TYPE_NULL
    {
        CompilerErrorCode::CAST_NODE_TYPE_NULL,
        {
            "node->type was nullptr in SemanticAnalysis::visit(CastNode *node).",
            "SemanticAnalysis::visit(CastNode *node) で node->type がヌルでした。"
        }
    }
};

inline std::string formatErrorMessage(std::string format, const std::vector<std::string>& args){
    for(size_t i = 0;i < args.size();i++){
        std::string placeholder = "%" + std::to_string(i);
        size_t pos = format.find(placeholder);
        while(pos != std::string::npos){
            format.replace(pos, placeholder.length(), args[i]);
            pos = format.find(placeholder, pos + args[i].length());
        }
    }
    return format;
}