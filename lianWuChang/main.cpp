//#include "parse_html.h"
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <string.h>
#include <vector>
#include "Stream_With_Lastchar.h"

static std::string findTarget(Stream_With_Lastchar *swl, const char* t) {
	int n = strlen(t);
	std::string ans;
	for(int i=0; i<n-1; i++) {
		ans += swl->LastChar; 
		swl->getc();
		if(swl->eof()) {
			// error
			break;
		}
	}

	while( 1 ) {
		if(swl->eof()) {
			// error
			break;
		}
		ans += swl->LastChar;
		if( t[n-1]==swl->LastChar && strcmp(ans.c_str()+ans.size()-n, t)==0 ) break;
		swl->getc();
	}
	for(int i=0; i<n; i++) {
		ans.pop_back();
	}
	for(int i=0; i<n-1; i++)
		swl->ugetc(t[n-2-i]);
	return ans;
}

class AttriValue {
	public:
		virtual ~AttriValue() {};
};

class StringAttriValue : public AttriValue {
	std::string val;
	public:
	StringAttriValue( std::string v) : val(v) {}
	virtual ~StringAttriValue() { }
};

class NullAttriValue : public AttriValue {
	public:
		NullAttriValue() {}
		virtual ~NullAttriValue() {}
};

class NameTokenAttriValue : public AttriValue {
	std::string val;
	public:
	NameTokenAttriValue( std::string v) : val(v) {}
	virtual ~NameTokenAttriValue() { }
};

class BaseElement {
	public:
		virtual ~BaseElement() {};
		virtual void render(FILE* out) {}
};

class StringElement: public BaseElement {
	std::string str;
	public:
	StringElement(std::string s) : str(s) {}
	virtual ~StringElement() {}
	virtual void render(FILE *out) {
		fprintf(out, "%s", str.c_str());
	}
};

class TagElement : public BaseElement {
	std::string tagName;
	std::map<std::string, AttriValue*> attrs; 
	std::vector<BaseElement*> children;
	public:
	TagElement(std::string tn, std::map<std::string, AttriValue*> attrs_, std::vector<BaseElement*> cd) : tagName(tn), attrs(attrs_), children(cd) {}
	virtual ~TagElement() {
		for(int i=0; i<children.size(); i++) delete children[i];
		for(auto x : attrs ) delete x.second;
	}

	virtual void render(FILE* out) {
		if( tagName == "script" ||
				tagName == "title" ||
				tagName == "head" ||
				tagName == "style" ) return; // data characters not for rendering

		for(int i=0; i<children.size(); i++) children[i]->render(out);
	}
};

static int CurrentToken=8;
static std::string data_str;

enum token_type {
	ATTR_STRING_TOKEN = 0,
	ATTR_EQUAL_TOKEN,
	ATTR_NAME_TOKEN, // attri name or attri value with name token
	STRING_ELEMENT_TOKEN,
	ERROR_TOKEN,
	START_TAG_TOKEN,
	END_TAG_TOKEN,
	DIRECT_END_TAG_TOKEN,
	EOF_TOKEN
};

enum tag_type { // html element type, innerText depend on the element type
	DIV=0,
};

static std::map<std::string, int> entity_characters(
	{
		{"quot", 34}, // "
		{"amp", '&'},
		{"lt", '<'},
		{"nbsp", 160},
		{"gt", '>'}
	});

static bool STRONG_MODE = false;
static const char* TARGET_STR;

