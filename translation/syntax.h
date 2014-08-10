//#include "source.h"
#include "abstractSyntax.h"

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
class PrefixOperator : public Operator {
	public:
		PrefixOperator(Parser* parser) \
			: Operator(parser, opStrTemp, prioTemp) {}
		virtual ~PrefixOperator() {delete expr;}
		string getName() {
			string ret = "";
			ret += Operator::getName();
			if (expr != NULL) {ret += expr->getName();}
			return ret;
		}
		Type* getType(Scope* s) {
			if (opStr == "&") {
				return new PointerType(expr->getType(s));
			} else if (opStr == "*") {
				//Magic wand of dereference, downcast expr->getType() to a 
				//PointerType and return pointeeType
				if (PointerType* downcast = \
						dynamic_cast<PointerType*>(expr->getType(s))) {
					return downcast->getPointeeType();
				} else {
					string err = "Dereference (prefix operator '*' requires a "\
								  "pointer type";
					throw new TypeError(err);
				}
			}
			return expr->getType(s);
		}
		void parse(Parser* parser);
	private:
		Op* expr = NULL; //The item to operate on
};

class InfixOperator : public Operator {
	public:
		InfixOperator(Parser* parser, const string* opStr, PriorityEnum prio) \
			: Operator(parser, opStr, prio) {}
		InfixOperator(Parser* parser, const string* opStr, PriorityEnum prio, \
				string LLVMOpStr) \
			: Operator(parser, opStr, prio, LLVMOpStr) {}
		virtual ~InfixOperator() {}
};

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
class PostfixOperator : public InfixOperator {
	public:
		PostfixOperator(Parser* parser, Expression* expr) \
			: InfixOperator(parser, opStrTemp, prioTemp), expr(expr) {}
		virtual ~PostfixOperator() {}
		string getName() {
			string ret = "";
			if (expr != NULL) {ret += expr->getName();}
			ret += Operator::getName();
			return ret;
		}
		void parse(Parser* parser);
	protected:
		Op* expr = NULL;
};

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
class BinaryOperator : public InfixOperator {
	public:
		/*BinaryOperator(Operator* rhs, Operator* lhs, string opStr,\
			   	PriorityEnum prio) : Operator(opStr, prio), rhs(rhs), \
									 lhs(lhs) {}*/
		BinaryOperator(Parser* parser, Op* lhs) : \
			InfixOperator(parser, opStrTemp, prioTemp), lhs(lhs) {}
		BinaryOperator(Parser* parser, Op* lhs, string LLVMOpStr) : \
			InfixOperator(parser, opStrTemp, prioTemp, LLVMOpStr), lhs(lhs) {}
		virtual ~BinaryOperator() {
			if (rhs != NULL) {delete rhs;}
			if (lhs != NULL) {delete lhs;}
		}
		string getName() {
			//TODO: Remove unnecessary parenthesis
			string ret = "";
			if (lhs !=NULL) {
				if (this->getPriority() < lhs->getPriority()) {
					ret += lhs->getName();
				} else {
					ret += "(";
					ret += lhs->getName();
					ret += ")";
				}
			}
			ret += Operator::getName();
			if (rhs !=NULL) {
				if (this->getPriority() < rhs->getPriority()) {
					ret += rhs->getName();
				} else {
					ret += "(";
					ret += rhs->getName();
					ret += ")";
				}
			}
			return ret;
		}
		virtual bool typeCheck(Scope* s) {
			Type* lhsT = lhs->getType(s);
			Type* rhsT = rhs->getType(s);
			if (lhsT == NULL || rhsT == NULL) {
				return false;
			}
			if (*lhsT != *rhsT) {
				string err = "Mismatched types '" + lhsT->getName() + "' and '" + \
							  rhsT->getName() + "' in binary operator " + \
							  this->getName();
				throw new TypeError(err);
			}
			return rhs->typeCheck(s) && lhs->typeCheck(s);
		}
		virtual Type* getType(Scope* s) {
			return lhs->getType(s);
		}
		virtual void genLLVM(Scope* s, Consumer<string>* o) {
			o->put("dunno binary op");
		}
		virtual void genLLVM(Scope* s, Consumer<string>* o, string result);
		void parse(Parser* parser);
	protected:
		Op* rhs = NULL; //The right hand side 
		Op* lhs = NULL; //The left hand side
};

template<class Left, class Middle, class Right, const string* opStrFirst, const string* opStrSecond, PriorityEnum prioTemp>
class TernaryOperator : public InfixOperator {
	public:
		TernaryOperator(Parser* parser, Left* lhs) : \
			InfixOperator(parser, opStrFirst, prioTemp), lhs(lhs) {}
		virtual ~TernaryOperator() {
			if (lhs != NULL) {delete lhs;}
			if (mhs != NULL) {delete mhs;}
			if (rhs != NULL) {delete rhs;}
		}
		string getName() {
			//TODO: Remove unnecessary parenthesis
			string ret = "";
			if (lhs != NULL) {
				if (this->getPriority() < lhs->getPriority()) {
					ret += lhs->getName();
				} else {
					ret += "(";
					ret += lhs->getName();
					ret += ")";
				}
			}
			ret += Operator::getName();
			if (mhs != NULL) {
				if (this->getPriority() < mhs->getPriority()) {
					ret += mhs->getName();
				} else {
					ret += "(";
					ret += mhs->getName();
					ret += ")";
				}
			}
			ret += *opStrSecond;
			if (rhs != NULL) {
				if (this->getPriority() < rhs->getPriority()) {
					ret += rhs->getName();
				} else {
					ret += "(";
					ret += rhs->getName();
					ret += ")";
				}
			}
			return ret;
		}
		void parse(Parser* parser);
	protected:
		Left* lhs = NULL;
		Middle* mhs = NULL;
		Right* rhs = NULL;
};

class TranslationUnit;
class ExternalDeclaration;
class Statement;
class CompoundStatement;
class ExpressionStatement;
class SelectionStatement;
class JumpStatement;
class LabeledStatement;
class IterationStatement;
class WhileStatement;
class DoWhileStatement;
class ForStatement;
class Identifier;
class IdentifierList;
class BlockItem;
class BlockItemList;
class Declaration;
class DeclarationList;
class InitDeclarator;
class InitDeclaratorList;
class Declarator;
class DirectDeclarator;
class Initializer;
class Pointer;
class Expression;
class IdentifierExpression;
class GenericAssociationList;
class GenericAssociation;
class DeclarationSpecifier;
class DeclarationSpecifierList;
class StorageClassSpecifier;
class TypeSpecifier;
class TypeQualifier;
class FunctionSpecifier;
class AlignmentSpecifier;
class Enumerator;
class EnumeratorList;
class FunctionDefinition;
class ParameterTypeList;
class ParameterList;
class ParameterDeclaration;
class TypeQualifierList;
class StructDeclarationList;
class StructDeclaration;
class SpecifierQualifierList;
class StructDeclaratorList;
class StructDeclarator;


//Constructs an AST from the input Tokens
class Parser {
	public:
		Parser(BufferedSource<Token>* source) : source(source) {
			this->c11Operators(); //Enter Operators
			this->declarationSpecifiers(); //Enter c11 declaration specifiers
		}
		BufferedSource<Token>* getSource() {return this->source;}
		TranslationUnit* parseTranslationUnit();
		ExternalDeclaration* parseExternalDeclaration();
		Statement* parseStatement();
		CompoundStatement* parseCompoundStatement();
		ExpressionStatement* parseExpressionStatement();
		SelectionStatement* parseSelectionStatement();
		JumpStatement* parseJumpStatement();
		LabeledStatement* parseLabeledStatement();
		IterationStatement* parseIterationStatement();
		WhileStatement* parseWhileStatement();
		DoWhileStatement* parseDoWhileStatement();
		ForStatement* parseForStatement();
		Expression* parseExpression();
		Expression* parseExpression(PriorityEnum priority);
		Identifier* parseIdentifier();
		BlockItemList* parseBlockItemList();
		BlockItem* parseBlockItem();
		Declarator* parseDeclarator();
		DirectDeclarator* parseDirectDeclarator();
		InitDeclarator* parseInitDeclarator();
		InitDeclaratorList* parseInitDeclaratorList();
		Initializer* parseInitializer();
		Declaration* parseDeclaration();
		DeclarationList* parseDeclarationList();
		DeclarationSpecifierList* parseDeclarationSpecifierList();
		DeclarationSpecifier* parseDeclarationSpecifier();
		StorageClassSpecifier* parseStorageClassSpecifier();
		TypeSpecifier* parseTypeSpecifier();
		TypeQualifier* parseTypeQualifier();
		FunctionSpecifier* parseFunctionSpecifier();
		AlignmentSpecifier* parseAlignmentSpecifier();
		EnumeratorList* parseEnumeratorList();
		FunctionDefinition* parseFunctionDefinition();
		ParameterTypeList* parseParameterTypeList();
		ParameterList* parseParameterList();
		ParameterDeclaration* parseParameterDeclaration();
		IdentifierList* parseIdentifierList();
		Pointer* parsePointer();
		TypeQualifierList* parseTypeQualifierList();
		DeclarationSpecifier* parseStructOrUnionSpecifier();
		StructDeclarationList* parseStructDeclarationList();
		StructDeclaration* parseStructDeclaration();
		SpecifierQualifierList* parseSpecifierQualifierList();
		StructDeclaratorList* parseStructDeclaratorList();
		StructDeclarator* parseStructDeclarator();
		map<string, string> mStorageClassSpecifier;
		map<string, string> mTypeSpecifier;
		map<string, string> mTypeQualifier;
		map<string, string> mFunctionSpecifier;
		map<string, string> mAlignmentSpecifier;
		map<string, Operator* (*) (Parser*)> mPrefix; 
		//Maps string to pointer to function which returns type Expression* and
		//takes types Parser*, Expression* as input
		map<string, InfixOperator* (*) (Parser*, Expression*)> mInfix; 
		//Maps string to pointer to function which returns type Expression* and
		//takes types Parser*, Expression* as input
		void c11Operators(); //Inserts operators to parse C11 into the maps
		void declarationSpecifiers(); //Inserts C11 declaration specifiers
	private:
		BufferedSource<Token>* source;
};

