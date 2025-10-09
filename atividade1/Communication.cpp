#include "Communication.h"
#include <string.h>

void setupCommunication(Communication *comms, int txPort, int rxPort) {
    comms->txPort = txPort;
    comms->rxPort = rxPort;
    queue_init(&comms->buffer);

    pinMode(comms->txPort, OUTPUT);
    pinMode(comms->rxPort, INPUT_PULLUP);
}

bool isCableConnected(Communication *comms) {
    digitalWrite(comms->txPort, LOW);
    delayMicroseconds(80);
    bool lowSeen = (digitalRead(comms->rxPort) == LOW);

    digitalWrite(comms->txPort, HIGH);
    delayMicroseconds(80);
    bool highSeen = (digitalRead(comms->rxPort) == HIGH);

    digitalWrite(comms->txPort, LOW);

    return lowSeen && highSeen;
}

void sendData(Communication *comms, String message) {
    if (!isCableConnected(comms)) {
        Serial.println(F("Jumper disconnected! Transmission aborted."));
        return;
    }

    Serial.println(F("Sending data..."));
    processDataTransmission(comms, message);
}

void receiveData(Communication *comms) {
    if (!isCableConnected(comms)) {
        Serial.println(F("Jumper disconnected! Reception aborted."));
        queue_init(&comms->buffer);
        return;
    }

    Serial.println(F("Receiving data..."));
    String received = processIncomingData(comms);
    if (received.length() > 0) {
        Serial.print(F("Data received: "));
        Serial.println(received);
    } else {
        Serial.println(F("No data in buffer."));
    }
}

void processDataTransmission(Communication *comms, String message) {
    const char *buffer = message.c_str();

    if (!isCableConnected(comms)) {
        Serial.println(F("Jumper disconnected during transmission! Aborting."));
        return;
    }

    while (*buffer) {
        char ch = *buffer;
        for (int i = 7; i >= 0; --i)
        {
            char bit = (ch >> i) & 1;
            queue_put(&comms->buffer, bit);
        }
        ++buffer;
    }
    Serial.println(F("Data transmission to buffer complete."));
}

String processIncomingData(Communication *comms) {
    String message = "";
    char bits[8];
    int bitCount = 0;
    char bit;
    while (queue_get(&comms->buffer, &bit)) {
        bits[bitCount++] = bit;
        if (bitCount == 8) {
            char ch = 0;
            for (int i = 0; i < 8; ++i) {
                ch = (ch << 1) | (bits[i] & 1);
            }
            message += ch;
            bitCount = 0;
        }
    }
    return message;
}