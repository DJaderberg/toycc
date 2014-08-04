#include <string>
using namespace std;

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
/* Implemened as a singly linked list
 */
template<class T>
class NodeList : public Node {
	public:
		NodeList(T* item) : item(item), next(NULL) {}
		NodeList(T* item, NodeList* next) : item(item), \
															  next(next) {}
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
	protected:
		T* item;
		NodeList* next = NULL; //Optional, might be NULL
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
	protected:
		Parser* parser;
	private:
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
		virtual bool operator ==(const Type& other) const {
			return this->equals(other);
		}
		virtual bool equals(const Type& other) const = 0;
		bool operator !=(const Type& other) const {
			return !(*this == other);
		}
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
				case LONG_DOUBLE : return 8;
			}
		}
	private:
		BasicTypeEnum basicType;
};

class PointerType : public Type {
	public:
		PointerType(Type& pointeeType) : Type(POINTER, 8), \
										 pointeeType(pointeeType) {}
		virtual bool equals(const Type& other) const {
			//Try to downcast
			if (const PointerType* otherDown = \
					dynamic_cast<const PointerType*>(&other)) {
				return pointeeType.equals(*otherDown);
			}
			return false;
		}
	private:
		Type& pointeeType;
};

class TypeList {
	public:
		TypeList(Type& item, TypeList* next) : item(item), next(next) {}
		TypeList(Type& item) : item(item), next(NULL) {}
		Type& getItem() const {return this->item;}
		TypeList* getNext() const {return this->next;}
		virtual bool equals(const TypeList& other) const {
			if (this->getNext() == NULL && \
					other.getNext() == NULL) {
				return item.equals(other.getItem());
			} else if (this->getNext() == NULL || \
					other.getNext() == NULL) {
				return false;
			} else {
				return item.equals(other.getItem()) && \
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
	protected:
		unsigned int getStructSize(unsigned int current) {
			unsigned int itemSize = item.getSize();
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
			return 3;
		}
		unsigned int getUnionSize(unsigned int current) {
			unsigned int itemSize = item.getSize();
			unsigned int curMax = itemSize > current ? itemSize : current;
			if (next == NULL) {
				return curMax;
			} else {
				return next->getUnionSize(curMax);
			}
		}
		Type& item;
		TypeList* next = NULL;
};


class FunctionType : public Type {
	public:
		FunctionType(Type& returnType, TypeList& params) : Type(FUNCTION, 8), \
   returnType(returnType), params(params) {}
		virtual bool equalsSubType(const FunctionType& other) const {
			return returnType.equals(other.getReturnType()) && \
				params.equals(other.getParams());
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
		Type& getReturnType() const {return this->returnType;}
		TypeList& getParams() const {return this->params;}
	private:
		Type& returnType;
		TypeList& params;
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
	private:
		TypeList* members = NULL;
};

