#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

#define BAUDRATE	B9600

int main(int argc,char* argv[]) {
  int fd;
  struct termios newtio;
  char *device;

  device="/dev/ttyS1";
  fd = open(device,O_RDWR);
  if(fd < 0) {
    fprintf(stderr,"*** open %s: %s\n",device,strerror(errno));
    exit(-1);
  }

  bzero(&newtio,sizeof(newtio));

  cfsetospeed(&newtio,BAUDRATE);

  newtio.c_cflag |= CS8;
  newtio.c_cflag |= CLOCAL;	// keine Auswirkung
  newtio.c_cflag |= CREAD;	// reversible Blockerung bei Entfall

  newtio.c_cflag &= ~CRTSCTS;	/* no output hardware flow control */
  newtio.c_cflag &= ~PARENB;	/* no parity */
  newtio.c_cflag &= ~CSTOPB;	/* no stopbit */

  newtio.c_iflag = IGNPAR;	/* ignore input parity */
  newtio.c_lflag = 0;		/* turn all off (non-canonocal, non-echo) */
  newtio.c_oflag = 0;		/* turn all off */
  newtio.c_cc[VTIME] = 20;	/* timeout in 1/10 sec */
  newtio.c_cc[VMIN] = 0;	/* minimum characters to read */
      
  tcflush(fd,TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
}