class Identifier : public Node {
	public:
		Identifier(Token token) : token(token) {}
		string getName() {return token.getName();}
		virtual ~Identifier() {}
		bool typeCheck(Scope* s) {return true;}
		Type* getType(Scope* s) {
			auto tempSearch = s->find(this->getName());
			Type* search = NULL;
			if (tempSearch != NULL) {
				search = tempSearch->getType();
			}
			if (search == NULL) {
				return new NoType();
			} 
			return search;
		}
	protected:
		Token token;
};

class IdentifierList : public NodeList<Identifier> {
	public:
		IdentifierList(Identifier* item) : NodeList(item) {}
		IdentifierList(Identifier* item, IdentifierList* next) \
			: NodeList(item, next) {}
		string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += ", " + next->getName();}
			return ret;
		}
};

class Keyword : public Identifier {
	public:
		Keyword(Token token) : Identifier(token) {}
		virtual ~Keyword() {}
};

class Constant : public Identifier {
	public:
		Constant(Token token) : Identifier(token) {}
		virtual ~Constant() {}
		Type* getType(Scope* s) {return this->getType(token.getName());}
		static Type* getType(string str); //ints and doubles only
		bool typeCheck(Scope* s) {return true;}
		void genLLVM(Scope* s, Consumer<string>* o) {
			//TODO: Fix for weird constants
			o->put(this->getName());
		}
};

class EnumerationConstant : public Constant {
	public:
		EnumerationConstant(Token token) : Constant(token) {}
		virtual ~EnumerationConstant() {}
};

class StringLiteral : public Identifier {
	public:
		StringLiteral(Token token) : Identifier(token) {}
		virtual ~StringLiteral() {}
};

class IdentifierExpression : public Expression {
	public:
		IdentifierExpression(Token token) : Expression(PRIMARY), \
											identifier(new Identifier(token)) {}
		IdentifierExpression(Identifier* identifier) : Expression(PRIMARY), \
													   identifier(identifier) {}
		virtual ~IdentifierExpression() {
			if (identifier != NULL) {delete identifier;}
		}
		string getName() {return identifier->getName();}
		void parse(Parser* parser) {}
		bool typeCheck(Scope* s) {return identifier->typeCheck(s);}
		Type* getType(Scope* s) {return identifier->getType(s);}
	private:
		Identifier* identifier = NULL;
};

class KeywordExpression : public Expression {
	public:
		KeywordExpression(Token token) : Expression(PRIMARY), \
										 keyword(new Keyword(token)) {}
		KeywordExpression(Keyword* keyword) : Expression(PRIMARY), \
											  keyword(keyword) {}
		virtual ~KeywordExpression() {
			if (keyword != NULL) {delete keyword;}
		}
		string getName() {return keyword->getName();}
		void parse(Parser* parser) {}
	private:
		Keyword* keyword = NULL;
};

class ConstantExpression : public Expression {
	public:
		ConstantExpression(Token token) : Expression(PRIMARY), \
										 constant(new Constant(token)) {}
		ConstantExpression(Constant* constant) : Expression(PRIMARY), \
											  constant(constant) {}
		virtual ~ConstantExpression() {
			if (constant != NULL) {delete constant;}
		}
		string getName() {return constant->getName();}
		void parse(Parser* parser) {}
		Type* getType(Scope* s) {return constant->getType(s);}
		bool typeCheck(Scope* s) {return constant->typeCheck(s);}
		void genLLVM(Scope* s, Consumer<string>* o) {
			if (constant != NULL) {
				constant->genLLVM(s, o);
			}
		}
	private:
		Constant* constant = NULL;
};

class StringLiteralExpression : public Expression {
	public:
		StringLiteralExpression(Token token) : Expression(PRIMARY), \
										 stringliteral(new StringLiteral(token)) {}
		StringLiteralExpression(StringLiteral* stringliteral) : Expression(PRIMARY), \
											  stringliteral(stringliteral) {}
		virtual ~StringLiteralExpression() {
			if (stringliteral != NULL) {delete stringliteral;}
		}
		string getName() {return stringliteral->getName();}
		void parse(Parser* parser) {}
	private:
		StringLiteral* stringliteral = NULL;
};

class ExpressionException : public SyntaxException {
	public:
		ExpressionException(string w) : SyntaxException(w) {}
		ExpressionException(char* w) : SyntaxException(w) {}
};

class ExternalDeclaration : public Node {
	public:
		ExternalDeclaration(FunctionDefinition* funcDef) : funcDef(funcDef) {}
		ExternalDeclaration(Declaration* decl) : decl(decl) {}
		string getName();
		bool typeCheck(Scope* s);
		void genLLVM(Scope* s, Consumer<string>* o);
	private:
		FunctionDefinition* funcDef = NULL;
		Declaration* decl = NULL;
};

class TranslationUnit : public NodeList<ExternalDeclaration> {
	public:
		TranslationUnit(ExternalDeclaration* decl, TranslationUnit* next) : \
			NodeList(decl, next) {}
		TranslationUnit(ExternalDeclaration* decl) : NodeList(decl) {}
};

class Statement : public Node {
	public:
		Statement() {}
		virtual ~Statement() {}
};

class BlockItem : public Node {
	public:
		BlockItem(Declaration* decl) : decl(decl) {}
		BlockItem(Statement* state) : state(state) {}
		virtual ~BlockItem();
		string getName();
		bool typeCheck(Scope* s); 
		Type* getType(Scope* s);
		void genLLVM(Scope* s, Consumer<string>* o);
	private:
		Declaration* decl = NULL;
		Statement* state = NULL;
};

class BlockItemList : public NodeList<BlockItem> {
	public:
		BlockItemList(BlockItem* item) : NodeList(item) {}
		BlockItemList(BlockItem* item, BlockItemList* next) : NodeList(item, next) {}
		string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += "\n" + next->getName();}
			return ret;
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			if (item != NULL) {item->genLLVM(s, o);}
			if (next != NULL) {
				o->put("\n");
				next->genLLVM(s, o);
			}
		}
};

class BlockItemListException : public SyntaxException {
	public:
		BlockItemListException(string w) : SyntaxException(w) {}
		BlockItemListException(char *w) : SyntaxException(w) {}
};

class CompoundStatement : public Statement {
	public:
		CompoundStatement() : itemList(NULL) {}
		CompoundStatement (BlockItemList* itemList) : itemList(itemList) {}
		virtual ~CompoundStatement() {delete itemList;}
		string getName() {
			string ret = "\{\n";
			if (itemList != NULL) {
				ret += itemList->getName();
			}
			ret += "\n}";
			return ret;
		}
		virtual Type* getType(Scope* s) {
			return new NoType();
		}
		virtual bool typeCheck(Scope* s) {
			Scope* localScope = new Scope(s);
			bool ret = itemList->typeCheck(localScope);
			delete localScope;
			return ret;
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			o->put("{\n");
			itemList->genLLVM(s, o);
			o->put("}\n");
		}
		string getLLVMName() const {
			string ret = "{\n";
			if (itemList != NULL) {ret += itemList->getLLVMName();}
			ret += "}\n";
			return ret;
		}
	private:
		BlockItemList* itemList; //Optional, may be NULL

};

class ExpressionStatement : public Statement {
	public:
		ExpressionStatement(Expression* expr) : expression(expr)  {}
		virtual ~ExpressionStatement() {delete expression;}
		string getName() {
			string ret = "";
			if (expression != NULL) {ret += expression->getName();}
			ret += ";";
			return ret;
		}
		virtual Type* getType(Scope* s) {
			return expression->getType(s);
		}
		virtual bool typeCheck(Scope* s) {
			return expression->typeCheck(s);
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			if (expression != NULL) {
				expression->genLLVM(s, o);
			}
		}
	private:
		Expression* expression = NULL; //Optional
};

class SelectionStatement : public Statement {
	public:
		SelectionStatement(string keyword, Expression* expr, Statement* state,\
				Statement* stateOpt = NULL) : keyword(keyword), expr(expr), \
			state(state), stateOpt(stateOpt) {}
		string getName() {
			string temp = keyword + " (";
			temp += expr->getName();
			temp += ") ";
			temp += state->getName();
			if (stateOpt != NULL) {
				temp += " else ";
				temp += stateOpt->getName();
			}
			return temp;
		}
		virtual ~SelectionStatement() {
			if (expr != NULL) {delete expr;}
			if (state != NULL) {delete state;}
			if (stateOpt != NULL) {delete stateOpt;}
		}
	private:
		string keyword;
		Expression* expr;
		Statement* state;
		Statement* stateOpt;
};

