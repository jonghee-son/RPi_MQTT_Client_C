#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <errno.h>
#include <MQTTClient.h>

#define ADDRESS "tcp://172.30.1.50:1883" //Broker IP Address
#define ADDRESS_SECURE "tcp://172.30.1.50:8883" //Broker IP Address but with TLS encryption
#define CLIENTID "CLIENT_SUB" //Client Name
#define TOPIC_1 "car/control" //Topic Configuration (control)
#define QOS 1 //QOS Configuration
#define TIMEOUT 10000L //Timeout required for finishing comm

MQTTClient client; //Client handle
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer; //variable for MQTT message
MQTTClient_deliveryToken token;

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    //Gets delivery token from finished communication and spits out message below
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

void connlost(void *context, char *cause) {
    //callback from connection lost event
    printf("\nConnection lost\n");
    printf("cause: %s\n", cause);
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    //send received message to preconfigured array msgarray[5] and parses command from that array
    int rc, ch;
    int i;
    char* payloadptr;
    char msgarray[5];

    int serial_port ;
    char dat;
    if ((serial_port = serialOpen ("/dev/ttyUSB0", 115200)) < 0) {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1 ;
    }

    if (wiringPiSetup () == -1) {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    printf("Message arrived\n");
    printf("topic: %s\n", topicName);
    printf("message: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++) {
        //getting received message from message pointer and saves each char to msgarray[5]
        msgarray[i] = *payloadptr++;
        putchar(msgarray[i]);
    }
    putchar('\n');

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    //frees memory allocated to received message

    if (msgarray[0] == 'w' || msgarray[0] == 'W') {
        dat = 'w';
        serialPutchar(serial_port, dat);
    } 
    else if (msgarray[0] == 'a' || msgarray[0] == 'A') {
        dat = 'a';
        serialPutchar(serial_port, dat);
    }
    else if (msgarray[0] == 's' || msgarray[0] == 'S') {
        dat = 's';
        serialPutchar(serial_port, dat);
    }
    else if (msgarray[0] == 'd' || msgarray[0] == 'D') {
        dat = 'd';
        serialPutchar(serial_port, dat);
    }
    else {
        dat = 'r';
        serialPutchar(serial_port, dat);
    }

    return 1;
}

int main(int argc, char* argv[]) {
    int rc, ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL); //create MQTTClient
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    //creates MQTTClient with client handle, address, client ID, etc.

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered); 
    //sets callback functions for connection lost, message arrived, message delivered events

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n", TOPIC_1, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC_1, QOS);
    //subscribe for predefined topic

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    //disconnect & destroy after receiving Q or q
    return rc;
}
