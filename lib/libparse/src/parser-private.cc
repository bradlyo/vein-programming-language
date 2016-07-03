//
// Copyright 2015-2016 Robert Fratto. See the LICENSE.txt file at the top-level
// directory of this distribution.
//
// Licensed under the MIT license <http://opensource.org/licenses/MIT>. This file
// may not be copied, modified, or distributed except according to those terms.
//

#include <stdexcept>

#include <libparse/parser.h>
#include <libast/type.h>
#include <libast/flag.h>
#include <libast/typecheck.h>
#include "lex_stream.h"
#include "lex_helpers.h"
#include "parser-private.h"

using namespace orange::ast;
using namespace orange::parser;
using namespace orange::parser::impl;

int impl::GetOperatorPrecedence(TokenType tok) {
	const static std::map<TokenType, int> OperatorPrecedence = {
		{TokenType::PLUS, 4}, {TokenType::MINUS, 4},
		{TokenType::SHIFT_LEFT, 5}, {TokenType::SHIFT_RIGHT, 5},
		{TokenType::LESS_THAN, 6}, {TokenType::GREATER_THAN, 6},
		{TokenType::LEQ, 6}, {TokenType::GEQ, 6},
		{TokenType::EQUALS, 7}, {TokenType::NEQ, 7},
		{TokenType::BIT_AND, 8},
		{TokenType::BIT_XOR, 9},
		{TokenType::BIT_OR, 10},
		{TokenType::AND, 11},
		{TokenType::OR, 12},
		{TokenType::QUESTION, 13}, {TokenType::ASSIGN, 13},
		{TokenType::PLUS_ASSIGN, 13}, {TokenType::MINUS_ASSIGN, 13},
		{TokenType::TIMES_ASSIGN, 13}, {TokenType::DIVIDE_ASSIGN, 13},
		{TokenType::REMAINDER_ASSIGN, 13}, {TokenType::SHIFT_LEFT_ASSIGN, 13},
		{TokenType::SHIFT_RIGHT_ASSIGN, 13}, {TokenType::BIT_OR_ASSIGN, 13},
		{TokenType::BIT_AND_ASSIGN, 13}, {TokenType::BIT_XOR_ASSIGN, 13}
	};

	auto it = OperatorPrecedence.find(tok);
	if (it == OperatorPrecedence.end()) return -1;
	return it->second;
}

OperatorAssociativity impl::GetAssociativity(TokenType tok) {
	auto prec = GetOperatorPrecedence(tok);

	if (prec == 13) return OperatorAssociativity::RIGHT;
	else return OperatorAssociativity::LEFT;
}

UnaryOp impl::GetUnaryOp(TokenType ty) {
	switch (ty) {
		case TokenType::INCREMENT: return UnaryOp::INCREMENT;
		case TokenType::DECREMENT: return UnaryOp::DECREMENT;
		case TokenType::MINUS:     return UnaryOp::MINUS;
		case TokenType::NOT:       return UnaryOp::NOT;
		case TokenType::TILDE:     return UnaryOp::TILDE;
		case TokenType::TIMES:     return UnaryOp::TIMES;
		case TokenType::BIT_AND:   return UnaryOp::REFERENCE;
		default:
			throw std::runtime_error("Not a valid unary operator token");
	}
}