class JumpStatement : public Statement {
	public:
		JumpStatement(string keyword) : keyword(keyword) {} \
			//continue, break, return (no expr)
		JumpStatement(Identifier* id) : keyword("goto"), id(id) {} \
		//goto
		JumpStatement(Expression* expr) : keyword("return"), expr(expr) {} \
		//return (with expr)
		string getName() {
			string temp = keyword;
			if (id != NULL) {
				temp += " ";
				temp += id->getName();
			}
			if (expr != NULL) {
				temp += " ";
				temp += expr->getName();
			}
			temp += ";";
			return temp;
		}
		virtual ~JumpStatement() {
			if (id != NULL) {delete id;}
			if (expr != NULL) {delete expr;}
		}
		Type* getType(Scope* s) {
			if (expr != NULL) {return expr->getType(s);}
			return new NoType();
		}
		bool typeCheck(Scope* s) {
			if (expr == NULL) {
				return true;
			}
			if (keyword == "return") {
				Type* search = s->find("return")->getType();
				return *search == *expr->getType(s);
			} else {
				return true;
			}
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			//TODO: Implement all keywords
			if (keyword == "return") {
				if (expr != NULL) {
					Type* exprType = expr->getType(s);
					if (exprType->getLLVMName() != "void") {
						string retOp = "%" + to_string(s->getTemp());
						expr->genLLVM(s, o, retOp);
						o->put("ret " + exprType->getLLVMName() + " " + retOp + \
								"\n");
					}
				}
			}
		}

	private:
		string keyword;
		Identifier* id = NULL;
		Expression* expr = NULL;
};

class LabeledStatement : public Statement {
	public:
		LabeledStatement(Token first, Statement* state) : first(first), \
														  state(state) {}
		LabeledStatement(Token first, Expression* expr, Statement* state) \
		: first(first),  constExpr(expr), state(state) {}
		string getName() {
			string ret = first.getName();
			ret += " ";
			if (constExpr != NULL) {ret += constExpr->getName();}
			ret += " : ";
			if (state != NULL) {ret += state->getName();}
			return ret;
		}
	private:
		Token first; //Should be 'case', 'default' or an identifier
		Expression* constExpr = NULL; //Optional
		Statement* state = NULL;
};

class WhileStatement;
class DoWhileStatement;
class ForStatement;

class IterationStatement : public Statement {
};

class WhileStatement : public IterationStatement {
	public:
		WhileStatement(Expression* expr, Statement* state) : expr(expr), \
															 state(state) {}
		string getName() {
			string ret = "while (";
			if (expr != NULL) {ret += expr->getName();}
			ret += ")";
			if (state != NULL) {ret += state->getName();}
			return ret;
		}
	private:
		Expression* expr = NULL;
		Statement* state = NULL;
};

class DoWhileStatement : public IterationStatement {
	public:
		DoWhileStatement(Expression* expr, Statement* state) : expr(expr), \
															 state(state) {}
		string getName() {
			string ret = "do ";
			if (state != NULL) {ret += state->getName();}
			ret += " while (";
			if (expr != NULL) {ret += expr->getName();}
			ret += ")";
			return ret;
		}
	private:
		Expression* expr = NULL;
		Statement* state = NULL;
};

class ForStatement : public IterationStatement {
	public:
		ForStatement(Expression* first, Expression* second, Expression* third, \
				Statement* state) : first(first), second(second), third(third),\
									state(state) {}
		string getName() {
			string ret = "for (";
			if (first != NULL) {ret += first->getName();}
			ret += "; ";
			if (second != NULL) {ret += second->getName();}
			ret += "; ";
			if (third != NULL) {ret += third->getName();}
			ret += ") ";
			if (state != NULL) {ret += state->getName();}
			return ret;
		}
	private:
		Expression* first = NULL;
		Expression* second = NULL;
		Expression* third = NULL;
		Statement* state = NULL;
};

class Pointer : public Node {
	public:
		Pointer(TypeQualifierList* typeQualList, Pointer* next) : \
			typeQualList(typeQualList), next(next) {}
		Pointer(TypeQualifierList* typeQualList) : \
			typeQualList(typeQualList), next(NULL) {}
		string getName();
		virtual ~Pointer();
		Pointer* getNext() {return next;}
	private:
		TypeQualifierList* typeQualList = NULL;
		Pointer* next = NULL;
};

class Declarator : public Node {
	public:
		Declarator(DirectDeclarator* dirDecl) : dirDecl(dirDecl), pointer(NULL) {}
		Declarator(DirectDeclarator* dirDecl, Pointer* pointer) : dirDecl(dirDecl), pointer(pointer) {}
		virtual ~Declarator(); 
		string getName();
		Pointer* getPointer() {return pointer;}
		DirectDeclarator* getDirectDeclarator() {return dirDecl;}
		bool insert(Scope* s, Type* t);
	private:
		DirectDeclarator* dirDecl = NULL;
		Pointer* pointer = NULL;
};

class DeclaratorException : public SyntaxException {
	public:
		DeclaratorException(string w) : SyntaxException(w) {}
		DeclaratorException(char * w) : SyntaxException(w) {}
};

//! Syntactic element direct-declarator
/*!
 * Each instance of DirectDeclarator is an element in a singly linked list,
 * using the member 'next'. Each instance also holds an entry in the list, 
 * which can be one of a few different things:
 * 1. An Identifier
 * 2. A Declarator within an enclosing pair of parenthesis
 * 3. A ParameterTypeList within an enclosing pair of parenthesis
 * 4. An optional IdentifierList within an enclosing pair of parenthesis
 * 5-8. A TypeQualifierList and AssignmentExpression within an enclosing pair of brackets.
 * Only 1 and 2 may occur at any point in the list, all others must be at the 
 * end of the list.
 * TODO: Implement 5-8
 */
class DirectDeclarator : public Node {
	public:
		DirectDeclarator() : next(NULL) {}
		DirectDeclarator(DirectDeclarator* next) : next(next) {}
		virtual ~DirectDeclarator() {
			if (next != NULL) {delete next;}
		}
		virtual string getName() {
			string ret = "";
			if (next != NULL) {ret = next->getName();}
			return ret;
		}
		virtual bool insert(Scope* s, Type* t) {return false;}
		DirectDeclarator* getNext() {return next;}
	protected:
		DirectDeclarator* next = NULL;
};

class IdentifierDirectDeclarator : public DirectDeclarator {
	public:
		IdentifierDirectDeclarator(Identifier* id) : \
			DirectDeclarator(NULL), id(id) {}
		IdentifierDirectDeclarator(Identifier* id, DirectDeclarator* dirDecl) : \
			DirectDeclarator(dirDecl), id(id) {}
		string getName() {
			string ret = "";
			if (id != NULL) {ret += id->getName();}
			if (next != NULL) {ret += next->getName();}
			return ret;
		}
		virtual ~IdentifierDirectDeclarator() {
			if (id != NULL) {delete id;}
		}
		bool insert(Scope* s, Type* t) {
			if (id != NULL) {
				s->insert(id->getName(), t);
				return true;
			} else {
				return false;
			}
		}
		Identifier* getIdentifier() {return id;}
	private:
		Identifier* id = NULL;
};

class DeclaratorDirectDeclarator : public DirectDeclarator {
	public:
		DeclaratorDirectDeclarator(Declarator* decl) : decl(decl) {}
		DeclaratorDirectDeclarator(Declarator* decl, DirectDeclarator* next) : \
			DirectDeclarator(next), decl(decl) {}
		string getName() {
			string ret = "";
			if (decl != NULL) {ret += "(" + decl->getName() + ")";}
			if (next != NULL) {ret += next->getName();}
			return ret;
		}
		virtual ~DeclaratorDirectDeclarator() {
			if (decl != NULL) {delete decl;}
		}
	private:
		Declarator* decl = NULL;
};


class ParameterDeclaration : public Node {
	//TODO: Implement abstract declarators
	public:
		ParameterDeclaration(DeclarationSpecifierList* declSpecList, Declarator* decl) : declSpecList(declSpecList), decl(decl) {}
		virtual ~ParameterDeclaration();
		string getName();
		Type* getType(Scope* s);
		Declarator* getDeclarator() {return decl;}
		void genLLVM(Scope* s, Consumer<string>* o);
	private:
		DeclarationSpecifierList* declSpecList = NULL;
		Declarator* decl = NULL;
};

class ParameterList : public NodeList<ParameterDeclaration> {
	public:
		ParameterList(ParameterDeclaration* item, ParameterList* list) \
			: NodeList(item, list) {}
		ParameterList(ParameterDeclaration* item) : NodeList(item) {}
		string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += ", " + next->getName();}
			return ret;
		}
		void getNames(Scope* s, list<string>* ret) {
			NodeList<ParameterDeclaration>* tempList = this;
			ParameterDeclaration* tempItem = item;
			while (tempItem != NULL) {
				Declarator* itemDecl = item->getDeclarator();
				if (itemDecl != NULL) {
					ret->push_back(item->getDeclarator()->getName());
				} else {
					//TODO: Check that the type is void
				}
				tempList = tempList->getNext();
				if (tempList != NULL) {
					tempItem = tempList->getItem();
				} else {
						tempItem = NULL;
					}
			}
		}
		//Inserts each element into the scope s with the correct type
		/* Expects that typeList and nameList contain the same number of 
		 * elements.
		 */
		static bool enterTypes(Scope* s, TypeList* typeList, list<string>* nameList) {
			if (typeList == NULL && nameList->empty()) {
				return true;
			} else if (typeList == NULL || nameList->empty()) {
				return false;
			} else {
				bool temp = s->insert(nameList->front(), typeList->getItem());
				nameList->pop_front();
				return temp && enterTypes(s, typeList->getNext(), nameList);
			}
		}
};

