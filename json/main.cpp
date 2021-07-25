#include "parse_json.h"
#include "parse_double.h"

int main(int argc, char** argv) {
	FILE *fp = stdin;
	if(argc>1)
		fp = fopen("data.inp", "r");
	Stream_With_Lastchar* swl = new File_Stream_With_Lastchar(fp, ' ');
	next_token(swl);
	BaseValue* V = parse_value(swl);
	if(V) V->render(0);
	else {
		printf("Wrong format Json at about rows=%d, cols=%d\n", swl->rc.rows, swl->rc.cols);
	}
	printf("\n\n");
	delete V;
	return 0;
}
