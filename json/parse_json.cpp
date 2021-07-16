#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <vector>
#include <string>

class BaseValue { // 6 types: false/null/true/object/array/number/string
	public:
	virtual ~BaseValue() {}
	virtual void render(int indent) = 0;
};

inline void printIndent(int n) {
	for(int i=0; i<n; i++) printf(" ");
}

class StringValue : public BaseValue {
	std::string str;
	public:
	virtual ~StringValue() {}
	virtual void render(int indent) {
		//printIndent(indent);
		printf("\"%s\"", str.c_str());
	}

	StringValue(std::string s) : str(s) {}
};

class ArrayValue : public BaseValue {
	std::vector<BaseValue*> v;
	public:
	virtual ~ArrayValue() {
		for(int i=0; i<v.size(); i++) delete v[i];
	}

	ArrayValue(const std::vector<BaseValue*> &v_) {
		v = v_;
	}

	virtual void render(int indent) {
		printf("[\n");
		for(auto x : v) {
			printIndent(indent+2);
			x->render(indent+2);
			printf(",\n");
		}
		printIndent(indent);
		printf("]");
	}

};

class NumberValue : public BaseValue {
	double val;
	public:
	NumberValue(double v) : val(v) { }
	virtual ~NumberValue() {}
	virtual void render(int indent) {
		//printIndent(indent);
		printf("%g", val);
	}
};

class DictValue : public BaseValue {
	//std::unordered_map<std::string, BaseValue*> mp;
	std::vector<std::pair<std::string, BaseValue*> > mp;
	public:
	DictValue(std::vector<std::string> keys, std::vector<BaseValue*> vals ) {
		for(int i = 0; i<keys.size(); i++) {
			//mp[keys[i]] = vals[i];
			mp.push_back(std::make_pair(keys[i], vals[i]));
		}
	}
	virtual ~DictValue() {
	//TODO
	}
	virtual void render(int indent) {
		printf("{\n");
		for(auto x : mp) {
			printIndent(indent+2);
			printf("\"%s\" : ", x.first.c_str());
		   	x.second->render(indent+2);
			printf(",\n");
		}
		printIndent(indent);
		printf("}");
	}
};

enum ConstType {
	False=0,
	True,
	Null,
};

class ConstValue : public BaseValue {
	ConstType ct;
	public:
	ConstValue(ConstType c) : ct(c) {}
	virtual ~ConstValue() {}
	virtual void render(int indent) {
		switch( ct ) {
			case False :
				printf("false"); break;
			case True:
				printf("true"); break;
			case Null :
				printf("null"); break;
		}
	}
};

static int CurrentToken;
static std::string Identifier_str;
static double numeric_val;
static int special_char;

enum token_type {
	NUM_TOKEN = 0,
	STRING_TOKEN,
	CONSTANT_TOKEN,
	SPECIAL_CHAR_TOKEN,
	EOF_TOKEN,
	UNKNOWN,
};

void next_token(FILE* fp) {
	static int LastChar = ' ';
	while( isspace(LastChar) ) LastChar = fgetc(fp);
	if(LastChar == '"') {
		CurrentToken = STRING_TOKEN;
		Identifier_str = "";
		LastChar = fgetc(fp); // eat "
		// parse escaped or non-escaped data
		while( LastChar != '"') {
			Identifier_str += LastChar;
			LastChar = fgetc(fp);
		}
		LastChar = fgetc(fp); // eat another "
	} else if(LastChar == 'f' || LastChar == 't' || LastChar == 'n' ) { // false/true/null
		CurrentToken = CONSTANT_TOKEN;
		Identifier_str = "";
		while(isalpha(LastChar) ) {
			Identifier_str += LastChar;
			LastChar = fgetc(fp);
		}
	} else if( LastChar == ':' || LastChar == '{' || LastChar == '[' || LastChar == '}' || LastChar == ']' || LastChar == ',' )  {
		CurrentToken = SPECIAL_CHAR_TOKEN;
		special_char = LastChar;
		LastChar = fgetc(fp);
	} else if (LastChar == EOF ) {
		CurrentToken = EOF_TOKEN;
	} else {
		ungetc(LastChar, fp); 
		if ( fscanf(fp, "%lf", &numeric_val) == 0) { // unable to read a number
			CurrentToken = UNKNOWN;
		} else 
			CurrentToken = NUM_TOKEN;
		LastChar = fgetc(fp);
	}
}