BinOp impl::GetBinOp(TokenType ty) {
	switch (ty) {
		case TokenType::PLUS:               return BinOp::ADD;
		case TokenType::MINUS:              return BinOp::SUBTRACT;
		case TokenType::DIVIDE:             return BinOp::DIVIDE;
		case TokenType::TIMES:              return BinOp::MULTIPLY;
		case TokenType::REMAINDER:          return BinOp::REMAINDER;
		case TokenType::BIT_OR:             return BinOp::BIT_OR;
		case TokenType::BIT_AND:            return BinOp::BIT_AND;
		case TokenType::BIT_XOR:            return BinOp::BIT_XOR;
		case TokenType::SHIFT_LEFT:         return BinOp::SHIFT_LEFT;
		case TokenType::SHIFT_RIGHT:        return BinOp::SHIFT_RIGHT;
		case TokenType::ASSIGN:             return BinOp::ASSIGN;
		case TokenType::EQUALS:             return BinOp::EQUALS;
		case TokenType::PLUS_ASSIGN:        return BinOp::PLUS_ASSIGN;
		case TokenType::MINUS_ASSIGN:       return BinOp::MINUS_ASSIGN;
		case TokenType::TIMES_ASSIGN:       return BinOp::TIMES_ASSIGN;
		case TokenType::DIVIDE_ASSIGN:      return BinOp::DIVIDE_ASSIGN;
		case TokenType::REMAINDER_ASSIGN:   return BinOp::REMAINDER_ASSIGN;
		case TokenType::SHIFT_LEFT_ASSIGN:  return BinOp::SHIFT_LEFT_ASSIGN;
		case TokenType::SHIFT_RIGHT_ASSIGN: return BinOp::SHIFT_RIGHT_ASSIGN;
		case TokenType::BIT_OR_ASSIGN:      return BinOp::BIT_OR_ASSIGN;
		case TokenType::BIT_AND_ASSIGN:     return BinOp::BIT_AND_ASSIGN;
		case TokenType::BIT_XOR_ASSIGN:     return BinOp::BIT_XOR_ASSIGN;
		case TokenType::LESS_THAN:          return BinOp::LESS_THAN;
		case TokenType::GREATER_THAN:       return BinOp::GREATER_THAN;
		case TokenType::LEQ:                return BinOp::LEQ;
		case TokenType::GEQ:                return BinOp::GEQ;
		case TokenType::NEQ:                return BinOp::NEQ;
		case TokenType::AND:                return BinOp::AND;
		case TokenType::OR:                 return BinOp::OR;
		default:
			throw std::runtime_error("Not a valid binary operator token");
	}
}

bool impl::Parser::isTerm(Token* tok) {
	return tok->type == NEWLINE || tok->type == SEMICOLON;
}

bool impl::Parser::checkNext(TokenType ty) {
	return mStream.peek() != nullptr && mStream.peek()->type == ty;
}

std::vector<Node *> impl::Parser::parse_opt_statements() {
	return parse_statements(true);
}

std::vector<Node *> impl::Parser::parse_statements(bool allow_eps) {
	std::vector<Node *> statements;

	if (mStream.peek() && isTerm(mStream.peek())) {
		mStream.get();
		return parse_statements();
	}

	if (mStream.peek() && mStream.peek()->type == COMMENT) {
		statements.push_back(CreateNode<CommentStmt>(mStream.get()->value));

		auto remaining = parse_statements();
		statements.insert(statements.end(), remaining.begin(), remaining.end());

		return statements;
	} else {
		auto stmt = parse_statement();

		if (stmt == nullptr) {
			if (allow_eps) return statements;
			throw std::runtime_error("Expected statement");
		}

		statements.push_back(stmt);

		if (mStream.peek() && isTerm(mStream.peek())) {
			parse_term();

			auto remaining = parse_statements();
			statements.insert(statements.end(), remaining.begin(), remaining.end());
		}

		return statements;
	}
}

Token* impl::Parser::parse_term() {
	if (!isTerm(mStream.peek())) return nullptr;
	return mStream.get();
}

LongBlockExpr* impl::Parser::parse_long_block() {
	if (checkNext(OPEN_CURLY) == false) return nullptr;
	mStream.get();

	auto stmts = parse_opt_statements();

	if (checkNext(CLOSE_CURLY) == false) throw std::runtime_error("Expected }");

	return CreateNode<LongBlockExpr>(stmts);
}

ShortBlockExpr* impl::Parser::parse_short_block() {
	if (checkNext(COLON) == false) return nullptr;

	auto stmt = parse_statement();
	if (stmt == nullptr) throw std::runtime_error("Expected statement");

	return CreateNode<ShortBlockExpr>(stmt);
}

BlockExpr* impl::Parser::parse_block() {
	BlockExpr* block = nullptr;

	if ((block = parse_long_block()) != nullptr) return block;
	if ((block = parse_short_block()) != nullptr) return block;

	throw std::runtime_error("Expected block");
}


