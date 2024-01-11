#ifndef __DEFINES_H__
#define __DEFINES_H__

#define BROADCAST "255.255.255.255"
#define QUEUE_LISTEN 10
#define LISTENPORT 1111

#define TIME_UNIT_WAITE 6

#undef max
#define max(x,y) ((x)>(y)?(x):(y))


typedef struct _message{
	int id;
	int temp;
	int brithness;
}message;


#endif