BaseValue* parse_value( FILE* fp );
BaseValue* parse_array( FILE*fp);
BaseValue* parse_dict( FILE*fp);

BaseValue* parse_array( FILE*fp) {
	next_token(fp); // eat [
	std::vector<BaseValue*> ans;
	while( CurrentToken != SPECIAL_CHAR_TOKEN || special_char != ']') {
		if(CurrentToken == UNKNOWN || CurrentToken == EOF_TOKEN) return NULL;
		BaseValue* tmp = parse_value(fp);
		if(!tmp) return NULL; // failed
		ans.push_back(tmp);
		if( CurrentToken == SPECIAL_CHAR_TOKEN && special_char == ']') break;
		if(CurrentToken == SPECIAL_CHAR_TOKEN && special_char == ',') next_token(fp);
		else return NULL;
	}
	next_token(fp); // eat ]
	return new ArrayValue(ans);
}

BaseValue* parse_dict( FILE*fp) {
	next_token(fp); // eat {
	std::vector<std::string> keys;
	std::vector<BaseValue*> vals;
	while( CurrentToken != SPECIAL_CHAR_TOKEN || special_char != '}') {
		if(CurrentToken != STRING_TOKEN ) return NULL;
		keys.push_back( Identifier_str );
		next_token(fp);
		if(CurrentToken != SPECIAL_CHAR_TOKEN || special_char != ':') return NULL;
		next_token(fp); // eat :
		BaseValue* tmp = parse_value(fp);
		if(!tmp) return NULL; // failed
		vals.push_back(tmp);
		if( CurrentToken == SPECIAL_CHAR_TOKEN && special_char == '}') break;
		if(CurrentToken == SPECIAL_CHAR_TOKEN && special_char == ',') next_token(fp);
		else return NULL;
	}
	next_token(fp); // eat }
	return new DictValue(keys, vals);
}

BaseValue* parse_value( FILE* fp ) {
	switch( CurrentToken ) {
		case NUM_TOKEN : {
			BaseValue* ans = new NumberValue(numeric_val);
			next_token(fp);
			return ans;
		 }
		case STRING_TOKEN: {
			BaseValue* ans = new StringValue(Identifier_str);
			next_token(fp);
			return ans;
		}
		case CONSTANT_TOKEN: {
			BaseValue *ans;
			if(Identifier_str == "false") ans = new ConstValue(False);
			else if(Identifier_str == "true") ans = new ConstValue(True);
			else if(Identifier_str == "null") ans = new ConstValue(Null);
			else return NULL;
			next_token(fp);
			return ans;
		}
		case SPECIAL_CHAR_TOKEN: {
			if( special_char=='[') {
				return parse_array(fp);
			} else if( special_char=='{') {
				return parse_dict(fp);
			} else {
				return NULL;
			}
		}
		default: 
			return NULL;
	}
}

int main() {
	FILE *fp = stdin;
	fp = fopen("data.inp", "r");
	next_token(fp);
	BaseValue* V = parse_value(fp);
	if(V) V->render(0);
	else {
		printf("Wrong format Json\n");
	}
	return 0;
}

/* two json example
 
example 1:
      {
        "Image": {
            "Width":  800,
            "Height": 600,
            "Title":  "View from 15th Floor",
            "Thumbnail": {
                "Url":    "http://www.example.com/image/481989943",
                "Height": 125,
                "Width":  100
            },
            "Animated" : false,
            "IDs": [116, 943, 234, 38793]
          }
      }

example 2:
      [
        {
           "precision": "zip",
           "Latitude":  37.7668,
           "Longitude": -122.3959,
           "Address":   "",
           "City":      "SAN FRANCISCO",
           "State":     "CA",
           "Zip":       "94107",
           "Country":   "US"
        },
        {
           "precision": "zip",
           "Latitude":  37.371991,
           "Longitude": -122.026020,
           "Address":   "",
           "City":      "SUNNYVALE",
           "State":     "CA",
           "Zip":       "94085",
           "Country":   "US"
        }
      ]


*/
