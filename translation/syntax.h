#include "source.h"

const string AdditionOpStr = "+";
enum PriorityEnum {PRIMARY, POSTFIX, UNARY, CAST, MULTIPLICATIVE, ADDITIVE,\
	SHIFT, RELATIONAL, EQUALITY, BITWISE_AND, BITWISE_XOR, BITWISE_OR, \
		LOGICAL_AND, LOGICAL_OR, CONDITIONAL, ASSIGNMENT};
class Parser;

//! Abstract base class for all classes in the AST
/*! A pure virtual function here forces all subclasses, i.e. all classes in
 *  the Abstract Syntax Tree (AST) to implement the function
 */
class Node {
	public:
		Node() {}
		virtual string getName() = 0; //Returns string representation of object
		virtual ~Node() {}
};

//A template class to handle repeating occurences of some class
template<class T>
class NodeList : public Node {
	public:
		NodeList(T* item) : item(item), next(NULL) {}
		NodeList(T* item, NodeList* next) : item(item), \
															  next(next) {}
		string getName() {
			string ret = item->getName();
			if (next != NULL) {
				ret += '\n' + next->getName();
			}
			return ret;
		}
		virtual ~NodeList() {
			if (next != NULL) {delete next;}
			delete item;
		}
	private:
		T* item;
		NodeList* next = NULL; //Optional, might be NULL
};

class Expression : public Node {
};

class Operator : public Expression {
	public:
		Operator(Parser* parser, const string* opStr, PriorityEnum prio) \
			: parser(parser), opStr(*opStr), prio(prio) {}
		virtual ~Operator() {}
		string getName() {return opStr;}
		PriorityEnum getPriority() {return prio;}
	protected:
		Parser* parser;
	private:
		const string opStr; //String representation of the punctuator for this
		//operator, e.g. "+" for Addition.
		PriorityEnum prio; //Priority for this operator
};

template<class T>
class PrefixOperator : public Operator {
	public:
		PrefixOperator(Parser* parser, const string* opStr, PriorityEnum prio) \
			: Operator(parser, opStr, prio), item(item) {}
		virtual ~PrefixOperator() {delete item;}
		string getName() {
			string ret = "";
			ret += Operator::getName();
			if (item != NULL) {ret += item->getName();}
			return ret;
		}
	private:
		T* item; //The item to operate on
};

class InfixOperator : public Operator {
	public:
		InfixOperator(Parser* parser, const string* opStr, PriorityEnum prio) \
			: Operator(parser, opStr, prio) {}
		virtual ~InfixOperator() {}
};

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
class BinaryOperator : public InfixOperator {
	public:
		/*BinaryOperator(Operator* rhs, Operator* lhs, string opStr,\
			   	PriorityEnum prio) : Operator(opStr, prio), rhs(rhs), \
									 lhs(lhs) {}*/
		BinaryOperator(Parser* parser, Op* lhs) : \
			InfixOperator(parser, opStrTemp, prioTemp), lhs(lhs) {
				parseRhs(parser);
			}
		void parseRhs(Parser* parser);
		virtual ~BinaryOperator() {
			if (rhs != NULL) {delete rhs;}
			if (lhs != NULL) {delete lhs;}
		}
		string getName() {
			string ret = "";
			if (lhs !=NULL) {ret += lhs->getName();}
			ret += Operator::getName();
			if (rhs !=NULL) {ret += rhs->getName();}
			return ret;
		}
		Expression* parse();
	private:
		Op* rhs; //The right hand side 
		Op* lhs; //The left hand side
};

class TranslationUnit;
class ExternalDeclaration;
class Statement;
class CompoundStatement;
class ExpressionStatement;
class SelectionStatement;
class JumpStatement;
class Identifier;
class BlockItem;
typedef NodeList<BlockItem> BlockItemList;
class Declaration;
class InitDeclarator;
typedef NodeList<InitDeclarator> InitDeclaratorList;
class Declarator;
class DirectDeclarator;
class Initializer;
class Pointer;
class Expression;
class IdentifierExpression;

//Constructs an AST from the input Tokens
class Parser {
	public:
		Parser(BufferedSource<Token>* source) : source(source) {}
		BufferedSource<Token>* getSource() {return this->source;}
		TranslationUnit* parseTranslationUnit();
		ExternalDeclaration* parseExternalDeclaration();
		Statement* parseStatement();
		CompoundStatement* parseCompoundStatement();
		ExpressionStatement* parseExpressionStatement();
		SelectionStatement* parseSelectionStatement();
		JumpStatement* parseJumpStatement();
		Expression* parseExpression();
		Identifier* parseIdentifier();
		BlockItemList* parseBlockItemList();
		BlockItem* parseBlockItem();
		Declarator* parseDeclarator();
		DirectDeclarator* parseDirectDeclarator();
		InitDeclarator* parseInitDeclarator();
		InitDeclaratorList* parseInitDeclaratorList();
		Declaration* parseDeclaration();
		map<string, PrefixOperator<Expression>* (*) (Parser*, Expression*)> mPrefix; 
		//Maps string to pointer to function which returns type Expression* and
		//takes types Parser*, Expression* as input
		map<string, InfixOperator* (*) (Parser*, Expression*)> mInfix; 
		//Maps string to pointer to function which returns type Expression* and
		//takes types Parser*, Expression* as input
	private:
		BufferedSource<Token>* source;
};

