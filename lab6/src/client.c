#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "multmod.h"

struct Server {
  char ip[255];
  int port;
};

/*
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }
  return result % mod;
}
*/

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  uint64_t fork_flag = 0;
  char servers[255] = {'\0'}; // TODO: explain why 255 = т.к. блок ip = 1 - 255, т.е. у нас всего 255 в локальной сети адресов

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {"fork_flag", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        if(k<0){
            printf("error k must be > 0 !\n");
            return 1;
        }
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        if(mod<0){
            printf("error mod must be > 0 !\n");
            return 1;
        }
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        break;
      case 3:
        ConvertStringToUI64(optarg, &fork_flag);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  //количество серверов
  unsigned int servers_num = 0;
  for(int i=0;i<255;i++){
      if(servers[i] == ':')
        servers_num++;
  }
  printf("servers_num %d \n", servers_num);
  // TODO: for one server here, rewrite with servers from file
  //unsigned int servers_num = 1;
  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  //разбитие файла servers
  char * sPoint1=servers;
  char * sPoint2=servers;
  for(int i=0; i<servers_num; i++){
      for(int j=0;j<255;j++){
          if(sPoint1[j] == ':'){
              sPoint2=&(sPoint1[j]);
              break;
          }
          if(sPoint1[j] == '\0'){
              printf("error servers ; or : \n");
              return -1;
          }
      }
      memcpy(to[i].ip, sPoint1, sizeof(char)*(sPoint2-sPoint1));
      sPoint1=(sPoint2+1);//след после :
      for(int j=0;j<255;j++){
          if(sPoint1[j] == ';'){
              sPoint2=&(sPoint1[j]);
              break;
          }
          if(sPoint1[j] == '\0'){
              printf("error servers ; or : \n");
              return -1;
          }
      }
      *(sPoint2)='\0';
      to[i].port = atoi(sPoint1);
      sPoint1=sPoint2+1;
  }
  // TODO: delete this and parallel work between servers
  //to[0].port = 20001;
  //memcpy(to[0].ip, "127.0.0.1", sizeof("127.0.0.1"));

  // TODO: work continiously, rewrite to make parallel
  int delta_serv=(k+1)/servers_num;
  int * sokets = malloc(sizeof(int) * servers_num);
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    sokets[i] = sck;
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // TODO: for one server
    // parallel between servers
    //uint64_t begin = 1;
    //uint64_t end = k;
    //delta_serv
    uint64_t begin = i*delta_serv;
    if(begin==0)
        begin=1;
    uint64_t end = k;
    if(i<(servers_num-1)){
        end=(i+1)*delta_serv;
    }
    else{
        end=k;
    }

    //char task[sizeof(uint64_t) * 3];
    char task[sizeof(uint64_t) * 4];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));
    memcpy(task + 3 * sizeof(uint64_t), &fork_flag, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

//ожидание ответа
    /*
    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }
    // TODO: from one server
    // unite results
    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    printf("answer: %llu\n", answer);
    close(sck);
    */
  }
  
  
  uint64_t itog = 1;
  for(int i=0; i<servers_num; i++){
    char response[sizeof(uint64_t)];
    if (recv(sokets[i], response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }
    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    printf("answer: %llu\n", answer);
    itog = MultModulo(itog, answer, mod);
    close(sokets[i]);
  }
  
  printf("itog: %llu\n", itog);
  free(to);
  free(sokets);
  return 0;
}