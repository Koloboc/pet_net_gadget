#ifndef __DEFINES_H__
#define __DEFINES_H__

#define BROADCAST "255.255.255.255"
//#define BROADCAST "127.255.255.255"
#define QUEUE_LISTEN 10

#define TIME_UNIT_WAITE 7

#undef max
#define max(x,y) ((x)>(y)?(x):(y))

#define IMREADY 0
#define TIMEOUT IMREADY + 2
#define MYDATA	IMREADY + 3
#define CHANGEID IMREADY + 4
#define SETDISPLAY IMREADY + 5
//#define SETVAL 3
//#define YOU_BIG_ID 4

typedef struct _data_sensor{
	int temp;
	int brithness;
}data_sensor;

typedef struct _message{
	int id_req;
	int id_res;
	int id;
	int type;
	data_sensor ds;
}message;


#endif