class Identifier : public Node {
	public:
		Identifier(Token token) : token(token) {}
		string getName() {return token.getName();}
		virtual ~Identifier() {}
	private:
		Token token;
};

class IdentifierExpression : public Expression {
	public:
		IdentifierExpression(Token token) : identifier(new Identifier(token)) {}
		IdentifierExpression(Identifier* identifier) : identifier(identifier) {}
		virtual ~IdentifierExpression() {
			if (identifier != NULL) {delete identifier;}
		}
		string getName() {return identifier->getName();}
	private:
		Identifier* identifier = NULL;
};

class ExpressionException : public SyntaxException {
	public:
		ExpressionException(string w) : SyntaxException(w) {}
		ExpressionException(char* w) : SyntaxException(w) {}
};

class ExternalDeclaration : public Node {
};

class TranslationUnit : public Node {
	public:
		TranslationUnit(list<ExternalDeclaration*>& list) : list(list) {}
		list<ExternalDeclaration*>& getList() {return this->list;}
	private:
		list<ExternalDeclaration*>& list;
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
	private:
		Declaration* decl;
		Statement* state;
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
	private:
		BlockItemList* itemList; //Optional, may be NULL

};

class ExpressionStatement : public Statement {
	public:
		ExpressionStatement(Expression* expr) : expression(expr)  {}
		virtual ~ExpressionStatement() {delete expression;}
		string getName() {return this->expression->getName() + ";";}
	private:
		Expression* expression;
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
	private:
		string keyword;
		Identifier* id = NULL;
		Expression* expr = NULL;
};

class Declarator : public Node {
	//TODO: Implement the optional Pointer here
	public:
		Declarator(DirectDeclarator* dirDecl) : dirDecl(dirDecl), pointer(NULL) {}
		Declarator(DirectDeclarator* dirDecl, Pointer* pointer) : dirDecl(dirDecl), pointer(pointer) {}
		virtual ~Declarator(); 
		string getName();
			private:
		DirectDeclarator* dirDecl;
		Pointer* pointer;
};

class DeclaratorException : public SyntaxException {
	public:
		DeclaratorException(string w) : SyntaxException(w) {}
		DeclaratorException(char * w) : SyntaxException(w) {}
};

class DirectDeclarator : public Node {
	//TODO: Implement complicated versions
	public:
		DirectDeclarator(Identifier* id) : id(id), decl(NULL) {}
		DirectDeclarator(Declarator* decl) : id(NULL), decl(decl) {}
		virtual ~DirectDeclarator() {
			if (id != NULL) {delete id;}
			if (decl != NULL) {delete decl;}
		}
		string getName() {
			string ret = "";
			if (id != NULL) {ret = id->getName();}
			if (decl != NULL) {ret = decl->getName();}
			return ret;
		}
	private:
		Identifier* id;
		Declarator* decl;
};

class DirectDeclaratorException : public SyntaxException {
	public:
		DirectDeclaratorException(string w) : SyntaxException(w) {}
		DirectDeclaratorException(char * w) : SyntaxException(w) {}
};

class InitDeclarator : public Node {
	public:
		InitDeclarator(Declarator* dec) : dec(dec), init(NULL) {}
		InitDeclarator(Declarator* dec, Initializer* init) : dec(dec), \
															 init(init) {}
		virtual ~InitDeclarator() {
			if (dec != NULL) {delete dec;}
			/*if (init != NULL) {delete init;} TODO: Implement Initializer */
		}
		string getName() {
			string ret = "";
			if (dec != NULL) {ret += dec->getName();}
			/*if (init != NULL) {ret += " = " + init->getName();}*/
			return ret;
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

class InitDeclaratorListException : public SyntaxException {
	public:
		InitDeclaratorListException(string w) : SyntaxException(w) {}
		InitDeclaratorListException(char * w) : SyntaxException(w) {}
};

class Declaration : public Node {
	//TODO: Implement Declaration specifiers and static_assert declarations
	public:
		Declaration() : initList(NULL) {}
		Declaration(InitDeclaratorList* initList) : initList(initList) {}
		virtual ~Declaration() {if (initList != NULL) {delete initList;}}
		string getName() {
			return initList->getName(); //TODO: Change
		}
	private:
		InitDeclaratorList* initList;
};

class DeclarationException : public SyntaxException {
	DeclarationException(string w) : SyntaxException(w) {}
	DeclarationException(char *w) : SyntaxException(w) {}
};

const string AssignmentOpStr = "=";
class Assignment : public BinaryOperator<Expression, &AssignmentOpStr, ASSIGNMENT> {
	public:
		Assignment(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BinaryOperator* create(Parser* parser, Expression* lhs)\
	   	{return new Assignment(parser, lhs);}
};

class Addition : public BinaryOperator<Expression, &AdditionOpStr, ADDITIVE> {
	public:
		Addition(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BinaryOperator* create(Parser* parser, Expression* lhs) \
		{return new Addition(parser, lhs);}
};

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void BinaryOperator<Op, opStrTemp, prioTemp>::parseRhs(Parser* parser) {
	rhs = parser->parseExpression();
}