static void next_token( Stream_With_Lastchar *swl ) {
	int PreviousToken = CurrentToken;
	CurrentToken = EOF_TOKEN;
	while(isspace(swl->LastChar) ) swl->getc();
	if(swl->eof()) return;

	while(1) { // eat all comment
		if( swl->LastChar == '<' ) {
			swl->getc();
			if( swl->eof() ) return;
			if(swl->LastChar == '!') { // eat comment
				while( swl->LastChar != '>' && !swl->eof() ) swl->getc();
				if(swl->eof()) return;
				//swl->getc(); // eat >; think the innerText is follow the >
				continue;
			} else if( swl->LastChar == '/') { // end-tag
				swl->getc(); // eat /
				data_str = "";
				while( isalnum(swl->LastChar) ) {
					data_str += tolower(swl->LastChar);
					swl->getc();
				}
				if(swl->LastChar!='>') {
					fprintf(stderr, "Unclosed end tag at rows=%d, cols=%d\n", swl->rc.rows, swl->rc.cols);
					CurrentToken = ERROR_TOKEN;
					return;
				}
				CurrentToken = END_TAG_TOKEN;
				//swl->getc(); // eat >
				return;
			} else if( isalpha(swl->LastChar) ) {
				CurrentToken = START_TAG_TOKEN;
				data_str = "";
				while( isalnum(swl->LastChar) ) {
					data_str += tolower(swl->LastChar);
					swl->getc();
				}
				return;
			} else { // an ordinary <, 
				fprintf(stderr, "Unexpected char after < at rows=%d, cols=%d\n", swl->rc.rows, swl->rc.cols);
				CurrentToken = ERROR_TOKEN;
				return;
			}
		} else if (swl->LastChar == '>') { // String Elememnt
			swl->getc(); // eat >
			if( STRONG_MODE ) { // for some data character in tags like script
				data_str = findTarget(swl, TARGET_STR);
				if( swl->eof() ) { // not found the target until eof
					CurrentToken = ERROR_TOKEN;
					return;
				}
				STRONG_MODE = false;
			} else {
				data_str = "";
				while(1) {
					if( swl->eof() ) {
						break; // end normally
					}
					if( swl->LastChar == '&') {
						swl->getc();
						if(swl->LastChar == '#') { // &#61
							int tmp = 0;
							while( !isspace(swl->LastChar) && swl->LastChar!=';' ) {
								if(swl->eof()) break;
								tmp=tmp*10 + (swl->LastChar-'0');
								swl->getc();
							}
							if( swl->LastChar == ';') swl->getc();
							if( tmp>=256 ) {
								fprintf(stderr,"Unexpected numeric char ref at rows=%d, cols=%d\n", swl->rc.rows, swl->rc.cols);
								CurrentToken = ERROR_TOKEN;
								return;
							}
							data_str += tmp;
						} else if( isalpha(swl->LastChar) ) {
							std::string tmp;
							while( !isspace(swl->LastChar) && swl->LastChar!=';' &&
									isalpha(swl->LastChar) ) {
								if(swl->eof()) break;
								tmp+=swl->LastChar;
								swl->getc();
							}
							if( entity_characters.count(tmp) ) {
								data_str += entity_characters[tmp];
								if(swl->LastChar == ';') swl->getc();
							} else {
								fprintf(stdout,"Warning: Unrecognized entity character: &%s at rows=%d, cols=%d\n", tmp.c_str(), swl->rc.rows, swl->rc.cols);
								//CurrentToken = ERROR_TOKEN;
								//return;
								// be kind!
								data_str += "&";
								data_str += tmp;
							}
						} else { // just norm &
							data_str += '&';
							swl->getc();
						}
					} else if( swl->LastChar == '<') {
						//break;
						swl->getc();
						if( swl->LastChar == '!' || 
							swl->LastChar == '/' ||
							isalpha(swl->LastChar) ) {
							swl->ugetc('<');
							break; // over
						} else {
							data_str += '<';
							continue;
						}
					} else {
						data_str += swl->LastChar;
						swl->getc();
					}
				}
			}

			if(data_str.size() > 0 ) {
				CurrentToken = STRING_ELEMENT_TOKEN;
				return;
			} else 
				continue;
		} else if ( isalpha(swl->LastChar) ) { // AttriName or AttriValue with Name token type
			data_str = "";
			if(PreviousToken != ATTR_EQUAL_TOKEN) {
				while( isalpha(swl->LastChar) ||
						swl->LastChar == '.' ||
						swl->LastChar == '-' ||
						swl->LastChar == ':' ||
						swl->LastChar == '_' ||
						isdigit(swl->LastChar) ) {
					data_str += swl->LastChar;
					swl->getc();
				}
			} else {
				while( swl->LastChar!= ' ' && swl->LastChar!= '>') {
					if(swl->eof()) {
						fprintf(stderr, "Unexpected end !");
						CurrentToken = ERROR_TOKEN;
						return;
					}
					data_str += swl->LastChar;
					swl->getc();
				}
			}
			CurrentToken = ATTR_NAME_TOKEN;
			return;
		} else if ( swl->LastChar == '=' ) {
			CurrentToken = ATTR_EQUAL_TOKEN;
			data_str = "="; // set for debug
			swl->getc();
			return;
		} else if ( swl->LastChar == '\'' || swl->LastChar == '"') { // AttriValue of string literal
			int quota = swl->LastChar;
			swl->getc();
			CurrentToken = ATTR_STRING_TOKEN;
			data_str = "";
			while( swl->LastChar != quota ) {
				if( swl->eof() ) {
					fprintf(stderr, "Uncompleted attribute string at rows = %d, cols = %d\n", swl->rc.rows, swl->rc.cols);
					CurrentToken = ERROR_TOKEN;
				   	return; // error
				}
				if( swl->LastChar == '&') {
					swl->getc();
					if(swl->LastChar == '#') { // &#61
						int tmp = 0;
						while( !isspace(swl->LastChar) && swl->LastChar!=';' ) {
							if(swl->eof()) break;
							tmp=tmp*10 + (swl->LastChar-'0');
							swl->getc();
						}
						if( tmp>=256 ) {
							fprintf(stderr,"Unexpected numeric char ref at rows=%d, cols=%d\n", swl->rc.rows, swl->rc.cols);
							CurrentToken = ERROR_TOKEN;
							return;
						}
						if( swl->LastChar == ';') swl->getc();
						data_str += tmp;
					} else if( isalpha(swl->LastChar) ) {
						std::string tmp;
						while( !isspace(swl->LastChar) && swl->LastChar!=';'
								&& isalpha(swl->LastChar) ) {
							if(swl->eof()) break;
							tmp+=swl->LastChar;
							swl->getc();
						}
						if( entity_characters.count(tmp) ) {
							data_str += entity_characters[tmp];
							if(swl->LastChar == ';') swl->getc();
						} else {
							fprintf(stdout,"Warning: Unrecognized entity character: &%s at rows=%d, cols=%d\n", tmp.c_str(), swl->rc.rows, swl->rc.cols);
							//CurrentToken = ERROR_TOKEN;
							//return;
							data_str += "&";
							data_str += tmp;
						}
						if(swl->LastChar == ';') swl->getc();
					} else { // just norm &
						data_str += '&';
						swl->getc();
					}
				} else {
					data_str += swl->LastChar;
					swl->getc();
				}
			}
			swl->getc(); // eat "
			return;
		} else if(swl->LastChar == '/') {
			CurrentToken = DIRECT_END_TAG_TOKEN;
			swl->getc();
			return;
		} else {
			// error
			fprintf(stderr, "Unexpected token start at rows = %d, cols = %d\n", swl->rc.rows, swl->rc.cols);
			CurrentToken = ERROR_TOKEN;
			return;
		}
	}
}

