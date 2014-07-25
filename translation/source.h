#include <sstream>
#include <deque>
#include <list>
#include <map>
#include "tokenizer.h"

using namespace std;

//! A type of exception relating input/output operations
class IOException : public runtime_error {
	public:
		IOException(string w) : runtime_error(w) {}
		IOException(char* w) : runtime_error(w) {}
};

class SyntaxException : public runtime_error {
	public:
		SyntaxException(string w) : runtime_error(w) {}
		SyntaxException(char* w) : runtime_error(w) {}
};


template<class T>
class Source
{
	public:
		virtual T get() = 0; //Gets the next item from the source
		virtual bool empty() = 0; //True if source has no more items
		virtual ~Source() {}
		Position getPosition() {return this->position;}
		void newLine() {position.setLine(position.getLine() + 1);
			position.setColumn(1);}
		void incrementPosition(unsigned int line, unsigned int column)
		{
			this->position.setLine(line+this->position.getLine());
			this->position.setColumn(column+this->position.getColumn());
		}
		void incrementPosition(Position inc)
		{
			this->incrementPosition(inc.getLine(), inc.getColumn());
		}
	private:
		Position position;
		
};

template<class T> 
class StreamSource : public virtual Source<T> {
	public:
	   	StreamSource(basic_istream<T>* i) : inputstream(i) {}
		StreamSource(string filename) : inputstream\
										(new ifstream(filename)) {}
		virtual T get() {T c; inputstream->get(c); return c;}
		bool empty() {return inputstream->eof();}
		virtual ~StreamSource() {delete inputstream;}
	private:
		basic_istream<T>* inputstream;
};

template<class T>
class BufferedSource : public Source<T> {
	public:
		BufferedSource(string filename) : source(new StreamSource<char>(filename)), \
				que(*new deque<T>()) {this->used = 0;}
		BufferedSource(Source<T>* s) : source(s), que(*new deque<T>()) {
				this->used = 0;
			}
		T get();
		T peek();
		T peek(unsigned int ahead);
		void reset() {used = 0;} //Move stream back to beginning of buffer
		void clear() {
			this->trim(0);
			this->reset();
		}
		void clearUsed();
		void trim(unsigned int leave) {
			while (que.size() > leave) {
				que.pop_front();
			}
			used = 0;
		} //Trim down buffer to the size of leave (if needed) and reset to beginning
		//of trimmed buffer
		/*bool empty() {return source->empty() && (this->used >= \
				this->que.size());}*/
		bool empty() {return source->empty();}
		virtual ~BufferedSource() {}
		unsigned int bufferSize() {return used;}
	private:
		unsigned int used;
		Source<T>* source;
		deque<T>& que;
};


template<class From, class To>
class Mapping {
};

template<class From, class To>
class Phase : public Source<To>, public Mapping<From, To> {
	public:
		Phase(string filename) : source(*new StreamSource<From>(filename)) {}
		Phase(Source<From> *s) : source(*s) {}
		Phase(Source<From>& s) : source(s) {}
		//Phase(BufferedSource<From>& s) : source(s) {}
		Source<From>& source;
};

template <class T>
T BufferedSource<T> :: get() {
	//If we've already used all items we've fetched from the unbuffered source, 
	//fetch one more, add it to the buffer and return it.
	if (this->used >= this->que.size()) {
		T gotten = this->source->get();
		que.push_back(gotten);
		used++; //Since we're using the item that we just got
		return gotten;
		//Otherwise, return the next element in the buffer and move one step
	} else {
		return que.at(used++);
	}
}

template <class T>
T BufferedSource<T> :: peek() {
	unsigned int temp_used = this->used;
	T val = this->get();
	this->used = temp_used; //Do not 'use' the element
	return val;
}

//Look ahead as many elements as given in the argument.
//@param ahead How many elements to look ahead. 0 (zero) is the same as peek().
template <class T>
T BufferedSource<T> :: peek(unsigned int ahead) {
	unsigned int old_used = this->used;
	T token = this->get();
	for (int i = ahead; i > 0; --i) {
		token = this->get();
	}
	this->used = old_used; //Restore the number of used items
	return token;
}

template <class T>
void BufferedSource<T> :: clearUsed() {
	for (unsigned int i = 0; i < this->used; ++i) {
		this->que.pop_front();
	}
	this->used = 0;
}

