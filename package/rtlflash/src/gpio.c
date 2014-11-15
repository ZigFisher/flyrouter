#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>

#include <unistd.h>

static	int gpio_fd = -1;

// GPIO Blinking speed (microsecond)
#define GPIO_NORMAL_SPEED       1000000
#define GPIO_FAST_SPEED          100000
#define GPIO_WDOG_SPEED         1000000

// GPIO Switch counter (second)
#define GPIO_BT_RESTORE_TIME    5

/////////////////////////////////////////////////////////////////////////

#if 0
void gpio_create_pidfile(char *fpid)
{
    FILE *pidfile;

    syslog(LOG_NOTICE, "gpio create pidfile %s", fpid);
    if ((pidfile = fopen(fpid, "w")) != NULL) {
                fprintf(pidfile, "%d\n", getpid());
                (void) fclose(pidfile);
    } else {
             syslog(LOG_NOTICE,"Failed to create pid file %s\n", fpid);
    }
}

/////////////////////////////////////////////////////////////////////////

void gpio_cleanup_pid(char *fpid)
{
        FILE *in;
        char pidnumber[32];
	
        in = fopen(fpid, "r");
        if (in) {
                if (fscanf(in, "%s", pidnumber) == 1) {
                    fclose(in);
                    if (atoi(pidnumber) != getpid()){
                                char cmd[64];
                                sprintf(cmd,"kill -9 %s",pidnumber);
                                system(cmd);
                    }
                } else
                        fclose(in);
        }
}

int send_signal_gpio(char *name,int id)
{

	FILE *fp;
	int pid;
	fp = fopen(name,"r");
	if (fp)
	{
		fscanf(fp,"%d",&pid);
		printf("gpio: restore default pid %d\n",pid);
		kill(pid,id);
		fclose(fp);
		return 0;
	}
	return -1;
}

void handle_gpio_action(unsigned long pin, unsigned long action)
{
	int  x;
	char *fpid;
	unsigned long state = 0;
	unsigned long frq = GPIO_NORMAL_SPEED;
	
	x = fork();

	switch (x)
	{
	    case -1:		// parent
	    	printf( "Can't fork!! GPIO Ending!\n");
	    default:
		exit(0);		
	    case 0:		//child
		switch(action)
		{
			case GPIO_DIRECT_LED_BLK: // Ste Direct LED blinking
				state = GPIO_DIRECT_LED_ON;
				fpid = GPIO_DIRECTLED;
				break;
			case GPIO_USB1_LED_BLK: // Set USB1 LED blinking
				state = GPIO_USB1_LED_ON;
				fpid = 	GPIO_USB1LED;
				break;	
			case GPIO_USB2_LED_BLK: // Set USb2 LED blinking
				state = GPIO_USB2_LED_ON;
				fpid = GPIO_USB2LED;
				break;
			case GPIO_WDOG_CLK: // Set WDOG clocking
				state = GPIO_WDOG_CLK_HIGH;
				frq  = GPIO_WDOG_SPEED;
				fpid = GPIO_WDOG;
				break;
			case GPIO_WLAN_LED_BLK: // Set WLAN LED blinking
				state = GPIO_WLAN_LED_ON;
				fpid = GPIO_WLANLED;
				break;
			case GPIO_SYS_LED_BLK_NLM: // Set SYS LED blinking
				state = GPIO_SYS_STATUS_LED_ON;
			        fpid = GPIO_SYSLED;
				break;
			case GPIO_SYS_LED_BLK_FST: // Set SYS LED blinking fater 
				state = GPIO_SYS_STATUS_LED_ON;
				frq  = GPIO_FAST_SPEED;
			        fpid = GPIO_SYSLED;
				break;				
			default: break;			
		}
		
		/* log process  id */	
		gpio_cleanup_pid(fpid);
		gpio_create_pidfile(fpid);
		break;	
	}
	
	for(;;)
	{
		ioctl(gpio_fd, (pin >> 8), state);
		usleep(frq);		
		ioctl(gpio_fd, (pin >> 8), ~state);
		usleep(frq);
	}
	
}

