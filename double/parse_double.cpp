#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

static int LastChar = ' ';

static double parse_float(FILE* fp) {
	//LastChar = fgetc(fp); // eat .
	double ans = 0.0;
	double base = 1.0;

	while( isdigit(LastChar) ) {
		base /= 10;
		ans += base*(LastChar-'0');
		LastChar = fgetc(fp);
	}
	return ans;
}

static int parse_int(FILE* fp) {
	int ans = 0;
	while( isdigit(LastChar) ) {
		ans = ans*10+(LastChar-'0');
		LastChar = fgetc(fp);
	}
	return ans;
}

// parse a double as long as I can, so make sure this is a number before calling
double parse_double(FILE* fp) {
	// assume parse [flag] a.b E c
	while( isspace(LastChar) ) LastChar = fgetc(fp);
	if(LastChar == EOF) {
		// failed
		return 0.0;
	}
	int flag=1;
	int a = 0;
	double b = 0.0;
	int c = 0;
	
	if(LastChar == '+' || LastChar == '-') {
		if(LastChar == '-') flag = -1;
		LastChar = fgetc(fp);
	}

	if(LastChar == EOF) return 0.0; //failed

	if(LastChar == '.') {
		LastChar = fgetc(fp);
		if( LastChar==EOF || !isdigit(LastChar) ) return 0.0; // failed
		b = parse_float(fp);
	} else if( isdigit(LastChar) ) {
		a = parse_int(fp);
		if(LastChar == '.') {
			LastChar = fgetc(fp);
			b = parse_float(fp);
		}
	} else {
		// failed
		return 0.0;
	}

	// align to e/E 
	if( LastChar=='e' || LastChar == 'E') {
		LastChar = fgetc(fp);
		int flag2 = 1;
		if(LastChar == '+' || LastChar == '-') {
			if(LastChar == '-') flag2 = -1;
			LastChar = fgetc(fp);
		}
		if( !isdigit(LastChar) ) {
			// failed
			return 0.0;
		}
		c = parse_int(fp);
		c *= flag2;
	} 

	return flag*(a+b)*pow(10, c);
}

int main() {
	double d;
	printf("input a double>>");
	d = parse_double(stdin);
	printf("d = %lf\n", d);
	return 0;
}

