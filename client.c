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






// Structs

typedef struct Client_data Client;
typedef struct Server_Data Server;
typedef struct UDP_PDU UDP;
typedef struct TCP_PDU TCP;


// functions declarations

void readCfg();
void storeID(char* line);

char* trimLine(char *buffer);


void parse_argv(int argc, char* argv[]);
bool checkFileName(char filename[]);

// global variables

bool debugMode = false;
char* network_config_name_file = "boot.cfg";

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
    char *Server;
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

        while (fgets(line, sizeof(line), fd)) {
            token = strtok(line, delim);
            if (token == NULL) {
                continue;
            }

            if (strcmp(token, "Id") == 0) {
                token = strtok(NULL, delim);
                if (token == NULL) {
                    continue;
                }

                strncpy(clientData.Id, token, sizeof(clientData.Id)-1);
                clientData.Id[MAX_LINE_LENGTH-1] = '\0';
                printf("Id = %s\n", clientData.Id);

            } else if (strcmp(token, "MAC") == 0) {
                token = strtok(NULL, delim);
                if (token == NULL) {
                    continue;
                }
                strncpy(clientData.mac_address, token, sizeof(clientData.mac_address));
                clientData.mac_address[MAX_LINE_LENGTH-1] = '\0';
                printf("Mac=%s\n", clientData.mac_address);

            } else if (strcmp(token, "NMS-Id") == 0) {
                token = strtok(NULL, delim);
                if (token == NULL) {
                    continue;
                }
                //strncpy(serverData.Server, token, sizeof(serverData.Server)-1);
                //serverData.Server[MAX_LINE_LENGTH-1] = '\0';
                serverData.Server = malloc(strlen(token) + 1);
                strcpy(serverData.Server,token);
                printf("localhost = %s\n", serverData.Server);

            } else if (strcmp(token, "NMS-UDP-port") == 0) {
                serverData.Server_UDP = atoi(strtok(NULL,delim));
                printf("UDP-PORt = %i\n",serverData.Server_UDP);
            }
        }
}


void parse_argv(int argc, char* argv[]) {
    
    for (int i = 1; i < argc; i++) { // argument 0 ==> name of program
        if (strcmp(argv[i], "-c") == 0) {
            if (checkFileName(argv[i+1])) {
                strcpy(clientCfgFile, argv[i+1]);
            } else {
                /*mensage de error*/
                exit(-1);
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            debugMode = true;
            /*mensage of debugMode is on*/
        } else if (strcmp(argv[i], "-f") == 0 && argc > (i + 1)) {
            network_config_name_file = malloc(sizeof(argv[i + 1]));
            strcpy(network_config_name_file, argv[i + 1]);
        }
    }
}

bool checkFileName(char filename[]) {
    char* extension = strchr(filename,'.');
    if (extension == NULL) return false;
    if (strcmp(extension,".cfg") != 0) return false;
    return true;
}