// Active Low on switch button
void handle_gpio_switch(int gpio_fd)
{
	int  x;
	unsigned long  state = -1;
	int  sw_count = 0;
	int  triger_flag = 0;	
	
	x = fork();
	switch (x)
	{
	    case -1:		// parent
	    	printf( "Can't fork!! GPIO Ending!\n");
	    default:
		return;		
	    case 0:		//child
	    	/* log process  id */	
		gpio_cleanup_pid(GPIO_BT);
		gpio_create_pidfile(GPIO_BT);
		break;	
	}
	
	for(;;)
	{
		state = ioctl(gpio_fd, (GPIO_B0_BT >> 8), GPIO_NONE);
		
		// triger switch counter		
		if((state == 0) && (triger_flag == 0))
			triger_flag = 1;
		
		// count ++
		if((state == 0) && (triger_flag))
			sw_count++;
			
		// check state			
		if(triger_flag)
		{
			if((state != 0) && (sw_count < GPIO_BT_RESTORE_TIME))
			{
				printf("system reboot\n");        // reboot
				sw_count = 0;
				triger_flag = 0;
				ioctl(gpio_fd, (GPIO_B7_SW_REBOOT >> 8), GPIO_SW_REBOOT_ENABLED);
			}
			else if(sw_count >= GPIO_BT_RESTORE_TIME)
			{
				printf("Restore to default\n");   // restore to default
				sw_count = 0;
				triger_flag = 0;
				send_signal_gpio(GPIO_CFG_DEFAULT, 51);
			}
		}	
		usleep(GPIO_NORMAL_SPEED);		
	}		
}
#endif

void usage(){
	fprintf(stderr, "\nUsage: \n"
			"\t gpio init pin direction\n"
			"\t gpio get pin\n"
			"\t gpio set pin data\n"
			"\t gpio blink pin speed\n\n");
	exit(1);
}

enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
};

typedef unsigned int uint32;
#define GPIO_ID(port,pin) ((uint32)port<<16|(uint32)pin)
unsigned int parse_pin(const char* str)
{
	int port = str[0]-'A';
	int pin = atoi(&str[1]);
	//fprintf(stderr, "port=%d, pin=%d\n", port, pin);

	if ( (port < 0) || (port > 8))
		return -1;
	if ( (pin < 0) || (pin > 8))
		return -1;

	return GPIO_ID(port, pin);
}

#define CMD_INIT   0
#define CMD_GET    1
#define CMD_SET    2
#define CMD_BLINK  3


#ifndef TEST_BIT
#define TEST_BIT( x, b )   (((x) & (1<<(b))) != 0 )
#define SET_BIT( x, b )    ((x) |= (1 << (b)))
#define CLEAR_BIT( x, b )  ((x) &= ~(1 << (b)))
#define TOGGLE_BIT( x, b ) ((TEST_BIT(x,b)) ?(CLEAR_BIT(x,b)):(SET_BIT(x,b)))
#endif


#define dbg if(0)printf

