#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <string.h>
#include <MQTTClient.h>

#define ADDRESS "tcp://172.30.1.50:1883" //Broker IP Address
#define ADDRESS_SECURE "tcp://172.30.1.50:8883" //Broker IP Address but with TLS encryption
#define CLIENTID "CLIENT_SUB" //Client Name
#define TOPIC_1 "lab/light" //Topic Configuration (Light)
#define TOPIC_2 "light/stat" //Topic for current status of light
#define PAYLOAD_1 "Switch is ON" //Defining message for status message publication
#define PAYLOAD_2 "Switch is OFF"
#define QOS 1 //QOS Configuration
#define TIMEOUT 10000L //Timeout required for finishing comm

MQTTClient client; //Client handle
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer; //variable for MQTT message
MQTTClient_deliveryToken token;

const int outpin = 23; // HIGH-LOW OUTPUT Pin - Broadcom pin 23, P1 pin 16

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

    wiringPiSetupGpio(); //WringPi Initialization

    pinMode(outpin, OUTPUT); //pin configuration

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

    if (msgarray[0] == 'o' && msgarray[1] == 'n') {
        wiringPiSetupGpio(); //WringPi Initialization

        pinMode(outpin, OUTPUT); //pin configuration

        digitalWrite(outpin, LOW); //write to outpin

        printf("Switch is ON\n");

        pubmsg.payload = PAYLOAD_1; //loads predefined payload(PAYLOAD_1, lab/light) to pubmsg
        pubmsg.payloadlen = strlen(PAYLOAD_1); 
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        deliveredtoken = 0;
        MQTTClient_publishMessage(client, TOPIC_2, &pubmsg, &token); //publishes message to predefined topic(TOPIC_2, light/stat)
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        printf("Status Message Delivered\n");
    } 
    else if (msgarray[0] == 'o' && msgarray[1] == 'f' && msgarray[2] == 'f') {
        wiringPiSetupGpio(); //WringPi Initialization

        pinMode(outpin, OUTPUT); //pin configuration
        
        digitalWrite(outpin, HIGH); //write to outpin

        printf("Switch is OFF\n");

        pubmsg.payload = PAYLOAD_2;
        pubmsg.payloadlen = strlen(PAYLOAD_2);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        deliveredtoken = 0;
        MQTTClient_publishMessage(client, TOPIC_2, &pubmsg, &token);
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        printf("Status Message Delivered\n");
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
    //subscribe for predefined topic (lab/light)

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    //disconnect & destroy after receiving Q or q
    return rc;
}