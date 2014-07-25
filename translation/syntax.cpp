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
}*/

Statement* Parser :: parseStatement() {
	Token current = source->peek();
	string currentName = current.getName();
	if (currentName == "goto" || currentName == "return" || currentName == "break"\
			|| currentName == "continue") {
		return parseJumpStatement();
	} else if (currentName == "if" || currentName == "switch") {
		return parseSelectionStatement();
	} else if (currentName == "\{") {
		return parseCompoundStatement();
	}
	return parseExpressionStatement();
}

CompoundStatement* Parser :: parseCompoundStatement() {
	Token current = source->get();
	CompoundStatement* ret = NULL;
	BlockItemList* itemList = NULL;
	string err;
	if (current.getName() == "\{") {
		try {
			itemList = parseBlockItemList();
		} catch (BlockItemListException) {
			//No list, which is okay since it is optional
			return new CompoundStatement();
		}
		current = source->get();
		if (current.getName() != "}") {
			err = "Expected '}' after compound statement";
			throw new SyntaxException(err);
		}
		ret = new CompoundStatement(itemList);
	} else {
		err = "Expected '{' to start compound statement";
		throw new SyntaxException(err);
	}
	return ret;
}

BlockItemList* Parser :: parseBlockItemList() {
	BlockItem* first = parseBlockItem();
	BlockItemList* next = NULL;
	if (source->peek().getName() == "}") {
		return new BlockItemList(first);
	}
	try {
		next = parseBlockItemList();
	} catch (BlockItemListException) {
		//No more lists, which is okay
	}
	return new BlockItemList(first, next);
}

BlockItem* Parser :: parseBlockItem() {
	//TODO: Implement declarations
	return new BlockItem(parseStatement());
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

SelectionStatement* Parser :: parseSelectionStatement() {
	Token current = source->get();
	string currentName = current.getName();
	string err = "Error while parsing selection statement";
	string keyword = currentName;
	Expression* expr;
	Statement* state;
	Statement* stateOpt = NULL;
	SelectionStatement* ret = NULL;
	if (currentName != "if" && currentName != "switch") {
		err = "Expected 'if' or 'switch' to start selection statement";
		throw new SyntaxException(err);
	}
	current = source->get();
	currentName = current.getName();
	if (currentName != "(") {
		err = "Expected '(' after '" + keyword + "'";
		throw new SyntaxException(err);
	}
	expr = parseExpression();
	current = source->get();
	currentName = current.getName();
	if (currentName != ")") {
		err = "Expected ')' after expression in selection statement";
		throw new SyntaxException(err);
	}
	state = parseStatement(); //TODO: NULL here sometimes
	if (state == NULL) {
		err = "Could not parse first statement arm in selection statement";
		throw new SyntaxException(err);
	}
	current = source->peek();
	currentName = current.getName();
	if (currentName  == "else") {
		source->get();
		stateOpt = parseStatement();
		if (keyword == "if") {
			ret = new SelectionStatement(keyword, expr, state, stateOpt);
		} else {
			err = "Did not expect 'else' when not in 'if' statement";
			throw new SyntaxException(err);
		}
	} else {
		ret = new SelectionStatement(keyword, expr, state);
	}
	return ret;
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

