/*
Copyright (c) 2005, David M Howard (daveh at dmh2000.com)
All rights reserved.

This product is licensed for use and distribution under the BSD Open Source License.
see the file COPYING for more details.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

/*
========================================================================================================
EXAMPLE : SETUP FOR GGA AND RMC SENTENCES WITH LINUX SERIAL IO
=======================================================================================================
*/   


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/io.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#include "nmeap.h"

#define FLOCK(f) {if(flock(fileno(f), LOCK_EX)) { perror("flock"); }; };
#define FUNLOCK(f) flock(fileno(f), LOCK_UN);

double        mylat = 0;
double        mylon = 0;
double        myalt;
unsigned long mytim;
double        myspd;
double        mycur;
unsigned long mydat;
char		  myval = 0;

nmeap_gga_t g_gga;

/** do something with the GGA data */
static void print_gga(nmeap_gga_t *gga)
{
    printf("found GPGGA message %.6f %.6f %.0f %lu %d %d %f %f\n",
            gga->latitude  ,
            gga->longitude, 
            gga->altitude , 
            gga->time     , 
            gga->satellites,
            gga->quality   ,
            gga->hdop      ,
            gga->geoid     
            );
			mytim = gga->time;
			mylat = gga->latitude;
			mylon = gga->longitude;
            myalt = gga->altitude;
			//myval = gga->quality > 0;
}

/** called when a gpgga message is received and parsed */
static void gpgga_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_gga_t *gga = (nmeap_gga_t *)data;
    
    printf("-------------callout\n");
    print_gga(gga);
}


/** do something with the RMC data */
static void print_rmc(nmeap_rmc_t *rmc)
{
    printf("found GPRMC Message %lu %c %.6f %.6f %f %f %lu %f\n",
            rmc->time,
            rmc->warn,
            rmc->latitude,
            rmc->longitude,
            rmc->speed,
            rmc->course,
            rmc->date,
            rmc->magvar
            );
            mytim = rmc->time;
            mylat = rmc->latitude;
            mylon = rmc->longitude;
            myspd = rmc->speed;
            mycur = rmc->course;
            mydat = rmc->date;
			myval = rmc->warn == 'A';
}

/** called when a gprmc message is received and parsed */
static void gprmc_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_rmc_t *rmc = (nmeap_rmc_t *)data;
    
    printf("-------------callout\n");
    print_rmc(rmc);
}


/*************************************************************************
 * LINUX IO
 */
 
/**
 * open the specified serial port for read/write
 * @return port file descriptor or -1
 */
