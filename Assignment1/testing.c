#include "myheader.h"

int main() {
	double time = getTime();
	printf("%f\n", time);
	sleep(2);
	double time2 = getTime();
	printf("%f\n", time2);
	printf("%f\n", time2-time);

}
