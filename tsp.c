//
// Students
// 88808 - João Miguel Nunes de Medeiros e Vasconcelos 
// 88886 - Tiago Carvalho Mendes
//
// AED, 2018/2019
//
// solution of the traveling salesman problem
//

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <limits.h>

#include "cities.h"
#include "elapsed_time.h"

//
// record best solutions for tsp_v1
//

static int min_length,max_length,histogram;
static int min_tour[max_n_cities + 1],max_tour[max_n_cities + 1];
static long n_tours;
static int hist[10000];

// tsp_v2 extra variables
static int visited_all;
static int best_distances[(1<<max_n_cities)][max_n_cities];
static int current_city;
static int control;
static int best_position[(1<<max_n_cities)][max_n_cities];

//
// first solution (brute force, distance computed at the end, compute best and worst tours)
//


int computeTourLength(int n, int *a)
{ // function to compute tour length in the tsp_v1
  int i, tourLength = 0;

  for(i = 0;i < n-1;i++)
  {
    tourLength += cities[a[i]].distance[a[i+1]];
  }
  tourLength += cities[a[n-1]].distance[a[0]];

  // histogram of the length of all tours for 12 and 15 cities
  if(histogram == 1 && (n == 12 || n == 15))
  {
    hist[tourLength]++;
  }
  return tourLength;
}

void updateLengths(int n, int tourLength, int *a)
{ // function to update distances and tours (best and worst) in the tsp_v1
  int i;

  if(tourLength < min_length)
  { // update min_length and min_tour
    min_length = tourLength;
    for(i = 0;i < n;i++)
     {
      min_tour[i] = a[i];
    }
  }
  else if(tourLength > max_length)
  { // update max_length and max_tour
    max_length = tourLength;
    for(i = 0;i < n;i++)
    {
      max_tour[i] = a[i];
    }
  }
}

void tsp_v1(int n,int m,int *a)
{
  
  int i,t,tourLength;
 
  if(m < n - 1)
    for(i = m;i < n;i++)
    {
      t = a[m];
      a[m] = a[i];
      a[i] = t;
      tsp_v1(n,m + 1,a);
      t = a[m];
      a[m] = a[i];
      a[i] = t;
    }
  else
  { // visit permutation
    n_tours++;
    tourLength = computeTourLength(n,a);  // compute tour length
    updateLengths(n,tourLength,a);  // update min_length, min_tour, max_length and max_tour
  }
}

void rand_perm(int n,int a[])
{ // function to generate random permutations in the tsp_v1
  int i,j,k,tourLength;

  for(i = 0;i < n;i++)
  {
    a[i] = i;
  }
  for(i = n - 1;i > 0;i--)
  {
    j = (int)floor((double)(i + 1) * (double)rand() / (1.0 + (double)RAND_MAX)); // range 0..i
    assert(j >= 0 && j <= i);
    k = a[i];
    a[i] = a[j];
    a[j] = k;
  }
  n_tours++;
  tourLength = computeTourLength(n,a);
  updateLengths(n,tourLength,a);  // update min_length, min_tour, max_length and max_tour
}


int tsp_v2(int mask, int position)
{
  if(mask == visited_all)
  {
    return cities[position].distance[0];
  }
  
  if(best_distances[mask][position] != 0)
  {
    return best_distances[mask][position];
  }

  int min = 100000000, max = 0, city;
  int best_city = 0;

  for(city = 0; city < current_city;city++)
  {
    if((mask&(1<<city)) == 0)
    {
      int d = cities[position].distance[city] + tsp_v2(mask|(1<<city),city);
      
      if(control == 0)
      {
        if(d < min)
        {
          min = d;
          best_city = city;
        }
      } else 
      {
        if(d > max)
        {
          max = d;
          best_city = city;
        }
      }
    }
  }
  
  best_position[mask][position] = best_city;
  if(control == 0)
  {
    best_distances[mask][position] = min;
  }
  else
  {
    best_distances[mask][position] = max;
  }
    
  return best_distances[mask][position];

}

//
// main program
//

