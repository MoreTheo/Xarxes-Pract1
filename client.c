#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>


// Constants

#define MAX_LINE_LENGTH 255





// Register packet types definitions

#define REGISTER_REQ 0x00
#define REGISTER_ACK 0x02
#define REGISTER_NACK 0x04
#define REGISTER_REJ 0x06
#define ERROR 0x0F

// Estates of client

#define DISCONNECTED 0xA0
#define WAIT_REG_RESPONSE 0xA2
#define WAIT_DB_CHECK 0xA4
#define REGISTERED 0xA6
#define SEND_ALIVE 0xA8

// periodical communications packet types



//global variables


// Structs

typedef struct Client_data Client;
typedef struct Server_Data Server;
typedef struct UDP_PDU UDP;
typedef struct TCP_PDU TCP;


// functions declarations

void readCfg();
void storeID(char* line);

char* trimLine(char *buffer);

// global variables

bool debugMode = false;

char clientCfgFile[] = "client.cfg";
Client clientData;
Server serverData;

// Client structure

struct Client_data {
    char Id[6];
    char mac_address[12];
    int ip_server;
    int UDP_port;
};

// Server structure

struct Server_Data {
    char Id[7];
    char mac_addres[11];
    char* Server;
    int Server_UDP;
    int Server_TCP;
    int newServer_UDP;
};


// UDP PDU Struct
struct UDP_PDU {
    unsigned char Type;
    char Id_Tans[11];
    char mac_address[13];
    char rand_num[7];
    char Data[50];
};

//  TCP PDU Struct
struct TCP_PDU {
    unsigned char Type;
    char Id[7];
    char mac_addres[13];
    char rand_num[7];
    char Data[150];
    //char Info[80];
};




// FUNTION MAIN

int main(int argc, char* argv[]) {

    readCfg();
    return 0;
}

char* trimLine(char *buffer) {
    char* line = strchr(buffer, '='); // Delete chars from the start to the '='
    line++;   // Delete the '=' char
    if (line[0] == ' ') line++; // If there's a whitespace next to the '=', delete it too
    unsigned long lineSize = strlen(line);
    if (line[lineSize-1] == '\n') line[lineSize-1] = '\0';  // Remove the /n char if it exists
    return line;
}


void readCfg() {

    FILE* fd = fopen(clientCfgFile, "r");

    if (fd == NULL) {
        /*tratamiento de erro*/
        perror("Error to open the client.cfg file");
        exit(-1);
    }

    char line[MAX_LINE_LENGTH];
    char delim[] = " \n";
    char *token;

    while (fgets(line,sizeof(line),fd)) {
        token = strtok;

        switch (line[0]) {
            case 'Id':
                token = strtok(NULL, delim);
                strcpy(clientData.Id,token);
            case 'MAC':
                token = strtok(NULL, delim);
                strcpy(clientData.mac_address,token);
            case 'NMS-Id':
                token = strtok(NULL, delim);
                serverData.Server = malloc(strlen(token) + 1);
                strcpy(serverData.Server,token);
            case 'NMS-UDP':
                serverData.Server_UDP = atoi(strtok(NULL, delim));
        }
    }
}