class ParameterTypeList : public Node {
	public:
		ParameterTypeList(ParameterList* paramList) : paramList(paramList) {}
		ParameterTypeList(ParameterList* paramList, bool hasTrailing) \
			: paramList(paramList), hasTrailing(hasTrailing) {}
		virtual ~ParameterTypeList() {
			if (paramList != NULL) {delete paramList;}
		}
		string getName() {
			string ret = "";
			if (paramList != NULL) {ret += paramList->getName();}
			if (hasTrailing) {ret += ", ...";}
			return ret;
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			if (paramList != NULL) {paramList->genLLVM(s, o);}
			if (hasTrailing) {o->put(", ...");}
		}
		TypeList* getTypes(Scope* s) {return paramList->getTypes(s);}
		void getNames(Scope* s, list<string>* ret) {
			paramList->getNames(s, ret);
		}
	private:
		ParameterList* paramList = NULL;
		bool hasTrailing = false; //If list ends with ', ...'
};

class ParameterTypeListDirectDeclarator : public DirectDeclarator {
	public:
		ParameterTypeListDirectDeclarator(ParameterTypeList* params) : params(params) {}
		virtual ~ParameterTypeListDirectDeclarator();
		string getName();
		ParameterTypeList* getParams() {return params;}
		void genLLVM(Scope* s, Consumer<string>* o) {
			if (params != NULL) {params->genLLVM(s, o);}
		}
	private:
		ParameterTypeList* params = NULL;
};

class IdentifierListDirectDeclarator : public DirectDeclarator {
	public:
		IdentifierListDirectDeclarator() : idList(NULL) {}
		IdentifierListDirectDeclarator(IdentifierList* idList) \
			: idList(idList) {}
		string getName() {
			string ret = "(";
			if (idList != NULL) {ret += idList->getName();}
			ret += ")";
			return ret;
		}
	private:
		IdentifierList* idList = NULL;
};

class DirectDeclaratorException : public SyntaxException {
	public:
		DirectDeclaratorException(string w) : SyntaxException(w) {}
		DirectDeclaratorException(char * w) : SyntaxException(w) {}
};

class Initializer : public Node {
	//TODO: Implement Initializer lists
	public:
		Initializer(Expression* expr) : expr(expr) {}
		string getName() {
			string ret = "";
			if (expr != NULL) {ret += expr->getName();}
			return ret;
		}
		virtual ~Initializer() {
			if (expr != NULL) {delete expr;}
		}
	private:
		Expression* expr = NULL;
};

class InitDeclarator : public Node {
	public:
		InitDeclarator(Declarator* dec) : dec(dec), init(NULL) {}
		InitDeclarator(Declarator* dec, Initializer* init) : dec(dec), \
															 init(init) {}
		virtual ~InitDeclarator() {
			if (dec != NULL) {delete dec;}
			if (init != NULL) {delete init;}
		}
		string getName() {
			string ret = "";
			if (dec != NULL) {ret += dec->getName();}
			if (init != NULL) {ret += " = " + init->getName();}
			return ret;
		}
		bool insert(Scope* s, Type* type) {
			//TODO: Initialize variable as well
			return dec->insert(s, type);
		}
	private:
		Declarator* dec;
		Initializer* init;
};

class InitDeclaratorException : public SyntaxException {
	public:
		InitDeclaratorException(string w) : SyntaxException(w) {}
		InitDeclaratorException(char * w) : SyntaxException(w) {}
};

class InitDeclaratorList : public NodeList<InitDeclarator> {
	public:
		InitDeclaratorList(InitDeclarator* item) : NodeList(item) {}
		InitDeclaratorList(InitDeclarator* item, InitDeclaratorList* next) \
			: NodeList(item, next) {}
		string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += ", " + next->getName();}
			return ret;
		}
};

class InitDeclaratorListException : public SyntaxException {
	public:
		InitDeclaratorListException(string w) : SyntaxException(w) {}
		InitDeclaratorListException(char * w) : SyntaxException(w) {}
};

class DeclarationSpecifier : public Node {
	public:
		DeclarationSpecifier(Token name) : name(name) {}
		virtual void parse(Parser* parser) = 0;
		virtual string getName() {
			return name.getName();
		}
	protected:
		Token name; //typedef, const, int, void, _Atomic, etc.
};

class DeclarationSpecifierList : public NodeList<DeclarationSpecifier> {
	public:
		DeclarationSpecifierList(DeclarationSpecifier* item, DeclarationSpecifierList* next) : NodeList(item, next) {
			if (mBasicTypes.empty()) {
				initBasicTypesMap();
			}
		}
		string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName() + " ";}
			if (next != NULL) {ret += next->getName();}
			return ret;
		}
		//TODO: Implement getType here, which will be the meat of insertion of 
		//new variables
		Type* getType(Scope* s); //Currently supports basic types
		static map<string, BasicTypeEnum> initBasicTypesMap();
	private:
		static map<string, BasicTypeEnum> mBasicTypes;
};

class Declaration : public Node {
	//TODO: Implement static_assert declarations
	public:
		Declaration() : initList(NULL) {}
		Declaration(InitDeclaratorList* initList) : initList(initList) {}
		Declaration(DeclarationSpecifierList* declList) : declList(declList) {}
		Declaration(DeclarationSpecifierList* declList, InitDeclaratorList* initList) : declList(declList), initList(initList) {}
		virtual ~Declaration() {if (initList != NULL) {delete initList;}}
		string getName() {
			string ret = "";
			if (declList != NULL) {ret += declList->getName();}
			if (initList != NULL) {ret += initList->getName();}
			ret += ";";
			return ret;
		}
		Type* getType(Scope* s) {
			return declList->getType(s);
		}
		bool typeCheck(Scope* s) {
			if (declList == NULL || initList == NULL) {
				return false;
			} else {
				//Make sure we are not redeclaraing anything. 
				//initList->typeCheck(s) will do this for us
				if (!initList->typeCheck(s)) {
					return false;
				} else {
					//Enter all declarations into the current scope
					//First, get the type that they should have
					Type* type = declList->getType(s);
					InitDeclarator* initDecl = NULL;
					NodeList<InitDeclarator>* initListTemp = initList;
					while ((initDecl = initListTemp->getItem()) != NULL && \
							initListTemp->getNext() != NULL)  {
						initDecl-> insert(s, type);
						initListTemp = initListTemp->getNext();
					}
					if ((initDecl = initListTemp->getItem()) != NULL) {
						initDecl-> insert(s, type);
					}
					return true;
				}
			}
		}
	private:
		DeclarationSpecifierList* declList = NULL;
		InitDeclaratorList* initList = NULL;
};

class DeclarationException : public SyntaxException {
	DeclarationException(string w) : SyntaxException(w) {}
	DeclarationException(char *w) : SyntaxException(w) {}
};

class DeclarationList : public NodeList<Declaration> {
	public:
		DeclarationList(Declaration* item) : NodeList(item) {}
		DeclarationList(Declaration* item, DeclarationList* next) \
			: NodeList(item, next) {}
};

