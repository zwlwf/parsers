#include "parse_html.h"

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
