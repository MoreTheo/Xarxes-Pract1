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
#define T 1
#define P 2
#define Q 3
#define U 2
#define N 6
#define O 2
#define R 2
#define S 3
#define W 3

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
#define ALIVE_INF 0x10
#define ALIVE_ACK 0x12
#define ALIVE_NACK 0x14
#define ALIVE_REJ 0x16

#define SEND_FILE 0x20
#define SEND_DATA 0x22
#define SEND_ACK 0x24
#define SEND_NACK 0x26
#define SEND_REJ 0x28
#define SEND_END 0x2A

// send to server packet
#define GET_FILE 0x30
#define GET_DATA 0x32 
#define GET_ACK 0x34  
#define GET_NACK 0x36
#define GET_REJ 0x38
#define GET_END 0x3A




// Structs

typedef struct Client_data Client;
typedef struct Server_Data Server;
typedef struct UDP_PDU UDP;
typedef struct TCP_PDU TCP;

UDP buildREGISTER_REQ();


// functions declarations

void readCfg();
void storeID(char* line);

char* trimLine(char *buffer);


void parse_argv(int argc, char* argv[]);
bool checkFileName(char filename[]);

void changes_client_state(char *state);
void show_msg(char *msg); 

// fuctions declarations register

void open_UDP_socket();
void config_direcction_struct_server_UDP();
void receive_register_packet_udp();
void treatment_packet_type(UDP packet);

void treatment_packet_type_error();
void save_register_packet_ack_data(UDP received_packet);
void treatment_packet_ACK(UDP packet);

void login();

// global variables

bool debugMode = false;
char* network_config_name_file = "boot.cfg";

char clientCfgFile[] = "client.cfg";
Client clientData;
Server serverData;

int udp_socket = -1;
int tcp_socket1 = -1;
int tcp_socket2 = -1;

char *client_state = NULL;

bool resetCommunication = false;

struct sockaddr_in clientAddrUDP, clientAddrTCP, serverAddrUDP, serverAddrTCP;

// Client structure

struct Client_data {
    char Id[6];
    char mac_address[12];
    int ip_server;
    int UDP_port;
    unsigned char state;
};

// Server structure

struct Server_Data {
    char Id[7];
    char mac_addres[11];
    char *Server;
    int Server_UDP;
    int Server_TCP;
    int newServer_UDP;
    char rand_num[7]; 
};


// UDP PDU Struct
struct UDP_PDU {
    unsigned char Type;
    char Id_Tans[7];
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

    parse_argv(argc,argv);
    readCfg();
    login();
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
            //printf("Id = %s\n", clientData.Id);

        } else if (strcmp(token, "MAC") == 0) {
            token = strtok(NULL, delim);
            if (token == NULL) {
                continue;
            }
            strncpy(clientData.mac_address, token, sizeof(clientData.mac_address));
            clientData.mac_address[MAX_LINE_LENGTH-1] = '\0';
            //printf("Mac=%s\n", clientData.mac_address);

        } else if (strcmp(token, "NMS-Id") == 0) {
            token = strtok(NULL, delim);
            if (token == NULL) {
                continue;
            }
            //strncpy(serverData.Server, token, sizeof(serverData.Server)-1);
            //serverData.Server[MAX_LINE_LENGTH-1] = '\0';
            serverData.Server = malloc(strlen(token) + 1);
            strcpy(serverData.Server,token);
            //printf("localhost = %s\n", serverData.Server);

        } else if (strcmp(token, "NMS-UDP-port") == 0) {
            serverData.Server_UDP = atoi(strtok(NULL,delim));
            //printf("UDP-PORt = %i\n",serverData.Server_UDP);
        }
    }
    if (debugMode) {
        printf("Okay to read file cfg\n");
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
            if(debugMode) {
                printf("DEBUG MODE IS ON\n");
            }
            /*mensage of debugMode is on*/
        } else if (strcmp(argv[i], "-f") == 0 && argc > (i + 1)) {
            network_config_name_file = malloc(sizeof(argv[i + 1]));
            strcpy(network_config_name_file, argv[i + 1]);
        }
    }
    if (debugMode) {
        printf("OK in checkParams\n");
    }
}

