/*
* Source Code From:
*	NFU, CSIE, Subject: [113-1] Data Compression
* Function:
*	Calculate a file's ENTROPY
* Input:
*	File: 
*		entropyText.txt
* Output:
*	Show on the CMD
* Usage:
*	cd /-113-1-Data-Compression/entropy/entropy/debug/
*	entropy.exe entropyText.txt
* Others:
*	Entropy = logBase2(1 / Pi)
*		Pi = probability of i
*/
#define _CRT_SECURE_NO_WARNINGS // For C4996 "fopen function"
#include <stdio.h>
#include <math.h>

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fin;
	float count[256];
	int i,c;
	float pb[256],total_num,entropy,inv;

	// Check argc(total number of argv)
	if (argc <= 1) {
		printf("Usage: entropy infile \n");
		exit (0);
	}
	// Open input file
	if ((fin=fopen(argv[1],"rb")) != NULL) {
		for (i=0;i<256;i++)
			count[i]=0;
		total_num=0;
		entropy=0;

		// Count character in file
		while ((c=getc(fin)) != EOF) {
			count[c]++;
			total_num++;
		}

		// Count each character's entropy
		for (i=0;i<256;i++) {
			pb[i]=((float) count[i])/total_num;
			if (pb[i]!=0) {
				inv=1.0/pb[i];
				entropy=entropy+pb[i]*log(inv);
			}
		}

		// logBase2(X) = logBase10(X) / logBase10(2)
		entropy=entropy/log(2);
		printf("\nentropy:%f\n",entropy);
		fclose(fin);
	}
	else
		printf("Error opening %s.\n",argv[1]);
}

