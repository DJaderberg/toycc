#include <string>
#include <map>
#include <list>
#include <ostream>
#include "source.h"
using namespace std;

template<class T>
class Consumer {
	public:
		virtual void put(T item) = 0;
		virtual ~Consumer() {}
};

class StreamConsumer : public Consumer<string> {
	public:
		StreamConsumer(basic_ostream<char>& filestream) : filestream(filestream) {}
		void put(string item) {
			filestream << item;
		}
	private:
		basic_ostream<char>& filestream;
};

template<class T>
class Buffer : public Source<T>, public Consumer<T> {
	public:
		Buffer() : list(new std::list<T>()) {}
		void put(T item) {
			list->push_back(item);
		}
		T get() {
			T first = list->front();
			list->pop_front();
			return first;
		}
		bool empty() {return list->empty();}
		//Empties the buffer into the given consumer
		void push_to(Consumer<T>* consumer) {
			while (!empty()) {
				consumer->put(get());
			}
		}
	private:
		list<T>* list;
};


enum PriorityEnum {DEFAULT, COMMA, ASSIGNMENT, CONDITIONAL, LOGICAL_OR, \
	LOGICAL_AND, BITWISE_OR, BITWISE_XOR, BITWISE_AND, EQUALITY, RELATIONAL, \
		SHIFT, ADDITIVE, MULTIPLICATIVE, CAST, UNARY, POSTFIX, PRIMARY};
class Parser;
class Type;
class Scope;
class TypeList;

//! Abstract base class for all classes in the AST
/*! A pure virtual function here forces all subclasses, i.e. all classes in
 *  the Abstract Syntax Tree (AST) to implement the function
 */
class Node {
	public:
		Node() {}
		virtual string getName() = 0; //Returns string representation of object
		virtual Type* getType(Scope* s);
		virtual bool typeCheck(Scope* s);
		virtual string genLLVM(Scope* s, Consumer<string>* o) {
			o->put(";No known assembly code for " + this->getName() + "\n");
			return "";
		}
		virtual string getLLVMName() const {
			return ";Deprecated";
		}
		virtual ~Node() {}
};

//A template class to handle repeating occurences of some class
/* Implemened as a singly linked list
 */
