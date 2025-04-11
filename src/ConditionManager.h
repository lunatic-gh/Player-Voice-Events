#pragma once

#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "Utils.h"

namespace PVE {
    class ConditionManager {
        using Value = std::variant<float, int, bool, std::string>;

    public:
        static void Init() {
            RegisterCondition("PlayerHealthPercentage", [] {
                const auto actorValueOwner = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();
                const float f1 = actorValueOwner->GetActorValue(RE::ActorValue::kHealth);
                const float f2 = actorValueOwner->GetPermanentActorValue(RE::ActorValue::kHealth);
                return (f1 / f2) * 100.0f;
            });
            RegisterCondition("PlayerStaminaPercentage", [] {
                const auto actorValueOwner = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();
                const float f1 = actorValueOwner->GetActorValue(RE::ActorValue::kStamina);
                const float f2 = actorValueOwner->GetPermanentActorValue(RE::ActorValue::kStamina);
                return (f1 / f2) * 100.0f;
            });
            RegisterCondition("PlayerMagickaPercentage", [] {
                const auto actorValueOwner = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();
                const float f1 = actorValueOwner->GetActorValue(RE::ActorValue::kMagicka);
                const float f2 = actorValueOwner->GetPermanentActorValue(RE::ActorValue::kMagicka);
                return (f1 / f2) * 100.0f;
            });
            RegisterCondition("PlayerEquippedWeaponTypeLeft", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (const auto form = actor->GetEquippedObject(true)) {
                    if (const auto weapon = form->As<RE::TESObjectWEAP>()) {
                        return static_cast<int>(weapon->GetWeaponType());
                    }
                }
                return -1;
            });
            RegisterCondition("PlayerEquippedWeaponTypeRight", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (const auto form = actor->GetEquippedObject(false)) {
                    if (const auto weapon = form->As<RE::TESObjectWEAP>()) {
                        return static_cast<int>(weapon->GetWeaponType());
                    }
                }
                return -1;
            });
            RegisterCondition("PlayerRaceName", [] {
                if (const auto actor = RE::PlayerCharacter::GetSingleton(); actor->GetRace()) {
                    return actor->GetRace()->GetName();
                }
                return "";
            });
            RegisterCondition("PlayerSex", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (actor->GetActorBase()) {
                    return static_cast<int>(actor->GetActorBase()->GetSex());
                }
                return -1;
            });
            RegisterCondition("PlayerName", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                return actor->GetName();
            });
            RegisterCondition("PlayerPosX", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                return actor->GetPositionX();
            });
            RegisterCondition("PlayerPosY", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                return actor->GetPositionY();
            });
            RegisterCondition("PlayerPosZ", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                return actor->GetPositionZ();
            });
            RegisterCondition("PlayerLocationName", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (actor->GetCurrentLocation()) {
                    return actor->GetCurrentLocation()->GetFullName();
                }
                return "";
            });
            RegisterCondition("PlayerLocationKeywords", [] {
                auto actor = RE::PlayerCharacter::GetSingleton();
                return Utils::GetFormKeywordsFormatted(actor->GetCurrentLocation());
            });
            RegisterCondition("RandomInt", [] {
                return Utils::GenerateRandomInt(1, 100);
            });
            RegisterCondition("RandomFloat", [] {
                return Utils::GenerateRandomFloat(0.0f, 100.0f);
            });
            RegisterCondition("RandomBool", [] {
                return Utils::GenerateRandomInt(1, 100) <= 50;
            });
            RegisterCondition("PlayerWerewolfState", [] {
                if (const auto globalVar = RE::TESForm::LookupByID(0xed06c)->As<RE::TESGlobal>()) {
                    if (const auto player = RE::PlayerCharacter::GetSingleton()) {
                        if (globalVar->value == 1.0f) {
                            if (player->GetRace()->GetFormID() == 0xCDD84) {
                                return 2;
                            }
                            return 1;
                        }
                    }
                }
                return 0;
            });
        }

        static void RegisterCondition(const std::string &conditionName, const std::function<Value()> &conditionFunction) {
            conditions[conditionName] = conditionFunction;
        }

        static void RegisterDynamicCondition(const std::string &eventName, const std::string &conditionName, const std::function<Value()> &conditionFunction) {
            dynamicConditions[eventName][conditionName] = conditionFunction;
        }

        static void ResetDynamicConditions(const std::string &eventName) {
            dynamicConditions.erase(eventName);
        }

        static bool EvaluateCondition(const std::string &eventName, const std::string &condition) {
            if (condition.empty()) {
                return true;
            }
            auto tokens = tokenize(condition);
            Parser parser(tokens);
            std::unique_ptr<Expr> expr = parser.parseExpression();
            if (!expr) {
                return false;
            }
            return expr->eval(eventName);
        }

    private:
        enum class TokenType { Identifier, Float, Int, String, Bool, Operator, LeftParen, RightParen, End };

        struct Token {
            TokenType type;
            std::string value;
        };

        static std::vector<Token> tokenize(const std::string &str) {
            std::vector<Token> tokens;
            size_t i = 0;
            while (i < str.size()) {
                // Ignore spaces
                if (std::isspace(str[i])) {
                    ++i;
                    continue;
                }
                // Base16 Numbers
                if (i + 1 < str.size() && str[i] == '0' && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
                    size_t j = i + 2;
                    while (j < str.size() && std::isxdigit(str[j])) {
                        j++;
                    }
                    tokens.push_back({TokenType::Int, str.substr(i, j - i)});
                    i = j;
                    continue;
                }
                // Base10 Numbers
                if (std::isdigit(str[i]) || (str[i] == '.' && i + 1 < str.size() && std::isdigit(str[i + 1]))) {
                    size_t j = i;
                    bool dotEncountered = false;
                    while (j < str.size() && (std::isdigit(str[j]) || str[j] == '.')) {
                        if (str[j] == '.') {
                            if (dotEncountered) break;
                            dotEncountered = true;
                        }
                        j++;
                    }
                    tokens.push_back({dotEncountered ? TokenType::Float : TokenType::Int, str.substr(i, j - i)});
                    i = j;
                    continue;
                }
                // Booleans & Identifiers
                if (std::isalpha(str[i]) || str[i] == '_') {
                    size_t j = i;
                    while (j < str.size() && (std::isalnum(str[j]) || str[j] == '_')) {
                        j++;
                    }
                    std::string tokenStr = str.substr(i, j - i);
                    if (tokenStr == "true" || tokenStr == "false") {
                        tokens.push_back({TokenType::Bool, tokenStr});
                    } else {
                        tokens.push_back({TokenType::Identifier, tokenStr});
                    }
                    i = j;
                    continue;
                }
                // Strings
                if (str[i] == '\'' || str[i] == '"') {
                    char quote = str[i];
                    i++;
                    std::string literal;
                    while (i < str.size()) {
                        if (str[i] == quote) {
                            if (i + 1 < str.size() && str[i + 1] == quote) {
                                literal.push_back(quote);
                                i += 2;
                            } else {
                                i++;
                                break;
                            }
                        } else if (str[i] == '\\') {
                            if (i + 1 < str.size()) {
                                literal.push_back(str[i + 1]);
                                i += 2;
                            } else {
                                literal.push_back(str[i]);
                                i++;
                            }
                        } else {
                            literal.push_back(str[i]);
                            i++;
                        }
                    }
                    tokens.push_back({TokenType::String, literal});
                    continue;
                }
                // Opening bracket
                if (str[i] == '(') {
                    tokens.push_back({TokenType::LeftParen, "("});
                    ++i;
                    continue;
                }
                // Closing Bracket
                if (str[i] == ')') {
                    tokens.push_back({TokenType::RightParen, ")"});
                    ++i;
                    continue;
                }
                // Comparison Operators
                if (i + 1 < str.size()) {
                    std::string two = str.substr(i, 2);
                    if (two == "&&" || two == "||" || two == "==" || two == "!=" || two == "<=" || two == ">=" || two == "*=") {
                        tokens.push_back({TokenType::Operator, two});
                        i += 2;
                        continue;
                    }
                }
                if (str[i] == '<' || str[i] == '>') {
                    tokens.push_back({TokenType::Operator, std::string(1, str[i])});
                    ++i;
                    continue;
                }
                ++i;
            }
            // Done
            tokens.push_back({TokenType::End, ""});
            return tokens;
        }

        class Expr {
        public:
            virtual ~Expr() = default;

            [[nodiscard]] virtual bool eval(const std::string &eventName) const = 0;
        };

        class LogicalExpr : public Expr {
        public:
            enum class Op { And, Or };

            std::unique_ptr<Expr> left;
            std::unique_ptr<Expr> right;
            Op op;

            LogicalExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, Op o) : left(std::move(l)), right(std::move(r)), op(o) {
            }

            bool eval(const std::string &eventName) const override {
                if (op == Op::And)
                    return left->eval(eventName) && right->eval(eventName);
                else
                    return left->eval(eventName) || right->eval(eventName);
            }
        };

        class CondExpr : public Expr {
        public:
            std::string var;
            std::string op;
            Value literal;

            CondExpr(const std::string &v, const std::string &o, const Value &lit) : var(v), op(o), literal(lit) {
            }

            bool eval(const std::string &eventName) const override {
                std::unordered_map<std::string, std::function<Value()> > map = conditions;
                if (dynamicConditions.contains(eventName)) {
                    for (auto [k, v] : dynamicConditions[eventName]) {
                        map[k] = v;
                    }
                }

                const auto it = map.find(var);
                if (it == map.end()) {
                    return false;
                }

                const auto &val = it->second();

                if ((val.index() != literal.index())) {
                    return false;
                }
                if (std::holds_alternative<float>(val)) {
                    const float lhs = std::get<float>(val);
                    const float rhs = std::get<float>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
                    if (op == "<") return lhs < rhs;
                    if (op == "<=") return lhs <= rhs;
                    if (op == ">") return lhs > rhs;
                    if (op == ">=") return lhs >= rhs;
                } else if (std::holds_alternative<int>(val)) {
                    const int lhs = std::get<int>(val);
                    const int rhs = std::get<int>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
                    if (op == "<") return lhs < rhs;
                    if (op == "<=") return lhs <= rhs;
                    if (op == ">") return lhs > rhs;
                    if (op == ">=") return lhs >= rhs;
                    return false;
                } else if (std::holds_alternative<bool>(val)) {
                    const bool lhs = std::get<bool>(val);
                    const bool rhs = std::get<bool>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
                    return false;
                } else if (std::holds_alternative<std::string>(val)) {
                    const std::string &lhs = std::get<std::string>(val);
                    const std::string &rhs = std::get<std::string>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
                    if (op == "<") return lhs < rhs;
                    if (op == "<=") return lhs <= rhs;
                    if (op == ">") return lhs > rhs;
                    if (op == ">=") return lhs >= rhs;
                    if (op == "*=") return lhs.contains(rhs);
                    return false;
                }
                return false;
            }
        };

        class Parser {
        public:
            Parser(const std::vector<Token> &tokens) : tokens(tokens), pos(0) {
            }

            std::unique_ptr<Expr> parseExpression() {
                return parseOr();
            }

        private:
            const std::vector<Token> &tokens;
            size_t pos;

            const Token &current() const {
                return tokens[pos];
            }

            void consume() {
                ++pos;
            }

            bool match(TokenType type, const std::string &val = "") {
                if (current().type == type && (val.empty() || current().value == val)) {
                    consume();
                    return true;
                }
                return false;
            }

            std::unique_ptr<Expr> parseOr() {
                auto left = parseAnd();
                while (current().type == TokenType::Operator && current().value == "||") {
                    consume();
                    auto right = parseAnd();
                    left = std::make_unique<LogicalExpr>(std::move(left), std::move(right), LogicalExpr::Op::Or);
                }
                return left;
            }

            std::unique_ptr<Expr> parseAnd() {
                auto left = parseFactor();
                while (current().type == TokenType::Operator && current().value == "&&") {
                    consume();
                    auto right = parseFactor();
                    left = std::make_unique<LogicalExpr>(std::move(left), std::move(right), LogicalExpr::Op::And);
                }
                return left;
            }

            std::unique_ptr<Expr> parseFactor() {
                if (match(TokenType::LeftParen)) {
                    auto expr = parseExpression();
                    match(TokenType::RightParen);
                    return expr;
                }
                return parseCondition();
            }

            std::unique_ptr<Expr> parseCondition() {
                if (current().type != TokenType::Identifier) {
                    return nullptr;
                }
                std::string varName = current().value;
                consume();

                if (current().type != TokenType::Operator) {
                    return nullptr;
                }
                std::string op = current().value;
                consume();

                if (current().type == TokenType::Float) {
                    float num = std::stof(current().value);
                    consume();
                    return std::make_unique<CondExpr>(varName, op, Value(num));
                }
                if (current().type == TokenType::Int) {
                    int num = std::stoi(current().value, nullptr, (current().value.starts_with("0x") || current().value.starts_with("0X")) ? 16 : 10);
                    consume();
                    return std::make_unique<CondExpr>(varName, op, Value(num));
                }
                if (current().type == TokenType::Bool) {
                    auto s = current().value;
                    std::ranges::transform(s, s.begin(), [](const unsigned char c) {
                        return std::tolower(c);
                    });
                    consume();
                    return std::make_unique<CondExpr>(varName, op, Value(s == "true"));
                }
                if (current().type == TokenType::String) {
                    std::string s = current().value;
                    consume();
                    return std::make_unique<CondExpr>(varName, op, Value(s));
                }
                return nullptr;
            }
        };

        inline static std::unordered_map<std::string, std::function<Value()> > conditions;
        inline static std::unordered_map<std::string, std::map<std::string, std::function<Value()> > > dynamicConditions;
    };
}
