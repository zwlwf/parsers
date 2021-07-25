#include "Stream_With_Lastchar.h"
#include <ctype.h>
#include <math.h>

// parse number like, .1234, where .1234 has been eatten, in fact this can be a private member function of Stream_With_Lastchar
static double parse_float( Stream_With_Lastchar *swl) {
	double ans = 0.0;
	double base = 1.0;
	while( isdigit( swl->LastChar) ) {
		base /= 10;
		ans += base*(swl->LastChar - '0');
		swl->getc();
	}
	return ans;
}

static int parse_uint( Stream_With_Lastchar *swl ) {
	int ans = 0;
	while( isdigit( swl->LastChar) ) {
		ans = ans*10 + (swl->LastChar - '0');
		swl->getc();
	}
	return ans;
}

// the caller should make sure : swl have constructed, and the data in stream is a double
double parse_double( Stream_With_Lastchar* swl ) {
	while( isspace(swl->LastChar) ) swl->getc();

	if( swl->LastChar == EOF ) {
		//failed
		return 0.0;
	}

	int flag = 1;
	int a = 0;
	double b = 0.0;
	int c = 0;
	if( swl->LastChar == '+' || swl->LastChar == '-') {
		if(swl->LastChar == '-') flag = -1;
		swl->getc();
	}

	if( swl->LastChar == EOF ) return 0.0; // Failed

	if( swl->LastChar == '.') {
		swl->getc();
		if(swl->LastChar == EOF || !isdigit( swl->LastChar ) ) return 0.0; // failed
		b = parse_float(swl);
	} else if( isdigit( swl->LastChar ) ) {
		a = parse_uint(swl);
		if( swl->LastChar == '.') {
			swl->getc();
			b = parse_float(swl);
		}
	} else {
		// failed
		return 0.0;
	}

	// align to e/E
	if( swl->LastChar == 'e' || swl->LastChar == 'E') {
		swl->getc();
		int flag2 = 1;
		if(swl->LastChar == '+' || swl->LastChar == '-') {
			if(swl->LastChar == '-' ) flag2 = -1;
			swl->getc();
		}
		if( !isdigit(swl->LastChar) ) {
			// failed
			return 0.0;
		}
		c = parse_uint(swl);
		c *= flag2;
	}

	return flag*(a+b)*pow(10,c);
}