template<class T>
class NodeList : public Node {
	public:
		NodeList(T* item) : item(item), next(NULL) {}
		NodeList(T* item, NodeList* next) : item(item), \
															  next(next) {}
		NodeList(T* item, NodeList* next, string delimiter) \
		: item(item), next(next), delimiter(delimiter) {}	
		virtual string getName() {
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
		void setNext(NodeList* n) {next = n;}
		virtual Type* getType(Scope* s) {return item->getType(s);}	
		virtual TypeList* getTypes(Scope* s);
		virtual bool typeCheck(Scope* scope);
		virtual string genLLVM(Scope* s, Consumer<string>* output);
		T* getItem() {return item;}
		NodeList* getNext() {return next;}
		void getNames(Scope* s, list<string>* ret);
		string getLLVMName() const;
	protected:
		bool typeCheck(Scope* s, Type* t);
		T* item;
		NodeList* next = NULL; //Optional, might be NULL
		string delimiter = "\n";
};

class Expression : public Node {
	public:
		Expression(PriorityEnum prio) : prio(prio) {}
		PriorityEnum getPriority() {return prio;}
		virtual ~Expression() {}
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
		virtual string getLLVMOpName(Type* t) {return "NoLLVMOpStr";}
	protected:
		Parser* parser;
		const string opStr; //String representation of the punctuator for this
		//operator, e.g. "+" for Addition.
};

enum BasicTypeEnum {_BOOL, CHAR, SIGNED_CHAR, UNSIGNED_CHAR, SHORT_INT, \
	UNSIGNED_SHORT_INT, INT, UNSIGNED_INT, LONG_INT, UNSIGNED_LONG_INT, \
		LONG_LONG_INT,  UNSIGNED_LONG_LONG_INT, FLOAT, DOUBLE, LONG_DOUBLE};
enum TypeKindEnum {NO_TYPE, BASIC, FUNCTION, POINTER, STRUCT, UNION};

class Type {
	public:
		Type(TypeKindEnum kind, unsigned int size) : kind(kind), size(size) {}
		virtual unsigned int getSize() {return this->size;}
		TypeKindEnum getKind() const {return this->kind;}
		virtual string getName() const = 0;
		virtual string getLLVMName() const = 0;
		virtual bool operator ==(const Type& other) const {
			return this->equals(other);
		}
		virtual bool equals(const Type& other) const = 0;
		bool operator !=(const Type& other) const {
			return !(*this == other);
		}
		virtual ~Type() {}
	private:
		TypeKindEnum kind;
		unsigned int size = 0; //In bytes
};

class NoType: public Type {
	public:
		NoType() : Type(NO_TYPE, 0) {}
		bool equals(const Type& other) const {
			//There is only one NO_TYPE, so true if kind matches
			return this->getKind() == other.getKind();
		}
		string getName() const {
			return "NO TYPE";
		}
		string getLLVMName() const {
			return "void";
		}
};

class BasicType : public Type {
	public:
		BasicType(BasicTypeEnum basicType) : \
			Type(BASIC, this->getSize(basicType)), basicType(basicType) {}
		BasicTypeEnum getBasicType() const {return this->basicType;}
		virtual bool equals(const Type& other) const {
			//Try to downcast other to BasicType
			if (const BasicType* otherDown = \
					dynamic_cast<const BasicType*>(&other)) {
				//Ah, they are both basic types!
				//Compare the BasicTypeEnum values
				if (this->getBasicType() == otherDown->getBasicType()) {
					return true;
				}
			}
			//If failed, return false
			return false;
		}
		static unsigned int getSize(BasicTypeEnum basicType) {
			switch(basicType) {
				case _BOOL : return 1;
				case CHAR : return 1;
				case SIGNED_CHAR : return 1;
				case UNSIGNED_CHAR : return 1;
				case SHORT_INT : return 4;
				case UNSIGNED_SHORT_INT : return 4;
				case INT : return 8;
				case UNSIGNED_INT : return 8;
				case LONG_INT : return 8;
				case UNSIGNED_LONG_INT : return 8;
				case LONG_LONG_INT : return 8;
				case UNSIGNED_LONG_LONG_INT : return 8;
				case FLOAT : return 4;
				case DOUBLE : return 8;
				case LONG_DOUBLE : return 16;
			}
		}
		string getName() const {
			switch(basicType) {
				case _BOOL : return "_Bool";
				case CHAR : return "char";
				case SIGNED_CHAR : return "signed char";
				case UNSIGNED_CHAR : return "unsigned char";
				case SHORT_INT : return "short int";
				case UNSIGNED_SHORT_INT : return "unsigned short int";
				case INT : return "int";
				case UNSIGNED_INT : return "unsigned int";
				case LONG_INT : return "long int";
				case UNSIGNED_LONG_INT : return "unsigned long int";
				case LONG_LONG_INT : return "long long int";
				case UNSIGNED_LONG_LONG_INT : return "unsigned long long int";
				case FLOAT : return "float";
				case DOUBLE : return "double";
				case LONG_DOUBLE : return "long double";
			}
		}
		string getLLVMName() const {
			switch(basicType) {
				case _BOOL : return "i1";
				case CHAR : return "i8";
				case SIGNED_CHAR : return "i8";
				case UNSIGNED_CHAR : return "i8";
				case SHORT_INT : return "i32";
				case UNSIGNED_SHORT_INT : return "i32";
				case INT : return "i64";
				case UNSIGNED_INT : return "i64";
				case LONG_INT : return "i64";
				case UNSIGNED_LONG_INT : return "i64";
				case LONG_LONG_INT : return "i128";
				case UNSIGNED_LONG_LONG_INT : return "i128";
				case FLOAT : return "float";
				case DOUBLE : return "double";
				case LONG_DOUBLE : return "fp128";

			}
		}
	private:
		BasicTypeEnum basicType;
};

class PointerType : public Type {
	public:
		PointerType(Type* pointeeType) : Type(POINTER, 8), \
										 pointeeType(pointeeType) {}
		virtual bool equals(const Type& other) const {
			//Try to downcast
			if (const PointerType* otherDown = \
					dynamic_cast<const PointerType*>(&other)) {
				return pointeeType->equals(*otherDown);
			}
			return false;
		}
		string getName() const {
			string ret = "*";
			if (pointeeType != NULL) {ret += pointeeType->getName();}
			return ret;
		}
		Type* getPointeeType() {return pointeeType;}
		virtual ~PointerType() {if (pointeeType != NULL) {delete pointeeType;}}
		string getLLVMName() const {
			if (pointeeType != NULL) {
				return pointeeType->getLLVMName() + " *";
			} else {
				return "i8 *"; //Pointer to void not allowed, this used instead
			}
		}
	private:
		Type* pointeeType = NULL;
};

class TypeList {
	public:
		TypeList(Type* item, TypeList* next) : item(item), next(next) {}
		TypeList(Type* item) : item(item), next(NULL) {}
		Type* getItem() const {return this->item;}
		TypeList* getNext() const {return this->next;}
		virtual bool equals(const TypeList& other) const {
			if (this->getNext() == NULL && \
					other.getNext() == NULL) {
				return item->equals(*other.getItem());
			} else if (this->getNext() == NULL || \
					other.getNext() == NULL) {
				return false;
			} else {
				return item->equals(*other.getItem()) && \
					next->equals(*other.getNext());
			}
		}
		unsigned int getStructSize() {
			//This should only be used on the first element in the list
			return this->getStructSize(0);
		}
		unsigned int getUnionSize() {
			//This should only be used on the first element in the list
			return this->getUnionSize(0);
		}
		string getName() const {
			string ret = "";
			if (item != NULL) {ret += item->getName();}
			if (next != NULL) {ret += " " + next->getName();}
			return ret;
		}
		string getLLVMName() const {
			string ret = "";
			if (item != NULL) {ret += item->getLLVMName();}
			if (next != NULL) {ret += next->getLLVMName();}
			return ret;
		}
		virtual ~TypeList() {
			if (item != NULL) {delete item;}
			if (next != NULL) {delete next;}
		}
	protected:
		unsigned int getStructSize(unsigned int current) {
			unsigned int itemSize = item->getSize();
			if (next == NULL) {
				if (current % itemSize == 0) {
					return current+itemSize;
				} else {
					return (current/itemSize + 1)*itemSize + itemSize;
				}
			}
			if (current % itemSize == 0) {
				return next->getStructSize(current+itemSize);
			} else {
				return next->getStructSize((current/itemSize + 1)*itemSize + itemSize);
			}
		}
		unsigned int getUnionSize(unsigned int current) {
			unsigned int itemSize = item->getSize();
			unsigned int curMax = itemSize > current ? itemSize : current;
			if (next == NULL) {
				return curMax;
			} else {
				return next->getUnionSize(curMax);
			}
		}
		Type* item = NULL;
		TypeList* next = NULL;
};

class FunctionType : public Type {
	public:
		FunctionType(Type* returnType, TypeList* params) : Type(FUNCTION, 8), \
   returnType(returnType), params(params) {}
		virtual bool equalsSubType(const FunctionType& other) const {
			return returnType->equals(*other.getReturnType()) && \
				params->equals(*other.getParams());
		}
		virtual bool equals(const Type& other) const {
			//Try to downcast
			if (const FunctionType* otherDown = \
					dynamic_cast<const FunctionType*>(&other)) {
				return this->equalsSubType(*otherDown);
			} else {
				return false;
			}
		}
		string getName() const {
			string ret = "";
			if (returnType != NULL) {ret += returnType->getName();}
			ret += " (*) (";
			if (params != NULL) {ret += params->getName();}
			ret += ")";
			return ret;
		}
		Type* getReturnType() const {return this->returnType;}
		TypeList* getParams() const {return this->params;}
		virtual ~FunctionType() {
			if (returnType != NULL) {delete returnType;}
			if (params != NULL) {delete params;}
		}
		string getLLVMName() const {
			string ret = "";
			if (returnType != NULL) {
				ret += returnType->getLLVMName();
			} else {
				ret += "void";
			}
			ret += " (";
			if (params != NULL) {
				ret += params->getLLVMName();
			}
			ret += ")";
			return ret;
		}
	private:
		Type* returnType = NULL;
		TypeList* params = NULL;
};

class StructType : public Type {
	public:
		StructType(TypeList* members) : Type(STRUCT, members->getStructSize()), \
										members(members) {}
		TypeList* getMembers() const {return this->members;}
		virtual bool equals(const Type& other) const {
			if (const StructType* otherDown = \
					dynamic_cast<const StructType*>(&other)) {
				return members->equals(*otherDown->getMembers());
			} else {
				return false;
			}
		}
		string getName() const {
			string ret = "struct {";
			if (members != NULL) {ret += members->getName();}
			ret += "}";
			return ret;
		}
		string getLLVMName() const {
			string ret = "{";
			if (members != NULL) {ret += members->getLLVMName();}
			ret += "}";
			return ret;
		}
		virtual ~StructType() {
			if (members != NULL) {delete members;}
		}
	private:
		TypeList* members = NULL;
};

class UnionType : public Type {
	public:
		UnionType(TypeList* members) : Type(STRUCT, members->getUnionSize()), \
									   members(members) {}
		TypeList* getMembers() const {return this->members;}
		virtual bool equals(const Type& other) const {
			if (const UnionType* otherDown = \
					dynamic_cast<const UnionType*>(&other)) {
				return members->equals(*otherDown->getMembers());
			} else {
				return false;
			}
		}
		string getName() const {
			string ret = "union {";
			if (members != NULL) {ret += members->getName();}
			ret += "}";
			return ret;
		}
		string getLLVMName() const {
			//TODO: Implement as union, not struct
			string ret = "{";
			if (members != NULL) {ret += members->getLLVMName();}
			ret += "}";
			return ret;
		}
		virtual ~UnionType() {
			if (members != NULL) {delete members;}
		}
	private:
		TypeList* members = NULL;
};

//Represents one entry in a symbol table
class Symbol {
	public:
		Symbol(Type* type) : type(type) {}
		Type* getType() {
			if (type != NULL) return type;
			type = new NoType();
			return type;
		}
		~Symbol() {
			if (type != NULL) {delete type;}
		}
	private:
		Type* type = NULL;
};

class SymbolTable {
	public:
		SymbolTable() {}
		Symbol* find(const string input) {
			auto search = mVarType.find(input);
			if (search != mVarType.end()) {
				return search->second;
			} else {
				return NULL;
			}
		}
		bool insert(string key, Type* val) {
			if (this->find(key) != NULL) {
				return false; //Already in the list, can't insert
			} else {
				mVarType[key] = new Symbol(val);
				return true;
			}
		}
		bool remove(string key) {
			if (this->find(key) != NULL) {
				mVarType.erase(key);
				return true;
			} else {
				return false; //Not found, can't delete
			}
		}
	private:
		map<string, Symbol*> mVarType;
};

class Scope {
	public:
		Scope() : table(new SymbolTable()), enclosing(NULL), tempNum(0) {}
		Scope(Scope* enclosing) : table(new SymbolTable()), \
								  enclosing(enclosing), tempNum(0) {}
		Symbol* find(string key) {
			Symbol* ret = NULL;
			if (table != NULL) {
				Symbol* localSearch = table->find(key);
				if (localSearch == NULL) {
					if (enclosing != NULL) {
						Symbol* globalSearch = enclosing->find(key);
						if (globalSearch != NULL) {
							ret = globalSearch;
						}
					}
				} else {
					ret = localSearch;
				}
			} else {
				if (enclosing != NULL) {
					Symbol* globalSearch = enclosing->find(key);
					if (globalSearch != NULL) {
						ret = globalSearch;
					}
				}
			}
			return ret;
		}
		bool insert(string str, Type* type) {
			return table->insert(str, type);
		}
		bool remove(string str) {
			return table->remove(str);
		}
		unsigned int getTemp() {return ++tempNum;} //Returns next temporary
		unsigned int peekTemp() {return tempNum;} //Returns last used temporary
		void setTemp(unsigned int val) {tempNum = val;}
		virtual ~Scope() {
			if (table != NULL) {delete table;}
		}
	private:
		SymbolTable* table = NULL;
		Scope* enclosing = NULL;
		unsigned int tempNum = 0;
};

class TypeError : public runtime_error {
	public:
		TypeError(string err) : runtime_error(err) {}
		TypeError(char* err) : runtime_error(err) {}
};


template<class T>
bool NodeList<T> :: typeCheck(Scope* s) {
	if (next == NULL && item == NULL) {
		return true;
	} else if (next == NULL && item != NULL) {
		return item->typeCheck(s);
	} else if (next != NULL && item == NULL) {
		//Should not happen very much
		return next->typeCheck(s, item->getType(s));
	} else {
		return item->typeCheck(s) && next->typeCheck(s, item->getType(s));
	}
}

template<class T>
bool NodeList<T> :: typeCheck(Scope* s, Type* t) {
	if (next == NULL) {
		return item->typeCheck(s) && *t == *item->getType(s);
	} else {
		return item->typeCheck(s) && *t == *item->getType(s) && next->typeCheck(s, t);
	}
}

template<class T>
string NodeList<T> :: genLLVM(Scope* s, Consumer<string>* o) {
	if (item != NULL) {
		item->genLLVM(s, o);
	}
	if (next != NULL) {
		o->put(this->delimiter);
		next->genLLVM(s, o);
	}
	return "";
}

template<class T>
string NodeList<T> :: getLLVMName() const {
	string ret = "";
	if (item != NULL) {ret += item->getLLVMName();}
	if (next != NULL) {ret += ", " + next->getLLVMName();}
	return ret;
}

template<class T>
TypeList* NodeList<T> :: getTypes(Scope* s) {
	if (next != NULL && item != NULL) {
		return new TypeList(item->getType(s), next->getTypes(s));
	} else if (item != NULL) {
		return new TypeList(item->getType(s));
	} else {
		return new TypeList(new NoType());
	}
}

