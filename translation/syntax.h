#include "source.h"

enum PriorityEnum {DEFAULT, ASSIGNMENT, CONDITIONAL, LOGICAL_OR, LOGICAL_AND, \
	BITWISE_OR, BITWISE_XOR, BITWISE_AND, EQUALITY, RELATIONAL, SHIFT, \
		ADDITIVE, MULTIPLICATIVE, CAST, UNARY, POSTFIX, PRIMARY};
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
	public:
		Expression(PriorityEnum prio) : prio(prio) {}
		PriorityEnum getPriority() {return prio;}
		~Expression() {}
		virtual void parse(Parser* parser) = 0; //Parse the rest of the 
		//Expression, with the Parser starting at the Token following the first
		//punctuator of the Expression. Unary expressions should not do anything
	private:
		PriorityEnum prio = DEFAULT;
};

class Operator : public Expression {
	public:
		Operator(Parser* parser, const string* opStr, PriorityEnum prio) \
			: Expression(prio), parser(parser), opStr(*opStr) {}
		virtual ~Operator() {}
		string getName() {return opStr;}
	protected:
		Parser* parser;
	private:
		const string opStr; //String representation of the punctuator for this
		//operator, e.g. "+" for Addition.
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
		void parse(Parser* parser) {} //Do not do anything, 
		//everything is already handled when the object is created
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
			InfixOperator(parser, opStrTemp, prioTemp), lhs(lhs) {}
		virtual ~BinaryOperator() {
			if (rhs != NULL) {delete rhs;}
			if (lhs != NULL) {delete lhs;}
		}
		string getName() {
			//TODO: Remove unnecessary parenthesis
			string ret = "";
			ret += "(";
			if (lhs !=NULL) {ret += lhs->getName();}
			ret += ")";
			ret += Operator::getName();
			ret += "(";
			if (rhs !=NULL) {ret += rhs->getName();}
			ret += ")";
			return ret;
		}
		void parse(Parser* parser);
	private:
		Op* rhs; //The right hand side 
		Op* lhs; //The left hand side
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
			//TODO: Remove unnecessary paranthesis
			string ret = "";
			ret += "(";
			if (lhs != NULL) {ret += lhs->getName();}
			ret += ")";
			ret += Operator::getName();
			ret += "(";
			if (mhs != NULL) {ret += mhs->getName();}
			ret += ")";
			ret += *opStrSecond;
			ret += "(";
			if (rhs != NULL) {ret += rhs->getName();}
			ret += ")";
			return ret;
		}
		void parse(Parser* parser);
	private:
		Left* lhs;
		Middle* mhs;
		Right* rhs;
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
		Expression* parseExpression(PriorityEnum priority);
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

class Keyword : public Identifier {
	public:
		Keyword(Token token) : Identifier(token) {}
		virtual ~Keyword() {}
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
		string getName() {
			string ret = "";
			if (expression != NULL) {ret += expression->getName();}
			ret += ";";
			return ret;
		}
	private:
		Expression* expression = NULL;
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

//Operator classes, eg. Addition, Multiplication, etc.

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


const string AdditionOpStr = "+";
class Addition : public BinaryOperator<Expression, &AdditionOpStr, ADDITIVE> {
	public:
		Addition(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BinaryOperator* create(Parser* parser, Expression* lhs) \
		{return new Addition(parser, lhs);}
};

const string SubtractionOpStr = "-";
class Subtraction : public BinaryOperator<Expression, &SubtractionOpStr, ADDITIVE> {
	public:
		Subtraction(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BinaryOperator* create(Parser* parser, Expression* lhs) \
		{return new Subtraction(parser, lhs);}
};

const string MultiplicationOpStr = "*";
class Multiplication : public BinaryOperator<Expression, &MultiplicationOpStr, MULTIPLICATIVE> {
	public:
		Multiplication(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static BinaryOperator* create(Parser* parser, Expression* lhs) \
		{return new Multiplication(parser, lhs);}
};

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void BinaryOperator<Op, opStrTemp, prioTemp>::parse(Parser* parser) {
	rhs = parser->parseExpression(prioTemp);
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

