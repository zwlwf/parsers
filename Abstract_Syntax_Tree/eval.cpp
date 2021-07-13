#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include <math.h>

using namespace std;

class BaseAST {
	public:
		virtual double eval()=0;
		virtual ~BaseAST() {}
};

class NumericAST : public BaseAST {
	double val;
	public:
	NumericAST( double v ) : val(v) {}
	virtual ~NumericAST() { 
		//printf("call deconstruct of NumericAST\n");
	}
	virtual double eval() {
		return val;
	}
};

class BinaryAST : public BaseAST {
	int binOp;
	BaseAST *LHS, *RHS;
	public:
	BinaryAST( int b, BaseAST* lhs, BaseAST* rhs) : binOp(b), LHS(lhs), RHS(rhs) {} 
	virtual ~BinaryAST() {
		//printf("call deconstruct of BinaryAST\n");
		delete LHS;
		delete RHS;
	}
	virtual double eval() {
		double l = LHS->eval();
		double r = RHS->eval();
		switch (binOp) {
			case '+' : return l+r;
			case '-': return l-r;
			case '*' : return l*r;
			case '/' : return l/r;
			case '^': return pow(l, r);
			default : return 0;
		}
	}
};

class FunctionAST : public BaseAST {
	std::string funName;
	BaseAST** params;
	int len; // number of parameters of function
	public:
	FunctionAST(std::string fn, BaseAST** pa, int l ) : funName(fn), params(pa), len(l) { }
	virtual ~FunctionAST() {
		//printf("call deconstruct of FunctionAST\n");
		for(int i=0; i<len; i++) delete params[i];
		delete []params;
	}

	virtual double eval() {
		double * pv = new double[len];
		for(int i=0; i<len; i++) {
			pv[i] = params[i]->eval();
		}
		if(funName == "sin") {
			return sin(pv[0]);
		} else if(funName == "exp") {
			return exp(pv[0]);
		} else if(funName == "cos") {
			return cos(pv[0]);
		} else if(funName == "tan") {
			return tan(pv[0]);
		} else if(funName == "acos") {
			return acos(pv[0]);
		} else if(funName == "asin") {
			return asin(pv[0]);
		} else if(funName == "atan") {
			return atan(pv[0]);
		} else if(funName == "atan2") {
			return atan2(pv[0], pv[1]);
		} else if(funName == "cosh") {
			return cosh(pv[0]);
		} else if(funName == "sinh") {
			return sinh(pv[0]);
		} else if(funName == "tanh") {
			return tanh(pv[0]);
		} else if(funName == "log") {
			return log(pv[0]);
		} else if(funName == "log10") {
			return log10(pv[0]);
		} else if(funName == "sqrt") {
			return sqrt(pv[0]);
		} else if(funName == "pow") {
			return pow(pv[0], pv[1]);
		} else {
			printf("function of %s is undefined!\n", funName.c_str());
			exit(-1);
		}
	}
};

static BaseAST* parse_primary();
static BaseAST* parse_bin_op(int old_prec, BaseAST* LHS);
static BaseAST* parse_expression();
static BaseAST** parse_paren_parameters(int*);
static BaseAST* parse_paren_expression();

static int CurrentToken;
static double numeric_val;
static std::string Identifier_str;
FILE* fp;
static std::map<int, int> Precedence;

enum token_type {
	NUM_TOKEN = 0,
	EOF_TOKEN,
	ID_TOKEN,
};

void next_token() {
	static int LastChar = ' ';
	while( isspace(LastChar) ) LastChar = fgetc(fp);

	if( isdigit(LastChar) || LastChar == '.') {
		CurrentToken = NUM_TOKEN;
		//fseek(fp, -1, SEEK_CUR);
		ungetc(LastChar, fp);
		fscanf(fp,"%lf", &numeric_val);
		LastChar = fgetc(fp);
		return;
	} else if( LastChar == EOF ) {
		CurrentToken = EOF_TOKEN;
		return;
	} else if( isalpha(LastChar) ) {
		CurrentToken = ID_TOKEN;
		Identifier_str = "";
		while( isalnum(LastChar) ) {
			Identifier_str += char(LastChar);
			LastChar = fgetc(fp);
		}
	} else {
		CurrentToken = LastChar;
		LastChar = fgetc(fp);
	}
}

