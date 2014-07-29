#include "source.h"

enum PriorityEnum {DEFAULT, COMMA, ASSIGNMENT, CONDITIONAL, LOGICAL_OR, \
	LOGICAL_AND, BITWISE_OR, BITWISE_XOR, BITWISE_AND, EQUALITY, RELATIONAL, \
		SHIFT, ADDITIVE, MULTIPLICATIVE, CAST, UNARY, POSTFIX, PRIMARY};
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
		void parse(Parser* parser);
	private:
		Op* expr = NULL; //The item to operate on
};

class InfixOperator : public Operator {
	public:
		InfixOperator(Parser* parser, const string* opStr, PriorityEnum prio) \
			: Operator(parser, opStr, prio) {}
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
		map<string, Operator* (*) (Parser*)> mPrefix; 
		//Maps string to pointer to function which returns type Expression* and
		//takes types Parser*, Expression* as input
		map<string, InfixOperator* (*) (Parser*, Expression*)> mInfix; 
		//Maps string to pointer to function which returns type Expression* and
		//takes types Parser*, Expression* as input
		void c11Operators(); //Inserts operators to parse C11 into the maps
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
			: BinaryOperator(parser, lhs) {}
		static Addition* create(Parser* parser, Expression* lhs) \
		{return new Addition(parser, lhs);}
};

const string SubtractionOpStr = "-";
class Subtraction : public BinaryOperator<Expression, &SubtractionOpStr, ADDITIVE> {
	public:
		Subtraction(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
		static Subtraction* create(Parser* parser, Expression* lhs) \
		{return new Subtraction(parser, lhs);}
};

const string MultiplicationOpStr = "*";
class Multiplication : public BinaryOperator<Expression, &MultiplicationOpStr, MULTIPLICATIVE> {
	public:
		Multiplication(Parser* parser, Expression* lhs) \
			: BinaryOperator(parser, lhs) {}
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
					//IDENTIFIER for used defined types
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
};







template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void PrefixOperator<Op, opStrTemp, prioTemp> :: parse(Parser* parser) {
	expr = parser->parseExpression(prioTemp);
}

template<class Op, const string* opStrTemp, PriorityEnum prioTemp>
void PostfixOperator<Op, opStrTemp, prioTemp> :: parse(Parser* parser) {
}

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

