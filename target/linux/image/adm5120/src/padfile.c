#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	FILE* fp;
	char* padding;
	long  nOrgSize;
	long  nNewSize;
	long  nPadSize;

	if (argc != 3)
	{
		printf("Usage: padfile <filename> <new length>\n");
		return -1;
	}

	nNewSize = atol(argv[2]);

	fp = fopen(argv[1], "ab+");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		nOrgSize = ftell(fp);

		if (nNewSize >= nOrgSize)
		{
			nPadSize = nNewSize - nOrgSize;
			padding = (char*) malloc(nPadSize);
			memset(padding, 0, nPadSize);
			fwrite(padding, sizeof(char), nPadSize, fp);
			free(padding);
		}
		else
		{
			printf("New length smaller than file size.\n");
			return -1;
		}

		fclose(fp);
	}
	else
	{
		printf("Cannot open: %s.\n", argv[1]);
		return -1;
	}

	return 0;
}
