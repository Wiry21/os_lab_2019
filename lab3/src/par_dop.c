#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"


int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  
  int delta = array_size/pnum;
  //
  
char str[256];
int procCount = 0;
FILE *fp;

if( (fp = fopen("/proc/cpuinfo", "r")) )
{
  while(fgets(str, sizeof str, fp))
  if( !memcmp(str, "processor", 9) ) procCount++;
}

if ( !procCount ) 
{ 
printf("Unable to get proc count. Defaulting to 2");
procCount=2;
}

printf("Proc Count:%d\n", procCount);
      

  int ncpus ;
    ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("cpus: %d\n", ncpus);

if ( pnum>ncpus*4 ) 
{ 
pnum=ncpus*4;
}

  
  int f_pipes[pnum/4][2];
  for (int i=0; i<pnum/4+1; i++) 
  {
        pipe(f_pipes[i]);
  }

  /*
  int *pip_mass=malloc(sizeof(int) * pnum*2);
  for(int i=0; i<pnum*2; i+=2){
      pipe((pip_mass+i));
  }
  */
  /*
  if((pipe(f_pipes) != 0) && (with_files == false)){
      printf("Pipe failed!\n");
      return 1;
  }
  */
  //
  
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        struct MinMax ret = GetMinMax(array, i*delta, (i+1)*delta);
        // parallel somehow

        if (with_files) {
          // use files here
          FILE *fp;
          //std::string filename = ".\\paral_itog_" + std::to_string(i) + ".txt";
          char filename[255] = "paral_itog_";
          char numb[255] = "";
          //itoa(i, numb, 10);
          sprintf(numb, "%d", i);
          strcat(filename, numb);
          strcat(filename, ".txt");
          fp = fopen(filename, "wb");
          fwrite(&ret, sizeof(struct MinMax), 1, fp);
          fclose(fp);
        } else {
          // use pipe here
//          write(f_pipes[1], &ret, sizeof(struct MinMax));
          //write((pip_mass+i*2)[1], &ret, sizeof(struct MinMax));
          for ( int i=0; i<pnum/4+1; i++)
          {
              write(f_pipes[i][1], &ret, sizeof(struct MinMax));
          }
        }
        return 0;
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    int status0;
    wait(&status0);

    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) 
  {
    int min = INT_MAX;
    int max = INT_MIN;
    struct MinMax *ret = malloc(sizeof(int)*2);
    
    if (with_files)
    {
        FILE *fp;
        char filename[255] = "paral_itog_";
        char numb[255] = "";
        //itoa(i, numb, 10);
        sprintf(numb, "%d", i);
        strcat(filename, numb);
        strcat(filename, ".txt");
        fp = fopen(filename, "rb");
        fread(ret, sizeof(struct MinMax), 1, fp);
        fclose(fp);
    } else 
    {
      // read from pipes
 //     read(f_pipes[0], ret, sizeof(struct MinMax));
      //read((pip_mass+i*2)[0], ret, sizeof(struct MinMax));
      for (int i=0; i<pnum/4+1; i++)
      {
         read(f_pipes[i][0], ret, sizeof(struct MinMax));
      }
    }
    
    min = ret->min;
    max = ret->max;
    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
    free(ret);
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  //free(pip_mass);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
