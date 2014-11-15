#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define FLYAVR_ADDRESS 0x10	
#define wait 5		/* 100ms per led			*/





#define CMD_GET_EEPROM		(0x01)
#define CMD_SET_EEPROM		(0x02)
#define CMD_GET_VERSION		(0x10)
#define CMD_GET_HW			(0x11)
#define CMD_DATA_ON			(0x1A)
#define CMD_DATA_OFF		(0x1B)
#define CMD_GET_COUNTER		(0x1C)
#define CMD_GET_REBOOTS		(0x1D)
#define CMD_GET_CRC16_XMODEM (0x20)



#define dbg if(0)printf
#define info printf
#define warn printf


int i2c;								
char buf[16];
__u8 recv_buf[128];
__u8 send_buf[128];

__u16 crc_ccitt_update (__u16 crc, __u8 data);
__u16 crc_xmodem_update (__u16 crc, __u8 data);
void crc1(__u8* buf);

void i2c_write(const char* buf, size_t size)
{
	int i;
	dbg("Write: ");
	for (i=0; i<size; i++)
		dbg("0x%x ", buf[i]);

	if ( write(i2c, buf, size) == size ) {
		dbg(" ok\n");
	} else { 
		dbg(" fail\n");
		warn(" write fail\n");
	}

}

void i2c_read(char* buf, size_t size)
{
	int i;
	size_t readed;
	readed = read(i2c, buf, size);
	dbg("Readed: ");
	for (i=0; i<size; i++)
		dbg("0x%2x ", (__u8)buf[i]);
	if (readed == size) {
		dbg(" readed %d ok\n", readed);
	} else {
		dbg(" readed %d fail\n", readed);
		warn(" read fail\n");
	}
}

int mysleep( unsigned int s)  	/* my own sleep		  	*/
{				/* usleep waits only 20ms 	*/
  unsigned int i;		/* and ignore the parameter	*/
  for (i=0;i<s;i++) {		/* but the parameter must exist!*/
    usleep(1);						
  }
}

int SendReceiveBlock(__u8 *send_buf, size_t send_buf_size, __u8 *recv_buf, size_t recv_buf_size)
{
	int ret, i;
	struct i2c_msg msgs[] = {
		{ FLYAVR_ADDRESS, 0, send_buf_size, send_buf },
		{ FLYAVR_ADDRESS, I2C_M_RD, recv_buf_size, recv_buf }
	};
	struct i2c_rdwr_ioctl_data data = {
		msgs, 2
	};

	ret = ioctl( i2c, I2C_RDWR, &data );

	dbg("Send:");
	for(i=0; i<send_buf_size; i++)
		dbg(" %2x", (__u8)send_buf[i]);
	dbg("   Read:");
	for(i=0; i<recv_buf_size; i++)
		dbg(" %2x", (__u8)recv_buf[i]);
	dbg("\n");

	if (ret < 0) {
		fprintf(stderr, "Error: Sending/Reading error:%d\n", ret);
		return 0;
	}
	return 1;
}

int data_on()
{
	send_buf[0] = CMD_DATA_ON;
	//SendReceiveBlock(send_buf, 1, recv_buf, 0);
	i2c_write(send_buf, 1);
}

int data_off()
{
	send_buf[0] = CMD_DATA_OFF;
	//SendReceiveBlock(send_buf, 1, recv_buf, 0);
	i2c_write(send_buf, 1);
}

unsigned int u16from_buf(unsigned char *buf)
{
	return ( buf[0] |  (buf[1] << 8) );
}
unsigned int u32from_buf(unsigned char *buf)
{
	return ( buf[0] |  (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24) );
}

unsigned int u32to_buf(unsigned int num, unsigned char* buf)
{
	buf[0] = (num & 0xFF);
	buf[1] = ((num >> 8) & 0xFF);
	buf[2] = ((num >> 16) & 0xFF);
	buf[3] = ((num >> 24) & 0xFF);
}