// now we are at start of elemnt like <div
BaseElement* parse_element( Stream_With_Lastchar *swl) {
	if(CurrentToken != START_TAG_TOKEN ) {
		fprintf(stderr, "not a start tag at rows = %d, cols = %d\n", swl->rc.rows, swl->rc.cols);
		return NULL;
	}
	std::string currentTagName = data_str;
	printf("--->parse a tag : %s\n", data_str.c_str());
	// set strong mode
	if( currentTagName == "script") {
		STRONG_MODE = true;
		TARGET_STR = "</script>";
	} else {
		STRONG_MODE = false;
	}

	next_token(swl);
	std::map<std::string, AttriValue*> attrs;

	while (CurrentToken == ATTR_NAME_TOKEN) {
		std::string attrName = data_str;
		next_token(swl);
		if(CurrentToken == ATTR_EQUAL_TOKEN) {
			next_token(swl);
			std::string attr_val = data_str;
			if( CurrentToken == ATTR_NAME_TOKEN) {
				attrs[attrName] = new NameTokenAttriValue(attr_val);
			} else // ATTR_STRING_TOKEN
				attrs[attrName] = new StringAttriValue(attr_val);
			next_token(swl);
		} else if (CurrentToken == ATTR_NAME_TOKEN) {
			attrs[attrName] = new NullAttriValue();
		} else {
			//Unexpected Token, throw this attri
			break;
		}
	}
	
	std::vector<BaseElement*> children;
	// parse nested elements
	do {
		if( CurrentToken == DIRECT_END_TAG_TOKEN) {
			next_token(swl);
			break;
		}
		if( CurrentToken == EOF_TOKEN ) break; // Add end_tag by my self, although not completed
		//if( CurrentToken == ERROR_TOKEN ) return NULL; // error happened
		if( CurrentToken == ERROR_TOKEN ) break; // error happened, be kind, the rest still return

		if( currentTagName == "br" || 
				currentTagName == "meta" ||
				currentTagName == "link" ||
				currentTagName == "base" ||
				currentTagName == "input" ||
				currentTagName == "img" ||
				currentTagName == "hr"
				) { // no end-tag TagElement
			break;
		}
		
		/*
		if(STRONG_MODE) { // strong tag, read all text until </script>, and now we are at LastChar='>'
			if(!data_str.empty())
				children.push_back( new StringElement(data_str) );
			STRONG_MODE = false;
			break;
		}
		*/

		while( 1 ) {
			if( CurrentToken == START_TAG_TOKEN) {
				if( currentTagName == data_str ) { // same name like <p> <li>
					if( data_str == "p" || data_str == "li" || data_str == "dt" || data_str == "dd" ) {
						break;
					} else {
						BaseElement* tmpv = parse_element(swl);
						if(!tmpv) break; // failed, ignore this child
						children.push_back( tmpv );
					}
				} else {
					BaseElement* tmpv = parse_element(swl);
					if(!tmpv) break; // failed
					children.push_back( tmpv );
				}
			} else if(CurrentToken == STRING_ELEMENT_TOKEN) {
				children.push_back( new StringElement(data_str) );
				next_token(swl);
			} else if( CurrentToken == END_TAG_TOKEN) {
				if(currentTagName != data_str ) {
					printf("Warning: Unmatched start-tag: %s and end-tag: %s\n", currentTagName.c_str(),  data_str.c_str());
					if( currentTagName == "p" || currentTagName == "li" || currentTagName == "dt" || currentTagName == "dd" ) {
						// case like : <p> is end up by </body>
						break;
					} else {
						// treat as invalid end-tag and ignore it
						next_token(swl);
						//return NULL;
					}
				} else {
					next_token(swl);
					break;
				}
			} else if(CurrentToken == ERROR_TOKEN) {
				break; // throw this child, and not forward
			}
		}

	} while(0);

	printf("<---end a tag :%s\n", currentTagName.c_str());
	return new TagElement(currentTagName, attrs, children);
}

// return the first element, move your swl to '<' before call this. that is html in most case.
BaseElement* parse_html(Stream_With_Lastchar *swl) {
	next_token(swl);
	while(CurrentToken != START_TAG_TOKEN || data_str != "html") {
		if(CurrentToken == EOF_TOKEN) return NULL; // not found any html 
		next_token(swl);
	}
	
	BaseElement* html = parse_element(swl);
	return html;
}


int main(int argc, char** argv) {
	FILE *fp = NULL;
	if( argc == 1) {
		fp = fopen("data.html", "r");
	} else 
		fp = fopen(argv[1], "r");
	if(!fp) return -1;

	FILE* out = fopen("dump.txt","w");
	Stream_With_Lastchar *swl = new File_Stream_With_Lastchar(fp, '>');
	BaseElement* html = parse_html(swl);
	if( !html ) {
		printf("Not a valid html file\n");
	} else {
		html->render(out);
	}
	fclose(fp);
	fclose(out);
	return 0;
}
