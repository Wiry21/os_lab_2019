#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "multmod.h"

  const int pidnum = 5;
//  pid_t child_pid[5];
  pid_t child_pid;
 // int f_pipes[5][2];
  
  int thisNum = 0;
  int tnum = -1;
  pid_t corepid = 0;
//  bool freepids[5]; 
  bool Cikl = true;
  int pidCount = 3;
  int client_fd = 0;

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
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
uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  // TODO: your code here
  //+
  for(uint64_t i = (args->begin); i < (args->end); i++)
    ans=ans*(i%args->mod);
   printf("thread ans = %d\n", ans);
  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  printf("thread server!!!\n");
  return (void *)(uint64_t *)Factorial(fargs);
}

void ChildFree(int signo)
{
    pidCount++;
}


void ExecuteSignal(int signo)
{
    printf("Execute   %d\n", thisNum);
    int soc = client_fd;    
//    read(f_pipes[thisNum][0], &soc, sizeof(int));

    bool flagStop=true;
    while (flagStop) {
      //unsigned int buffer_size = sizeof(uint64_t) * 3;
      unsigned int buffer_size = sizeof(uint64_t) * 4;
      char from_client[buffer_size];
      //читаем вход из сокета client_fd клиента в from_client размером с buffer_size
      //вернет n байт сколько считали
       printf("Soc   %d\n", soc);
      int read = recv(soc, from_client, buffer_size, 0);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];
//разбивка всего от клиента
      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      uint64_t flag_fork = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));
      //
      memcpy(&flag_fork, from_client + 3 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu %llu\n", begin, end, mod, flag_fork);
      
      
      
//fork bil zdes'
  
  
          struct FactorialArgs args[tnum];
          //вычисляем на сервере
          int delta = (end+1)/tnum;
          for (uint32_t i = 0; i < tnum; i++) {
            // TODO: parallel somehow
            //мой код
            args[i].mod=mod;
            args[i].begin= begin + i*delta;
            if(args[i].begin==0)
               args[i].begin=1;
            if(i<(tnum-1)){
                args[i].end=(i+1)*delta+1;
            }
            else{
                args[i].end=end+1;
            }
            //код препода
            //args[i].begin = 1;
            //args[i].end = 1;
            //args[i].mod = mod;
    
            if (pthread_create(&threads[i], NULL, ThreadFactorial,
                               (void *)&args[i]) != 0) {
              printf("Error: pthread_create failed!\n");
              return 1;
            }
          }
    //принимаем результат
          uint64_t total = 1;
          for (uint32_t i = 0; i < tnum; i++) {
            uint64_t result = 0;
            pthread_join(threads[i], (void **)&result);
            total = MultModulo(total, result, mod);
          }
    
          printf("Total: %llu\n", total);
    
          char buffer[sizeof(total)];
          memcpy(buffer, &total, sizeof(total));
          //отправляем результат клиенту
          send(soc, buffer, sizeof(total), 0);

          //конец сервера       
          flagStop=false;
          printf("child end \n"); 
    }
    //уведомляем об окончании, усыпить клиента
    shutdown(soc, SHUT_RDWR);
    //закрываемся
    close(soc);
kill(corepid,SIGUSR1);
    Cikl=false;
}


int main(int argc, char **argv) {

signal(SIGUSR1,ChildFree);
  int port = -1;
  corepid = getpid();
 
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        // TODO: your code here
        break;
      case 1:
        tnum = atoi(optarg);
        // TODO: your code here
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }
//создаем сокет, получаем его id
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

//устанавливаем флаг игнора TIME_WAIT состояние сервера (для соединения, что бы не получить ошибку)
  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
//привязываем адрес к сокету
  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }
//слушаем сокет, 128 = максимальная длина очереди
  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);
  bool flagEnd=true;
  bool ListenFlag = true;
  
  // fork
/*    
  for (int i=0; i<pidnum; i++) 
  {
        pipe(f_pipes[i]);
  }
 */
   bool flag_fork_end=true;

          //делаем форк
          flag_fork_end=false;
                  /*
           
           for (int i=0;i<pidnum;i++)
           {
               freepids[i] = true;
               child_pid[i] = fork();
               if(child_pid[i] < 0){
                    printf("Error: fork failed!\n");
                    return -1;
               }
               if(child_pid[i] == 0){
                   flag_fork_end = true;
                   ListenFlag = false;
                   thisNum = i;
                   signal(SIGUSR1,ExecuteSignal);
                   sigset_t set; 
                   sigemptyset(&set);
                   sigaddset(&set, SIGUSR1);
                   int sigw=0;
                   printf("Child wait   %d\n", thisNum);
              while (true)
                   {
                       printf("Child wait   %d\n", thisNum);
             //  sigwait(&set,&sigw);
              sleep(10);
                   printf("Child dont wait   %d\n", thisNum);
                  }
     
                   break;
               }
       }
  */
  /*
  for(int i=0; i<pidnum;i++)
  {
       freepids[i] = true;
       
  }
  */
//цикл прослушки
    if (ListenFlag == true)
    {
        printf("Listening   %d\n", port);
        flagEnd = true;
    while (flagEnd) {
        printf("Listening  2 %d\n", port);
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    //берем из очереди с адресом другой стороны и принимаем сокет клиента
     client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);
printf("Accept   %d\n", client_fd);
        if (client_fd < 0) {
          fprintf(stderr, "Could not establish new connection\n");
          continue;
        }
 //       int j=0;
 /*       for (j=0;j<pidnum-1;j++)
        {
             if (freepids[j]==true)
              break;
              
        }
        */
//if(pidCount == 0) бывают ошибки
if (false)
{
        //уведомляем об окончании, усыпить клиента
    shutdown(client_fd, SHUT_RDWR);
    //закрываемся
    close(client_fd);

}
else {
  //      freepids[j] = false;
  pidCount--;
        child_pid=fork();
        if(child_pid == 0){
                   flag_fork_end = true;
                   ListenFlag = false;
                   thisNum = 0;
                   signal(SIGUSR1,ExecuteSignal);
                   sigset_t set; 
                   sigemptyset(&set);
                   sigaddset(&set, SIGUSR1);
                   int sigw = 0;
                   printf("Child wait   %d\n", thisNum);
              while (Cikl)
                   {
                       printf("Child wait   %d\n", thisNum);
             //  sigwait(&set,&sigw);
              sleep(10);
                   printf("Child dont wait   %d\n", thisNum);
                  }   
               }
               else
               {
               sleep(2);
             //       write(f_pipes[j][1], &client_fd, sizeof(int));
               //     printf("Kill to    %d\n", j);
                    kill(child_pid,SIGUSR1);    
            }

    }

     }
    }
printf("server close! \n");
  return 0;
}