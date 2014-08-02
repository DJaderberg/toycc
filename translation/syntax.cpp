#include "syntax.h"

string ExternalDeclaration :: getName() {
	string ret = "";
	if (funcDef != NULL) {ret += funcDef->getName();}
	if (decl != NULL) {ret += decl->getName();}
	return ret;
}

ParameterDeclaration :: ~ParameterDeclaration() {
	if (declSpecList != NULL) {delete declSpecList;}
	if (decl != NULL) {delete decl;}
}

string ParameterDeclaration :: getName() {
	string ret = "";
	if (declSpecList != NULL) {ret += declSpecList->getName();}
	if (decl != NULL) {ret += decl->getName();}
	return ret;
}

ParameterTypeListDirectDeclarator :: ~ParameterTypeListDirectDeclarator() {
	if (params != NULL) {delete params;}
}

string ParameterTypeListDirectDeclarator :: getName() {
	string ret = "";
	if (params != NULL) {ret += "(" + params->getName() + ")";}
	return ret;
}

Declarator :: ~Declarator() {
	if (dirDecl != NULL) {delete dirDecl;}
	//if (pointer != NULL) {delete pointer;}
}

string Declarator :: getName() {
	string ret = "";
	if (pointer != NULL) {
		ret += pointer->getName() + " ";
	}
	if( dirDecl != NULL) {ret += dirDecl->getName();}
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

string Pointer :: getName() {
	string ret = "*";
	if (typeQualList != NULL) {ret += typeQualList->getName();}
	if (next != NULL) {ret += next->getName();}
	return ret;
}
Pointer :: ~Pointer() {
	if (typeQualList != NULL) {delete typeQualList;}
	if (next != NULL) {delete next;}
}

/*TranslationUnit* Parser :: parseTranslationUnit() {
	list<ExternalDeclaration*> list;
	ExternalDeclaration* ptr;
	while ((ptr = parseExternalDeclaration())) {
		list.push_back(ptr);
	}
	return new TranslationUnit(list);
}*/

ExternalDeclaration* Parser :: parseExternalDeclaration() {
	/* First, attempt to parse as a declaration, using the trailing ';' to 
	 * determine if there was a match. If no match, reset the buffered source 
	 * to its previous state and parse as a function declaration
	 */
	ExternalDeclaration* ret = NULL;
	unsigned int bufferUsed = source->bufferSize();
	DeclarationSpecifierList* declSpecList = parseDeclarationSpecifierList();
	InitDeclaratorList* initDeclList = parseInitDeclaratorList();
	Token token = source->peek();
	if (token.getName() == ";") {
		source->get();
		Declaration* decl = new Declaration(declSpecList, initDeclList);
		ret = new ExternalDeclaration(decl);
	} else {
		//Not a declaration, try a function definition instead
		source->setUsed(bufferUsed);
		FunctionDefinition* funcDef = parseFunctionDefinition();
		ret = new ExternalDeclaration(funcDef);
	}
	return ret;
}

FunctionDefinition* Parser :: parseFunctionDefinition() {
	FunctionDefinition* ret = NULL;
	DeclarationSpecifierList* declSpecList = parseDeclarationSpecifierList();
	if (declSpecList == NULL) {
		return NULL;
	}
	Declarator* decl = parseDeclarator();
	if (decl == NULL) {
		return NULL;
	}
	DeclarationList* declList = NULL;
	Token token = source->peek();
	if (token.getName() != "{") {
		declList = parseDeclarationList();
	}
	CompoundStatement* state = parseCompoundStatement();
	ret = new FunctionDefinition(declSpecList, decl, declList, state);
	return ret;
}

Statement* Parser :: parseStatement() {
	Token current = source->peek();
	string currentName = current.getName();
	if (currentName == "goto" || currentName == "return" || currentName == "break"\
			|| currentName == "continue") {
		return parseJumpStatement();
	} else if (currentName == "if" || currentName == "switch") {
		return parseSelectionStatement();
	} else if (currentName == "for" || currentName == "do" || \
			currentName == "while") {
		return parseIterationStatement();
	} else if (currentName == "case" || currentName == "default" || \
			source->peek(1).getName() == ":") {
		return parseLabeledStatement();
	} else if (currentName == "\{") {
		return parseCompoundStatement();
	} else if (currentName == ";") {
		//Empty ExpressionStatement, OK, since the actual expression is optional
		source->get();
		return new ExpressionStatement(NULL);
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
	BlockItem* ret = NULL;
	unsigned int bufferUsed = source->bufferSize();
	Declaration* decl = NULL;
	try {
		decl = parseDeclaration();
	} catch (ExpressionException) {
		decl = NULL;
	} catch (DirectDeclaratorException) {
		decl = NULL;
	}
	Statement* state = NULL;
	unsigned int declUsed = source->bufferSize();
	unsigned int stateUsed = 0;
	source->setUsed(bufferUsed);
	try {
		state = parseStatement();
		stateUsed = source->bufferSize();
	} catch (ExpressionException) {
		state = NULL;
	}
	if (state != NULL && decl != NULL) {
		//Both matches, compare length
		if (state->getName().length() > decl->getName().length()) {
			source->setUsed(stateUsed);
			ret = new BlockItem(state);
		} else {
			source->setUsed(declUsed);
			ret = new BlockItem(decl);
		}
	} else if (decl != NULL) {
		source->setUsed(declUsed);
		ret = new BlockItem(decl);
	} else if (state != NULL) {
		source->setUsed(stateUsed);
		ret = new BlockItem(state);
	} else {
		source->setUsed(bufferUsed);
		ret = NULL;
	}
	return ret;
}


ExpressionStatement* Parser :: parseExpressionStatement() {
	Expression* expr = NULL;
	expr = parseExpression();
	Token after = source->get();
	if (after.getName() != ";") {
		return new ExpressionStatement(NULL);
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

LabeledStatement* Parser :: parseLabeledStatement() {
	LabeledStatement* ret = NULL;
	Token first = source->get();
	if (first.getKey() == KEYWORD || first.getKey() == IDENTIFIER) {
		Expression* constExpr = NULL;
		if (source->peek().getName() == ":") {
			//No constant expression
		} else {
			constExpr = parseExpression(CONDITIONAL);
			if (source->peek().getName() != ":") {
				return NULL;
			}
		}
		source->get(); //Eat ':' token
		Statement* state = parseStatement();
		ret = new LabeledStatement(first, constExpr, state);
	}
	return ret;
}

IterationStatement* Parser :: parseIterationStatement() {
	Token token = source->peek();
	IterationStatement* ret = NULL;
	if (token.getName() == "while") {
		ret = parseWhileStatement();
	} else if (token.getName() == "do") {
		ret = parseDoWhileStatement();
	} else if (token.getName() == "for") {
		ret = parseForStatement();
	}
	return ret;
}

WhileStatement* Parser :: parseWhileStatement() {
	Token token = source->get();
	if (token.getName() != "while") {
		string err = "Expected 'while'";
		throw new SyntaxException(err);
	}
	token = source->get();
	if (token.getName() != "(") {
		string err = "Expected '('";
		throw new SyntaxException(err);
	}
	Expression* expr = parseExpression();
	token = source->get();
	if (token.getName() != ")") {
		string err = "Expected ')'";
		throw new SyntaxException(err);
	}
	Statement* state = parseStatement();
	return new WhileStatement(expr, state);
}

DoWhileStatement* Parser :: parseDoWhileStatement() {
	Token token = source->get();
	if (token.getName() != "do") {
		string err = "Expected 'do'";
		throw new SyntaxException(err);
	}
	Statement* state = parseStatement();
	token = source->get();
	if (token.getName() != "while") {
		string err = "Expected 'while'";
		throw new SyntaxException(err);
	}
	token = source->get();
	if (token.getName() != "(") {
		string err = "Expected '('";
		throw new SyntaxException(err);
	}
	Expression* expr = parseExpression();
	token = source->get();
	if (token.getName() != ")") {
		string err = "Expected ')'";
		throw new SyntaxException(err);
	}
	return new DoWhileStatement(expr, state);
}

ForStatement* Parser :: parseForStatement() {
	Token token = source->get();
	if (token.getName() != "for") {
		string err = "Expected 'for'";
		throw new SyntaxException(err);
	}
	token = source->get();
	if (token.getName() != "(") {
		string err = "Expected '('";
		throw new SyntaxException(err);
	}
	token = source->peek();
	Expression* first = NULL;
	if (token.getName() == ";") {
		//Omitted first expression
		source->get();
	} else {
		first = parseExpression(); //TODO:support declarations here
	}
	token = source->get();
	if (token.getName() != ";") {
		string err = "Expected ';'";
		throw new SyntaxException(err);
	}
	token = source->peek();
	Expression* second = NULL;
	if (token.getName() == ";") {
		//Omitted second expression is replaced with non-zero constant
		//Here, non-zero means 1
		source->get();
		second = new IdentifierExpression(new Constant(Token(token.getPosition(), "1", CONSTANT)));
	} else {
		second = parseExpression();
	}
	token = source->get();
	Expression* third = NULL;
	if (token.getName() == ")") {
		//Omitted third expression
		source->get();
	} else {
		third = parseExpression();
	}
	token = source->get();
	if (token.getName() != ")") {
		string err = "Expected ')'";
		throw new SyntaxException(err);
	}
	Statement* state = NULL;
	state = parseStatement();
	return new ForStatement(first, second, third, state);
}

DeclarationSpecifierList* Parser :: parseDeclarationSpecifierList() {
	DeclarationSpecifierList* ret = NULL;
	DeclarationSpecifier* current;
	current = parseDeclarationSpecifier();
	if (current != NULL) {
		//Recursion!
		ret = new DeclarationSpecifierList(current, parseDeclarationSpecifierList());
	} else {
		ret = NULL;
	}
	return ret;
}

DeclarationSpecifier* Parser :: parseDeclarationSpecifier() {
	//TODO: Implement struct-or-union-specifiers
	DeclarationSpecifier* ret = NULL;
	//Storage class specifiers
	Token token = source->peek();
	auto search = this->mStorageClassSpecifier.find(token.getName());
	if (search != this->mStorageClassSpecifier.end()) {
		return parseStorageClassSpecifier();
	}
	//Function specifiers
	search = this->mFunctionSpecifier.find(token.getName());
	if (search != this->mFunctionSpecifier.end()) {
		return parseFunctionSpecifier();
	}
	//Type Qualifiers
	search = this->mTypeQualifier.find(token.getName());
	if (search != this->mTypeQualifier.end()) {
		if (token.getName() == "_Atomic") {
			if (source->peek(1).getName() == "(") {
			} else {
				return parseTypeQualifier();
			}
		} else {
			return parseTypeQualifier();
		}
	}
	//Type Specifiers
	search = this->mTypeSpecifier.find(token.getName());
	if (search != this->mTypeSpecifier.end()) {
		if (token.getName() == "_Atomic") {
			//If _Atomic, make there's a pair of parenthesis after,
			//otherwise do nothing (is TypeQualifier instead)
			if (source->peek(1).getName() == "(") {
				return parseTypeSpecifier();
			}
		} else {
			return parseTypeSpecifier();
		}
	}
	//Alignment Specifiers
	search = this->mAlignmentSpecifier.find(token.getName());
	if (search != this->mAlignmentSpecifier.end()) {
		return parseAlignmentSpecifier();
	}
	return ret;

}

StorageClassSpecifier* Parser :: parseStorageClassSpecifier() {
	StorageClassSpecifier* ret = NULL;
	Token token = source->get();
	auto search = this->mStorageClassSpecifier.find(token.getName());
	if (search != this->mStorageClassSpecifier.end()) {
		ret = new StorageClassSpecifier(token);
		ret->parse(this);
	}
	return ret;
}

TypeSpecifier* Parser :: parseTypeSpecifier() {
	//TODO: Add typedef'd types
	TypeSpecifier* ret = NULL;
	Token token = source->get();
	auto search = this->mTypeSpecifier.find(token.getName());
	if (search != this->mTypeSpecifier.end()) {
		if (token.getName() == "_Atomic") {
			ret = new AtomicTypeSpecifier(token);
		} else if (token.getName() == "enum") {
			ret = new EnumTypeSpecifier(token);
		} else if (token.getName() == "struct" || token.getName() == "union") {

		} else {
			ret = new TypeSpecifier(token);
		}
	}
	if (ret != NULL) {ret->parse(this);}
	return ret;
}

EnumeratorList* Parser :: parseEnumeratorList() {
	EnumeratorList* ret = NULL;
	Token token = source->get();
	if (token.getName() != "," && token.getName() != "}") {
		ret = new EnumeratorList(new Enumerator(token), parseEnumeratorList());
	} else if (token.getName() == ",") {
		ret = parseEnumeratorList();
	} else if (token.getName() == "}") {
		ret = NULL;
	}
	return ret;
}

TypeQualifier* Parser :: parseTypeQualifier() {
	TypeQualifier* ret = NULL;
	Token token = source->get();
	auto search = mTypeQualifier.find(token.getName());
	if (search != mTypeQualifier.end()) {
		ret = new TypeQualifier(token);
	}
	return ret;
}

FunctionSpecifier* Parser :: parseFunctionSpecifier() {
	FunctionSpecifier* ret = NULL;
	Token token = source->get();
	auto search = mFunctionSpecifier.find(token.getName());
	if (search != mFunctionSpecifier.end()) {
		ret = new FunctionSpecifier(token);
	}
	return ret;
}

AlignmentSpecifier* Parser :: parseAlignmentSpecifier() {
	AlignmentSpecifier* ret = NULL;
	Token token = source->get();
	auto search = mAlignmentSpecifier.find(token.getName());
	if (search != mAlignmentSpecifier.end()) {
		ret = new AlignmentSpecifier(token);
		ret->parse(this);
	}
	return ret;
}



/* Thanks to 
 * http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
 * for attempting to explain Pratt parsers.
 */
Expression* Parser :: parseExpression() {
	return parseExpression(DEFAULT);
}

Expression* Parser :: parseExpression(PriorityEnum priority) {
	Expression* ret = NULL;
	Token token = source->get();
	auto searchPrefix = mPrefix.find(token.getName());
	Expression* left = NULL;
	if (token.getName() == "(") {
		if (source->peek().getKey() == KEYWORD) {
			//Sould be a type cast
			left = new TypeCast(this);
			left->parse(this);
		} else {
			//Normal parenthesis, look for expression (DEFAULT priority) inside
			left = parseExpression();
			token = source->get();
			if (token.getName() != ")") {
				string err = "Expected ')'";
				throw new SyntaxException(err);
			}
		}
	} else if (searchPrefix != mPrefix.end()) {
		//Found current token as prefix operator
			left = searchPrefix->second(this);
			left->parse(this);
	} else if (token.getKey() == KEYWORD) {
		left = new KeywordExpression(token);
	} else if (token.getKey() == IDENTIFIER) {
		left = new IdentifierExpression(token);
	} else if (token.getKey() == CONSTANT) {
		left = new ConstantExpression(token);
	} else if (token.getKey() == STRINGLITERAL) {
		left = new StringLiteralExpression(token);
	} else {
		string err = "Could not parse '" + token.getName() + "'";
	}
	
	token = source->peek();
	auto searchInfix = mInfix.find(token.getName());
	if (searchInfix != mInfix.end()) {
		//Found current token as infix operator
		Expression* dummy = searchInfix->second(this, NULL);
		//Reset buffer after unecessary parsing performed by the initializer of dummy
		while (searchInfix != mInfix.end() && \
				priority < dummy->getPriority()) {
			source->get(); //Actually consume the peeked token
			left = searchInfix->second(this, left);
			left->parse(this);
			token = source->peek();
			searchInfix = mInfix.find(token.getName());
			delete dummy;
			if (searchInfix != mInfix.end()) {
				dummy = searchInfix->second(this,NULL);
			}
		}
		ret = left;
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
	Declarator* ret = NULL;
	Pointer* ptr = NULL;
	if (source->peek().getName() == "*") {
		ptr = parsePointer();
	}
	DirectDeclarator* dirDecl = NULL;
	try {
		dirDecl = parseDirectDeclarator();
		ret = new Declarator(dirDecl, ptr);
	} catch (DirectDeclaratorException& error) {
		string err = "Could not parse direct declarator in declarator";
		throw new DeclaratorException(err);
	}
	return ret;
}

Pointer* Parser :: parsePointer() {
	Pointer* ret = NULL;
	Token token = source->peek();
	if (token.getName() != "*") {
		return NULL;
	}
	source->get();
	unsigned int bufferUsed = source->bufferSize();
	TypeQualifierList* typeQualList = parseTypeQualifierList();
	if (typeQualList == NULL) {
		source->setUsed(bufferUsed);
	}
	Pointer* next = parsePointer();
	ret = new Pointer(typeQualList, next);
	return ret;
}

TypeQualifierList* Parser :: parseTypeQualifierList() {
	TypeQualifierList* ret = NULL;
	TypeQualifier* item = parseTypeQualifier();
	if (item == NULL) {
		ret = NULL;
	} else {
		TypeQualifierList* next = parseTypeQualifierList();
		ret = new TypeQualifierList(item, next);
	}
	return ret;
}

DirectDeclarator* Parser :: parseDirectDeclarator() {
	//TODO: Expand when DirectDeclarator has been expanded
	DirectDeclarator* ret = NULL;
	Token peek = source->peek();
	if (peek.getName() == "(") {
		//Can be Declarator, ParameterTypeList, or IdentifierList
		//Currently only Declarator is implemented
		source->get();
		unsigned int bufferBefore = source->bufferSize();
		Declarator* decl = parseDeclarator();
		unsigned int declUsed = source->bufferSize();
		if (decl == NULL) {
			declUsed = bufferBefore;
		}
		
		source->setUsed(bufferBefore);
		ParameterTypeList* paramTypeList = parseParameterTypeList();
		unsigned int paramUsed = source->bufferSize();
		if (paramTypeList == NULL) {
			paramUsed = bufferBefore;
		}

		source->setUsed(bufferBefore);
		IdentifierList* idList = parseIdentifierList();
		unsigned int idListUsed = source->bufferSize();
		if (idList == NULL) {
			idListUsed = bufferBefore;
		}
		
		if (declUsed > paramUsed) {
			if (declUsed > idListUsed) {
				ret = new DeclaratorDirectDeclarator(decl);
				source->setUsed(declUsed);
			} else {
				ret = new IdentifierListDirectDeclarator(idList);
				source->setUsed(idListUsed);
			}
		} else {
			if (paramUsed > idListUsed) {
				ret = new ParameterTypeListDirectDeclarator(paramTypeList);
				source->setUsed(paramUsed);
			} else {
				ret = new IdentifierListDirectDeclarator(idList);
				source->setUsed(idListUsed);
			}
		}
		if (source->get().getName() != ")") {
			//string err = "Expected ')'";
			//throw new SyntaxException(err);
			ret = NULL;
		}
	} else if (peek.getKey() == IDENTIFIER) {
		Identifier* id = parseIdentifier();
		DirectDeclarator* next = parseDirectDeclarator();
		ret = new IdentifierDirectDeclarator(id, next);
	} else {
		ret = NULL;
	}
	return ret;
}

IdentifierList* Parser :: parseIdentifierList() {
	IdentifierList* ret = NULL;
	Identifier* id = NULL;
	//To avoid SyntaxException("Expected identifier")
	if (source->peek().getKey() == IDENTIFIER) {
		id = parseIdentifier();
	}
	if (id == NULL) {
		ret = NULL;
	} else {
		if (source->peek().getName() == ",") {
			source->get();
			ret = new IdentifierList(id, parseIdentifierList());
		} else {
			ret = new IdentifierList(id);
		}
	}
	return ret;
}

ParameterTypeList* Parser :: parseParameterTypeList() {
	ParameterTypeList* ret = NULL;
	ParameterList* paramList = parseParameterList();
	if (source->peek().getName() == ",") {
		source->get();
		int i = 0;
		while (source->get().getName() == "." && i < 3) {
			++i;
		}
		//If some dot not found
		if (i < 3) {
			string err = "Expected '...'";
			throw new SyntaxException(err);
		}
		ret = new ParameterTypeList(paramList, true);
	} else {
		ret = new ParameterTypeList(paramList, false);
	}
	if (paramList == NULL) {
			ret = NULL;
	}
	return ret;
}

ParameterList* Parser :: parseParameterList() {
	ParameterList* ret = NULL;
	ParameterDeclaration* paramDecl = NULL;
	paramDecl = parseParameterDeclaration();
	if (paramDecl == NULL) {
		ret = NULL;
	} else {
		if (source->peek().getName() == ",") {
			source->get();
			ret = new ParameterList(paramDecl, parseParameterList());
		} else {
			ret = new ParameterList(paramDecl);
		}
	}
	return ret;
}

ParameterDeclaration* Parser :: parseParameterDeclaration() {
	ParameterDeclaration* ret = NULL;
	DeclarationSpecifierList* declSpecList = parseDeclarationSpecifierList();
	Declarator* decl = parseDeclarator();
	ret = new ParameterDeclaration(declSpecList, decl);
	return ret;
}

Declaration* Parser :: parseDeclaration() {
	Declaration* ret = NULL;
	//First, a specifier list
	DeclarationSpecifierList* declList = parseDeclarationSpecifierList();
	InitDeclaratorList* initList = NULL;
	try {
		initList = parseInitDeclaratorList();
		Token token = source->peek(); //TODO: Check if this is okay
		if (token.getName() != ";") {
			return NULL;
		} else {
			source->get();
		}
	} catch (InitDeclaratorListException) {
		//No problem, since the list is optional
		ret = new Declaration(declList);
	}
	if (declList == NULL && initList == NULL) {
		ret = NULL;
	} else {
		ret = new Declaration(declList, initList);
	}
	return ret;
}

DeclarationList* Parser :: parseDeclarationList() {
	DeclarationList* list = NULL;
	Declaration* item = parseDeclaration();
	if (item != NULL) {
		list = new DeclarationList(item, parseDeclarationList());
	}
	return list;
}

InitDeclaratorList* Parser :: parseInitDeclaratorList() {
	InitDeclarator* first = NULL;
	try {
		first = parseInitDeclarator();
		Token after = source->peek();
		if (after.getName() == ",") {
			source->get();
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

void Parser :: c11Operators() {
	//Prefix
	this->mPrefix["+"] = (Operator* (*)(Parser*)) UnaryPlus::create;
	this->mPrefix["-"] = (Operator* (*)(Parser*)) UnaryMinus::create;
	this->mPrefix["&"] = (Operator* (*)(Parser*)) Reference::create;
	this->mPrefix["*"] = (Operator* (*)(Parser*)) Indirection::create;
	this->mPrefix["~"] = (Operator* (*)(Parser*)) BitwiseNOT::create;
	this->mPrefix["!"] = (Operator* (*)(Parser*)) LogicalNOT::create;
	this->mPrefix["++"] = (Operator* (*)(Parser*)) IncrementPrefix::create;
	this->mPrefix["--"] = (Operator* (*)(Parser*)) DecrementPrefix::create;
	this->mPrefix["sizeof"] = (Operator* (*)(Parser*)) Sizeof::create;
	this->mPrefix["_Alignof"] = (Operator* (*)(Parser*)) Alignof::create;
	this->mPrefix["_Generic"] = (Operator* (*)(Parser*)) Generic::create;
		
	//Postfix
	this->mInfix["++"] = (InfixOperator* (*)(Parser*, Expression*)) IncrementPostfix::create;
	this->mInfix["--"] = (InfixOperator* (*)(Parser*, Expression*)) DecrementPostfix::create;
	this->mInfix["->"] = (InfixOperator* (*)(Parser*, Expression*)) StructureDereference::create;
	this->mInfix["."] = (InfixOperator* (*)(Parser*, Expression*)) StructureReference::create;
	
	
	//Binary
	this->mInfix[","] = (InfixOperator* (*)(Parser*, Expression*)) Comma::create;
	this->mInfix["="] = (InfixOperator *(*)(Parser *, Expression *)) StandardAssignment::create;
	this->mInfix["*="] = (InfixOperator *(*)(Parser *, Expression *)) MultiplicationAssignment::create;
	this->mInfix["/="] = (InfixOperator *(*)(Parser *, Expression *)) DivisionAssignment::create;
	this->mInfix["%="] = (InfixOperator *(*)(Parser *, Expression *)) ModuloAssignment::create;
	this->mInfix["+="] = (InfixOperator *(*)(Parser *, Expression *)) AdditionAssignment::create;
	this->mInfix["-="] = (InfixOperator *(*)(Parser *, Expression *)) SubtractionAssignment::create;
	this->mInfix["<<="] = (InfixOperator *(*)(Parser *, Expression *)) LeftShiftAssignment::create;
	this->mInfix[">>="] = (InfixOperator *(*)(Parser *, Expression *)) RightShiftAssignment::create;
	this->mInfix["&="] = (InfixOperator *(*)(Parser *, Expression *)) AddressAssignment::create;
	this->mInfix["^="] = (InfixOperator *(*)(Parser *, Expression *)) XORAssignment::create;
	this->mInfix["|="] = (InfixOperator *(*)(Parser *, Expression *)) ORAssignment::create;
	this->mInfix["||"] = (InfixOperator *(*)(Parser *, Expression *)) LogicalOR::create;
	this->mInfix["&&"] = (InfixOperator *(*)(Parser *, Expression *)) LogicalAND::create;
	this->mInfix["|"] = (InfixOperator *(*)(Parser *, Expression *)) BitwiseOR::create;
	this->mInfix["^"] = (InfixOperator *(*)(Parser *, Expression *)) BitwiseXOR::create;
	this->mInfix["&"] = (InfixOperator *(*)(Parser *, Expression *)) BitwiseAND::create;
	this->mInfix["=="] = (InfixOperator *(*)(Parser *, Expression *)) Equality::create;
	this->mInfix["!="] = (InfixOperator *(*)(Parser *, Expression *)) NonEquality::create;
	this->mInfix["<"] = (InfixOperator *(*)(Parser *, Expression *)) LessThan::create;
	this->mInfix[">"] = (InfixOperator *(*)(Parser *, Expression *)) GreaterThan::create;
	this->mInfix["<="] = (InfixOperator *(*)(Parser *, Expression *)) LessThanOrEqual::create;
	this->mInfix[">="] = (InfixOperator *(*)(Parser *, Expression *)) GreaterThanOrEqual::create;
	this->mInfix["<<"] = (InfixOperator *(*)(Parser *, Expression *)) ShiftLeft::create;
	this->mInfix[">>"] = (InfixOperator *(*)(Parser *, Expression *)) ShiftRight::create;
	this->mInfix["+"] = (InfixOperator *(*)(Parser *, Expression *)) Addition::create;
	this->mInfix["-"] = (InfixOperator *(*)(Parser *, Expression *)) Subtraction::create;
	this->mInfix["*"] = (InfixOperator *(*)(Parser *, Expression *)) Multiplication::create;
	this->mInfix["/"] = (InfixOperator *(*)(Parser *, Expression *)) Division::create;
	this->mInfix["%"] = (InfixOperator *(*)(Parser *, Expression *)) Modulo::create;
	this->mInfix["["] = (InfixOperator *(*)(Parser *, Expression *)) ArraySubscript::create;
	this->mInfix["("] = (InfixOperator *(*)(Parser *,Expression*)) FunctionCall::create; //This is for function calls, type casts are handled specially along with parenthesis
	
	//Ternary
	this->mInfix["?"] = (InfixOperator *(*)(Parser *, Expression *)) ConditionalExpression::create;
}

void Parser :: declarationSpecifiers() {
	//Storage class specifiers
	this->mStorageClassSpecifier["typedef"] = "typedef";
	this->mStorageClassSpecifier["extern"] = "extern";
	this->mStorageClassSpecifier["static"] = "static";
	this->mStorageClassSpecifier["_Thread_local"] = "_Thread_local";
	this->mStorageClassSpecifier["auto"] = "auto";
	this->mStorageClassSpecifier["register"] = "register";

	//Type class specifiers
	this->mTypeSpecifier["v"] = "v";
	this->mTypeSpecifier["void"] = "void";
	this->mTypeSpecifier["char"] = "char";
	this->mTypeSpecifier["short"] = "short";
	this->mTypeSpecifier["int"] = "int";
	this->mTypeSpecifier["long"] = "long";
	this->mTypeSpecifier["float"] = "float";
	this->mTypeSpecifier["double"] = "double";
	this->mTypeSpecifier["signed"] = "signed";
	this->mTypeSpecifier["unsigned"] = "unsigned";
	this->mTypeSpecifier["_Bool"] = "_Bool";
	this->mTypeSpecifier["_Complex"] = "_Complex";
	this->mTypeSpecifier["_Atomic"] = "_Atomic";
	this->mTypeSpecifier["struct"] = "struct";
	this->mTypeSpecifier["union"] = "union";
	this->mTypeSpecifier["enum"] = "enum";
	//Typedefs are missing here, since they are only a generic identifier
	
	//Type qualifiers
	this->mTypeQualifier["const"] = "const";
	this->mTypeQualifier["restrict"] = "restrict";
	this->mTypeQualifier["volatile"] = "volatile";
	this->mTypeQualifier["_Atomic"] = "_Atomic";

	//Function specifiers
	this->mTypeSpecifier["inline"] = "inline";
	this->mTypeSpecifier["_Noreturn"] = "_Noreturn";

	//Alignment specifiers
	this->mAlignmentSpecifier["_Alignas"] = "_Alignas";
}

