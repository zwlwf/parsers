#include "parse_csv.h"

int main(int argc, char** argv) {
	FILE *fp;
	if (argc>1) {
		fp = fopen(argv[1], "r");
		if(!fp) {
			printf("Failed to open %s\n", argv[1]);
			return -1;
		}
	}
	else 
		fp = stdin;
	Stream_With_Lastchar *swl = new File_Stream_With_Lastchar(fp, '\r');
	Table x = parse_csv(swl);
	render_as_python_listlist(x);
	freeTable(x);
	printf("\n");
	fclose(fp);
	return 0;
}
