#include "source.h"

class Node {
	public:
		Node() {}
		virtual string getName() = 0;
		virtual ~Node() {}
};

class TranslationUnit;
class ExternalDeclaration;
class Statement;
class ExpressionStatement;
class JumpStatement;
class Expression;
class Identifier;

class SyntaxTree {
	public:
		SyntaxTree(Node& root) : root(root) {}
		Node& getRoot() {return this->root;}
	private:
		Node& root;
};

class Parser {
	public:
		Parser(BufferedSource<Token>* source) : source(source) {}
		BufferedSource<Token>* getSource() {return this->source;}
		TranslationUnit* parseTranslationUnit();
		ExternalDeclaration* parseExternalDeclaration();
		Statement* parseStatement();
		ExpressionStatement* parseExpressionStatement();
		JumpStatement* parseJumpStatement();
		Expression* parseExpression();
		Identifier* parseIdentifier();
	private:
		BufferedSource<Token>* source;
};

class Identifier : public Node {
	public:
		Identifier(Token token) : token(token) {}
		string getName() {return token.getName();}
	private:
		Token token;
};

class Expression : public Node {
	public:
		Expression() {}
		Expression(Token token) : identifier(token) {}
		virtual ~Expression() {}
		string getName() {return this->identifier.getName();}
	private:
		Token identifier;
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

class ExpressionStatement : public Statement {
	public:
		ExpressionStatement(Expression* expr) : expression(expr)  {}
		virtual ~ExpressionStatement() {}
		string getName() {return this->expression->getName() + ";";}
	private:
		Expression* expression;
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
	private:
		string keyword;
		Identifier* id = NULL;
		Expression* expr = NULL;
};



