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

uint64_t total = 1;

#define mem_block_size 4
int recognize(char* buff, uint32_t* pos, struct Server* to)
{
	char* temp = buff;
	uint32_t offset = 0;
	for(int j = 0; j < 3; j++)
	for(int i = 0; i < 4; i++)
	{
		if((*(temp + offset) >= '0') && (*(temp + offset) <= '9'))
		{
			offset++;
			continue;
		}
		if(*(temp + offset) == '.')
		{
			offset++;
			break;
		}
		return 1;
		
	}
	for(int i = 0; i < 4; i++)
	{
		if((*(temp + offset) >= '0') && (*(temp + offset) <= '9'))
		{
			offset++;
			continue;
		}
		if(*(temp + offset) == ':')	break;
		return 1;
	}
	memcpy(&to->ip, temp, offset);
	to->ip[offset] = '\0';
	offset++;
	temp += offset;
	*pos += offset;
	offset = 0;
	while((*(temp + offset) >= '0') && (*(temp + offset) <= '9'))
		offset++;
	if((*(temp + offset) != '\n') && (*(temp + offset) != ' '))
		return 1;
	temp[offset] = '\0';
	to->port = atoi(temp);
	*pos += offset;
	return 0;
}

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
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
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
	if(k == 0)
		k = -1;
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
		
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
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

  // TODO: for one server here, rewrite with servers from file
  // TODO: delete this and parallel work between servers
	FILE* f = fopen(servers, "rb");
	if(!f)
	{
		printf("server file error!\n");
		return 1;
	}
  	struct Server *to = malloc(sizeof(struct Server) * mem_block_size);
	uint32_t pos = 0;
	char buff[24];
	uint32_t mem_serv_counter = 1;
	uint32_t nserv = 0;
	while(!feof(f) || (nserv > k))
	{
		if(nserv == (mem_block_size*mem_serv_counter))
		{
			mem_serv_counter++;
			to = (struct Server *) realloc(to, mem_block_size*mem_serv_counter*sizeof(struct Server));
		}
		fseek(f, pos, SEEK_SET);
		if(fread(&buff, 1, 24, f) == 0)
		{
			printf("rw file error!\n");
			fclose(f);
			return 1;
		}
		if(recognize(buff, &pos, to + nserv))
		{
			fclose(f);
			printf("file format error!\n");
			return 1;
		}
		pos++;
		nserv++;
	}
	fclose(f);
	to = (struct Server *) realloc(to, nserv*sizeof(struct Server));

  // TODO: work continiously, rewrite to make parallel
  for (int i = 0; i < nserv; i++) {
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
    uint64_t begin = i*k/nserv + 1;
    uint64_t end = (i == (nserv-1)) ? k + 1 : (i+1)*k/nserv + 1;

    memcpy(buff, &begin, sizeof(uint64_t));
    memcpy(buff + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(buff + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, buff, 3 * sizeof(uint64_t), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

    uint64_t answer = 0;
    if (recv(sck, (char*)&answer, sizeof(answer), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }

    // TODO: from one server
    // unite results
    //memcpy(&answer, response, sizeof(uint64_t));
	total = MultModulo(total, answer, mod);
    close(sck);
  }
    printf("answer: %llu\n", total);
  free(to);
	remove(servers);

  return 0;
}