int main(int argc, char *argv[])
{
	unsigned int pin = -1;
	unsigned int gpio_id = -1;
	unsigned int direction = -1;
	unsigned int data = -1;
	unsigned int gpio_action = 0;
	
	int ret;
	
	gpio_fd = open( "/dev/gpio_ioctl", O_RDONLY);
	if (gpio_fd < 0 ) {
		printf( "ERROR : cannot open gpio\n");
		return	-1;
	}
	
	//fprintf(stderr, "argc: %d\n ", argc);
	if ( argc > 1 )  {
		if ( ( strcmp(argv[1], "init") == 0 ) && ( argc == 4 ) ) {
			gpio_action = CMD_INIT;
			if ( strcmp(argv[3], "in") == 0 )
				direction = 0;
			else 
				direction = 1;

			// set direction bit (3)
			if (direction)
				gpio_action = SET_BIT(gpio_action, 3);

		} else if ( ( strcmp(argv[1], "get") == 0 ) && ( argc == 3 ) ) {
			gpio_action = CMD_GET;
		} else if ( ( strcmp(argv[1], "set") == 0 ) && ( argc == 4 ) ) {
			gpio_action = CMD_SET;
			// set data bit (4)
			if ( atoi(argv[3]) )
				gpio_action = SET_BIT(gpio_action, 4);
		} else if ( ( strcmp(argv[1], "blink") == 0 ) && ( argc == 4 ) ) {
			gpio_action = CMD_BLINK;
			int period = atoi(argv[3]);
			dbg("period: %d (0x%x)\n ", period, period);
			gpio_action |= (period << 8);
			dbg("gpio_action: %x\n", gpio_action);

		} else 
			usage();
	} else 
		usage();

	if ( (argv[2][0] == '0') && (argv[2][1] == 'x') )
		sscanf(argv[2], "%x", &gpio_id);
	else
		gpio_id = parse_pin(argv[2]);
	//fprintf(stderr, "GPIO_ID=0x%08x  CMD=0x%08x\n", gpio_id, gpio_action);

	if ( gpio_id == -1 )
		usage();

	

	//fprintf(stderr, "GPIO_ID=0x%08x  CMD=0x%08x\n", gpio_id, gpio_action);
	//fprintf(stderr, "--------------------\n");
	ret = ioctl(gpio_fd, gpio_id, gpio_action);
	//fprintf(stderr, "--------------------\n");
	printf("%d\n", ret);

#if 0


	if((argc == 2) && (atoi(argv[1]) == 1)) 
	{
		// start Setup switch daemon
		handle_gpio_switch(gpio_fd);
		close(gpio_fd);
	
		return	0;
	}
	else if(argc != 5) return 0;
	
	//preprocess command, PIN, & action
	argv1 = atoi(argv[1]);
	argv2 = atoi(argv[2]);
	argv3 = atoi(argv[3]);
	
	// Command
	if(argv1 == 1)
		input = GPIO_STATIC_CMD;
	else if(argv1 == 2)
		input = GPIO_ACTION_CMD;

	// State
	if(argv2 == 1)
		input |= GPIO_SET_STATE;
	else if(argv2 == 2)
		input |= GPIO_GET_STATE;		
	
	// PIN
	switch(argv3)
	{
		case 1: input |= GPIO_B1_DIRECT_LED; break;
		case 2: input |= GPIO_B2_USB_LED1; break;
		case 3: input |= GPIO_B3_USB_LED2; break;
		case 4: input |= GPIO_B4_WDOG_CLK; break;
		case 5: input |= GPIO_B5_ICE_USE; break;
		case 6: input |= GPIO_B6_FW_USE; break;
		case 7: input |= GPIO_B7_SW_REBOOT; break;
		case 8: input |= GPIO_C3_TRST; break;
		case 9: input |= GPIO_C4_WLAN_LED; break;
		case 10: input |= GPIO_C5_SYS_STATUS_LED; break;
		default: break;
	}			
	
	// Action
	gpio_action = atoi(argv[4]);	
	
	// preprocess end
	#ifdef GPIO_DEBUG	
	printf("GPIO input = %lx, action = %d\n", input, gpio_action);
	#endif
	command = input & GPIO_CMD_MASK;
	gpio_pin = input & GPIO_PIN_MASK;
	
	switch(command)
	{
		case (GPIO_STATIC_CMD | GPIO_SET_STATE):	// Set GPIO static state
		case (GPIO_STATIC_CMD | GPIO_GET_STATE):        // Get GPIO state
			switch(gpio_action)
			{
				case GPIO_DIRECT_LED_BLK: // Ste Direct LED ON
					state = GPIO_DIRECT_LED_ON;
					break;
				case GPIO_USB1_LED_BLK: // Set USB1 LED ON
					state = GPIO_USB1_LED_ON;
					break;	
				case GPIO_USB2_LED_BLK: // Set USB2 LED ON
					state = GPIO_USB2_LED_ON;
					break;
				case GPIO_WDOG_CLK: // Set WDOG ON
					state = GPIO_WDOG_CLK_HIGH;
					break;
				case GPIO_FW_USE_OFF: // Set FW USE OFF; Active LOW for ENABLE
					state = GPIO_FW_USE_DISABLED;
					break;
				case GPIO_SW_REBOOT_ON: // Set SW REBOOT ON 
					state = GPIO_SW_REBOOT_ENABLED;
					break;					
				case GPIO_WLAN_LED_BLK: // Set WLAN LED ON
					state = GPIO_WLAN_LED_ON;
					break;
				case GPIO_SYS_LED_BLK_NLM: // Set SYS LED ON
					state = GPIO_SYS_STATUS_LED_ON;
					break;
				default:
					state = 0; 
					break;	// gpio_action = 0, selected PIN = High		
			}
			ret = ioctl(gpio_fd, (input >> 8), state);
			break;
		
		case (GPIO_ACTION_CMD | GPIO_SET_STATE): // Set GPIO action state
			handle_gpio_action(gpio_pin, gpio_action);	
			break;			
		default: break;
	}
#endif
	close(gpio_fd);
	
	return	0;	
}
	
