# include "buffer.h"

buffer_item buffer[BUFFER_SIZE+1];
int head;//before the first one in the buffer
int tail;//last one in the buffer

void buffer_init()				//buffer initialize
{
	head=0;
	tail=0;
}

int insert_item(buffer_item item)
{
	if((tail+1)%(BUFFER_SIZE+1)==head)	//full
		return -1;
	tail = (tail+1)%(BUFFER_SIZE+1);
	buffer[tail]=item;
	return 0;
}

int remove_item(buffer_item *item)
{
	if(head==tail)				//empty
		return -1;
	head=(head+1)%(BUFFER_SIZE+1);
	*item=buffer[head];
	return 0;
}