int main(int argc,char **argv)
{
  int n_mec,special,random,print,n,i,a[max_n_cities],tsp_v;
  char file_name[64], file_name2[64];
  double dt1;
  FILE *file, *file2;

  n_mec = 88886; // change later to n_mec = 88808 
  special = 0;   // if you want asymmetric distances, change this to special = 1
  random = 0;    // if you want random permutations, change this to random = 1
  histogram = 0; // if you want to make an histogram of the length of all tours, change this to histogram = 1
  print = 0;     // if you want to save the data to a .csv file, change this to 1
  tsp_v = 2;     // tsp_v = 1 (for Brute Force) or tsp_v = 2 (for Dynamic Programming)

  init_cities_data(n_mec,special);
  printf("data for init_cities_data(%d,%d)\n",n_mec,special);
  fflush(stdout);

  if(print != 0)
  { // open file and initialize it
    sprintf(file_name,"./Data/Special_%d/%d/%s.csv",special,n_mec,(random == 0) ? "tsp_data" : "tsp_random_data");
    file = fopen(file_name,"w");
    fprintf(file, "%s;%s;%s;%s;%s;%s\n","Number of cities (n)","Execution time (s)","minLength","minPath","maxLength","maxPath");
  }
  
#if 1
  print_distances();
#endif
    for(n = 3;n <= max_n_cities;n++)
    {
      //
      // try tsp_v1
      //
      dt1 = -1.0;
      if(n <= 18)
      {
        (void)elapsed_time();
        for(i = 0;i < n;i++)
          a[i] = i;
        min_length = 1000000000;
        max_length = 0;
        n_tours = 0l;

        memset(hist, 0, sizeof(hist));

        // choose permutations type
        if(random == 0)
        {
          if(tsp_v == 1)
          {
            tsp_v1(n,1,a); // no need to change the starting city, as we are making a tour
          } 
          else
          { // tsp_v2 - dynamic programming approach
            current_city = n;
            for(int c = 0;c < 2;c++)
            {
              visited_all = (1<<current_city) - 1;
              for(int mask = 0;mask < (1<<max_n_cities);mask++)
                for(int position = 0;position < max_n_cities;position++)
                {
                  best_distances[mask][position] = 0;
                  best_position[mask][position] = 0;
                }
              // update min_tour or max_tour
              if(control == 0)
              {
                min_length = tsp_v2(1,0);
                int pos = 0; int indice = 0;
                for(int x1 = 1; x1 != visited_all;)
                {
                  int x2 = best_position[x1][pos];
                  min_tour[indice] = x2;
                  x1 = x1|(1<<x2);
                  pos=x2;
                  indice++;
                }
                for(int o = indice; o > 0; o--)
                {
                  min_tour[o] = min_tour[o-1];
                }
                min_tour[0] = 0;
                control = 1;
              }
              else
              {
                max_length = tsp_v2(1,0);
                int pos = 0; int indice = 0;
                for(int x1 = 1; x1 != visited_all;)
                {
                  int x2 = best_position[x1][pos];
                  max_tour[indice] = x2;
                  x1 = x1|(1<<x2);
                  pos=x2;
                  indice++;
                }
                for(int o = indice; o > 0; o--)
                {
                  max_tour[o] = max_tour[o-1];
                }
                max_tour[0] = 0;
                control = 0;
              }
            }
          }
          
        }
        else 
        { // random permutations
          for(i = 0;i < 20000000;i++)
          {
            rand_perm(n,a);
          }
        }
  
        dt1 = elapsed_time();
        printf("%d) tsp_v1() finished in %8.3fs (%ld tours generated)\n",n,dt1,n_tours);

        // print min_tour by index
        printf("  min %5d [",min_length);
        for(i = 0;i < n;i++)
        {
          printf("%2d%s",min_tour[i],(i == n - 1) ? "]\n" : ",");
        }

        // print max_tour by index
        printf("  max %5d [",max_length);
        for(i = 0;i < n;i++)
        {
          printf("%2d%s",max_tour[i],(i == n - 1) ? "]\n\n" : ",");
        }

        // save the computed data into a file
        if(print != 0)
        {
          fprintf(file,"%d;%8.3f;%d;[",n,dt1,min_length);
          for(i = 0; i < n;i++)
          {
            fprintf(file,"%d%s",min_tour[i],(i == n - 1) ? "];" : ",");
          }
          fprintf(file,"%d;[",max_length);
          for(i = 0; i < n;i++)
          {
            fprintf(file,"%d%s",max_tour[i],(i == n - 1) ? "]\n" : ",");
          }
        }
        
        if(histogram == 1 && (n == 12 || n == 15))
        {
          sprintf(file_name2,"./Data/Special_%d/%d/%d_cities_tours_histogram.csv",special,n_mec,n);
          file2 = fopen(file_name2,"w");
          //fprintf(file2, "%s;%s\n","Tour","Occurrences");
          for(i = 0; i < 10000; i++)
          {
            fprintf(file2,"%d,%d\n",i,hist[i]);
          }
          fclose(file2);
        }
        
        fflush(stdout);
        if(argc == 2 && strcmp(argv[1],"-f") == 0)
        {
          min_tour[n] = -1;
          sprintf(file_name,"./Maps/Special_%d/%d/%s/min_%02d.svg",special,n_mec,(random == 0) ? "Normal" : "Random",n);
          make_map(file_name,min_tour);
          max_tour[n] = -1;
          sprintf(file_name,"./Maps/Special_%d/%d/%s/max_%02d.svg",special,n_mec,(random == 0) ? "Normal" : "Random",n);
          make_map(file_name,max_tour);
        }      
      }
    }
  if(print == 1) {
    fclose(file);
  }
  return 0;
}
