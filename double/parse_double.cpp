#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "Stream_With_Lastchar.h"
#include "parse_double.h"

static double parse_float(Stream_With_Lastchar* swl) {
	double ans = 0.0;
	double base = 1.0;

	while( isdigit(swl->LastChar) ) {
		base /= 10;
		ans += base*(swl->LastChar-'0');
		swl->getc();
	}
	return ans;
}

static int parse_int(Stream_With_Lastchar* swl) {
	int ans = 0;
	while( isdigit(swl->LastChar) ) {
		ans = ans*10+(swl->LastChar-'0');
		swl->getc();
	}
	return ans;
}

// parse a double as long as I can, so make sure this is a number before calling
double parse_double(Stream_With_Lastchar* swl, int*status) {
	// assume parse [flag] a.b E c
	while( isspace(swl->LastChar) ) swl->getc();
	if(swl->LastChar == EOF) {
		// failed
		if(status) *status = -1;
		return 0.0;
	}
	int flag=1;
	int a = 0;
	double b = 0.0;
	int c = 0;
	
	if(swl->LastChar == '+' || swl->LastChar == '-') {
		if(swl->LastChar == '-') flag = -1;
		swl->getc();
	}

	if(swl->LastChar == EOF) {
		if(status) *status = -1;
	   	return 0.0; //failed
	}

	if(swl->LastChar == '.') {
		swl->getc();
		if( swl->LastChar==EOF || !isdigit(swl->LastChar) ) {
			if(status) *status = -1;
			return 0.0; // failed
		}
		b = parse_float(swl);
	} else if( isdigit(swl->LastChar) ) {
		a = parse_int(swl);
		if(swl->LastChar == '.') {
			swl->getc();
			b = parse_float(swl);
		}
	} else {
		// failed
		if(status) *status = -1;
		return 0.0;
	}

	// align to e/E 
	if( swl->LastChar=='e' || swl->LastChar == 'E') {
		swl->getc();
		int flag2 = 1;
		if(swl->LastChar == '+' || swl->LastChar == '-') {
			if(swl->LastChar == '-') flag2 = -1;
			swl->getc();
		}
		if( !isdigit(swl->LastChar) ) {
			// failed
			if(status) *status = -1;
			return 0.0;
		}
		c = parse_int(swl);
		c *= flag2;
	} 

	if(status) status = 0;
	return flag*(a+b)*pow(10, c);
}
