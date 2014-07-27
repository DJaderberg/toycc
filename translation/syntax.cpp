#include "syntax.h"

Declarator :: ~Declarator() {
	if (dirDecl != NULL) {delete dirDecl;}
	/*if (pointer != NULL) {delete pointer;}*/
}

string Declarator :: getName() {
	string ret = "";
	if (pointer != NULL) {
		//ret += pointer->getName() + " ";
	}
	ret += dirDecl->getName();
	return ret;
}

BlockItem :: ~BlockItem() {
	if (state != NULL) delete state;
	if (decl != NULL) delete decl;
}

string BlockItem :: getName() {
	if (state != NULL) {return state->getName();}
	if (decl != NULL) {return decl->getName();}
	return "";
}

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
	BlockItem* ret = NULL;
	unsigned int bufferUsed = source->bufferSize();
	try {
		ret = new BlockItem(parseStatement());
	} catch (ExpressionException) {
		//No match with Statement, keep trying other things
	}
	//If it was not a Statement, it should be a Declaration
	if( ret == NULL) {
		source->setUsed(bufferUsed);
		Declaration* decl = parseDeclaration();
		ret = new BlockItem(decl);
	}
	return ret;
}


ExpressionStatement* Parser :: parseExpressionStatement() {
	Expression* expr = NULL;
	expr = parseExpression();
	Token after = source->get();
	if (after.getName() != ";") {
		string err = "Expected ';' after ExpressionStatement";
		throw ExpressionException(err);
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
	state = parseStatement();
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

/* Thanks to 
 * http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
 * for attempting to explain Pratt parsers.
 */
Expression* Parser :: parseExpression() {
	/*Token token = source->get();
	Expression* ptr = NULL;
	if (token.getKey() == IDENTIFIER) {
		ptr = new IdentifierExpression(token);
	} else {
		string err = "Expected identifier";
		throw ExpressionException(err);
	}
	return ptr;*/
	Expression* ret = NULL;
	Token token = source->get();
	auto searchPrefix = mPrefix.find(token.getName());
	Expression* left = NULL;
	if (token.getKey() == IDENTIFIER) {
		left = new IdentifierExpression(token);
	} else if (searchPrefix != mPrefix.end()) {
		//Found current token as prefix operator
		left = searchPrefix->second(this, parseExpression());
	} else {
		string err = "Could not parse '" + token.getName() + "'";
	}
	
	token = source->peek();
	auto searchInfix = mInfix.find(token.getName());
	if (searchInfix != mInfix.end()) {
		//Found current token as infix operator
		source->get(); //Actually consume the peeked token
		ret = searchInfix->second(this, left);
	} else {
		//No infix here, so just return the prefix parsing
		ret = left;
	}
	return ret;
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

Declarator* Parser :: parseDeclarator() {
	//TODO: Implement optional pointers
	Declarator* ret = NULL;
	DirectDeclarator* dirDecl = NULL;
	try {
		dirDecl = parseDirectDeclarator();
		ret = new Declarator(dirDecl);
	} catch (DirectDeclaratorException& error) {
		string err = "Could not parse direct declarator in declarator";
		throw new DeclaratorException(err);
	}
	return ret;
}

DirectDeclarator* Parser :: parseDirectDeclarator() {
	//TODO: Expand when DirectDeclarator has been expanded
	DirectDeclarator* ret = NULL;
	Token peek = source->peek();
	if (peek.getName() == "(") {
		source->get();
		Declarator* decl = parseDeclarator();
		if (source->get().getName() == ")") {
			ret = new DirectDeclarator(decl);
		} else {
			string err = "Expected ')' after declarator in direct declarator";
			throw new DirectDeclaratorException(err);
		}
	} else if (peek.getKey() == IDENTIFIER) {
		ret = new DirectDeclarator(parseIdentifier());
	} else {
		ret = NULL;
	}
	return ret;
}

Declaration* Parser :: parseDeclaration() {
	//TODO: Expand
	Declaration* ret = NULL;
	try {
		ret = new Declaration(parseInitDeclaratorList());
	} catch (InitDeclaratorListException) {
		//No problem, since the list is optional
		ret = new Declaration();
	}
	return ret;
}

InitDeclaratorList* Parser :: parseInitDeclaratorList() {
	InitDeclarator* first = NULL;
	try {
		first = parseInitDeclarator();
		Token after = source->get();
		if (after.getName() == ",") {
			return new InitDeclaratorList(first, parseInitDeclaratorList());
		}
	} catch (InitDeclaratorException) {
		string err = "Could not parse init declarator in init declarator list";
		throw new InitDeclaratorListException(err);
	}
	return new InitDeclaratorList(first);
}

InitDeclarator* Parser :: parseInitDeclarator() {
	//TODO: Implement ' = Initializer'
	InitDeclarator* ret = NULL;
	try {
		ret = new InitDeclarator(parseDeclarator());
	} catch (DeclaratorException) {
		string err = "Could not parse declarator in init declarator";
		throw new InitDeclaratorException(err);
	}
	return ret;
}


