/*
 * parse csv file into Table( vector<vector<BaseCell*>> ) data structure, follows the instruction of rfc 4180
 * in main function, render the Table as a list of list in python
 */
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "Stream_With_Lastchar.h"

class BaseCell {
	public:
		virtual ~BaseCell() {}
		virtual void render() = 0;
};

class NumericCell: public BaseCell{
	double val;
	public:
	virtual ~NumericCell() {}
	virtual void render() {
		printf("%g",val);
	}
};

class StringCell: public BaseCell{
	std::string str;
	public:
	StringCell(std::string s) : str(s) {}
	virtual ~StringCell() {}
	virtual void render() {
		printf("'%s'", str.c_str());
	}
};

class NullCell : public BaseCell {
	public:
		virtual ~NullCell() { }
		virtual void render() {
			printf("None");
		}
};

bool is_textdata(int c) {
	return c!=EOF && c!=',' && c!='\r' && c!='\n'; // in order to read utf-8
	return ( c>=0x20 && c<=0x21 ) || // follow the instruction in rfc 4180
		(c>=0x23 && c<=0x2b) ||
		(c>=0x2d && c<=0x7e);
}

enum cellType {
	IN_LINE=0,
	BEGIN_LINE,
	END_CSV,
};

static int CurrentCellType;

static BaseCell* parse_cell(Stream_With_Lastchar* swl) {
	std::string ans = "";
	if(swl->LastChar == ',') {
		CurrentCellType = IN_LINE;
		swl->getc();
	} else if( swl->LastChar == '\r') {
		swl->getc(); // eat \n
		swl->getc(); 
		CurrentCellType = BEGIN_LINE;
	} else if( swl->LastChar == '\n') {
		swl->getc();
		CurrentCellType = BEGIN_LINE;
	} 

	if(swl->LastChar == EOF) {
		CurrentCellType = END_CSV;
		return NULL;
	}

	if(swl->LastChar == '"') { // parse escaped cell
		while(1) {
			swl->getc();
			if(swl->LastChar == EOF) return NULL; 
			if(swl->LastChar=='"') {
				swl->getc();
				if(swl->LastChar == '"') ans+='"';
				else break;
			} else {
				ans+=char(swl->LastChar);
			}
		}
		swl->getc(); // LastChar point to char after "
		if(swl->LastChar != ',' && swl->LastChar != '\r' && swl->LastChar != '\n') return NULL; 
	} else { // parse non-escaped cell
		while( is_textdata(swl->LastChar) ) { 
			ans+=char(swl->LastChar);
			swl->getc();
		}
		// end with , or CRLF or EOF
		if(swl->LastChar != ',' && swl->LastChar != '\r' && swl->LastChar != '\n' && swl->LastChar != EOF ) return NULL; 
	}

	if(ans.empty()) return new NullCell();
	else return new StringCell(ans);
}

typedef std::vector<std::vector<BaseCell*>> Table;

Table parse_csv(Stream_With_Lastchar *swl) {
	Table ans;
	while(1) {
		BaseCell *c = parse_cell(swl);
		if(CurrentCellType==END_CSV) return ans;
		if(!c) {
			printf("Some thing is wrong with your csv file\n");
			return ans;
		}
		if( CurrentCellType == BEGIN_LINE) {
			ans.push_back(std::vector<BaseCell*>());
		}
		ans.back().push_back(c);
	}
}

void freeTable(Table tb) {
	for(int i=0; i<tb.size(); i++) {
		for(int j=0; j<tb[i].size(); j++) 
			delete tb[i][j];
	}
}

// render the csv as a python listof list
static void render_as_python_listlist(Table& tb) { 
	printf("[");
	for(int i=0; i<tb.size(); i++) {
		printf("[");
		for(int j=0; j<tb[i].size(); j++) {
			tb[i][j]->render();
			printf(",");
		}
		printf("],");
	}
	printf("]");
}

