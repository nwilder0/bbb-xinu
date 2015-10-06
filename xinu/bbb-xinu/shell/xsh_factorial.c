/* xsh_factorial.c - xsh_factorial */

#include <xinu.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xsh_factorial - calculate the factorial of the provided integer (0-500)
 *------------------------------------------------------------------------
 */
shellcmd xsh_factorial(int nargs, char *args[])
{

	int n = 0;
	int intFact = 0;
	long result = 1;
	int intRepeat = 0;
	char *charPrint = NULL;

	while(n<(nargs-1)) {

			if(!strncmp("-intFact",args[n],8)) {
				intFact = atol(args[n+1]);
				n++;
			}
			else if(!strncmp("-charPrint",args[n], 10)) {
				charPrint = args[n+1];
				n++;
			}
			else if(!strncmp("-intRepeat",args[n],10)) {
				intRepeat = atol(args[n+1]);
				n++;
			}
			else
			{
				if(strncmp("factorial",args[n],9)) {
					printf("Invalid parameter: %s\n  expecting...\n",args[n]);
					printf("    factorial -intFact [integer] -intRepeat [integer] -charPrint [string]\n");
					return -1;
				}
			}
			n++;

	}

	if((intFact < 0) || (intFact > 500)) {
		printf("Error: parameter intFact must be an integer from 0 to 500\n");
		return -1;
	}

	if((intRepeat < 0) || (intRepeat > 65535)) {
		printf("Warning: intRepeat is out of range (0 to 65535), ignoring\n");
		intRepeat = 0;
	}

	// need check for %d in string
	char *percent = NULL;

	int percentCount = 0;
	if(charPrint) {
		percent = strchr(charPrint, '%');
		if(percent) {
			percentCount++;
		}
	}

	if(charPrint && !(percentCount)) {
		printf("Warning: charPrint does not include a % character, using default string\n");
		charPrint = "Result is %d";
		percentCount = 1;
	}
	printf("Calculating factorial for %d, %d time(s)\n",intFact,intRepeat+1);
	do {
		int i = 1;
		for(i = 1; i<=intFact; i++) result *= i;
		if(percentCount >= 1) {
			printf(charPrint, result);
			printf("\n");
		}
		result = 1;
	} while (intRepeat--);

    return 0;
}