int get_ver()
{
	send_buf[0] = CMD_GET_VERSION;
	SendReceiveBlock(send_buf, 1, recv_buf, 1);
	printf("Version: %d\n", recv_buf[0]);
}

int get_hw()
{
	send_buf[0] = CMD_GET_HW;
	SendReceiveBlock(send_buf, 1, recv_buf, 1);
	printf("Hardware: %d\n", recv_buf[0]);
}

int get_counter()
{
	int counter = 0;
	send_buf[0] = CMD_GET_COUNTER;
	SendReceiveBlock(send_buf, 1, recv_buf, 4);
	counter = u32from_buf(recv_buf);

	printf("Counter: %d\n", counter);
}

int get_eeprom(__u8 offset)
{
	send_buf[0] = CMD_GET_EEPROM;
	send_buf[1] = offset;
	SendReceiveBlock(send_buf, 2, recv_buf, 1);
	printf("EEPROM[%d]: %d\n", offset, recv_buf[0]);
	return recv_buf[0];
}

void set_eeprom(__u8 offset, __u8 value)
{
	send_buf[0] = CMD_SET_EEPROM;
	send_buf[1] = offset;
	send_buf[2] = value;
	i2c_write(send_buf, 3);
	//SendReceiveBlock(send_buf, 3, recv_buf, 1);
	//printf("EEPROM[%d]: %d\n", offset, recv_buf[0]);
}

int get_crc16_xmodem(unsigned int num)
{
	int crc = 0;
	send_buf[0] = CMD_GET_CRC16_XMODEM;
	u32to_buf(num, &send_buf[1]);
	SendReceiveBlock(send_buf, 5, recv_buf, 2);
	crc = u16from_buf(recv_buf);

	printf("CRC16 XMODEM: 0x%x\n", crc);
}

int get_reboots()
{
	send_buf[0] = CMD_GET_REBOOTS;
	SendReceiveBlock(send_buf, 1, recv_buf, 4);
	printf("Reboots: %d\n", u16from_buf(recv_buf));
	printf("WDT Reboots: %d\n", u16from_buf(recv_buf+2));
}

int get_crc1(unsigned int num)
{
	__u8 crc_buf[4];
	int flyavr_crc = 0, calc_crc = 0, i;
	send_buf[0] = CMD_GET_CRC16_XMODEM;
	u32to_buf(num, &send_buf[1]);
	SendReceiveBlock(send_buf, 5, recv_buf, 4);
	flyavr_crc = u32from_buf(recv_buf);
	//printf("CRC1 FROM FLYAVR: 0x%X\n", flyavr_crc);

	u32to_buf(num, crc_buf);
	//for (i = 0; i < 4; i++) info("%2X ", crc_buf[i]); info("\n");
	crc1(crc_buf);

	//for (i = 0; i < 4; i++) info("%2X ", crc_buf[i]); info("\n");
	calc_crc = u32from_buf(crc_buf);
	//printf("CRC1  CALCULATED: 0x%X\n", calc_crc);
	if ( calc_crc == flyavr_crc ) {
		info("ok ");
	} else {
		info("fail %X   %X\n", flyavr_crc, calc_crc);
	}
}

