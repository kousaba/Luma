
// Generated from /home/etomunsama/Luma/grammar/LumaLexer.g4 by ANTLR 4.13.2


#include "LumaLexer.h"


using namespace antlr4;

using namespace Luma;


using namespace antlr4;

namespace {

struct LumaLexerStaticData final {
  LumaLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  LumaLexerStaticData(const LumaLexerStaticData&) = delete;
  LumaLexerStaticData(LumaLexerStaticData&&) = delete;
  LumaLexerStaticData& operator=(const LumaLexerStaticData&) = delete;
  LumaLexerStaticData& operator=(LumaLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag lumalexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<LumaLexerStaticData> lumalexerLexerStaticData = nullptr;

void lumalexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (lumalexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(lumalexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<LumaLexerStaticData>(
    std::vector<std::string>{
      "VAR", "IDENTIFIER", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'var'"
    },
    std::vector<std::string>{
      "", "VAR", "IDENTIFIER", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,3,20,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,1,0,1,0,1,0,1,0,1,1,4,1,13,8,1,
  	11,1,12,1,14,1,2,1,2,1,2,1,2,0,0,3,1,1,3,2,5,3,1,0,2,3,0,48,57,65,90,
  	97,122,2,0,9,9,32,32,20,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,1,7,1,0,0,
  	0,3,12,1,0,0,0,5,16,1,0,0,0,7,8,5,118,0,0,8,9,5,97,0,0,9,10,5,114,0,0,
  	10,2,1,0,0,0,11,13,7,0,0,0,12,11,1,0,0,0,13,14,1,0,0,0,14,12,1,0,0,0,
  	14,15,1,0,0,0,15,4,1,0,0,0,16,17,7,1,0,0,17,18,1,0,0,0,18,19,6,2,0,0,
  	19,6,1,0,0,0,2,0,14,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  lumalexerLexerStaticData = std::move(staticData);
}

}

LumaLexer::LumaLexer(CharStream *input) : Lexer(input) {
  LumaLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *lumalexerLexerStaticData->atn, lumalexerLexerStaticData->decisionToDFA, lumalexerLexerStaticData->sharedContextCache);
}

LumaLexer::~LumaLexer() {
  delete _interpreter;
}

std::string LumaLexer::getGrammarFileName() const {
  return "LumaLexer.g4";
}

const std::vector<std::string>& LumaLexer::getRuleNames() const {
  return lumalexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& LumaLexer::getChannelNames() const {
  return lumalexerLexerStaticData->channelNames;
}

const std::vector<std::string>& LumaLexer::getModeNames() const {
  return lumalexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& LumaLexer::getVocabulary() const {
  return lumalexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView LumaLexer::getSerializedATN() const {
  return lumalexerLexerStaticData->serializedATN;
}

const atn::ATN& LumaLexer::getATN() const {
  return *lumalexerLexerStaticData->atn;
}




void LumaLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  lumalexerLexerInitialize();
#else
  ::antlr4::internal::call_once(lumalexerLexerOnceFlag, lumalexerLexerInitialize);
#endif
}
