#pragma once

#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace PVE {

    class ConditionParser {
        using Value = std::variant<float, int, bool, std::string>;

    public:
        static void Init() {
            RegisterCondition("GetPlayerHealthPercentage", [] {
                const auto actorValueOwner = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();
                const float f1 = actorValueOwner->GetActorValue(RE::ActorValue::kHealth);
                const float f2 = actorValueOwner->GetPermanentActorValue(RE::ActorValue::kHealth);
                return (f1 / f2) * 100.0f;
            });
            RegisterCondition("GetPlayerStaminaPercentage", [] {
                const auto actorValueOwner = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();
                const float f1 = actorValueOwner->GetActorValue(RE::ActorValue::kStamina);
                const float f2 = actorValueOwner->GetPermanentActorValue(RE::ActorValue::kStamina);
                return (f1 / f2) * 100.0f;
            });
            RegisterCondition("GetPlayerMagickaPercentage", [] {
                const auto actorValueOwner = RE::PlayerCharacter::GetSingleton()->AsActorValueOwner();
                const float f1 = actorValueOwner->GetActorValue(RE::ActorValue::kMagicka);
                const float f2 = actorValueOwner->GetPermanentActorValue(RE::ActorValue::kMagicka);
                return (f1 / f2) * 100.0f;
            });
            RegisterCondition("GetPlayerEquippedWeaponTypeLeft", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (actor->GetEquippedObject(true)->Is(RE::FormType::Weapon)) {
                    return static_cast<int>(actor->GetEquippedObject(true)->As<RE::TESObjectWEAP>()->GetWeaponType());
                }
                return -1;
            });
            RegisterCondition("GetPlayerEquippedWeaponTypeRight", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (actor->GetEquippedObject(false)->Is(RE::FormType::Weapon)) {
                    return static_cast<int>(actor->GetEquippedObject(false)->As<RE::TESObjectWEAP>()->GetWeaponType());
                }
                return -1;
            });
            RegisterCondition("GetPlayerRace", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (actor->GetRace()) {
                    return actor->GetRace()->GetName();
                }
                return "";
            });
            RegisterCondition("GetPlayerSex", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                if (actor->GetActorBase()) {
                    return static_cast<int>(actor->GetActorBase()->GetSex());
                }
                return -1;
            });
            RegisterCondition("GetPlayerName", [] {
                const auto actor = RE::PlayerCharacter::GetSingleton();
                return actor->GetName();
            });
        }

        static void RegisterCondition(const std::string &name, const std::function<Value()> &fnct) {
            conditions[name] = fnct;
        }

        static void UnregisterCondition(const std::string &name) {
            conditions.erase(name);
        }

        static bool EvaluateCondition(const std::string &condition) {
            if (condition.empty()) {
                return true;
            }
            auto tokens = tokenize(condition);
            Parser parser(tokens);
            std::unique_ptr<Expr> expr = parser.parseExpression();
            if (!expr) {
                std::cerr << "Parse error in condition.\n";
                return false;
            }
            return expr->eval();
        }

    private:
        enum class TokenType { Identifier, Number, String, Operator, LeftParen, RightParen, End };

        struct Token {
            TokenType type;
            std::string value;
        };

        static std::vector<Token> tokenize(const std::string &str) {
            std::vector<Token> tokens;
            size_t i = 0;
            while (i < str.size()) {
                if (std::isspace(str[i])) {
                    ++i;
                    continue;
                }
                if (std::isalpha(str[i]) || str[i] == '_') {
                    size_t j = i;
                    while (j < str.size() && (std::isalnum(str[j]) || str[j] == '_')) j++;
                    tokens.push_back({TokenType::Identifier, str.substr(i, j - i)});
                    i = j;
                    continue;
                }
                if (std::isdigit(str[i]) || ((str[i] == '.') && (i + 1 < str.size()) && std::isdigit(str[i + 1]))) {
                    size_t j = i;
                    bool dotEncountered = false;
                    while (j < str.size() && (std::isdigit(str[j]) || str[j] == '.')) {
                        if (str[j] == '.') {
                            if (dotEncountered) break;
                            dotEncountered = true;
                        }
                        j++;
                    }
                    tokens.push_back({TokenType::Number, str.substr(i, j - i)});
                    i = j;
                    continue;
                }
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
                if (str[i] == '(') {
                    tokens.push_back({TokenType::LeftParen, "("});
                    ++i;
                    continue;
                }
                if (str[i] == ')') {
                    tokens.push_back({TokenType::RightParen, ")"});
                    ++i;
                    continue;
                }
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
            tokens.push_back({TokenType::End, ""});
            return tokens;
        }

        class Expr {
        public:
            virtual ~Expr() = default;

            [[nodiscard]] virtual bool eval() const = 0;
        };

        class LogicalExpr : public Expr {
        public:
            enum class Op { And, Or };

            std::unique_ptr<Expr> left;
            std::unique_ptr<Expr> right;
            Op op;

            LogicalExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, Op o) : left(std::move(l)), right(std::move(r)), op(o) {}

            bool eval() const override {
                if (op == Op::And)
                    return left->eval() && right->eval();
                else
                    return left->eval() || right->eval();
            }
        };

        class CondExpr : public Expr {
        public:
            std::string var;
            std::string op;
            Value literal;

            CondExpr(const std::string &v, const std::string &o, const Value &lit) : var(v), op(o), literal(lit) {}

            bool eval() const override {
                auto it = conditions.find(var);
                if (it == conditions.end()) return false;
                const auto &val = it->second();
                if (val.index() != literal.index()) return false;
                if (std::holds_alternative<float>(val)) {
                    float lhs = std::get<float>(val);
                    float rhs = std::get<float>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
                    if (op == "<") return lhs < rhs;
                    if (op == "<=") return lhs <= rhs;
                    if (op == ">") return lhs > rhs;
                    if (op == ">=") return lhs >= rhs;
                } else if (std::holds_alternative<int>(val)) {
                    int lhs = std::get<int>(val);
                    int rhs = std::get<int>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
                    if (op == "<") return lhs < rhs;
                    if (op == "<=") return lhs <= rhs;
                    if (op == ">") return lhs > rhs;
                    if (op == ">=") return lhs >= rhs;
                } else if (std::holds_alternative<bool>(val)) {
                    bool lhs = std::get<bool>(val);
                    bool rhs = std::get<bool>(literal);
                    if (op == "==") return lhs == rhs;
                    if (op == "!=") return lhs != rhs;
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
                }
                return false;
            }
        };

        class Parser {
        public:
            Parser(const std::vector<Token> &tokens) : tokens(tokens), pos(0) {}

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
                if (current().type != TokenType::Identifier) return nullptr;
                std::string varName = current().value;
                consume();

                if (current().type != TokenType::Operator) return nullptr;
                std::string op = current().value;
                consume();

                if (current().type == TokenType::Number) {
                    float num = std::stof(current().value);
                    consume();
                    return std::make_unique<CondExpr>(varName, op, Value(num));
                } else if (current().type == TokenType::String) {
                    std::string s = current().value;
                    consume();
                    return std::make_unique<CondExpr>(varName, op, Value(s));
                }
                return nullptr;
            }
        };
        inline static std::map<std::string, std::function<Value()>> conditions;
    };
}