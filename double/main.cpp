#include "parse_double.h"

int main() {
	double d;
	int status;
	Stream_With_Lastchar *swl = new File_Stream_With_Lastchar(stdin,' ');
	d = parse_double(swl, NULL);
	printf("d = %lf\n", d);
	Stream_With_Lastchar *swl2 = new String_Stream_With_Lastchar("12.34",' ');
	d = parse_double(swl2, NULL);
	printf("d = %lf\n", d);
	return 0;
}

