#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <mosquitto.h>

#define mqtt_host "test.muabaobao.com"
#define mqtt_port 1883

static bool run = true;
pthread_t mqttthreadID;

void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

	mosquitto_topic_matches_sub("/test", message->topic, &match);
	if (match) {
		printf("got message for ADC topic\n");
	}

}

static void stop_Mqtt_SigHandler(int dwSigNo)
{
	printf("get stop signal\r\n");

	run = false;
}

void *mqtt_Thread_Func(void *arg)
{
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	mosquitto_lib_init();

	memset(clientid, 0, 24);
	snprintf(clientid, 23, "mysql_log_%d", getpid());
	mosq = mosquitto_new(clientid, true, 0);

	if(mosq){
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

		mosquitto_subscribe(mosq, NULL, "/test", 0);

		while(run){
			rc = mosquitto_loop(mosq, -1, 1);
			if(run && rc){
				printf("connection error!\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}
		}
		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();

	return 0;
}

struct sigaction actions;
void start_matt(void)
{
	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask); /* 将参数set信号集初始化并清空 */
	actions.sa_flags = 0;
	actions.sa_handler = stop_Mqtt_SigHandler;

	sigaction(SIGALRM,&actions,NULL);

	pthread_create(&mqttthreadID,NULL,mqtt_Thread_Func,NULL);
}

//int main(int argc, char *argv[])
//{
//
//}
