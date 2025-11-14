#include "semantic/Symbol.h"
#include "types/Type.h" // Include the full definition of TypeNode here

Symbol::Symbol(const std::string& name, TypeNode* type) 
    : kind(SymbolKind::VAR), name(name), type(type) {}

Symbol::Symbol(const std::string& name, TypeNode* returnType, const std::vector<TypeNode*>& args) 
    : kind(SymbolKind::FUNC), name(name), type(returnType), argTypes(args) {}
