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
class CompoundStatement;
class ExpressionStatement;
class SelectionStatement;
class JumpStatement;
class Expression;
class Identifier;
class BlockItemList;
class BlockItem;

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
		CompoundStatement* parseCompoundStatement();
		ExpressionStatement* parseExpressionStatement();
		SelectionStatement* parseSelectionStatement();
		JumpStatement* parseJumpStatement();
		Expression* parseExpression();
		Identifier* parseIdentifier();
		BlockItemList* parseBlockItemList();
		BlockItem* parseBlockItem();
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

class BlockItem : public Node {
	public:
		/*BlockItem(Declaration* decl) : decl(decl) {} TODO: Implement*/
		BlockItem(Statement* state) : state(state) {}
		virtual ~BlockItem() {
			if (state != NULL) delete state;
			/*if (decl != NULL) delete decl;*/
		}
		string getName() {
			if (state != NULL) {return state->getName();}
			/*if (decl != NULL) {return decl->getName();}*/
			return "";
		}

	private:
		/*Declaration* decl;*/
		Statement* state;
};
		
class BlockItemList : public Node {
	public:
		BlockItemList(BlockItem* item) : item(item), next(NULL) {}
		BlockItemList(BlockItem* item, BlockItemList* next) : item(item), \
															  next(next) {}
		string getName() {
			string ret = item->getName();
			if (next != NULL) {
				ret += '\n' + next->getName();
			}
			return ret;
		}
		virtual ~BlockItemList() {
			if (next != NULL) {delete next;}
			delete item;
		}
	private:
		BlockItem* item;
		BlockItemList* next = NULL; //Optional, might be NULL
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
		virtual ~ExpressionStatement() {}
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
		virtual ~SelectionStatement() {}
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
		virtual ~JumpStatement() {}
	private:
		string keyword;
		Identifier* id = NULL;
		Expression* expr = NULL;
};



