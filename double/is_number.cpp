#include <string>

bool isNumber(string s) {
	int i=0;
	while( i<s.size() && isspace(s[i]) ) i++;

	if(i==s.size() ) return false;
	if( s[i]=='+' || s[i]=='-') i++;

	if( i==s.size() ) return false;

	if( s[i]=='.') {
		i++;
		if(i==s.size() || !isdigit(s[i]) ) return false; // -. is not ok
		while(i<s.size() && isdigit(s[i])) i++;
	} else if( isdigit(s[i]) ) {
		while(i<s.size() && isdigit(s[i])) i++;
		if( i==s.size() ) return true;
		if( s[i]=='.') i++;
		// if(i==s.size()) return false; // "1." is ok
		while(i<s.size() && isdigit(s[i])) i++;
	} else return false;

	if( i==s.size() ) return true;

	if( s[i]=='e' || s[i]=='E' ) {
		i++;
		if(i==s.size() ) return false;
		if(s[i]=='+' || s[i]=='-') i++;
		if(i==s.size() ) return false;
		if(!isdigit(s[i]) ) return false;
		while(i<s.size() && isdigit(s[i]) ) i++;
	}

	if(i==s.size()) return true;

	if( s[i]==' ') {
		while( i<s.size() && s[i]==' ') i++;
		if(i==s.size()) return true;
	}

	return false;
}
