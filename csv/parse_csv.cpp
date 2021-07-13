/*
 * parse csv file into Table( vector<vector<BaseCell*>> ) data structure, follows the instruction of rfc 4180
 * in main function, render the Table as a list of list in python
 */
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

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

static BaseCell* parse_cell(FILE* fp) {
	static int LastChar = '\r';
	std::string ans = "";
	if(LastChar == ',') {
		CurrentCellType = IN_LINE;
		LastChar = fgetc(fp);
	} else if( LastChar == '\r') {
		LastChar = fgetc(fp); // eat \n
		LastChar = fgetc(fp); 
		CurrentCellType = BEGIN_LINE;
	} else if( LastChar == '\n') {
		LastChar = fgetc(fp);
		CurrentCellType = BEGIN_LINE;
	} 

	if(LastChar == EOF) {
		CurrentCellType = END_CSV;
		return NULL;
	}

	if(LastChar == '"') { // parse escaped cell
		while(1) {
			LastChar = fgetc(fp);
			if(LastChar == EOF) return NULL; 
			if(LastChar=='"') {
				LastChar = fgetc(fp);
				if(LastChar == '"') ans+='"';
				else break;
			} else {
				ans+=char(LastChar);
			}
		}
		LastChar = fgetc(fp); // LastChar point to char after "
		if(LastChar != ',' && LastChar != '\r' && LastChar != '\n') return NULL; 
	} else { // parse non-escaped cell
		while( is_textdata(LastChar) ) { 
			ans+=char(LastChar);
			LastChar = fgetc(fp);
		}
		// end with , or CRLF or EOF
		if(LastChar != ',' && LastChar != '\r' && LastChar != '\n' && LastChar != EOF ) return NULL; 
	}

	if(ans.empty()) return new NullCell();
	else return new StringCell(ans);
}

typedef std::vector<std::vector<BaseCell*>> Table;

Table parse_csv(FILE *fp) {
	Table ans;
	while(1) {
		BaseCell *c = parse_cell(fp);
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

int main(int argc, char** argv) {
	FILE *fp = fopen(argv[1], "r");
	if(!fp) {
		printf("Failed to open %s\n", argv[1]);
		return -1;
	}
	Table x = parse_csv(fp);
	render_as_python_listlist(x);
	printf("\n");
	fclose(fp);
	return 0;
}
