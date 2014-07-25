#include "syntax.h"

/*TranslationUnit* Parser :: parseTranslationUnit() {
	list<ExternalDeclaration*> list;
	ExternalDeclaration* ptr;
	while ((ptr = parseExternalDeclaration())) {
		list.push_back(ptr);
	}
	return new TranslationUnit(list);
}

ExternalDeclaration* Parser :: parseExternalDeclaration() {
	//TODO: Implement
	return new ExternalDeclaration();
}*/

Statement* Parser :: parseStatement() {
	Token current = source->peek();
	string currentName = current.getName();
	if (currentName == "goto" || currentName == "return" || currentName == "break"\
			|| currentName == "continue") {
		return parseJumpStatement();
	}
	return parseExpressionStatement();
}

ExpressionStatement* Parser :: parseExpressionStatement() {
	Expression* expr = NULL;
	expr = parseExpression();
	Token after = source->get();
	if (after.getName() != ";") {
		string err = "Expected ';' after ExpressionStatement";
		throw SyntaxException(err);
	}
	return new ExpressionStatement(expr);
}

JumpStatement* Parser :: parseJumpStatement() {
	Token current = source->get();
	string currentName = current.getName();
	JumpStatement* ret = NULL;
	if (currentName == "continue" || currentName == "break") {
		ret = new JumpStatement(currentName);
	} else if (currentName == "return") {
		Expression* expr = parseExpression();
		if (expr != NULL) {
			ret = new JumpStatement(expr);
		} else {
			ret = new JumpStatement(currentName);
		}
	} else if (currentName == "goto") {
		Identifier* id = parseIdentifier();
		if (id != NULL) {
			ret = new JumpStatement(id);
		} else {
			string err = "Expected identifier after goto in jump statement";
			throw new SyntaxException(err);
		}
	} else {
		string err = "Expected 'return', 'break', 'continue', or 'goto' in jump"\
					  "statement";
		throw new SyntaxException(err);
	}
	current = source->get();
	if (current.getName() != ";") {
		string err = "Expected ';' after jump statement";
		throw new SyntaxException(err);
	}
	return ret;
}

Expression* Parser :: parseExpression() {
	Token token = source->get();
	Expression* ptr = NULL;
	if (token.getKey() == IDENTIFIER) {
		ptr = new Expression(token);
	} else {
		string err = "Expected identifier";
		throw SyntaxException(err);
	}
	return ptr;
}

Identifier* Parser :: parseIdentifier() {
	Token current = source->get();
	if (current.getKey() == IDENTIFIER) {
		return new Identifier(current);
	} else {
		string err = "Expected identifier";
		throw new SyntaxException(err);
	}
}

