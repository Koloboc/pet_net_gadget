#ifndef __DEFINES_H__
#define __DEFINES_H__

#define BROADCAST "127.255.255.255"
#define QUEUE_LISTEN 10

#define TIME_UNIT_WAITE 15

#undef max
#define max(x,y) ((x)>(y)?(x):(y))

#define IMREADY 0
//#define GIVE_ME_DATA 0
//#define SEND_DATA 1
//#define SETVAL 3
//#define YOU_BIG_ID 4

typedef struct _data_sensor{
	int temp;
	int brithness;
}data_sensor;

typedef struct _message{
	int nom;
	int id;
	int type;
	data_sensor ds;
}message;


#endif
