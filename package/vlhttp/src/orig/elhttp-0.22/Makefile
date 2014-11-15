
linux:
	gcc -O -W -Wall -o elhttp elhttp.c
	strip elhttp

sunos:
	gcc -O -W -Wall -o elhttp elhttp.c -lsocket -lnsl
	strip elhttp

unix:
	cc -O -o elhttp elhttp.c
	strip elhttp

