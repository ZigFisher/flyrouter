/*
 *	packer.c -- this program is used for mass production.
 *
 *  Given addresses and binary image file, this program can pack them into one file.
 *
 *	$id$
 *
 *	To compile the source code.
 *	# gcc -o packer packer.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>


#define DEFAULT_CHAR 0xff
#define VERSION "00.04"

#ifndef min
#define min(a,b) ((a>b)?b:a)
#endif//min

int option2ByteSwap = 0;
int optionEditMode = 0;

void help( char* argv0, char* reason )
{
printf(""
"!! ERROR: %s !!\n"
"\n"
"Usage: %s [options] outputfile address={filename[,offset[,limit]]|hex:[hex:[...]]} ...\n"
"\n"
"address:    address (must be heximal) to store the corresponding image file.\n"
"filename:   file to be packed.\n"
"hex:        hex data to be packed.\n"
"\n"
"Options:\n"
"        -2  2-byte byte-swap\n"
"        -e  edit mode (do not replace the existing file)\n"
"\n"
"Example:\n"
"        %s -2 images/mp.rom \\\n"
"              0x00000=loader_srcroot/ldr.bin,0x0,0x4000 \\\n"
"              0x04000=00:00:10:11:12:00 \\\n"
"              0x06000=CCFG \\\n"
"              0x08000=loader_srcroot/ldr.bin,0x4000,0x18000 \\\n"
"              0x20000=images/run.bix\n"
"\n",
	reason,
	argv0,
	argv0 );
}


/*
 *  mathStrtol() -- enhanced strtol(), supporting plus and minus operation.
 *
 *	Return: uint32   -- converted value
 *
 */
unsigned int mathStrtol( char* szSrc )
{
	char szTemp[4096];
	char *pszToken;
	unsigned int retVal = 0;
	char prevOperator = '\0';
	char *pszLast; /* for strtok_r() */

	strncpy( szTemp, szSrc, min( strlen(szSrc), sizeof(szTemp)-1 ) + 1 );

	for( pszToken = strtok_r( szTemp, "+-", &pszLast );
	     pszToken;
	     pszToken = strtok_r( NULL, "+-", &pszLast ) )
	{
		char chDeletedChar;

		/* find out what character was deleted by strtok(). */
		chDeletedChar = szSrc[pszToken-szTemp+strlen(pszToken)];
		
		switch( prevOperator )
		{
			case '+':
				retVal += strtol( pszToken, NULL, 16 );
				break;
			case '-':
				retVal -= strtol( pszToken, NULL, 16 );
				break;
			default:
				/* initial value */
				retVal = strtol( pszToken, NULL, 16 );
				break;
		}

		prevOperator = chDeletedChar;
	}

	return retVal;
}


/*
 *  ByteSwap() -- swap byte-order in file
 *
 *  Return: -1        -- error.
 *          otherwise -- success and the numbers of swapped.
 *
 */
int ByteSwap( char* szOutputFile, int ByteGroup )
{
	int nRead, nWritten;
	char szBuf[1024];
	int fdOrg, fdNew;
	int i;
	int retVal = 0;

	fdOrg = open( szOutputFile, O_RDONLY );
	if ( fdOrg == -1 )
	{
		/* printf( "file(R) '%s' for Byte-swap: %s, but it is OK.\n", szOutputFile, strerror(errno) ); */
		return -1;
	}

	// unlink the filename (the old file will be automatically deleted after 2-byte swap).
	unlink( szOutputFile );

	fdNew = open( szOutputFile, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU );
	if ( fdNew == -1 )
	{
		printf( "open file(W) '%s' error: %s\n", szOutputFile, strerror(errno) );
		return -1;
	}

	//
	// main loop
	//
	while( 1 )
	{
		nRead = read( fdOrg, szBuf, sizeof( szBuf ) );

		if ( nRead == 0 ) break;
		if ( nRead == -1 ) return -1;

		// swap bytes!
		for( i = 0; i < ( nRead+ByteGroup-1 ) / ByteGroup * ByteGroup; i += ByteGroup )
		{
			int temp;

			temp       = szBuf[i+0];
			szBuf[i+0] = szBuf[i+1];
			szBuf[i+1] = temp;
		}

		nWritten = 0;
		while( nWritten < nRead )
		{
			int ret;

			ret = write( fdNew, &szBuf[nWritten], nRead - nWritten );
			if ( ret <= 0 ) return -1;

			nWritten += ret;
		}
		retVal += nWritten;
	}

	close( fdOrg );
	close( fdNew );

	return retVal;
}