bool checkFileName(char filename[]) {
    char* extension = strchr(filename,'.');
    if (extension == NULL) return false;
    if (strcmp(extension,".cfg") != 0) return false;
    return true;
}


// LOGIN or Register

void open_UDP_socket() {
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_socket < 0) {
        /*try to error*/
        perror("Error opening the UDP socket");
        exit(-1);
    }

    // configuration client addres with => sockaddr_in
    memset(&clientAddrUDP,0,sizeof (struct sockaddr_in));
    clientAddrUDP.sin_family = AF_INET;
    clientAddrUDP.sin_port = htons(0);
    clientAddrUDP.sin_addr.s_addr = (INADDR_ANY);

    // bind
    if (bind(udp_socket,(const struct sockaddr *) &clientAddrUDP, sizeof( struct sockaddr_in)) < 0) {
        /*try to error*/
        perror("Error to bind");
        exit(-1);
    }

    if (debugMode) {
        printf("OKAY OPEN UDP SOCKET\n");
    } 
}

void login() {

    // define signal handler
    // open udp socket
    if (udp_socket < 0) {
        open_UDP_socket();
    }

    config_direcction_struct_server_UDP();

    clientData.state = DISCONNECTED;
    changes_client_state("DISCONNECTED");
    /*messsage*/

    UDP register_req_packet = buildREGISTER_REQ();
    

    int sendto_ = sendto(udp_socket, &register_req_packet,sizeof(UDP),0,(struct sockaddr *) &serverAddrUDP, (socklen_t) sizeof(serverAddrUDP));
    if (sendto_ < 0) {
        /*message error*/
        perror("Error in send the UDP");
        exit(-1);
    }

    if(debugMode) {
        printf("First reg_req\n");
    }

    //cahnges status of client
    clientData.state = WAIT_REG_RESPONSE;
    changes_client_state("WAIT_REG_RESPONSE");

    struct timeval timeout;
    int acc;
    fd_set read_fds;

    for (int sign_ups = 0; sign_ups < O; sign_ups++) {
        acc = 0;
        /*debug mode*/

        for (int packets_For_Signup = 0; packets_For_Signup < N; packets_For_Signup++) {
            timeout.tv_sec = T;
            timeout.tv_usec = 0;
            if(packets_For_Signup >= P && packets_For_Signup < Q) {
                acc = acc + T;
                timeout.tv_sec += acc;
            } else if (packets_For_Signup >= Q) {
                timeout.tv_sec = Q * T;
            }

            FD_ZERO(&read_fds);
            FD_SET(udp_socket, &read_fds);
            if(select(udp_socket + 1, &read_fds, NULL, NULL, &timeout) > 0) {
                receive_register_packet_udp();
                if (!resetCommunication) {
                    return;
                }
            }
            /*
            COMPROBAR PAQUETE
            ->REGISTER_REJ
            ->REGISTER_NACK
            ->REGISTER_ACK
            */
            int sendto_ = sendto(udp_socket, &register_req_packet, sizeof(UDP),0,
                            (struct sockaddr *) &serverAddrUDP, (socklen_t) sizeof(serverAddrUDP));
            
            if (sendto_ < 0) {
                /*message error*/
                perror("Error to send another packet");
                exit(-1);
            }
            clientData.state = WAIT_REG_RESPONSE;
            changes_client_state("WAIT_REG_RESPONSE");

            /*ddebug mode*/

            if(resetCommunication) {
                clientData.state = WAIT_REG_RESPONSE;
                changes_client_state("WAIT_REG_RESPONSE");
                show_msg("client state changed");
                resetCommunication = false;
                config_direcction_struct_server_UDP();
            }
            sleep(T);
        }
        sleep(U);
    }
    /*message error*/
    printf("The server can't connect\n");
    exit(-1);
}