class FunctionDefinition : public Node {
	public:
		FunctionDefinition(DeclarationSpecifierList* declSpecList, \
				Declarator* decl, DeclarationList* declList, \
				CompoundStatement* state) \
			: declSpecList(declSpecList), decl(decl), declList(declList), \
			state(state) {}
		FunctionDefinition(DeclarationSpecifierList* declSpecList, \
				Declarator* decl, CompoundStatement* state) \
			: FunctionDefinition(declSpecList, decl, NULL, state) {}
		string getName() {
			string ret = "";
			if (declSpecList != NULL) {ret += declSpecList->getName();}
			if (decl != NULL) {ret += decl->getName();}
			if (declList != NULL) {ret += declList->getName();}
			if (state != NULL) {ret += state->getName();}
			return ret;
		}
		virtual ~FunctionDefinition() {
			if (declSpecList != NULL) {delete declSpecList;}
			if (decl != NULL) {delete decl;}
			if (declList != NULL) {delete declList;}
			if (state != NULL) {delete state;}
		}
		virtual Type* getType(Scope* s) {
			if (typeOfThis == NULL) {
				ParameterTypeListDirectDeclarator* paramDirDecl = NULL;
				if (!(paramDirDecl = dynamic_cast<ParameterTypeListDirectDeclarator*>\
							(decl->getDirectDeclarator()->getNext()))) {
					string err = "Expected parameter type list in function "\
								  "declaration";
					throw new TypeError(err);
				}
				ParameterTypeList* paramList = paramDirDecl->getParams();
				TypeList* paramTypes = NULL;
				list<string>* paramNames = new list<string>();
				if (paramList != NULL) {
					paramTypes = paramList->getTypes(s);
					paramList->getNames(s, paramNames);
					typeOfThis = new FunctionType(declSpecList->getType(s), paramTypes);
					ParameterList::enterTypes(s, paramTypes, \
							paramNames);
				}
			} else {
				typeOfThis = new FunctionType(declSpecList->getType(s), \
						new TypeList(new NoType()));
			}
			return typeOfThis;
		}
		virtual bool typeCheck(Scope* s) {
			//TODO: Improve this
			bool ret = true;
			DirectDeclarator* dirDecl = decl->getDirectDeclarator();
			//Expect that dirDecl is an identifier direct declarator,
			//i.e. the name of the function
			IdentifierDirectDeclarator* idDirDecl = NULL;
			if (!(idDirDecl = dynamic_cast<IdentifierDirectDeclarator*>\
						(dirDecl))) {
					string err = "Expected direct declarator with identifier type"\
								  " in function definition";
					throw new TypeError(err);
			}
			//Fetch name of function
			string name = idDirDecl->getIdentifier()->getName();
			//Add this function to the scope, unless it is already declared
			//Create local scope for the function
			Scope* functionScope = new Scope(s);
			//Insert all parameters into the functionScope
			//Do this by first getting the next DirectDeclarator from the first
			//one and downcast to ParameterTypeListDirectDeclarator
			ParameterTypeListDirectDeclarator* paramDirDecl = NULL;
			if (!(paramDirDecl = dynamic_cast<ParameterTypeListDirectDeclarator*>\
						(dirDecl->getNext()))) {
				string err = "Expected parameter type list in function "\
							  "declaration";
				throw new TypeError(err);
			}
			//Then get all the types from the ParameterTypeList
			ParameterTypeList* paramList = paramDirDecl->getParams();
			TypeList* paramTypes = NULL;
			list<string>* paramNames = new list<string>();
			if (paramList != NULL) {
				paramTypes = paramList->getTypes(functionScope);
				paramList->getNames(functionScope, paramNames);
				typeOfThis = new FunctionType(declSpecList->getType(s), paramTypes);
				ParameterList::enterTypes(functionScope, paramTypes, \
						paramNames);
			} else {
				typeOfThis = new FunctionType(declSpecList->getType(s), \
						new TypeList(new NoType()));
			}
			//Else, no parameters, which is OK
			//And enter the special case 'return' as well
			functionScope->insert("return", declSpecList->getType(s));
			//Seriously not sure what to do with declList, this is my best guess
			if (declList != NULL) {
				ret = ret && declList->typeCheck(functionScope);
			}
			//Just type check the compound statement
			ret = ret && state->typeCheck(functionScope);
			if (ret == false) {
				string err = "Error when type checking function definition of '" +\
							  name + "'";
				throw new TypeError(err);
			}
			return ret;
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			o->put("\n;Function Attrs: ");
			//TODO: Implement function attributes, if necessary
			o->put("\ndefine ");
			FunctionType* type = dynamic_cast<FunctionType*>(this->getType(s));
			Type* returnType = type->getReturnType();
			if (returnType != NULL) {
				o->put(returnType->getLLVMName() + " @");
			} else {
				o->put("void @");
			}
			//Enter parameters, with types and %names
			IdentifierDirectDeclarator* dirDecl \
				= dynamic_cast<IdentifierDirectDeclarator*>\
				(decl->getDirectDeclarator());
			if (dirDecl == NULL) {
				string err = "Expected identifier as function name";
				throw new TypeError(err);
			}
			o->put(dirDecl->getIdentifier()->getName() + " (");
			ParameterTypeListDirectDeclarator* paramDirDecl \
				= dynamic_cast<ParameterTypeListDirectDeclarator*>\
				(dirDecl->getNext());
			if (paramDirDecl == NULL) {
				string err = "Expected parameter list in function declaration";
				throw new TypeError(err);
			}
			paramDirDecl->genLLVM(s, o);
			o->put(") ");
			state->genLLVM(s, o);
		}
	private:
		DeclarationSpecifierList* declSpecList = NULL;
		Declarator* decl = NULL;
		DeclarationList* declList = NULL; //Optional
		CompoundStatement* state = NULL;
		FunctionType* typeOfThis = NULL; //Set when running typeCheck(s)
};

class TypeSpecifier : public DeclarationSpecifier {
	public:
		TypeSpecifier(Token name) : DeclarationSpecifier(name) {}
		virtual void parse(Parser* parser) {}
};

class AtomicTypeSpecifier : public TypeSpecifier {
	public:
		AtomicTypeSpecifier(Token name) : TypeSpecifier(name) {}
		void parse(Parser* parser) {
			Token token = parser->getSource()->get();
			if (token.getName() != "(") {
				string err = "Expected '('";
				throw new SyntaxException(err);
			}
			type = parser->getSource()->get();
			token = parser->getSource()->get();
			if (token.getName() != ")") {
				string err = "Expected ')'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			string ret = TypeSpecifier::getName();
			ret += "(" + type.getName() + ")";
			return ret;
		}
	private:
		Token type;
};

class Enumerator : public Expression {
	public:
		Enumerator(Token enumConst) \
			: Expression(CONDITIONAL), \
			enumConst(new EnumerationConstant(enumConst)) {}
		void parse(Parser* parser) {
			Token token = parser->getSource()->peek();
			if (token.getName() == "=") {
				parser->getSource()->get();
				constExpr = parser->parseExpression(CONDITIONAL);
			}
		}
		string getName() {
			string ret = "";
			if (enumConst != NULL) {ret += enumConst->getName();}
			if (constExpr != NULL) {ret += " = " + constExpr->getName();}
			return ret;
		}
		virtual ~Enumerator() {
			if (enumConst != NULL) {delete enumConst;}
			if (constExpr != NULL) {delete constExpr;}
		}
	private:
		EnumerationConstant* enumConst = NULL;
		Expression* constExpr = NULL;
};

class EnumeratorList : public NodeList<Enumerator> {
	public:
		EnumeratorList(Enumerator* item) : NodeList(item) {}
		EnumeratorList(Enumerator* item, EnumeratorList* next) : \
			NodeList(item, next) {}
		string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += ", " + next->getName();}
			return ret;
		}
};


class EnumTypeSpecifier : public TypeSpecifier {
	public:
		EnumTypeSpecifier(Token name) : TypeSpecifier(name) {}
		void parse(Parser* parser) {
			Token token = parser->getSource()->get();
			if (token.getKey() == IDENTIFIER) {
				id = token;
				usingId = true;
				token = parser->getSource()->get();
			}
			if (token.getName() == "{") {
				list = parser->parseEnumeratorList();
			}
		}
		string getName() {
			string ret = "";
			ret += DeclarationSpecifier::getName();
			if (usingId) {ret += " " + id.getName();}
			if (list != NULL) {ret += "{" + list->getName() + "}";}
			return ret;
		}
	private:
		bool usingId = false;
		Token id;
		EnumeratorList* list = NULL;
};

class StorageClassSpecifier : public DeclarationSpecifier {
	public:
		StorageClassSpecifier(Token name) : DeclarationSpecifier(name) {}
		void parse(Parser* parser) {}
};

class TypeQualifier : public DeclarationSpecifier {
	public:
		TypeQualifier(Token name) : DeclarationSpecifier(name) {}
		virtual void parse(Parser* parser) {}
};

class TypeQualifierList : public NodeList<TypeQualifier> {
	public:
		TypeQualifierList(TypeQualifier* item, TypeQualifierList* next) : \
			NodeList(item, next) {}
		TypeQualifierList(TypeQualifier* item) : NodeList(item) {}
		string getName() {
			string ret = "";
			if (item != NULL) {ret = item->getName();}
			if (next != NULL) {ret = next->getName();}
			return ret;
		}
};

class FunctionSpecifier : public DeclarationSpecifier {
	public:
		FunctionSpecifier(Token name) : DeclarationSpecifier(name) {}
		virtual void parse(Parser* parser) {}
};

