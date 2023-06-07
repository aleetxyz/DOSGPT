#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qdnet.h"
#include "qdcfg.h"
#include "qdmap.h"
#include "qdhttp.h"
#include "qdjson.h"
#include "splash.h"

const int SOCKET_TCPW_BUFSIZ = 16384;
const int SOCKET_SEND_BUFSIZ = 4096;
const int SOCKET_RECV_BUFSIZ = 4096;

//msdos input size and buffer
const int STDIN_INPUT_BUFSIZE = 128;
char inputBuffer[STDIN_INPUT_BUFSIZE];

int main(int argc, char *argv[]) {
  //configuration map struct
  Map map;
  initializeMap(&map);
  //user arguments input
  char* configFilePath = NULL;
  uint8_t showSplash = 0;

  //parse file path argument
  int option;
  while ((option = getopt(argc, argv, "c:s")) != -1) {
    switch (option) {
      case 'c':
        configFilePath = optarg;
        break;
      case 's':
        showSplash = 1;
        break;
      default:
        printf("Usage: %s -c [CONFIG_FILE]\n", argv[0]);
        return 1;
    }
  }

  if(!configFilePath){
    printf("DOSGPT88: usage: %s -c [CONFIG_FILE] -ns\n", argv[0]);
    return 0;
  }

  if (parseConfigFile(configFilePath, &map) < 0) {
    printf("Error opening file: %s\n", configFilePath);
    return 0;
  }

  //check configuration mandatory elements
  const char* mandatoryKeys[] = {"PRXYNAME", "PRXYPORT", "GPTTOKEN", "GPTRPATH", "GPTMODEL", "GPTTEMPR", "RECVTOUT"};
  int configErrs = 0;
  for (size_t i = 0; i < sizeof(mandatoryKeys)/sizeof(mandatoryKeys[0]); i++){
    if(getValue(&map, mandatoryKeys[i]) == NULL){
      printf("Mandatory element %s not found in configuration file.\n", mandatoryKeys[i]);
      configErrs++;
    }
  }

  //if at least one of the mandatory elements is missing, return
  if(configErrs > 0)
    return 0;

  char* hostName = (char*)getValue(&map, "PRXYNAME");
  char* hostPort = (char*)getValue(&map, "PRXYPORT");
  char* hostPath = (char*)getValue(&map, "GPTRPATH");
  IpAddr_t hostAddr;

  //chat gpt request configuration options
  char* gptToken = (char*)getValue(&map, "GPTTOKEN");
  char* gptModel = (char*)getValue(&map, "GPTMODEL");
  char* gptTempr = (char*)getValue(&map, "GPTTEMPR");

  //send and receive data tiemouts
  int16_t recvTout = atoi((char*)getValue(&map, "RECVTOUT"));

  //request body and extracted response json text dynamic pointers
  char* bodyString = NULL;
  char* textOutput = NULL;

  //pointers to response line, response headers, response body
  char* rspLine;
  char* rspHeaders;
  char* rspBody;

  //length of message to be sent
  int16_t msgLen;

  // Allocate memory
  uint8_t* sendBuffer = (uint8_t*)calloc(SOCKET_SEND_BUFSIZ, sizeof(char));
  uint8_t* recvBuffer = (uint8_t*)calloc(SOCKET_RECV_BUFSIZ, sizeof(char));

  if (recvBuffer == NULL || sendBuffer == NULL) {
    fprintf(stderr, "Could not allocate memory\n");
    exit(1);
  }

  if(showSplash == 1)
    printSplash();
  else
    printNormal();

  // Initialize TCP/IP
  if (Utils::parseEnv() != 0) {
    exit(1);
  }

  if (Utils::initStack(1, TCP_SOCKET_RING_SIZE, breakHandler, breakHandler)) {
    fprintf(stderr, "Failed to initialize TCP/IP - exiting\n");
    exit(1);
  }

  // From this point forward you have to call the shutdown( ) routine to
  // exit because we have the timer interrupt hooked.
  if (resolveHost(hostName, hostAddr) < 0) {
    fprintf(stderr, "Cannot resolve hostname\n");
    shutdown(1);
  }

  if (connectSocket(hostAddr, hostPort, SOCKET_TCPW_BUFSIZ) < 0) {
    fprintf(stderr, "Cannot connect to socket\n");
    shutdown(1);
  }

  printf("CONNECTED TO: %s:%s!\n", hostName, hostPort);
  printf("NOTE: Type \"exit\" while prompted to finish your conversation.\n");

  while(1) {
    printf("\nYour message: ");
    fgets(inputBuffer, STDIN_INPUT_BUFSIZE, stdin);

    if(inputBuffer[0] == '\n'){
      continue;
    }

    const char* exitKeyw = "exit";
    if(strncmp(inputBuffer, exitKeyw, strlen(exitKeyw)) == 0){
      printf("Bye!");
      break;
    }
    
    if((strlen(inputBuffer) > 0) && (inputBuffer[strlen(inputBuffer) - 1] == '\n'))
      inputBuffer[strlen(inputBuffer) - 1] = '\0';
  
    if(buildCompletionJson(&bodyString, gptModel, inputBuffer, gptTempr) < 0){
      fprintf(stderr, "Cannot build completion JSON\n");
      break;
    }
    //printf("JSONSEND: %s\n", bodyString);

    if ((msgLen = buildAuthPostRequest(sendBuffer, hostName, hostPath, bodyString, gptToken)) < 0) {
      fprintf(stderr, "Cannot build POST request\n");
      break;
    }

    //printf("SEND: \n%s\n", sendBuffer);
    if(sendMessage(sendBuffer, msgLen) < 0) {
      fprintf(stderr, "Cannot send HTTP request\n");
      break;
    }

    if(recvMessage(recvBuffer, SOCKET_RECV_BUFSIZ, recvTout) < 0) {
      fprintf(stderr, "Cannot recv HTTP request\n");
      break;
    }
    //printf("RECV: \n%s\n", recvBuffer);

    if(extractResponse((char*)recvBuffer, &rspLine, &rspHeaders, &rspBody) < 0) {
      fprintf(stderr, "Cannot extract HTTP CONNECT response\n");
      break;
    }
    //printf("RSPBODY: %s\n", rspBody);

    if(findValueForKeyJson(rspBody, "content", &textOutput) < 0){
      fprintf(stderr, "Cannot parse JSON response\n");
      break;
    }
    printf("\nChatGPT says: %s\n", textOutput);

    if(sock != NULL){
      sock->close();
      TcpSocketMgr::freeSocket(sock);
      sock = NULL;
    }
    
    if (connectSocket(hostAddr, hostPort, SOCKET_TCPW_BUFSIZ) < 0) {
      fprintf(stderr, "Cannot connect to socket\n");
      break;
    }

    //clear dynamic responses
    free(rspLine);
    free(rspHeaders);
    free(rspBody);

    //clear dynamic strings
    free(bodyString);
    free(textOutput);
  }

  if(rspLine != NULL)
    free(rspLine);

  if(rspHeaders != NULL)
    free(rspHeaders);
  
  if(rspBody != NULL)
    free(rspBody);

  if(bodyString != NULL)
    free(bodyString);

  if(textOutput != NULL)
    free(textOutput);

  releaseSocket();
  shutdown(0);
}