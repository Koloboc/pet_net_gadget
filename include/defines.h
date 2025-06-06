#ifndef __DEFINES_H__
#define __DEFINES_H__

#define BROADCAST "127.255.255.255"
#define ADDRESS "127.0.0.1"
#define QUEUE_LISTEN 10
#define LISTENPORT 1111

#define TIME_UNIT_WAITE 5

#undef max
#define max(x,y) ((x)>(y)?(x):(y))

#ifdef DEBUG
#	define LOG(fmt, args...) printf(fmt, ## args)
#else
#	define LOG(fmt, args...)
#endif

typedef struct _message{
	int id;
	float temp;
	float br;
}message;


#endif