class AlignmentSpecifier : public DeclarationSpecifier {
	public:
		AlignmentSpecifier(Token name) : DeclarationSpecifier(name) {}
		void parse(Parser* parser) {
			Token token = parser->getSource()->get();
			if (token.getName() == "(") {
				token = parser->getSource()->peek();
				auto search = parser->mTypeSpecifier.find(token.getName());
				if (search != parser->mTypeSpecifier.end()) {
					parser->getSource()->get();
					type = token;
				} else {
					constExpr = parser->parseExpression(CONDITIONAL);
				}
				token = parser->getSource()->get();
				if (token.getName() != ")") {
					string err = "Expected ')'";
					throw new SyntaxException(err);
				}
			} else {
				string err = "Expected '(' after '_Alignas'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			string ret = DeclarationSpecifier::getName() + "(";
			if (constExpr != NULL) {
				ret += constExpr->getName();
			} else {
				ret += type.getName();
			}
			ret += ")";
			return ret;
		}
	private:
		Token type;
		Expression* constExpr = NULL;
};

class StructDeclarator : public Node {
	public:
		StructDeclarator(Declarator* decl) : decl(decl) {}
		StructDeclarator(Expression* expr) : expr(expr) {}
		StructDeclarator(Declarator* decl, Expression* expr) : decl(decl), \
			expr(expr) {}
		string getName() {
			string ret = "";
			if (decl != NULL) {ret += decl->getName();}
			if (expr != NULL) {ret += ":" + expr->getName();}
			return ret;
		}
	private:
		Declarator* decl = NULL;
		Expression* expr = NULL;
};

class StructDeclaratorList : public NodeList<StructDeclarator> {
	public:
		StructDeclaratorList(StructDeclarator* item) : NodeList(item) {}
		StructDeclaratorList(StructDeclarator* item, StructDeclaratorList* next)\
			: NodeList(item, next) {}
		virtual string getName() {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += next->getName();}
			return ret;
		}
};

class SpecifierQualifierList : public Node {
	public:
		SpecifierQualifierList(TypeSpecifier* spec) : spec(spec) {}
		SpecifierQualifierList(TypeQualifier* qual) : qual(qual) {}
		SpecifierQualifierList(TypeSpecifier* spec, SpecifierQualifierList* next) : spec(spec), next(next) {}
		SpecifierQualifierList(TypeQualifier* qual, SpecifierQualifierList* next) : qual(qual), next(next) {}
		string getName() {
			string ret = "";
			if (spec != NULL) {ret += spec->getName();}
			if (qual != NULL) {ret += qual->getName();}
			if (next != NULL) {ret += " " + next->getName();}
			return ret;
		}
	private:
		TypeSpecifier* spec = NULL;
		TypeQualifier* qual = NULL;
		SpecifierQualifierList* next = NULL;
};

class StructDeclaration : public Node {
	public:
		StructDeclaration(SpecifierQualifierList* specQualList) : \
			specQualList(specQualList) {}
		StructDeclaration(SpecifierQualifierList* specQualList, StructDeclaratorList* structDeclList) : specQualList(specQualList), \
   structDeclList(structDeclList) {}
		string getName() {
			string ret = "";
			if (specQualList != NULL) {ret += specQualList->getName();}
			if (structDeclList != NULL) {
				ret += " " + structDeclList->getName() + ";";
			}
			return ret;
		}
		virtual ~StructDeclaration() {
			if (specQualList != NULL) {delete specQualList;}
			if (structDeclList != NULL) {delete structDeclList;}
		}
	private:
		SpecifierQualifierList* specQualList = NULL;
		StructDeclaratorList* structDeclList = NULL;
};

class StructDeclarationList : public NodeList<StructDeclaration> {
	public:
		StructDeclarationList(StructDeclaration* item) : NodeList(item) {}
		StructDeclarationList(StructDeclaration* item, StructDeclarationList* next) : NodeList(item, next) {}
};

class StructSpecifier : public DeclarationSpecifier {
	public:
		StructSpecifier(Token name) : DeclarationSpecifier(name) {}
		virtual string getName() {
			string ret = DeclarationSpecifier::getName() + " ";
			if (id != NULL) {ret += id->getName();}
			ret += " {\n";
			if (list != NULL) {ret += list->getName();}
			ret += "\n}";
			return ret;
		}
		void parse(Parser* parser) {
			Token token = parser->getSource()->peek();
			if (token.getKey() == IDENTIFIER) {
				id = new Identifier(token);
				parser->getSource()->get();
			}
			token = parser->getSource()->peek();
			if (token.getName() == "{") {
				parser->getSource()->get();
				list = parser->parseStructDeclarationList();
				token = parser->getSource()->get();
				if (token.getName() != "}") {
					string err = "Expected '}' to end struct or union specifier";
					throw new SyntaxException(err);
				}
			}
		}
	private:
		Identifier* id = NULL; //Optional
		StructDeclarationList* list = NULL; //Optional
};

class UnionSpecifier : public StructSpecifier {
	public:
		UnionSpecifier(Token name) : StructSpecifier(name) {}
};










//Operator classes, eg. Addition, Multiplication, etc.

const string commaOpStr = ",";
class Comma : public BinaryOperator<Expression, &commaOpStr, COMMA> {
	public:
		Comma(Parser* parser, Expression* lhs) \
			: BinaryOperator<Expression, &commaOpStr, COMMA>(parser, lhs) {}
		static Comma* create(Parser* parser, Expression* lhs)\
	   	{return new Comma(parser, lhs);}
};

template<const string* opStrTemp>
class Assignment : public BinaryOperator<Expression, opStrTemp, ASSIGNMENT> {
	public:
		Assignment(Parser* parser, Expression* lhs) \
			: BinaryOperator<Expression, opStrTemp, ASSIGNMENT>(parser, lhs) {}
};

const string StandardAssignmentOpStr = "=";
class StandardAssignment : public Assignment<&StandardAssignmentOpStr> {
	public:
		StandardAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static StandardAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new StandardAssignment(parser, lhs);}
		void genLLVM(Scope* s, Consumer<string>* o) {
			//Assume that the name of rhs is an identifier and store the
			//result of rhs there
			if (IdentifierExpression* idDowncast = dynamic_cast<IdentifierExpression*>(rhs)) {
				o->put("%" + lhs->getName() + " = %" + idDowncast->getName());
			} else if (ConstantExpression* constDowncast = \
						dynamic_cast<ConstantExpression*>(rhs)) {
				o->put("%" + lhs->getName() + " = " + constDowncast->getName());
			} else {
				rhs->genLLVM(s, o, "%" + lhs->getName());
			}
		}
};

const string MultiplicationAssignmentOpStr = "*=";
class MultiplicationAssignment : public Assignment<&MultiplicationAssignmentOpStr> {
	public:
		MultiplicationAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static MultiplicationAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new MultiplicationAssignment(parser, lhs);}
};

const string DivisionAssignmentOpStr = "/=";
class DivisionAssignment : public Assignment<&DivisionAssignmentOpStr> {
	public:
		DivisionAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static DivisionAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new DivisionAssignment(parser, lhs);}
};

const string ModuloAssignmentOpStr = "%=";
class ModuloAssignment : public Assignment<&ModuloAssignmentOpStr> {
	public:
		ModuloAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static ModuloAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new ModuloAssignment(parser, lhs);}
};

const string AdditionAssignmentOpStr = "+=";
class AdditionAssignment : public Assignment<&AdditionAssignmentOpStr> {
	public:
		AdditionAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static AdditionAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new AdditionAssignment(parser, lhs);}
};

const string SubtractionAssignmentOpStr = "-=";
class SubtractionAssignment : public Assignment<&SubtractionAssignmentOpStr> {
	public:
		SubtractionAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static SubtractionAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new SubtractionAssignment(parser, lhs);}
};

const string LeftShiftAssignmentOpStr = "<<=";
class LeftShiftAssignment : public Assignment<&LeftShiftAssignmentOpStr> {
	public:
		LeftShiftAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static LeftShiftAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new LeftShiftAssignment(parser, lhs);}
};

const string RightShiftAssignmentOpStr = ">>=";
class RightShiftAssignment : public Assignment<&RightShiftAssignmentOpStr> {
	public:
		RightShiftAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static RightShiftAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new RightShiftAssignment(parser, lhs);}
};

const string AddressAssignmentOpStr = "&=";
class AddressAssignment : public Assignment<&AddressAssignmentOpStr> {
	public:
		AddressAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static AddressAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new AddressAssignment(parser, lhs);}
};

const string XORAssignmentOpStr = "^=";
class XORAssignment : public Assignment<&XORAssignmentOpStr> {
	public:
		XORAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static XORAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new XORAssignment(parser, lhs);}
};

const string ORAssignmentOpStr = "|=";
class ORAssignment : public Assignment<&ORAssignmentOpStr> {
	public:
		ORAssignment(Parser* parser, Expression* lhs) \
			: Assignment(parser, lhs) {}
		static ORAssignment* create(Parser* parser, Expression* lhs)\
	   	{return new ORAssignment(parser, lhs);}
};

const string ConditionalOpStr = "?";
const string SecondConditionalOpStr = ":";
class ConditionalExpression : public TernaryOperator<Expression, Expression, Expression, &ConditionalOpStr, &SecondConditionalOpStr, CONDITIONAL> {
	public:
		ConditionalExpression(Parser* parser, Expression* lhs) \
			: TernaryOperator(parser, lhs) {}
		static ConditionalExpression* create(Parser* parser, Expression* lhs) \
		{return new ConditionalExpression(parser, lhs);}
};

