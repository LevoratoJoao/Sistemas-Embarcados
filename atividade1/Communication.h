#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include "Queue.h"

typedef struct {
    int txPort;
    int rxPort;
    Queue buffer;
} Communication;

void setupCommunication(Communication *comms, int txPort, int rxPort);
bool isCableConnected(Communication *comms);

void sendData(Communication *comms, String message);
void receiveData(Communication *comms);

void processDataTransmission(Communication *comms, String message);
String processIncomingData(Communication *comms);

#endif