#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include "Stream_With_Lastchar.h"

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
};

class StringElement: public BaseElement {
	std::string str;
	public:
	StringElement(std::string s) : str(s) {}
	virtual ~StringElement() {}
};

class TagElement : public BaseElement {
	std::string tagName;
	std::map<std::string, AttriValue*> attrs; 
	std::vector<BaseElement*> children;
	public:
	TagElement(std::string tn, std::map<std::string, AttriValue*> attrs_, std::vector<BaseElement*> cd) : tagName(tn), attrs(attrs_), children(cd) {}
};

static int CurrentToken;
static std::string data_str;

enum token_type {
	ATTR_STRING_TOKEN = 0,
	ATTR_EQUAL_TOKEN,
	ATTR_NAME_TOKEN, // attri name or attri value with name token
	STRING_ELEMENT_TOKEN,
	ERROR_TOKEN,
	START_TAG_TOKEN,
	END_TAG_TOKEN,
	EOF_TOKEN
};

enum tag_type { // html element type, innerText depend on the element type
	DIV=0,
};

std::map<std::string, int> entity_characters(
		{{"quot", 34}, // "
		{"amp", '&'},
		{"lt", '<'},
		{"gt", '>'}});

static void next_token( Stream_With_Lastchar *swl ) {
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
				CurrentToken = END_TAG_TOKEN;
				data_str = "";
				while( isalpha(swl->LastChar) ) {
					data_str += tolower(swl->LastChar);
					swl->getc();
				}
				//swl->getc(); // eat >
				return;
			} else if( isalpha(swl->LastChar) ) {
				CurrentToken = START_TAG_TOKEN;
				data_str = "";
				while( isalpha(swl->LastChar) ) {
					data_str += tolower(swl->LastChar);
					swl->getc();
				}
				return;
			} else { // an ordinary <, 
				printf("Un expected char after <\n");
				return;
			}
		} else if (swl->LastChar == '>') { // String Elememnt
			swl->getc(); // eat >
			data_str = "";
			while(1) {
				if( swl->eof() ) break;
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
						data_str += tmp;
					} else if( isalpha(swl->LastChar) ) {
						std::string tmp;
						while( !isspace(swl->LastChar) && swl->LastChar!=';' ) {
							if(swl->eof()) break;
							tmp+=swl->LastChar;
							swl->getc();
						}
						if(swl->LastChar == ';') swl->getc();
						if( entity_characters.count(tmp) ) {
							data_str += entity_characters[tmp];
						} else {
							printf("Unrecognized entity character\n");
							CurrentToken = ERROR_TOKEN;
							return;
						}
					} else { // just norm &
						data_str += '&';
						swl->getc();
					}
				} else if( swl->LastChar == '<') {
					break;
					/*
					swl->getc();
					if( swl->LastChar == '!' || 
						swl->LastChar == '/' ||
						isalpha(swl->LastChar) ) {
						break; // over
					} else {
						data_str += '<';
						continue;
					}
					*/
				} else {
					data_str += swl->LastChar;
					swl->getc();
				}
			}
			if(data_str.size() > 0 ) {
				CurrentToken = STRING_ELEMENT_TOKEN;
				return;
			} else 
				continue;
		} else if ( isalpha(swl->LastChar) ) { // AttriName or AttriValue with Name token type
			data_str = "";
			while( isalpha(swl->LastChar) ||
					swl->LastChar == '.' ||
					swl->LastChar == '-' ||
					isdigit(swl->LastChar) ) {
				data_str += swl->LastChar;
			}
			CurrentToken = ATTR_NAME_TOKEN;
			return;
		} else if ( swl->LastChar == '=' ) {
			CurrentToken = ATTR_EQUAL_TOKEN;
			swl->getc();
			return;
		} else if ( swl->LastChar == '\'' || swl->LastChar == '"') { // AttriValue of string literal
			int quota = swl->LastChar;
			swl->getc();
			CurrentToken = ATTR_STRING_TOKEN;
			data_str = "";
			while( swl->LastChar != quota ) {
				if( swl->eof() ) {
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
						if( swl->LastChar == ';') swl->getc();
						data_str += tmp;
					} else if( isalpha(swl->LastChar) ) {
						std::string tmp;
						while( !isspace(swl->LastChar) && swl->LastChar!=';' ) {
							if(swl->eof()) break;
							tmp+=swl->LastChar;
							swl->getc();
						}
						if(swl->LastChar == ';') swl->getc();
						if( entity_characters.count(tmp) ) {
							data_str += entity_characters[tmp];
						} else {
							printf("Unrecognized entity character\n");
							CurrentToken = ERROR_TOKEN;
							return;
						}
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
		} else {
			// error
			CurrentToken = ERROR_TOKEN;
			return;
		}
	}
}

// now we are at start of elemnt like <div
BaseElement* parse_element( Stream_With_Lastchar *swl) {
	if(CurrentToken != START_TAG_TOKEN ) return NULL;
	next_token(swl);
	std::string currentTagName = data_str;
	std::map<std::string, AttriValue*> attrs;
	while (CurrentToken == ATTR_NAME_TOKEN) {
		next_token(swl);
		std::string attrName = data_str;
		if(CurrentToken == ATTR_EQUAL_TOKEN) {
			next_token(swl);
			std::string attr_val = data_str;
			if( CurrentToken == ATTR_NAME_TOKEN) {
				attrs[attrName] = new NameTokenAttriValue(attr_val);
			} else // ATTR_STRING_TOKEN
				attrs[attrName] = new StringAttriValue(attr_val);
			next_token(swl);
		} else {
			attrs[attrName] = new NullAttriValue();
		}
	}

	// parse nested elements
	std::vector<BaseElement*> children;
	if( currentTagName == "br") { // no end-tag TagElement
		return new TagElement(currentTagName, attrs, children);
	}
	while( 1 ) {
		if( CurrentToken == EOF ) break; // Add end_tag by my self, although not completed
		if( CurrentToken == START_TAG_TOKEN) {
			if( currentTagName == data_str ) { // same name like <p> <li>
				if( data_str == "p" || data_str == "li" || data_str == "dt" || data_str == "dd" ) {
					break;
				} else {
					BaseElement* tmpv = parse_element(swl);
					if(!tmpv) return NULL; // failed
					children.push_back( tmpv );
				}
			} else {
				BaseElement* tmpv = parse_element(swl);
				if(!tmpv) return NULL; // failed
				children.push_back( tmpv );
			}
		} else if(CurrentToken == STRING_ELEMENT_TOKEN) {
			children.push_back( new StringElement(data_str) );
			next_token(swl);
		} else if( CurrentToken == END_TAG_TOKEN) {
			if(currentTagName != data_str ) {
				printf("Unmatched start-tag: %s and end-tag: %s\n", currentTagName.c_str(),  data_str.c_str());
				return NULL;
			} else {
				break;
			}
		}
	}
	return new TagElement(currentTagName, attrs, children);
}

// return the first element, move your swl to '<' before call this. that is html in most case.
BaseElement* parse_html(Stream_With_Lastchar *swl) {
	next_token(swl);
	while(CurrentToken != START_TAG_TOKEN || data_str != "html") {
		next_token(swl);
	}

	BaseElement* html = parse_element(swl);
	return html;
}

