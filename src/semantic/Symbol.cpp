#include "Symbol.h"

// シンボルをスコープに定義する
bool Scope::define(std::shared_ptr<Symbol> sym){
    if(symbols.count(sym->name)){
        return false; // このスコープで定義済み
    }
    symbols[sym->name] = sym;
    return true;
}

// シンボルを現在のスコープから再帰的に親スコープまで探す
std::shared_ptr<Symbol> Scope::lookup(const std::string& name){
    // 現在のスコープ
    auto it = symbols.find(name);
    if(it != symbols.end()) return it->second; // 見つかったとき
    // 見つからなければ親スコープで探す
    if(parent) return parent->lookup(name); // 再起で全部調べる
    return nullptr;
}

// シンボルを現在のスコープのみで探す
std::shared_ptr<Symbol> Scope::lookupCurrent(const std::string& name){
    auto it = symbols.find(name);
    if(it != symbols.end()) return it->second;
    return nullptr;
}

void Scope::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Scope {" << std::endl;
    for(const auto& pair : symbols){
        pair.second->dump(os, indent + 2);
        os << std::endl;
    }
    os << std::string(indent, ' ') << "}" << std::endl;
}