void treatment_packet_type(UDP packet) {
    unsigned char packet_type = packet.Type;
        
    switch (packet_type)
    {
    case REGISTER_ACK:
        clientData.state = REGISTERED;
        changes_client_state("REGISTERD");
        treatment_packet_ACK(packet);
    case REGISTER_NACK:
        break;
    
    case REGISTER_REJ: 
        clientData.state = DISCONNECTED;
        changes_client_state("DISCONNECTED");
        exit(-1);

    case ERROR:
        treatment_packet_type_error();
    default:
        show_msg("not packet");;
    }
}

void treatment_packet_type_error() {
    show_msg("PACKET RECIVED ERROR");
    exit(-1);
}

void config_direcction_struct_server_UDP() {
    memset(&serverAddrUDP, 0, sizeof(struct sockaddr_in));
    serverAddrUDP.sin_family = AF_INET;
    serverAddrUDP.sin_port = htons(serverData.Server_UDP);
    struct hostent *host = gethostbyname(serverData.Server);
    serverAddrUDP.sin_addr.s_addr = (((struct in_addr*) host->h_addr_list[0])->s_addr);   
}

void save_register_packet_ack_data(UDP received_packet) {
    if (clientData.state != WAIT_REG_RESPONSE) {
        /*error message*/
        printf("client status error\n");
        /*login*/
        return;
    }
    // copy the communication ID
    strcpy(serverData.Id, received_packet.Id_Tans);
    strcpy(serverData.rand_num, received_packet.rand_num);

}


/*BUILD DE PACKET*/

UDP buildREGISTER_REQ() {
    UDP packet;
    packet.Type = REGISTER_REQ;
    strcpy(packet.Id_Tans, clientData.Id);
    strcpy(packet.mac_address, clientData.mac_address);
    strcpy(packet.rand_num,"000000");
    strcpy(packet.Data,"");
    return packet;
}

void changes_client_state(char *state_now) {
    client_state = malloc(sizeof(state_now) + 1);
    strcpy(client_state, state_now);
    char msg[MAX_LINE_LENGTH];
    snprintf(msg,sizeof(msg), "MSG. => Client state changed to: %s\n",client_state);
    show_msg(msg);
}

void show_msg(char *msg) {
    time_t current_time;
    char time_str[100];
    time(&current_time);
    strftime(time_str,sizeof(time_str),"%H:%M:%S",localtime(&current_time));
    printf("%s | %s\n",time_str,msg);
    fflush(stdout);
}

void receive_register_packet_udp() {
    UDP packet;
    socklen_t serverAddrSize = sizeof(serverAddrUDP);
    //long size_received_server;
    
    if((recvfrom(udp_socket,&packet,sizeof(UDP),MSG_WAITALL,
                        (struct sockaddr *) &serverAddrUDP, &serverAddrSize)) < 0) {
        /*message of error*/
        perror("Error in recive th UDP packet of the server");
        exit(-1);
    }
    /*debug mode*/
    if(debugMode) {
        printf("Okay in recived the packet of server\n");
    }

    treatment_packet_type(packet);
}

void treatment_packet_ACK(UDP packet) {
    //if (clientData.state != WAI_)
    strcpy(serverData.rand_num,packet.rand_num);
    strcpy(serverData.Id,packet.Id_Tans);
    strcpy(serverData.mac_addres,packet.mac_address);
    char serverIP[MAX_LINE_LENGTH];
    strcpy(serverIP,inet_ntoa(serverAddrUDP.sin_addr));
    strcpy(serverData.Server, serverIP);
}

/*
int compute_time(int packets_For_Signup) {
    
} 
*/

//  PERIODICAL COMMUNICATION PROCES