const string LogicalOROpStr = "||";
class LogicalOR : public BinaryOperator<Expression, &LogicalOROpStr, LOGICAL_OR> {
	public:
		LogicalOR(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static LogicalOR* create(Parser* parser, Expression* lhs)\
	   	{return new LogicalOR(parser, lhs);}
};

const string LogicalANDOpStr = "&&";
class LogicalAND : public BinaryOperator<Expression, &LogicalANDOpStr, LOGICAL_AND> {
	public:
		LogicalAND(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static LogicalAND* create(Parser* parser, Expression* lhs)\
	   	{return new LogicalAND(parser, lhs);}
};

const string BitwiseOROpStr = "|";
class BitwiseOR : public BinaryOperator<Expression, &BitwiseOROpStr, BITWISE_OR> {
	public:
		BitwiseOR(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BitwiseOR* create(Parser* parser, Expression* lhs)\
	   	{return new BitwiseOR(parser, lhs);}
};

const string BitwiseXOROpStr = "^";
class BitwiseXOR : public BinaryOperator<Expression, &BitwiseXOROpStr, BITWISE_XOR> {
	public:
		BitwiseXOR(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BitwiseXOR* create(Parser* parser, Expression* lhs)\
	   	{return new BitwiseXOR(parser, lhs);}
};

const string BitwiseANDOpStr = "&";
class BitwiseAND : public BinaryOperator<Expression, &BitwiseANDOpStr , BITWISE_AND> {
	public:
		BitwiseAND(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BitwiseAND* create(Parser* parser, Expression* lhs)\
	   	{return new BitwiseAND(parser, lhs);}
};

const string EqualityOpStr = "==";
class Equality : public BinaryOperator<Expression, &EqualityOpStr, EQUALITY> {
	public:
		Equality(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static Equality* create(Parser* parser, Expression* lhs)\
	   	{return new Equality(parser, lhs);}
};

const string NonEqualityOpStr = "!=";
class NonEquality : public BinaryOperator<Expression, &NonEqualityOpStr, EQUALITY> {
	public:
		NonEquality(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static NonEquality* create(Parser* parser, Expression* lhs)\
	   	{return new NonEquality(parser, lhs);}
};

const string LessThanOpStr = "<";
class LessThan : public BinaryOperator<Expression, &LessThanOpStr, RELATIONAL> {
	public:
		LessThan(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static LessThan* create(Parser* parser, Expression* lhs)\
	   	{return new LessThan(parser, lhs);}
};

const string GreaterThanOpStr = ">";
class GreaterThan : public BinaryOperator<Expression, &GreaterThanOpStr, RELATIONAL> {
	public:
		GreaterThan(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static GreaterThan* create(Parser* parser, Expression* lhs)\
	   	{return new GreaterThan(parser, lhs);}
};

const string LessThanOrEqualOpStr = "<=";
class LessThanOrEqual : public BinaryOperator<Expression, &LessThanOrEqualOpStr, RELATIONAL> {
	public:
		LessThanOrEqual(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static LessThanOrEqual* create(Parser* parser, Expression* lhs)\
	   	{return new LessThanOrEqual(parser, lhs);}
};

const string GreaterThanOrEqualOpStr = ">=";
class GreaterThanOrEqual : public BinaryOperator<Expression, &GreaterThanOrEqualOpStr, RELATIONAL> {
	public:
		GreaterThanOrEqual(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static GreaterThanOrEqual* create(Parser* parser, Expression* lhs)\
	   	{return new GreaterThanOrEqual(parser, lhs);}
};

const string ShiftLeftOpStr = "<<";
class ShiftLeft : public BinaryOperator<Expression, &ShiftLeftOpStr, SHIFT> {
	public:
		ShiftLeft(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static ShiftLeft* create(Parser* parser, Expression* lhs)\
	   	{return new ShiftLeft(parser, lhs);}
};

const string ShiftRightOpStr = ">>";
class ShiftRight : public BinaryOperator<Expression, &ShiftRightOpStr, SHIFT> {
	public:
		ShiftRight(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static ShiftRight* create(Parser* parser, Expression* lhs)\
	   	{return new ShiftRight(parser, lhs);}
};

const string AdditionOpStr = "+";
class Addition : public BinaryOperator<Expression, &AdditionOpStr, ADDITIVE> {
	public:
		Addition(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs, "add") {}
		static Addition* create(Parser* parser, Expression* lhs) \
		{return new Addition(parser, lhs);}
};

const string SubtractionOpStr = "-";
class Subtraction : public BinaryOperator<Expression, &SubtractionOpStr, ADDITIVE> {
	public:
		Subtraction(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs, "sub") {}
		static Subtraction* create(Parser* parser, Expression* lhs) \
		{return new Subtraction(parser, lhs);}
};

const string MultiplicationOpStr = "*";
class Multiplication : public BinaryOperator<Expression, &MultiplicationOpStr, MULTIPLICATIVE> {
	public:
		Multiplication(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs, "mul") {}
		static Multiplication* create(Parser* parser, Expression* lhs) \
		{return new Multiplication(parser, lhs);}
};

const string DivisionOpStr = "/";
class Division : public BinaryOperator<Expression, &DivisionOpStr, MULTIPLICATIVE> {
	public:
		Division(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static Division* create(Parser* parser, Expression* lhs) \
		{return new Division(parser, lhs);}
};

const string ModuloOpStr = "%";
class Modulo : public BinaryOperator<Expression, &ModuloOpStr, MULTIPLICATIVE> {
	public:
		Modulo(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static Modulo* create(Parser* parser, Expression* lhs) \
		{return new Modulo(parser, lhs);}
};

const string TypeCastOpStr = "(";
class TypeCast : public PrefixOperator<Expression, &TypeCastOpStr, CAST> {
	public:
		TypeCast(Parser* parser) \
			: PrefixOperator(parser) {}
		static TypeCast* create(Parser* parser) \
		{return new TypeCast(parser);}
		void parse(Parser* parser) {
			Token token = parser->getSource()->get();
			if (token.getKey() != KEYWORD) {
				string err = "Exprected type name in type cast";
			} else {
				typeName = token;
			}
			token = parser->getSource()->get();
			if (token.getName() != ")") {
				string err = "Expected ')' after TypeCast";
				throw new SyntaxException(err);
			}
			PrefixOperator::parse(parser); //Normal prefixOp parsing
		}
		string getName() {
			string	ret = "";
			ret += PrefixOperator::getName();
			ret = ret.substr(1);
			ret = "(" + typeName.getName() + ")" + ret;
			return ret;
		}
	private:
		Token typeName; //Name of the type to convert to
};

const string UnaryPlusOpStr = "+";
//Performs integer promotion in C
class UnaryPlus : public PrefixOperator<Expression, &UnaryPlusOpStr, CAST> {
	public:
		UnaryPlus(Parser* parser) : PrefixOperator(parser) {}
		static UnaryPlus* create(Parser* parser) {
			return new UnaryPlus(parser);
		}
};

const string UnaryMinusOpStr = "-";
class UnaryMinus : public PrefixOperator<Expression, &UnaryMinusOpStr, CAST> {
	public:
		UnaryMinus(Parser* parser) : PrefixOperator(parser) {}
		static UnaryMinus* create(Parser* parser) {
			return new UnaryMinus(parser);
		}
};

const string ReferenceOpStr = "&";
class Reference : public PrefixOperator<Expression, &ReferenceOpStr, CAST> {
	public:
		Reference(Parser* parser) : PrefixOperator(parser) {}
		static Reference* create(Parser* parser) {
			return new Reference(parser);
		}
};

const string IndirectionOpStr = "*";
class Indirection : public PrefixOperator<Expression, &IndirectionOpStr, CAST> {
	public:
		Indirection(Parser* parser) : PrefixOperator(parser) {}
		static Indirection* create(Parser* parser) {
			return new Indirection(parser);
		}
};

const string BitwiseNOTOpStr = "~";
class BitwiseNOT : public PrefixOperator<Expression, &BitwiseNOTOpStr, CAST> {
	public:
		BitwiseNOT(Parser* parser) : PrefixOperator(parser) {}
		static BitwiseNOT* create(Parser* parser) {
			return new BitwiseNOT(parser);
		}
};

const string LogicalNOTOpStr = "!";
class LogicalNOT : public PrefixOperator<Expression, &LogicalNOTOpStr, CAST> {
	public:
		LogicalNOT(Parser* parser) : PrefixOperator(parser) {}
		static LogicalNOT* create(Parser* parser) {
			return new LogicalNOT(parser);
		}
};

const string IncrementPrefixOpStr = "++";
class IncrementPrefix : public PrefixOperator<Expression, &IncrementPrefixOpStr, UNARY> {
	public:
		IncrementPrefix(Parser* parser) : PrefixOperator(parser) {}
		static IncrementPrefix* create(Parser* parser) {
			return new IncrementPrefix(parser);
		}
};

const string DecrementPrefixOpStr = "--";
class DecrementPrefix : public PrefixOperator<Expression, &DecrementPrefixOpStr, UNARY> {
	public:
		DecrementPrefix(Parser* parser) : PrefixOperator(parser) {}
		static DecrementPrefix* create(Parser* parser) {
			return new DecrementPrefix(parser);
		}
};

const string SizeofOpStr = "sizeof";
class Sizeof : public PrefixOperator<Expression, &SizeofOpStr, UNARY> {
	public:
		Sizeof(Parser* parser) : PrefixOperator(parser) {}
		static Sizeof* create(Parser* parser) {
			return new Sizeof(parser);
		}
		void parse(Parser* parser) {
			Token token = parser->getSource()->peek();
			if (token.getName() == "(") {
				parser->getSource()->get(); //Actually get "(" token
				token = parser->getSource()->get();
				if (token.getKey() == KEYWORD || token.getKey() == IDENTIFIER) {
					//IDENTIFIER for user defined types
					type = token;
					token = parser->getSource()->get();
					if (token.getName() != ")") {
						string err = "Expected ')' in sizeof";
						throw new SyntaxException(err);
					}
				} else {
					string err = "Expected type name in sizeof(...)";
					throw new SyntaxException(err);
				}
			} else {
				expr = parser->parseExpression(UNARY);
			}
		}
		string getName() {
			string ret = "sizeof";
			if (expr != NULL) {
				ret += " " + expr->getName();
			} else {
				ret += "(" + type.getName() + ")";
			}
			return ret;
		}
		virtual ~Sizeof() {
			if (expr != NULL) {delete expr;}
		}
	private:
		Token type;
		Expression* expr = NULL;
};

const string AlignofOpStr= "_Alignof";
class Alignof: public PrefixOperator<Expression, &SizeofOpStr, UNARY> {
	public:
		Alignof(Parser* parser) : PrefixOperator(parser) {}
		static Alignof* create(Parser* parser) {
			return new Alignof(parser);
		}
		void parse(Parser* parser) {
			Token token = parser->getSource()->peek();
			if (token.getName() == "(") {
				parser->getSource()->get(); //Actually get "(" token
				token = parser->getSource()->get();
				if (token.getKey() == KEYWORD || token.getKey() == IDENTIFIER) {
					//IDENTIFIER for used defined types
					type = token;
					token = parser->getSource()->get();
					if (token.getName() != ")") {
						string err = "Expected ')' in _Alignof";
						throw new SyntaxException(err);
					}
				} else {
					string err = "Expected type name in _Alignof(...)";
					throw new SyntaxException(err);
				}
			} else {
				string err = "Expected '(' after '_Alignof'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			string ret = "_Alignof";
			if (expr != NULL) {
				ret += " " + expr->getName();
			} else {
				ret += "(" + type.getName() + ")";
			}
			return ret;
		}
		virtual ~Alignof() {
			if (expr != NULL) {delete expr;}
		}
	private:
		Token type;
		Expression* expr = NULL;
};

const string IncrementPostfixOpStr = "++";
class IncrementPostfix : public PostfixOperator<Expression, &IncrementPostfixOpStr, POSTFIX> {
	public:
		IncrementPostfix(Parser* parser, Expression* expr) : PostfixOperator(parser, expr) {}
		static IncrementPostfix* create(Parser* parser, Expression* expr) {
			return new IncrementPostfix(parser, expr);
		}
};

const string DecrementPostfixOpStr = "--";
class DecrementPostfix : public PostfixOperator<Expression, &DecrementPostfixOpStr, POSTFIX> {
	public:
		DecrementPostfix(Parser* parser, Expression* expr) : PostfixOperator(parser, expr) {}
		static DecrementPostfix* create(Parser* parser, Expression* expr) {
			return new DecrementPostfix(parser, expr);
		}
};

const string StructureDereferenceOpStr = "->";
class StructureDereference : public PostfixOperator<Expression, &StructureDereferenceOpStr, POSTFIX> {
	public:
		StructureDereference(Parser* parser, Expression* expr) : PostfixOperator(parser, expr) {}
		static StructureDereference* create(Parser* parser, Expression* expr) {
			return new StructureDereference(parser, expr);
		}
		void parse(Parser* parser) {
			Token token = parser->getSource()->get();
			if (token.getKey() == IDENTIFIER) {
				id = token;
			} else {
				string err = "Expected identifier after '->'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			string ret = PostfixOperator::getName();
			ret += id.getName();
			return ret;
		}
	private:
		Token id;
};

const string StructureReferenceOpStr = ".";
class StructureReference : public PostfixOperator<Expression, &StructureReferenceOpStr, POSTFIX> {
	public:
		StructureReference(Parser* parser, Expression* expr) : PostfixOperator(parser, expr) {}
		static StructureReference* create(Parser* parser, Expression* expr) {
			return new StructureReference(parser, expr);
		}
		void parse(Parser* parser) {
			Token token = parser->getSource()->get();
			if (token.getKey() == IDENTIFIER) {
				id = token;
			} else {
				string err = "Expected identifier after '->'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			string ret = PostfixOperator::getName();
			ret += id.getName();
			return ret;
		}
	private:
		Token id;
};

const string ArraySubscriptOpStr = "[";
class ArraySubscript : public BinaryOperator<Expression, &ArraySubscriptOpStr, POSTFIX> {
	public:
		ArraySubscript(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static ArraySubscript* create(Parser* parser, Expression* lhs)\
	   	{return new ArraySubscript(parser, lhs);}
		void parse(Parser* parser) {
			rhs = parser->parseExpression(); //This is done with 
			//DEFAULT priority, the brackets are like a pair of parenthesis
			Token token = parser->getSource()->get();
			if (token.getName() != "]") {
				string err = "Expected ']'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			return BinaryOperator::getName() + "]";
		}
};

const string FunctionCallOpStr = "(";
class FunctionCall : public BinaryOperator<Expression, &FunctionCallOpStr, POSTFIX> {
	public:
		FunctionCall(Parser* parser, Expression* lhs) : \
			BinaryOperator(parser, lhs) {}
		static FunctionCall* create(Parser* parser, Expression* lhs) \
		{return new FunctionCall(parser, lhs);}
		void parse(Parser* parser) {
			rhs = parser->parseExpression(); //With DEFAULT priority, since we're in a
			//pair of parenthesis
			Token token = parser->getSource()->get();
			if (token.getName() != ")") {
				string err = "Expected ')' at end of function ";
				if (lhs != NULL) {err += lhs->getName();}
				throw new SyntaxException(err);
			}
		}
		string getName() {
			if (rhs->getPriority() > POSTFIX) {
				return BinaryOperator::getName() + ")";
			} else {
				string ret =  "";
				if (lhs != NULL) {ret += lhs->getName();}
				ret += "(";
				if (rhs != NULL) {ret += rhs->getName();}
				ret += ")";
				return ret;
			}
		}
		void genLLVM(Scope* s, Consumer<string>* o) {
			//We should not care about any return values
			o->put("call ");
			Symbol* symbol = s->find(lhs->getName());
			if (symbol != NULL) {
				Type* symbolType = symbol->getType();
				if (symbolType != NULL) {
					o->put(symbolType->getLLVMName());
				} else {
					o->put("void");
				}
				o->put(" " + lhs->getName() + "(");
			} else {
				string err = "Unknown return type of function";
				throw new TypeError(err);
			}
		}
		void genLLVM(Scope* s, Consumer<string>* o, string retOp) {
			o->put(retOp + " = ");
			this->genLLVM(s, o);
		}
};

class GenericAssociation : public Expression {
	public:
		GenericAssociation(Token type) : Expression(PRIMARY), type(type) {}
		virtual ~GenericAssociation() {delete assignmentExpr;}
		string getName() {
			string ret = type.getName() + " : ";
			if (assignmentExpr != NULL) {ret += assignmentExpr->getName();}
			return ret;
		}
		void parse(Parser* parser) {
			Token colon = parser->getSource()->get();
			if (colon.getName() != ":") {
				string err = "Expected ':' in generic association";
			}
			assignmentExpr = parser->parseExpression(COMMA);
		}
	private:
		Token type;
		Expression* assignmentExpr = NULL;
};

class GenericAssociationList : public NodeList<GenericAssociation> {
	public:
		GenericAssociationList(GenericAssociation* assoc) : NodeList(assoc) {}
		GenericAssociationList(GenericAssociation* assoc, GenericAssociationList* list) : NodeList(assoc, list) {}
};

const string GenericOpStr = "_Generic";
class Generic : public PrefixOperator<Expression, &GenericOpStr, UNARY> {
	public:
		Generic(Parser* parser) : PrefixOperator(parser) {}
		static Generic* create(Parser* parser) {
			return new Generic(parser);
		}
		~Generic() {
			if (expr != NULL) {delete expr;}
			if (list != NULL) {delete list;}
		}
		void parse(Parser* parser) {
			Token token = parser->getSource()->peek();
			if (token.getName() == "(") {
				parser->getSource()->get(); //Actually get "(" token
				expr = parser->parseExpression(COMMA);
				token = parser->getSource()->get();
				if (token.getName() == ",") {
					token = parser->getSource()->get();
					GenericAssociation* first = new GenericAssociation(token);
					first->parse(parser);
					list = new GenericAssociationList(first);
				}
				GenericAssociationList* currentList = list;
				GenericAssociationList* nextList = NULL;
				GenericAssociation* assoc = NULL;
				token = parser->getSource()->get();
				while (token.getName() == ",") {
					assoc = new GenericAssociation(token);
					assoc->parse(parser);
					nextList = new GenericAssociationList(assoc);
					currentList->setNext(nextList);
					currentList = nextList;
					token = parser->getSource()->get();
				}
				if (token.getName() != ")") {
					string err = "Expected ')' after _Generic";
					throw new SyntaxException(err);
				}
			} else {
				string err = "Expected ')'";
				throw new SyntaxException(err);
			}
		}
		string getName() {
			string ret = "_Generic(";
			if (expr != NULL) {
				ret += expr->getName();
			}
			ret += ", ";
			ret += list->getName();
			ret += ")";
			return ret;
		}
	private:
		Expression* expr = NULL;
		GenericAssociationList* list = NULL;
};









template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void PrefixOperator<Op, opStrTemp, prioTemp> :: parse(Parser* parser) {
	expr = parser->parseExpression(prioTemp);
}

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void PostfixOperator<Op, opStrTemp, prioTemp> :: parse(Parser* parser) {
}

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void BinaryOperator<Op, opStrTemp, prioTemp> :: parse(Parser* parser) {
	rhs = parser->parseExpression(prioTemp);
}

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void BinaryOperator<Op, opStrTemp, prioTemp> :: genLLVM(Scope* s, Consumer<string>* o, string result) {
	string op1 = "%";
	string op2 = "%";
	IdentifierExpression* downcast = NULL;
	ConstantExpression* downcastConst = NULL;
	//If lhs is a variable name, use that as an operand.
	//Otherwise, compute the result and store it in a temporary
	//Also checks for constants instead of using temporaries
	if ((downcast = dynamic_cast<IdentifierExpression*>(lhs))) {
		op1 += downcast->getName();
	} else if ((downcastConst = dynamic_cast<ConstantExpression*>(lhs))) {
		op1 += downcast->getName();
	} else {
		op1 += to_string(s->getTemp());
		lhs->genLLVM(s, o, op1);
	}
	//Same for rhs
	if ((downcast = dynamic_cast<IdentifierExpression*>(rhs))) {
		op2 += downcast->getName();
	} else if ((downcastConst = dynamic_cast<ConstantExpression*>(rhs))) {
		op1 += downcastConst->getName();
	} else {
		op2 += to_string(s->getTemp());
		lhs->genLLVM(s, o, op2);
	}
	o->put(result + " = ");
	o->put(this->getLLVMOpName() + " ");
	o->put(lhs->getType(s)->getLLVMName() + " ");
	o->put(op1 + ", " + op2 + "\n");
}


template<class Left, class Middle, class Right, const string* opStrFirst, const string* opStrSecond, PriorityEnum prioTemp>
void TernaryOperator<Left, Middle, Right, opStrFirst, opStrSecond, prioTemp> :: parse(Parser* parser) {
	mhs = parser->parseExpression(prioTemp);
	Token secondOp = parser->getSource()->get();
	if (secondOp.getName() == *opStrSecond) {
		rhs = parser->parseExpression(prioTemp);
	} else {
		string err = "Expected '" + *opStrSecond + "' as second operator in ternary"\
					  " operator";
		throw new SyntaxException(err);
	}
}



