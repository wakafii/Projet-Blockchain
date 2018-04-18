#ifndef EPID_MESSAGE_H
#define EPID_MESSAGE_H
#include <sys/types.h>


struct EpidMessage
{
	char id[65];
	int code;
	size_t msg_size;
	size_t sig_size;
};

typedef struct EpidMessage EpidMessage;

#endif