int main( int argc, char* argv[] )
{
	int parsingArgc;
	char* parsingArgv;
	char* szOutputFile;
	int fdOutput, fdInput;
	uint OutputFileLength = 0;

	printf( "\n(c)Copyright Realtek, Inc. 2003\n" 
	        "Image Packer Version: %s\n\n", VERSION );


	if ( argc < 2 ) { help( argv[0], "output file must be given" ); exit( 1 ); }

	for( parsingArgc = 1;
	     parsingArgc < argc;
	     parsingArgc++ )
	{
		parsingArgv = argv[parsingArgc];

		// option
		if ( !strncmp( "-2", parsingArgv, 2 ) )
		{
			option2ByteSwap = 1;
		}
		else if ( !strncmp( "-e", parsingArgv, 2 ) )
		{
			optionEditMode = 1;
		}
		else
		{
			break; // the reminding arguments are image files.
		}
	}

	if ( parsingArgc >= argc ) { help( argv[0], "output file must be given" ); exit( 1 ); }

	szOutputFile = argv[parsingArgc];
	parsingArgc++;

	//
	//	2-byte swap
	//
	if ( option2ByteSwap )
	{
		if ( -1 == ByteSwap( szOutputFile, 2 ) )
		{
			if ( errno == ENOENT )
			{
				// fine, the file may be not created.
			}
			else
			{
				// else, this is TRUE error.
				printf( "ByteSwap('%s') error: %s\n", szOutputFile, strerror( errno ) ); 
				exit( 40 );
			}
		}
	}

	if ( optionEditMode )
	{
		// In Edit Mode, do not truncate file when opening.
		fdOutput = open( szOutputFile, O_RDWR|O_CREAT, S_IRWXU );
		if ( fdOutput == -1 ) { printf( "output file '%s' create error: %s\n", szOutputFile, strerror(errno) ); exit( 1 ); }
	}
	else
	{
		fdOutput = open( szOutputFile, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU );
		if ( fdOutput == -1 ) { printf( "output file '%s' create error: %s\n", szOutputFile, strerror(errno) ); exit( 1 ); }
	}

	if ( parsingArgc >= argc ) { help( argv[0], "At least one pair of 'address=filename' must be assigned" ); exit( 1 ); }

	for( ;
	     parsingArgc < argc;
	     parsingArgc++ )
	{
		char* szAddress = NULL;
		char* szFilename = NULL;
		char* szOffset = NULL;
		char* szLimit = NULL;
		unsigned int nAddress = 0x00000000;
		unsigned int nOffset = 0x00000000;
		unsigned int nLimit = 0x00000000;
		struct stat fsstat;

		parsingArgv = argv[parsingArgc];

		// address=filename,offset,limit
		szAddress = strtok( parsingArgv, "=" );
		if ( szAddress==NULL ) { help( argv[0], "image format is address=filename" ); exit( 10 ); }
		/*if ( strncasecmp( szAddress, "0x", 2 ) ) { help( argv[0], "address must be heximal" ); exit( 10 ); }*/

		// filename
		szFilename = strtok( NULL, "," );
		if ( szFilename==NULL ) { help( argv[0], "filename must be given" ); exit( 10 ); }

		//
		// If the input string is hex, store them to a temp file.
		//
		if ( strchr( szFilename, ':' ) )
		{
			char* szInputData = strdup( szFilename );
			char* szData;
			int fdTemp;
			char szBuf;
			szFilename = "/tmp/packer.tmp";

			fdTemp = open( szFilename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU );
			if ( fdTemp == -1 ) { printf( "temp file '%s' create error: %s\n", szFilename, strerror(errno) ); exit( 1 ); }

			printf("szInputData:%s\n", szInputData );

			for( szData = strtok( szInputData, ":" );
			     szData;
			     szData = strtok( NULL, ":" ) )
			{
				int nWritten;

				szBuf = mathStrtol( szData ); // convert from 0xYY to char
				nWritten = write( fdTemp, &szBuf, sizeof( szBuf ) );

				if ( nWritten != sizeof( szBuf ) ) {  printf( "write('%s') error: %s\n", szFilename, strerror(errno) ); exit( 1 ); }
			}

			close( fdTemp );
			free( szInputData );
		}

		fdInput = open( szFilename, O_RDONLY );
		if ( fdInput == -1 )
		{
			// file open error. it is ok. use default value.
			//printf( "open('%s') error: %d, skip it.\n", szFilename, strerror(errno) );
			nOffset = 0x00000000;
			nLimit = 0x00000000;
		}

		// offset and limit
		szOffset = strtok( NULL, "," );
		if ( szOffset ) szLimit = strtok( NULL, "," );
	
		// convert to integer
		nAddress = mathStrtol( szAddress );
		if ( szOffset ) nOffset = mathStrtol( szOffset ); else nOffset = 0;
		if ( szLimit ) nLimit = mathStrtol( szLimit ); else nLimit = 0;

		// nOffset and nLimit should be bound in [0,filesize].
		fstat( fdInput, &fsstat );
		if ( nOffset > fsstat.st_size ) nOffset = fsstat.st_size;
		if ( szLimit )
			nLimit = min( nLimit, fsstat.st_size-nOffset );
		else
			nLimit = fsstat.st_size - nOffset;
	
		if ( fdInput!=-1 && 
		     nLimit > 0 )
		{
			int nRemind;

			// copy original file content
			printf( "packing from 0x%08X to 0x%08X with '%s':%08X .... \n",
					nAddress, nAddress+nLimit-1, szFilename, nOffset );

			// check if we need to enlarge file size ?
			if ( nAddress+nLimit > OutputFileLength )
			{
				unsigned int NewOutputFileLength;
				int nFileTail;
				char szBuf[1024];
				int nWrite, nRemind;

				NewOutputFileLength = nAddress+nLimit;

				nFileTail = lseek( fdOutput, 0, SEEK_END );

				// expand output file size
				if ( lseek( fdOutput, NewOutputFileLength, SEEK_SET ) == -1 )
				{
					printf( "lseek() error: %s\n", strerror( errno ) );
					exit( 20 );
				}

				nRemind = NewOutputFileLength - nFileTail;
				lseek( fdOutput, NewOutputFileLength-nRemind, SEEK_SET );

				// set defautl char
				memset( szBuf, DEFAULT_CHAR, sizeof( szBuf ) );

				for( ;
				     nRemind > 0;
				     nRemind -= nWrite )
				{
					if ( nRemind > sizeof( szBuf ) )
					{
						nWrite = sizeof( szBuf );
					}
					else
					{
						nWrite = nRemind;
					}

					switch( nWrite = write( fdOutput, szBuf, nWrite ) )
					{
						case -1:
						case 0:
							printf( "write() error: %s\n", strerror( errno ) );
							exit( 21 );
							break;
						default:
							break;
					}
				}

				// update new file length
				OutputFileLength = NewOutputFileLength;

			}

			lseek( fdInput, nOffset, SEEK_SET );
			lseek( fdOutput, nAddress, SEEK_SET );

			//
			// start copy file .....
			//

			nRemind = nLimit;
			while ( 1 )
			{
				char szBuf[1024];
				int nRead, nWrite;

				memset( szBuf, DEFAULT_CHAR, sizeof( szBuf ) );

				if ( nRemind > sizeof( szBuf ) )
				{
					nRead = sizeof( szBuf );
				}
				else
				{
					nRead = nRemind;
				}

				nRead = read( fdInput, szBuf, nRead );

				if ( nRead == 0 )
				{
					// end of file
					break;
				}
				else if ( nRead == -1 )
				{
					// read error
					printf( "read() error: %s\n", strerror( errno ) );
					break;
				}

				switch( nWrite = write( fdOutput, szBuf, nRead ) )
				{
					case -1:
					case 0:
						printf( "write() error: %s\n", strerror( errno ) );
						exit( 21 );
						break;
					default:
						break;
				}

				if ( nWrite != nRead )
				{
					printf( "write(): nWrite != nRead, error.\n" );
					exit( 22 );
				}

				nRemind -= nWrite;
			}

		}
		else
		{
			// pass copying file
			printf( "packing from 0x%08X to 0x%08X with '%s':%08X .... skipped\n",
					nAddress, nAddress, szFilename, nOffset );
		}

		if ( fdInput != -1 ) close( fdInput );
	}

	if ( optionEditMode )
	{
		struct stat fsstat;

		// In Edit Mode, the file length is not OutputFileLength.
		fstat( fdOutput, &fsstat );
		OutputFileLength = fsstat.st_size;
	}
	printf( "\nPacker completed. Output file '%s' (length:0x%08X).\n\n", szOutputFile, OutputFileLength );

	close( fdOutput );


	//
	//	2-byte swap
	//
	if ( option2ByteSwap )
	{
		if ( -1 == ByteSwap( szOutputFile, 2 ) )
		{
			printf( "ByteSwap('%s') error: %s\n", szOutputFile, strerror( errno ) ); 
			exit( 40 );
		}
	}

	return 0;
}