int openPort(const char *tty,int baud)
{
    int fd;
    struct termios newtio;

    fd = open(tty,O_RDWR);
    if(fd < 0) {
		fprintf(stderr,"*** open %s: %s\n",tty,strerror(errno));
		exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    cfsetospeed(&newtio, baud);

    newtio.c_cflag |= CS8;
    newtio.c_cflag |= CLOCAL;     // keine Auswirkung
    newtio.c_cflag |= CREAD;      // reversible Blockerung bei Entfall
      
    newtio.c_cflag &= ~CRTSCTS;   /* no output hardware flow control */
    newtio.c_cflag &= ~PARENB;    /* no parity */
    newtio.c_cflag &= ~CSTOPB;    /* no stopbit */
	    
    newtio.c_iflag = IGNPAR;      /* ignore input parity */
    newtio.c_lflag = 0;           /* turn all off (non-canonocal, non-echo) */
    newtio.c_oflag = 0;           /* turn all off */

    newtio.c_cc[VTIME] = 20;      /* timeout in 1/10 sec */
    newtio.c_cc[VMIN] = 0;        /* minimum characters to read */
    
    tcflush(fd,TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
	return fd;
}


/* ---------------------------------------------------------------------------------------*/
/* STEP 1 : allocate the data structures. be careful if you put them on the stack because */
/*          they need to be live for the duration of the parser                           */
/* ---------------------------------------------------------------------------------------*/
static nmeap_context_t nmea;	   /* parser context */
static nmeap_gga_t     gga;		   /* this is where the data from GGA messages will show up */
static nmeap_rmc_t     rmc;		   /* this is where the data from RMC messages will show up */
static int             user_data; /* user can pass in anything. typically it will be a pointer to some user data */

static char *config = NULL;
static int config_size=0;

void readconf(int argc, char* argv[])
{
	char default_filename[]="/etc/navigation/nmeap2kml.cfg";
	char *filename = default_filename;
	int i;
	if ( argc >= 2 ) {
		for ( i = 0; i < argc-1; i++)
			if ( strcmp("-c", argv[i] ) == 0 )
				filename = argv[i+1];
	}

	fprintf(stderr, "reading config from %s \n", filename);

	FILE *f = fopen(filename, "r");
	if ( f ) {
		config = malloc(32*1024);
		if ( !config ) {
			perror("malloc");
			exit(1);
		}
		config_size = fread(config, 1, 32*1024-1, f);
		config[config_size]='\0';
		fprintf(stderr, "readed %d bytes:\n%s\n", config_size, config); 
		fclose(f);
		for (i=0; i < config_size; i++)
			if (config[i] == '\n')
				config[i] = '\0';
	} else {
		perror("connot open config");
		exit(1);
	}
}

char* getparam(const char* name)
{
	char lname[256];
	char *result = NULL;
	char *pstr = config;
	snprintf(lname, sizeof(lname), "%s=", name);
	while( pstr < config+config_size ) {
		if ( pstr[0] == '\0' ) {
			pstr++;
			continue;
		}
		if ( 0 == strncmp(lname, pstr, strlen(lname)) ) {
			result = pstr+strlen(lname);
			return result;
		} else {
			pstr+=strlen(pstr);
		};
	}
	fprintf(stderr, "Cannot find variable '%s' at config file\n", name);
	exit(1);
	return NULL; // dumb
}

int main(int argc,char *argv[])
{
    int  status;
    int  rem;
	int  offset;
	int  len;
	char buffer[32];
	char wgetbuf[1024];
    int  fd;
    const char *port = "/dev/ttyS0";
	FILE *f;
	time_t t;
    
	t=time(NULL);
    // default to ttyS0 or invoke with 'linux_nmeap <other serial device>' 
    if (argc == 2) {
        port = argv[1];
    }
    
	readconf(argc, argv);


    /* --------------------------------------- */
    /* open the serial port device             */
    /* using default 9600 baud for most GPS    */
    /* --------------------------------------- */
    fd = openPort(getparam("gps_port"), atoi(getparam("baudrate")));
    if (fd < 0) {
        /* open failed */
        printf("openPort %d\n",fd);
        return fd;
    }
    
	/* ---------------------------------------*/
	/*STEP 2 : initialize the nmea context    */                                                
	/* ---------------------------------------*/
    status = nmeap_init(&nmea,(void *)&user_data);
    if (status != 0) {
        printf("nmeap_init %d\n",status);
        exit(1);
    }
    
	/* ---------------------------------------*/
	/*STEP 3 : add standard GPGGA parser      */                                                
	/* -------------------------------------- */
    status = nmeap_addParser(&nmea,"GPGGA",nmeap_gpgga,gpgga_callout,&gga);
    if (status != 0) {
        printf("nmeap_add %d\n",status);
        exit(1);
    }

	/* ---------------------------------------*/
	/*STEP 4 : add standard GPRMC parser      */                                                
	/* -------------------------------------- */
    status = nmeap_addParser(&nmea,"GPRMC",nmeap_gprmc,gprmc_callout,&rmc);
    if (status != 0) {
        printf("nmeap_add %d\n",status);
        exit(1);
    }
    
	/* ---------------------------------------*/
	/*STEP 5 : process input until done       */                                                
	/* -------------------------------------- */
    for(;;) {
		/* ---------------------------------------*/
		/*STEP 6 : get a buffer of input          */                                                
		/* -------------------------------------- */
        len = rem = read(fd,buffer,sizeof(buffer));
        if (len <= 0) {
            perror("read");
            break;
        }
        
        
		/* ----------------------------------------------*/
		/*STEP 7 : process input until buffer is used up */                                                
		/* --------------------------------------------- */
		offset = 0;
        while(rem > 0) {
			/* --------------------------------------- */
			/*STEP 8 : pass it to the parser           */
			/* status indicates whether a complete msg */
			/* arrived for this byte                   */
			/* NOTE : in addition to the return status */
			/* the message callout will be fired when  */
			/* a complete message is processed         */
			/* --------------------------------------- */
            status = nmeap_parseBuffer(&nmea,&buffer[offset],&rem);
			offset += (len - rem); 
            
			/* ---------------------------------------*/
			/*STEP 9 : process the return code        */
            /* DON"T NEED THIS IF USING CALLOUTS      */
            /* PICK ONE OR THE OTHER                  */
			/* -------------------------------------- */
            switch(status) {
            case NMEAP_GPGGA:
                printf("-------------switch\n");
                print_gga(&gga);
                printf("-------------\n");
                break;
            case NMEAP_GPRMC:
                printf("-------------switch\n");
                print_rmc(&rmc);
                printf("-------------\n");
                break;
            default:
                break;
            }

	if (!(f=fopen(getparam("kml_file"), "w"))) {
		perror("fopen");
		exit(1);
	};
	FLOCK(f);
	fprintf(f, 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
"<kml xmlns=\"http://earth.google.com/kml/2.2\">\n"\
"<Document>\n"\
"<name>location.kml</name>\n"\
"   <Style id=\"location\">\n"\
"      <IconStyle>\n"\
"         <color>%s</color>\n"\
"         <scale>%s</scale>\n"\
"         <Icon>\n"\
"            <href>%s</href>\n"\
"         </Icon>\n"\
"      </IconStyle>\n"\
"      <LabelStyle>\n"\
"         <color>%s</color>\n"\
"         <scale>%s</scale>\n"\
"      </LabelStyle>\n"\
"      <ListStyle>\n"\
"      </ListStyle>\n"\
"   </Style>\n"\
"   <Placemark>\n"\
"      <name>%s</name>\n"\
"      <LookAt>\n"\
"         <longitude>%.6f</longitude>\n"\
"         <latitude>%.6f</latitude>\n"\
"         <altitude>%.0f</altitude>\n"\
"         <range>%s</range>\n"\
"         <tilt>%s</tilt>\n"\
"         <heading>%s</heading>\n"\
"      </LookAt>\n"\
"      <styleUrl>#location</styleUrl>\n"\
"      <description>Date: %.6lu \n"\
"                   Time: %.6lu \n"\
"		    Speed: %.1f</description>\n"\
"      <Point>\n"\
"         <coordinates>%.6f,%.6f,%.0f</coordinates>\n"\
"      </Point>\n"\
"   </Placemark>\n"\
"</Document>\n"\
"</kml>\n" , getparam("icon_color"), getparam("icon_scale"), getparam("icon_normal"), getparam("label_color"), getparam("label_scale"), getparam("object_name"), mylon, mylat, myalt, getparam("look_range"), getparam("look_tilt"), getparam("look_heading"), mydat, mytim, myspd, mylon, mylat, myalt);

	if (myval && mylat != 0 && mylon != 0) {
		int interval= atoi(getparam("interval"));
		if (!interval)
			interval = 30;
		if ( t+interval < time(NULL) ) {
			snprintf(wgetbuf, sizeof(wgetbuf), "wget 'http://trackme.org.ua/gps/flygps?ver=1&id=%s&lat=%f&lng=%f&alt=%.0f&speed=%f' -q -O /dev/null && gpio set A1 0 > /dev/null ; usleep 200000 ; gpio set A1 1 > /dev/null", getparam("object_name"), mylat, mylon, myalt, myspd);
			system(wgetbuf);
			t = time(NULL);
		}
	}


	fflush(f);
	FUNLOCK(f);
	fclose(f);
		}
    }
    
    /* close the serial port */
    close(fd);
    
    return 0;
}

