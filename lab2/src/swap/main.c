#include <stdio.h>

#include "swap.h"

int main()
{
	char ch1 = 'a';
	char ch2 = 'b';

	Swap(&ch1, &ch2);

	printf("%c %c\n", ch1, ch2);

	char testStr[] = "345./ 56723fgh";

	RevertString(testStr);
	
	printf("Reversed string: %s\n", testStr);

	return 0;
}
