#include <stdio.h> /* this should have 'void perror(const char *s);' */
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include "srtp.h"
#include "srtp-packet.h"
#include "srtp-common.h"
#include "srtp-fsm.h"
#include "srtp-pcb.h"
#include "byteorder64.h"
#include "d_print.h"

/* RTO for re-tranimission*/
#define MAX_SEQ_NUM 100
#define RETRANS_TIMEOUT_SEC 0
#define RETRANS_TIMEOUT_USEC 500000

extern Srtp_Pcb_t G_pcb;
extern const char *G_state_str[];

static void print_error(const char *function_name, int error_number);

void srtp_initialise(){ 
  printf("Initialising end-point\n");
}

int srtp_start(uint16_t port){
  Srtp_Pcb_reset(); // when socket set-up for listening

  if ((G_pcb.sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {   //estblishesh a UDP connection using IPv4
    perror("failed to establish a UDP connection from the server\n");
    print_error("socket", errno);   //error value if connection fails 
  }

  /*setting local endpoint address and port number*/
  G_pcb.local.sin_family = AF_INET;
  G_pcb.local.sin_port = htons(port);   //converting the port number to the network byte order
  G_pcb.local.sin_addr.s_addr = htonl(INADDR_ANY);   //converting the 'any' address value to the network byte order
  G_pcb.port = port;   //sets the port number for the server

  if (bind(G_pcb.sd, (struct sockaddr *) &G_pcb.local, sizeof(G_pcb.local)) < 0) {   //binds the socket descriptor and address
    print_error("bind", errno);
    close(G_pcb.sd);
    return SRTP_ERROR_api;    //error value if binding fails 
  }

  G_pcb.state = SRTP_STATE_listen;   //sets the current state in the PCB to listening
  G_pcb.start_time = srtp_timestamp();   //sets the time when the server starts listening

  return G_pcb.sd;  //returns the socket descriptor
}

int srtp_accept(int sd){

  struct Srtp_Packet_s *pkt = (struct Srtp_Packet_s *) malloc(sizeof(struct Srtp_Packet_s));   //packet struct
  struct Srtp_Header_s *header = (struct Srtp_Header_s *) malloc(sizeof(struct Srtp_Header_s));   //header struct
  size_t stream_size = sizeof(Srtp_Header_t) + sizeof(uint16_t) + SRTP_MAX_PAYLOAD_SIZE;   //size of the byte stream
  uint8_t *bytes_to = malloc(stream_size);   //byte stream to send
  uint8_t *bytes_from = malloc(stream_size);  //byte stream to receive
  uint8_t seq = rand() % 100;   //acknowledgement number 
  int sent;   //return value of sendto()
  int recv;   //return value of recvfrom()
  socklen_t size = sizeof(G_pcb.remote);

  while(1){

    recv = recvfrom(G_pcb.sd, (void *) bytes_from, stream_size, 0, (struct sockaddr *) &G_pcb.remote, &size);
    if (recv < 0) {   //gets the ope_req sent from the client
      perror("failed to receive packet from server");
      return SRTP_ERROR_api;
    }

    /* gets the client address and stores it in the PCB packet */
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &G_pcb.remote.sin_addr, client_ip, INET_ADDRSTRLEN);

    G_pcb.start_time = srtp_timestamp(); // When open_req received.

    /* Making sure the byte streams are set to 0 */
    (void) memset(bytes_to, 0, stream_size);  

    Srtp_Header_t ph;
    memcpy(&ph, bytes_from, sizeof(Srtp_Header_t));   //unpacks the header of the packet

    uint16_t ps;
    memcpy(&ps, bytes_from + sizeof(Srtp_Header_t), sizeof(uint16_t));   //unpacks the paylaod size

    void* p = malloc(ntohs(ps));
    memset(p, 0, ntohs(ps));
    memcpy(p, bytes_from + sizeof(Srtp_Header_t) + sizeof(uint16_t), ntohs(ps));   //unpacks the payload of the packet
    u_int8_t *payload = (uint8_t *) p;


    if(ph.type == SRTP_TYPE_open_req){
      /* setting up the payload of the TCP packet */
      header->type = SRTP_TYPE_open_ack;   //setting the type of the packet to open_ack
      header->seq_num = seq;
      header->ack_num = ph.seq_num + 1;   //takes the sequence number of the client and increments it

      /* Setting up the payload of the packet*/
      pkt->header = header;

      pkt->payload = malloc(SRTP_MAX_PAYLOAD_SIZE);
      uint8_t *payload = (uint8_t *)pkt->payload;

      for(int i = 0; i < SRTP_MAX_PAYLOAD_SIZE; ++i){ 
        payload[i] = i;   //adding values to the payload
      } 

      pkt->payload_size = htons(SRTP_MAX_PAYLOAD_SIZE);    //setting the max size of the payload

      /* Converting the packet struct to a byte stream to send to the server */
      memcpy(bytes_to, pkt->header, sizeof(Srtp_Header_t));   //overlaying the header to the byte stream variable first
      uint16_t *payload_size_ptr = (uint16_t*)(bytes_to + sizeof(Srtp_Header_t));   //storing the starting point of the payload in the byte stream
      *payload_size_ptr = pkt->payload_size;
      memcpy(bytes_to + sizeof(Srtp_Header_t) + sizeof(uint16_t), pkt->payload, pkt->payload_size);   //then overlaying the payload to the byte stream variable
      
      sent = sendto(G_pcb.sd, (void *) bytes_to, stream_size, 0, (struct sockaddr *) &G_pcb.remote, size); //sends an open_ack packet to the client

      /* Checks that there are not errors in sending the packet */
      if(sent < 0){   
          perror("sendto() error");
          return SRTP_ERROR_fsm;
      }

      G_pcb.state = SRTP_STATE_open_ack; //sets the current state to sent acknowledgement
      break;
    }
    else{
      perror("Incorrect packet received");
      return SRTP_ERROR_fsm;
    }
  }

  ++G_pcb.open_req_rx;
  ++G_pcb.open_ack_tx;

  free(header);
  free(pkt);
  free(bytes_from);
  free(bytes_to);

  return sd;
}

int srtp_open(const char *fqdn, uint16_t port){
    
  Srtp_Pcb_reset(); // when socket set-up for opening

  struct Srtp_Packet_s *pkt = (struct Srtp_Packet_s *) malloc(sizeof(struct Srtp_Packet_s));   //packet struct
  struct Srtp_Header_s *header = (struct Srtp_Header_s *) malloc(sizeof(struct Srtp_Header_s));   //header struct
  size_t stream_size = sizeof(Srtp_Header_t) + sizeof(uint16_t) + SRTP_MAX_PAYLOAD_SIZE;   //size of the byte stream
  uint8_t *bytes_to = malloc(stream_size);   //byte stream to send
  uint8_t *bytes_from = malloc(stream_size);  //byte stream to receive
  uint8_t ack = rand() % 100;   //sequence number
  uint8_t seq = rand() % 100;   //acknowledgement number 
  struct hostent *he;  
  struct in_addr **addr_list;  
  int sent;   //return value of sendto()
  int recv;   //return value of recvfrom()
  int reply;  //return value of select()
  socklen_t size =  sizeof(G_pcb.remote);

  he = gethostbyname(fqdn);   //resolves the FQDN into IPv4
  
  if(he == NULL){   //gets the IP address of the server
      perror("couldn't resolve hostname\n");
      return SRTP_ERROR;   //error value if it fails
  }
  
  if ((G_pcb.sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {   //estblishesh a UDP connection using IPv4
      perror("failed to establish a UDP connection from the server\n");
      print_error("socket", errno);   //error value if connection fails 
  }
  
  /*setting local endpoint address and port number*/
  G_pcb.local.sin_family = AF_INET;
  G_pcb.local.sin_port = htons(port);   //converting the port number to the network byte order
  G_pcb.local.sin_addr.s_addr = htonl(INADDR_ANY);   //converting the 'any' address value to the network byte order
  G_pcb.port = port;   //sets the port number for the client
  
  if (bind(G_pcb.sd, (struct sockaddr *) &G_pcb.local, sizeof(G_pcb.local)) < 0) {   //binds the socket descriptor and address
      print_error("bind", errno);
      close(G_pcb.sd);
      return SRTP_ERROR_api;    //error value if binding fails 
  }

  /* setting remote endpoint address and port number */
  G_pcb.remote.sin_family = AF_INET;
  G_pcb.remote.sin_port = htons(port);   //converting the port number to the network byte order

  addr_list = (struct in_addr **) he->h_addr_list;

  if(addr_list[0] == NULL){   //gets the first ip address in the list which corresponds to the FQDN of the server
    printf("No IP addresses found for %s\n", he->h_name);
    return SRTP_ERROR;
  }

  if(inet_pton(AF_INET, inet_ntoa(*addr_list[0]), &(G_pcb.remote.sin_addr)) <= 0){   //stores the address of the server in the PCB packet
    printf("Invalid IP address format\n");
    return SRTP_ERROR;
  }

  /* Making sure the byte streams are set to 0 */
  (void) memset(bytes_to, 0, stream_size);  
  (void) memset(bytes_from, 0, stream_size);  

  /* setting up the payload of the TCP packet */
  header->type = SRTP_TYPE_open_req;   //setting the type of the packet to open_ack
  header->seq_num = seq;
  header->ack_num = ack;

  /* Setting up the payload of the packet*/
  pkt->header = header;

  pkt->payload = malloc(SRTP_MAX_PAYLOAD_SIZE);
  uint8_t *payload = (uint8_t *)pkt->payload;
  
  for(int i = 0; i < SRTP_MAX_PAYLOAD_SIZE; ++i){ 
    payload[i] = i;   //adding values to the payload
  } 

  pkt->payload_size = htons(SRTP_MAX_PAYLOAD_SIZE);   //setting the max size of the payload

  /* Converting the packet struct to a byte stream to send to the server */
  memcpy(bytes_to, header, sizeof(Srtp_Header_t));   //overlaying the header to the byte stream variable first
  uint16_t *payload_size_ptr = (uint16_t*)(bytes_to + sizeof(Srtp_Header_t));   //storing the starting point of the payload in the byte stream
  *payload_size_ptr = pkt->payload_size;
  memcpy(bytes_to + sizeof(Srtp_Header_t) + sizeof(uint16_t), pkt->payload, pkt->payload_size);   //then overlaying the payload to the byte stream variable
  
  /* Set up loop to wait for re-transmission if RTO is exceded */
  for(int counter = 0; counter < SRTP_MAX_RETX; counter++) {
    sent = sendto(G_pcb.sd, (void *) bytes_to, stream_size, 0, (struct sockaddr *) &G_pcb.remote, size); //sends an open_req packet to the server

    G_pcb.state = SRTP_STATE_open_req;  //sets the current state to sent request
    G_pcb.start_time = srtp_timestamp();  // When open_req transmitted.

    /* Checks that there are not errors in sending the packet */
    if(sent < 0){   
      perror("sendto() error");
      return SRTP_ERROR_api;
    }

    /* set-up for select() */
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(G_pcb.sd, &read_fds);

    /* sets the time interval before re-transmission */
    struct timeval timeout;
    timeout.tv_sec = RETRANS_TIMEOUT_SEC;
    timeout.tv_usec = RETRANS_TIMEOUT_USEC; 

    reply = select(G_pcb.sd + 1, &read_fds, NULL, NULL, &timeout);  //checks the sd every 500ms for activity

    if(reply == -1) {   //if an error occured with select()
      perror("select() failed");
      return SRTP_ERROR_api;
    } else if(reply == 0) {    //if there wasn't any reply from the server after 500ms
      printf("timeout, retransmitting...\n");
      continue;
    } else {   // if the server responds in time
      recv = recvfrom(G_pcb.sd, (void *) bytes_from, stream_size, 0, (struct sockaddr *) &G_pcb.remote, &size);
      if (recv < 0) {   //gets opne_ack sent from the server
        perror("failed to receive packet from server");
        return SRTP_ERROR_api;
      }
      
      break;
    }
  }

  /* generate error if no response packets are received */
  if(reply <= 0){
    perror("Failed to get an acknowledgement packet");
    return SRTP_ERROR_protocol;
  }

  Srtp_Header_t ph;
  memcpy(&ph, bytes_from, sizeof(Srtp_Header_t));   //unpacks the header of the packet
  
  uint16_t ps;
  memcpy(&ps, bytes_from + sizeof(Srtp_Header_t), sizeof(uint16_t));   //unpacks the paylaod size

  void* p = malloc(ps);
  memcpy(p, bytes_from + sizeof(Srtp_Header_t) + sizeof(uint16_t), ps);   //unpacks the payload of the packet

  if((ph.type != SRTP_TYPE_open_ack) || (ph.ack_num != seq + 1)){
    perror("Incorrect packet received");
    return SRTP_ERROR_fsm;
  }

  G_pcb.state = SRTP_STATE_connected;   //changes current state to connected  
  ++G_pcb.open_req_tx;
  ++G_pcb.open_ack_rx;

  /* freeing up memory */
  free(header);
  free(pkt);
  free(bytes_from);
  free(bytes_to);

  return G_pcb.sd; 
}

int srtp_tx(int sd, void *data, uint16_t data_size){

  struct Srtp_Packet_s *pkt = (struct Srtp_Packet_s *) malloc(sizeof(struct Srtp_Packet_s));   //packet struct
  struct Srtp_Header_s *header = (struct Srtp_Header_s *) malloc(sizeof(struct Srtp_Header_s));   //header struct
  size_t stream_size = sizeof(Srtp_Header_t) + sizeof(uint16_t) + data_size;   //size of the byte stream
  uint8_t *bytes_to = malloc(stream_size);   //byte stream to send
  uint8_t *bytes_from = malloc(stream_size);  //byte stream to receive
  uint8_t ack = rand() % 100;   //sequence number
  uint8_t seq = rand() % 100;   //acknowledgement number  
  int sent;   //return value of sendto()
  int reply;  //return value of select()
  socklen_t size =  sizeof(G_pcb.remote);

  /* Making sure the byte streams are set to 0 */
  (void) memset(bytes_to, 0, stream_size);  
  (void) memset(bytes_from, 0, stream_size);  

  /* setting up the payload of the TCP packet */
  header->type = SRTP_TYPE_data_req;   //setting the type of the packet to data_req
  header->seq_num = seq;
  header->ack_num = ack;

  /* Setting up the payload of the packet*/
  pkt->header = header; 
  pkt->payload = data;   //assigning the value to send
  pkt->payload_size = htons(data_size);   //assigning the max size of the payload
  uint8_t *array_to = (uint8_t *) pkt->payload;

  /* Converting the packet struct to a byte stream to send to the server */
  memcpy(bytes_to, header, sizeof(Srtp_Header_t));   //overlaying the header to the byte stream variable first
  uint16_t *payload_size_ptr = (uint16_t*)(bytes_to + sizeof(Srtp_Header_t));   //storing the starting point of the payload in the byte stream
  *payload_size_ptr = pkt->payload_size;
  memcpy(bytes_to + sizeof(Srtp_Header_t) + sizeof(uint16_t), pkt->payload, pkt->payload_size);   //then overlaying the payload to the byte stream variable
  
  /* Set up loop to wait for re-transmission if RTO is exceded */
  for(int counter = 0; counter < SRTP_MAX_RETX; counter++) {
    sent = sendto(G_pcb.sd, (void *) bytes_to, stream_size, 0, (struct sockaddr *) &G_pcb.remote, size); //sends an open_req packet to the server

    G_pcb.state = SRTP_STATE_open_req;  //sets the current state to sent request
    G_pcb.start_time = srtp_timestamp();  // When data_req transmitted

    /* Checks that there are not errors in sending the packet */
    if(sent < 0){   
      perror("sendto() error");
      return SRTP_ERROR_api;
    }

    /* set-up for select() */
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(G_pcb.sd, &read_fds);

    /* sets the time interval before re-transmission */
    struct timeval timeout;
    timeout.tv_sec = RETRANS_TIMEOUT_SEC;
    timeout.tv_usec = RETRANS_TIMEOUT_USEC; 

    reply = select(G_pcb.sd + 1, &read_fds, NULL, NULL, &timeout);  //checks the sd every 500ms for activity

    if(reply == -1) {   //if an error occured with select()
      perror("select() failed");
      return SRTP_ERROR_api;
    } else if(reply == 0) {    //if there wasn't any reply from the server after 500ms
      printf("timeout, retransmitting...\n");
      continue;
    } else {   // if the server responds in time
      if (recvfrom(G_pcb.sd, (void *) bytes_from, stream_size, 0, (struct sockaddr *) &G_pcb.remote, &size) < 0) {   //gets opne_ack sent from the server
        perror("failed to receive packet from server");
        return SRTP_ERROR_api;
      }
      
      break;
    }
  }

  /* generate error if no response packets are received */
  if(reply <= 0){
    perror("Failed to get an acknowledgement packet");
    return SRTP_ERROR_protocol;
  }

  Srtp_Header_t ph;
  memcpy(&ph, bytes_from, sizeof(Srtp_Header_t));   //unpacks the header of the packet

  uint16_t ps;
  memcpy(&ps, bytes_from + sizeof(Srtp_Header_t), sizeof(uint16_t));   //unpacks the paylaod size

  void* p = malloc(ntohs(ps));
  memset(p, 0, ntohs(ps));
  memcpy(p, bytes_from + sizeof(Srtp_Header_t) + sizeof(uint16_t), ntohs(ps));   //unpacks the payload of the packet
  uint8_t *array_from = (uint8_t *) p;

  if((ph.type != SRTP_TYPE_data_ack) || (ph.ack_num != seq + 1)){
    perror("Incorrect packet received");
    return SRTP_ERROR_fsm;
  }

  for(int counter = 0; counter < data_size; counter++){

  }

  int b = sent - (sizeof(Srtp_Header_t) + sizeof(uint16_t));  // takes the return value of sendto() and subtracts the size of the header and payload_size field from it
  ++G_pcb.data_req_tx;
  G_pcb.data_req_bytes_tx += b;
  ++G_pcb.data_ack_rx;

  /* freeing up memory */
  free(header);
  free(pkt);
  free(bytes_from);
  free(bytes_to);

  return b;
}

int srtp_rx(int sd, void *data, uint16_t data_size){

  struct Srtp_Packet_s *pkt = (struct Srtp_Packet_s *) malloc(sizeof(struct Srtp_Packet_s));   //packet struct
  struct Srtp_Header_s *header = (struct Srtp_Header_s *) malloc(sizeof(struct Srtp_Header_s));   //header struct
  size_t stream_size = sizeof(Srtp_Header_t) + sizeof(uint16_t) + data_size;   //size of the byte stream
  uint8_t *bytes_to = malloc(stream_size);   //byte stream to send
  uint8_t *bytes_from = malloc(stream_size);  //byte stream to receive
  uint8_t seq = rand() % 100;   //acknowledgement number 
  int sent;   //return value of sendto()
  int recv;  //return value of recvfrom()
  socklen_t size = sizeof(G_pcb.remote);

  while(1){

    recv = recvfrom(G_pcb.sd, (void *) bytes_from, stream_size, 0, (struct sockaddr *) &G_pcb.remote, &size);
    if (recv < 0) {   //gets the ope_req sent from the client
      perror("failed to receive packet from server");
      return SRTP_ERROR_api;
    }

    G_pcb.start_time = srtp_timestamp(); // When data_req received

    /* Making sure the byte streams are set to 0 */
    (void) memset(bytes_to, 0, stream_size);  

    Srtp_Header_t ph;
    memcpy(&ph, bytes_from, sizeof(Srtp_Header_t));   //unpacks the header of the packet

    uint16_t ps;
    memcpy(&ps, bytes_from + sizeof(Srtp_Header_t), sizeof(uint16_t));   //unpacks the paylaod size

    void* p = malloc(ntohs(ps));
    memset(p, 0, ntohs(ps));
    memcpy(p, bytes_from + sizeof(Srtp_Header_t) + sizeof(uint16_t), ntohs(ps));   //unpacks the payload of the packet
    uint8_t *array_from = (uint8_t *) p;

    for(int counter = 0; counter < data_size; counter++){
      memcpy(&data[counter], &p[counter], sizeof(uint8_t));
    }

    if(ph.type == SRTP_TYPE_data_req){
      /* setting up the payload of the TCP packet */
      header->type = SRTP_TYPE_data_ack;   //setting the type of the packet to data_ack
      header->seq_num = seq;
      header->ack_num = ph.seq_num + 1;   //takes the sequence number of the client and increments it

      /* Setting up the payload of the packet*/
      pkt->header = header;
      pkt->payload = data;
      pkt->payload_size = htons(data_size);    //assigning the max size of the payload

      /* Converting the packet struct to a byte stream to send to the server */
      memcpy(bytes_to, pkt->header, sizeof(Srtp_Header_t));   //overlaying the header to the byte stream variable first
      uint16_t *payload_size_ptr = (uint16_t*)(bytes_to + sizeof(Srtp_Header_t));   //storing the starting point of the payload in the byte stream
      *payload_size_ptr = pkt->payload_size;
      memcpy(bytes_to + sizeof(Srtp_Header_t) + sizeof(uint16_t), pkt->payload, pkt->payload_size);   //then overlaying the payload to the byte stream variable
      
      sent = sendto(G_pcb.sd, (void *) bytes_to, stream_size, 0, (struct sockaddr *) &G_pcb.remote, size); //sends an open_ack packet to the client
      uint8_t *array_to = (uint8_t *) pkt->payload;

      /* Checks that there are not errors in sending the packet */
      if(sent < 0){   
          perror("sendto() error");
          return SRTP_ERROR_fsm;
      }

      break;
    }
    else{
      perror("Incorrect packet received");
      return SRTP_ERROR_fsm;
    }
  }

  int b = recv - (sizeof(Srtp_Header_t) + sizeof(uint16_t)); // takes the return value of recvfrom() and subtracts the size of the header and payload_size field from it
  ++G_pcb.data_req_rx;
  G_pcb.data_req_bytes_rx += b;
  ++G_pcb.data_ack_tx;

  /* freeing up memory */
  free(header);
  free(pkt);
  free(bytes_from);
  free(bytes_to);

  return b;
}

int srtp_close(int sd){
  
  G_pcb.finish_time = srtp_timestamp(); // When open_ack transmitted.
  ++G_pcb.close_req_rx;
  ++G_pcb.close_ack_tx;

  return SRTP_SUCCESS;
}

static void print_error(const char *function_name, int error_number) {
  fprintf(stderr, "%s failed: %s\n", function_name, strerror(error_number));
}