static BaseAST* parse_expression() {
	BaseAST* LHS = parse_primary();
	if(!LHS) return NULL;
	LHS = parse_bin_op(0, LHS);
	return LHS;
}

void init_precedence() {
	Precedence['+'] = 10;
	Precedence['-'] = 10;
	Precedence['*'] = 20;
	Precedence['/'] = 20;
	Precedence['^'] = 30;
}

int getBinOpPrecedence() {
	if( CurrentToken==EOF_TOKEN ) return -1;
	if( Precedence.find(CurrentToken) == Precedence.end() ) return -1;
	return Precedence[CurrentToken];
}

static BaseAST* parse_bin_op(int old_prec, BaseAST* LHS) {
	while(1) {
		int now_prec = getBinOpPrecedence();
		if(now_prec<old_prec) return LHS;
		int BinOp = CurrentToken;
		next_token();
		BaseAST* RHS = parse_primary();
		if(!RHS) return NULL;
		int next_prec = getBinOpPrecedence();
		if(next_prec>now_prec) {
			RHS = parse_bin_op(now_prec+1, RHS);
			if(!RHS) return NULL;
		}
		LHS = new BinaryAST( BinOp, LHS, RHS);
	}
}

static BaseAST** parse_paren_parameters(int *len) {
	next_token(); // eat '(
	std::vector<BaseAST*> ans;
	while(CurrentToken != ')') {
		BaseAST* tmp = parse_expression(); // each parameter can be an expression
		if(!tmp) return NULL;
		ans.push_back(tmp);
		if(CurrentToken != ',' && CurrentToken != ')') return NULL; 
		if(CurrentToken == ',')
			next_token(); // eat ,
		if(CurrentToken==EOF_TOKEN) return NULL; // failed to find ')'
	}
	next_token(); // eat )
	BaseAST** ret = new BaseAST*[ans.size()];
	int i;
	for(i=0; i<ans.size(); i++) ret[i] = ans[i];
	*len = ans.size();
	return ret;
}

static map<std::string, double> ConstantTable;

void init_constant_table() {
	ConstantTable["e"] = exp(1);
	ConstantTable["pi"] = atan(1.0)*4.0;
}

static BaseAST* parse_paren_expression() {
	next_token(); // eat (
	BaseAST* V = parse_expression();
	if(CurrentToken != ')') return NULL;
	next_token(); // eat )
	return V;
}

static BaseAST* parse_primary() {
	if( CurrentToken == NUM_TOKEN) {
		BaseAST* ans = new NumericAST(numeric_val);
		next_token();
		return ans;
	} else if( CurrentToken == ID_TOKEN) {
	    std::string	tmp = Identifier_str;
		next_token();
		if( CurrentToken == '(') { // function call AST
			int len;
			BaseAST** params = parse_paren_parameters(&len);
			if(!params) return NULL;
			BaseAST* ans = new FunctionAST( tmp, params, len );
			return ans;
		} else { // constant symbol
			if( ConstantTable.find(tmp) == ConstantTable.end() ) return NULL;
			BaseAST* ans = new NumericAST( ConstantTable[tmp] );
			return ans;
		}
	} else if (CurrentToken == '(') {
		BaseAST* ans = parse_paren_expression();
		return ans;
	}
}

void Driver() {
	while(CurrentToken!=EOF_TOKEN) {
		BaseAST* V = parse_expression();
		if(!V) {
			printf("Syntax error! Please check again\n");
			return;
		}
		printf("%lf\n", V->eval());
		delete V;
	}
}

int main() {
	fp = stdin;
	init_constant_table();
	init_precedence();
	next_token();
	Driver();
	return 0;
}
