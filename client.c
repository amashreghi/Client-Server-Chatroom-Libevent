#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <string.h>
#include <stdlib.h>

#include "defs.h"

char *client_name;

void* wait_for_input(void *arg){
	struct bufferevent *bev = (struct bufferevent *)arg;
	
	char buff[BUFF_SIZE];

	while(1){
		bzero(buff, sizeof(buff));
        int n = 0;
		
		for(int i = 0; i < strlen(client_name); i++){
			buff[n++] = client_name[i];
		}
		
		buff[n++] = ':';
		buff[n++] = ' ';
		
        while (n + 1 < BUFF_SIZE && (buff[n++] = getchar()) != '\n');
		
		evbuffer_add(bufferevent_get_output(bev), buff, BUFF_SIZE);
	}
}

static void set_tcp_no_delay(evutil_socket_t fd)
{
	int one = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			&one, sizeof one);
}

static void timeoutcb(evutil_socket_t fd, short what, void *arg)
{
	struct event_base *base = arg;
	printf("timeout\n");

	event_base_loopexit(base, NULL);
}

static void readcb(struct bufferevent *bev, void *ctx)
{
	/* This callback is invoked when there is data to read on bev. */
	struct evbuffer *input = bufferevent_get_input(bev);

	char *received = malloc(BUFF_SIZE);
	evbuffer_copyout(input, received, BUFF_SIZE);
	evbuffer_drain(input, BUFF_SIZE);
	
	printf(">>>>> %s\n", received);
}

static void writecb(struct bufferevent *bev, void *ctx){
	//this will be called but it's of no use for now
}

static void eventcb(struct bufferevent *bev, short events, void *ptr)
{
	if (events & BEV_EVENT_CONNECTED) {
		printf("%s was successfully connected\n", client_name);
		evutil_socket_t fd = bufferevent_getfd(bev);
		set_tcp_no_delay(fd);
	} else if (events & BEV_EVENT_ERROR) {
		printf("NOT Connected\n");
	}
}

int main(int argc, char **argv)
{
	struct event_base *base;
	struct bufferevent **bevs;
	struct sockaddr_in sin;
	struct event *evtimeout;
	struct timeval timeout;
	int i;

	if(argc > 1){
		client_name = argv[1];
	}
	else{
		printf("Must provide name\n");
		exit(0);
	}

	base = event_base_new();
	if (!base) {
		puts("Couldn't open event base");
		return 1;
	}

	evtimeout = evtimer_new(base, timeoutcb, base);
	evtimer_add(evtimeout, &timeout);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sin.sin_port = htons(PORT);

	struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(bev, readcb, writecb, eventcb, NULL);
	bufferevent_enable(bev, EV_READ|EV_WRITE);

	if (bufferevent_socket_connect(bev,
				(struct sockaddr *)&sin, sizeof(sin)) < 0) {
		/* Error starting connection */
		bufferevent_free(bev);
		puts("error connect");
		return -1;
	}

	pthread_t waiting_thread_id;
	
	pthread_create(&waiting_thread_id, NULL, wait_for_input, (void *)bev);
	
	event_base_dispatch(base);

	pthread_join(waiting_thread_id, NULL);

	bufferevent_free(bev);

	event_free(evtimeout);
	event_base_free(base);

	return 0;
}
