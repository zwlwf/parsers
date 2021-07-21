#ifndef __STREAM_WITH_LAST_CHAR__
#define __STREAM_WITH_LAST_CHAR__
#include <stdio.h>
#include <stdlib.h>

struct Position {
	int rows;
	int cols;
};

// char be pass between different parser
class Stream_With_Lastchar {
	public: 
	Position rc;
	int LastChar;
	virtual void getc() = 0; 
	// c LastChar [cursor], push_back LastChar, and set LastChar as c
	virtual void ugetc(int c) = 0;
	bool eof() {
		return LastChar == EOF;
	}
	// useful for debug
};

class File_Stream_With_Lastchar : public Stream_With_Lastchar {
	public:
	FILE* fp;
	File_Stream_With_Lastchar( FILE* fp_, int startChar = ' ' ) {
		rc.rows = 1;
		rc.cols = 0;
		fp = fp_;
		LastChar = startChar;
	}

	virtual void getc() {
		if(LastChar == '\n') {
			rc.rows++;
			rc.cols=0;
		} else if(LastChar!=EOF) {
			rc.cols++;
		}
		LastChar = fgetc(fp);
	}

	virtual void ugetc(int c) {
		if(LastChar=='\n') { // now cols is undetermined
			rc.rows--;
		}
		ungetc(LastChar, fp);
		LastChar = c;
	}
};

class String_Stream_With_Lastchar : public Stream_With_Lastchar {
	public:
	const char* s;
	int pos_;
	String_Stream_With_Lastchar( const char* const sp_, int startChar = ' ' ) {
		rc.rows = 1;
		rc.cols = 0;
		s = sp_;
		pos_ = 0;
		LastChar = startChar;
	}

	virtual void getc() {
		if(LastChar == '\n') {
			rc.rows++;
			rc.cols=0;
		} else if(LastChar!=EOF) {
			rc.cols++;
		}
		if( !s[pos_] ) LastChar = EOF; // change the \0 to EOF in LastChar
		else LastChar = s[pos_++];
	}

	virtual void ugetc(int c) {
		if( pos_ == 0 ) return;
		if(LastChar=='\n') { // now cols is undetermined
			rc.rows--;
		}
		--pos_;
		LastChar = c;
	}
};

double parse_double( Stream_With_Lastchar* swl );

#endif
