#include "translation.h"

int main(int argc, char *argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "translation.h";
	}
	ifstream filestream = ifstream(filename);
	stringstream stream1to2 = stringstream();
	return phase1(&filestream, &stream1to2);
}

int phase1(istream *input, ostream *output) {
	bool eof = false;
	while(!eof) {
		char read = input->get();
		if (input->eof()) {
			eof = true;
			break;
		}
		while (read == '\\' && input->peek() == '\n') {
			input->get();
			read = input->get();
		}
		cout << read << ": " << (int) read << '\n';
		output->put(read);
	}
	return 0;
}

