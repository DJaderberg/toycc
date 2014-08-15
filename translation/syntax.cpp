#include "syntax.h"

string ExternalDeclaration :: getName() {
	string ret = "";
	if (funcDef != NULL) {ret += funcDef->getName();}
	if (decl != NULL) {ret += decl->getName();}
	return ret;
}

string ExternalDeclaration :: genLLVM(Scope* s, Consumer<string>* o) {
	if (funcDef != NULL) {return funcDef->genLLVM(s, o);}
	if (decl != NULL) {
		//TODO: Implement global declarations
		return decl->genLLVM(s, o);
	}
	return "";
}

string CompoundStatement :: genLLVM(Scope* s, Consumer<string>* o, ParameterTypeListDirectDeclarator* paramDirDecl) {
			if  (paramDirDecl != NULL) {
				if (ParameterTypeList* paramTypeList = paramDirDecl->getParams()) {
					o->put("{");
					TypeList* typeList = paramTypeList->getTypes(s);
					list<string>* nameList = new list<string>();
					paramTypeList->getNames(s, nameList);
					string curTypeName = "";
					unsigned int iter = 1;
					while (typeList != NULL && !nameList->empty()) {
						curTypeName = typeList->getItem()->getLLVMName();
						o->put("\n%" + nameList->front() + " = alloca ");
						o->put(curTypeName);
						o->put("\nstore " + curTypeName + " %__arg_");
						o->put(to_string(iter) + ", " + curTypeName);
						o->put("* %" + nameList->front() + "\n");
						nameList->pop_front();
						typeList = typeList->getNext();
						++iter;
					}
					//Handle 'void' argument
					if (typeList != NULL && typeList->getItem() != NULL && \
							typeList->getItem()->getLLVMName() == "void") {
						typeList = NULL;
						o->put("\n");
					}
					if (typeList != NULL || !nameList->empty()) {
						string err = "Mismatched number of types and names";
						throw new TypeError(err);
					}
					Scope* localScope = new Scope(s);
					itemList->genLLVM(localScope, o);
					delete localScope;
					o->put("\n}\n");
					return "";
				}
		}
		return this->genLLVM(s, o);
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

string ParameterDeclaration :: genLLVM(Scope* s, Consumer<string>* o) {
	if (declSpecList != NULL) {
		//Should never need scope in getting type
		Type* declSpecType = declSpecList->getType(s);
		string typeName = declSpecType->getLLVMName();
		if (typeName != "void") {
			//A parameter of 'void' in C is nothing in LLVM IR, eg.
			//C: int main(void)
			//becomes
			//LLVM IR: i64 @main()
			//So, do not enter anything if typeName is void
			o->put(typeName);
		}
		delete declSpecType;
	}
	if (decl != NULL) {o->put(" %__arg_" + to_string(s->getTemp()));}
	return "";
}

Type* ParameterDeclaration :: getType(Scope* s) {
	return declSpecList->getType(s);
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

bool BlockItem :: typeCheck(Scope* s) {
	if (decl != NULL) {return decl->typeCheck(s);}
	if (state != NULL) {return state->typeCheck(s);}
	return false;
}

Type* BlockItem :: getType(Scope* s) {
	if (decl != NULL) {return decl->getType(s);}
	if (state != NULL) {return state->getType(s);}
	return new NoType();
}

string BlockItem :: genLLVM(Scope* s, Consumer<string>* o) {
	if (decl != NULL) {return decl->genLLVM(s, o);}
	if (state != NULL) {return state->genLLVM(s, o);}
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

TranslationUnit* Parser :: parseTranslationUnit() {
	TranslationUnit* ret = NULL;
	ExternalDeclaration* decl = NULL;
	decl = parseExternalDeclaration();
	if (decl == NULL) {
		return NULL;
	} else {
		TranslationUnit* next = parseTranslationUnit();
		ret = new TranslationUnit(decl, next);
	}
	return ret;
}

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
		if (funcDef != NULL) {
			ret = new ExternalDeclaration(funcDef);
		}
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
		if (stateUsed >= declUsed) {
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
		//Struct or Union Specifiers
		} else if (token.getName() == "struct") {
			source->get();
			ret = new StructSpecifier(token);
			ret->parse(this);
			return ret;
		} else if (token.getName() == "union") {
			source->get();
			ret = new UnionSpecifier(token);
			ret->parse(this);
			return ret;
		} else {
			return parseTypeSpecifier();
		}
	}
	//Alignment Specifiers
	search = this->mAlignmentSpecifier.find(token.getName());
	if (search != this->mAlignmentSpecifier.end()) {
		return parseAlignmentSpecifier();
	}

	//Struct or Union Specifiers
	if (token.getName() == "struct") {
		ret = new StructSpecifier(token);
		ret->parse(this);
		return ret;
	} else if (token.getName() == "union") {
		ret = new UnionSpecifier(token);
		ret->parse(this);
		return ret;
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

DeclarationSpecifier* Parser :: parseStructOrUnionSpecifier() {
	DeclarationSpecifier* ret = NULL;
	Token token = source->get();
	if (token.getName() == "struct") {
		ret = new StructSpecifier(token);
	} else if (token.getName() == "union") {
		ret = new UnionSpecifier(token);
	} else {
		ret = NULL;
	}
	if (ret != NULL) {
		ret->parse(this);
	} return ret;
}

SpecifierQualifierList* Parser :: parseSpecifierQualifierList() {
	SpecifierQualifierList* ret = NULL;
	SpecifierQualifierList* next = NULL;
	Token token = source->peek();
	auto search = mTypeSpecifier.find(token.getName());
	if (search != mTypeSpecifier.end()) {
		source->get();
		TypeSpecifier* spec = new TypeSpecifier(token);
		next = parseSpecifierQualifierList();
		return new SpecifierQualifierList(spec, next);
	}
	search = mTypeQualifier.find(token.getName());
	if (search != mTypeQualifier.end()) {
		source->get();
		TypeQualifier* qual = new TypeQualifier(token);
		next = parseSpecifierQualifierList();
		return new SpecifierQualifierList(qual, next);
	}
	return ret;
}

StructDeclarator* Parser :: parseStructDeclarator() {
	Declarator* decl = parseDeclarator();
	Expression* expr = NULL;
	if (source->peek().getName() == ":") {
		source->get();
		expr = parseExpression(CONDITIONAL);
	} else if (decl == NULL) {
		return NULL;
	}
	return new StructDeclarator(decl, expr);
}

StructDeclaratorList* Parser :: parseStructDeclaratorList() {
	StructDeclaratorList* ret = NULL;
	StructDeclarator* item = parseStructDeclarator();
	if (item == NULL) {
		ret = NULL;
	} else {
		StructDeclaratorList* next = parseStructDeclaratorList();
		ret = new StructDeclaratorList(item, next);
	}
	return ret;
}

StructDeclaration* Parser :: parseStructDeclaration() {
	SpecifierQualifierList* specQualList = parseSpecifierQualifierList();
	StructDeclaratorList* structDeclList = parseStructDeclaratorList();
	if (source->peek().getName() == ";") {
		source->get();
		return new StructDeclaration(specQualList, structDeclList);
	} else if (specQualList == NULL) {
		return NULL;
	}
	return new StructDeclaration(specQualList, structDeclList);
}

StructDeclarationList* Parser :: parseStructDeclarationList() {
	StructDeclarationList* ret = NULL;
	StructDeclaration* item = parseStructDeclaration();
	if (item == NULL) {
		ret = NULL;
	} else {
		StructDeclarationList* next = parseStructDeclarationList();
		ret = new StructDeclarationList(item, next);
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
		if (dirDecl == NULL) {
			ret = NULL;
		} else {
			ret = new Declarator(dirDecl, ptr);
		}
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
	InitDeclarator* ret = NULL;
	Declarator* decl = NULL;
	Initializer* init = NULL;
	try {
		decl = parseDeclarator();
	} catch (DeclaratorException) {
		string err = "Could not parse declarator in init declarator";
		throw new InitDeclaratorException(err);
	}
	if (source->peek().getName() == "=") {
		source->get();
		init = parseInitializer();
	}
	ret = new InitDeclarator(decl, init);
	return ret;
}

Initializer* Parser :: parseInitializer() {
	//TODO: Implement initializer-lists
	Expression* expr = parseExpression(ASSIGNMENT);
	return new Initializer(expr);
}

ArgumentList* Parser :: parseArgumentList() {
	Expression* expr = parseExpression(ASSIGNMENT);
	if (source->peek().getName() == ",") {
		source->get();
		return new ArgumentList(expr, parseArgumentList());
	} else {
		return new ArgumentList(expr, NULL);
	}
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













//Type checking


bool ExternalDeclaration :: typeCheck(Scope* s) {
	if (funcDef != NULL) return funcDef->typeCheck(s);
	if (decl != NULL) return decl->typeCheck(s);
	return false;
}

Type* DeclarationSpecifierList :: getType(Scope* s) {
	Type* ret = NULL;
	string basicTypeName = "";
	DeclarationSpecifier* curItem = item;
	NodeList<DeclarationSpecifier>* curNext = next;
	TypeSpecifier* curTypeSpec = NULL;
	//Add all type specifiers to the string
	while ((curTypeSpec = dynamic_cast<TypeSpecifier*>(curItem))) {
		basicTypeName += curTypeSpec->getName();
		if (curNext != NULL) {
			curItem = curNext->getItem();
			curNext = curNext->getNext();
			basicTypeName += " ";
		} else {
			break;
		}
	}
	auto search = mBasicTypes.find(basicTypeName);
	if (search != mBasicTypes.end()) {
		ret = new BasicType(search->second);
	} else if (basicTypeName == "void") {
		ret = new NoType();
	}
	return ret;
}

map<string, BasicTypeEnum> DeclarationSpecifierList :: mBasicTypes \
		= DeclarationSpecifierList::initBasicTypesMap();

map<string, BasicTypeEnum> DeclarationSpecifierList :: initBasicTypesMap() {
	map<string, BasicTypeEnum> ret = {\
		{"_Bool", _BOOL}, {"char", CHAR}, {"signed char", SIGNED_CHAR}, \
		{"unsigned char", UNSIGNED_CHAR}, {"short int", SHORT_INT}, \
		{"unsigned short int", UNSIGNED_SHORT_INT}, {"int", INT}, \
		{"unsigned int", UNSIGNED_INT}, {"long int", LONG_INT}, \
		{"unsigned long int", UNSIGNED_LONG_INT}, \
		{"long long int", LONG_LONG_INT}, \
		{"unsigned long long int", UNSIGNED_LONG_LONG_INT}, \
		{"float", FLOAT}, {"double", DOUBLE}, {"long double", LONG_DOUBLE}, \
		{"short", SHORT_INT}, {"signed short", SHORT_INT}, \
		{"signed short int", SHORT_INT}, {"unsigned short", UNSIGNED_SHORT_INT},\
		{"signed", INT}, {"signed int", INT}, {"unsigned", UNSIGNED_INT}, \
		{"long", LONG_INT}, {"signed long", LONG_INT}, \
		{"signed long int", LONG_INT}, {"unsigned long", UNSIGNED_LONG_INT}, \
		{"long long", LONG_LONG_INT}, {"signed long long", LONG_LONG_INT}, \
		{"signed long long int", LONG_LONG_INT}\
	};
	return ret;
}

bool Declarator ::  insert(Scope* s, Type* t) {
	Pointer* ptrTmp = pointer;
	while (ptrTmp != NULL) {
		t = new PointerType(t);
		ptrTmp = ptrTmp->getNext();
	}
	return dirDecl->insert(s, t);
}

Type* Constant :: getType(string str) {
	string::size_type search = str.find('.');
	if (search != string::npos) {
		//double
		return new BasicType(DOUBLE);
	} else {
		//int
		return new BasicType(INT);
	}
}