int main(int argc, char** argv) 
{
	int cont = 0;
	int count;

	i2c = open("/dev/i2c/0",O_RDWR);	/* open the device on first i2c-bus 	*/
	if ( i2c > 0 ) {
		ioctl(i2c,I2C_SLAVE, FLYAVR_ADDRESS); 	/* set the i2c chip address 		*/

		if (argc == 1) {
			exit(1);
		}

		if ( ( strcmp(argv[1], "s1r1") == 0 ) && (argc == 3) ) {
			send_buf[0] = atoi(argv[2]);
			SendReceiveBlock(send_buf, 1, recv_buf, 1);
		} else if ( (argc == 2) && ( strcmp(argv[1], "ver") == 0 ) ) {
			get_ver();
		} else if ( (argc == 2) && ( strcmp(argv[1], "hw") == 0 ) ) {
			get_hw();
		} else if ( (argc == 2) && ( strcmp(argv[1], "counter") == 0 ) ) {
			get_counter();
		} else if ( (argc == 3) && ( strcmp(argv[1], "crc16") == 0 ) ) {
			get_crc16_xmodem(atoi(argv[2]));
		} else if ( (argc == 3) && ( strcmp(argv[1], "crc1") == 0 ) ) {
			get_crc1(atoi(argv[2]));
		} else if ( (argc == 2) && ( strcmp(argv[1], "reboots") == 0 ) ) {
			get_reboots();
		} else if ( (argc == 3) && ( strcmp(argv[1], "get") == 0 ) ) {
			get_eeprom(atoi(argv[2]));
		} else if ( (argc == 4) && ( strcmp(argv[1], "set") == 0 ) ) {
			set_eeprom(atoi(argv[2]), atoi(argv[3]));
		} else if ( strcmp(argv[1], "on") == 0 ) {
			data_on();
		} else if ( strcmp(argv[1], "off") == 0 ) {
			data_off();
		} else if ( strcmp(argv[1], "onoff") == 0 ) {
			data_on();
			mysleep(1);
			data_off();
		} 

	}
	i2c = close(i2c);
	return 0;
} 
  


__u8 lo8(__u16 n)
{
	return (n & 0xFF);
}

__u8 hi8(__u16 n)
{
	return (n >> 8);
}


__u16 crc_ccitt_update (__u16 crc, __u8 data)
{
	data ^= lo8 (crc);
	data ^= data << 4;

	crc =  ((((__u16)data << 8) | hi8 (crc)) ^ (__u8)(data >> 4) 
			^ ((__u16)data << 3));
	//dbg("CRC16_CCITT: %04x\n", crc);
	return crc;
}


__u16 crc_xmodem_update (__u16 crc, __u8 data)
{
	int i;

	crc = crc ^ ((__u16)data << 8);
	for (i=0; i<8; i++)
	{
		if (crc & 0x8000)
			crc = (crc << 1) ^ 0x1021;
		else
			crc <<= 1;
	}

	//dbg("CRC16_XMODEM: %04x\n", crc);
	return crc;
}

//
//   FLYAVR CODE
//
// crc16_xmodem = _crc_xmodem_update(0, buf[0]);
// crc16_xmodem = _crc_xmodem_update(crc16_xmodem, buf[1]);
// crc16_ccitt  = _crc_ccitt_update(crc16_xmodem, buf[2]);
// crc16_ccitt  = _crc_ccitt_update(crc16_xmodem, buf[3]);
// usiTwiTransmitByte( (crc16_xmodem >> 0) & 0xff );
// usiTwiTransmitByte( (crc16_xmodem >> 8) & 0xff );
// usiTwiTransmitByte( (crc16_ccitt >> 0) & 0xff );
// usiTwiTransmitByte( (crc16_ccitt >> 8) & 0xff );
//
void crc1(__u8* buf)
{
	__u16 crc16_xmodem;
	__u16 crc16_ccitt;

	crc16_xmodem = crc_xmodem_update(0, buf[0]);
	crc16_xmodem = crc_xmodem_update(crc16_xmodem, buf[1]);
	crc16_ccitt  = crc_ccitt_update(crc16_xmodem, buf[2]);
	crc16_ccitt  = crc_ccitt_update(crc16_xmodem, buf[3]);
	buf[0] = ( (crc16_xmodem >> 0) & 0xff );
	buf[1] = ( (crc16_ccitt >> 0)  & 0xff );
	buf[2] = ( (crc16_xmodem >> 8) & 0xff );
	buf[3] = ( (crc16_ccitt >> 8)  & 0xff );
}
