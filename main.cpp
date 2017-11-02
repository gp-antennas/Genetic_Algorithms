#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

int main()
{
    //Read file into array
    char buffer[1000]; char *word, *line; float val;
    float import[15][200]; int array_size=0; int array_size2=0;
    //Read file into array
    FILE *fptr=fopen("C:\\Users\\samro\\Documents\\Spherical Harmonic Fits - Vpol Gain.csv","r");
    while((line=fgets(buffer,sizeof(buffer),fptr)) != NULL)   //Runs down lines
       {
           int j=0;
        word = strtok(line,"\t,");
        while(word != NULL)
        {
            val=strtof(word,NULL);
            if (j==2 && int(val)==0)
            {
                break;
            }
            else if (j>=2)
            {
                import[array_size][array_size2]=val;
                array_size++;
                if (array_size==13)
                {
                    array_size2++;
                    array_size=0;
                }
            }
            word = strtok(NULL,"\t,");
            j++;
        }
       }
       array_size=13;
       /*int j=0; float perfect[array_size][array_size2];
       for (j=0;j<array_size2;j++)
       {
           int i=0;
           for (i=0;i<array_size;i++)
           {
               perfect[i][j]=import[i][j];
           }
       }*/
       float perfect[array_size];
       int i=0;
       for (i=0;i<array_size;i++)
       {
           perfect[i]=import[i][0];
       }
    fclose(fptr);

    do {
    int max_gen = 0;
    int max_val = 100;
    long double sum_val = 0.00;
    //float f_max_val = 100.0;
    float target_score=0;
    /*printf("\nMaximum Value: ");   //Query
   std::cin >> max_val; //Answer*/
    printf("\nTarget Difference (Percentage): ");   //Query
   std::cin >> target_score; //Answer
    printf("\nGeneration Limit: ");   //Query
   std::cin >> max_gen; //Answer
   char redo;
   int test_no=13; int array_length=13; //int max_val=10000;

   //Perfect ("Goal") Array
   //float perfect[array_length]={2.52181,2.83E-06,-1.1483,-1.09E-06,0.0125636,4.73E-07,0.00261664,-8.40E-08,0.00131323,-2.76E-07};
   //float perfect[array_length]={2.52181,2.83E-06,-1.1483,-1.09E-06,0.0125636,4.73E-07,0.00261664,-8.40E-08,0.00131323,-2.76E-07,-0.000701828,-4.51E-07,0.0000289247};
   int t; float min_val=0.0f;
   for (t=0;t<array_length;t++)
   {

       if (perfect[t]<-min_val)
       {
           min_val=fabs(perfect[t]);
       }

   }
   for (t=0;t<array_length;t++)
   {
       perfect[t]+=min_val;
       sum_val+=fabs(perfect[t]);
   }
   //Elite Array
   float elite[array_length][test_no];
   float fitness[test_no]={0.0f};
   float best_fit={1000.0f};
   float best_fit2={0.0f};
   float best_choice[array_length]={0.0f};
   float best_choice_val=10000.0f;
   float final_choice[array_length]={0.0f};
   float final_choice_percent=0.0f;
   float best_fit_percent={0.0f};
   int winner=0; int winner2=0; int check;

   //Roulette Array
   float roulette[array_length][test_no];
   float roulette_winner[array_length]; float roulette_score=0.0; float roulette_percentage=0.0; int roulette_pool;
   float best_roulette_winner[array_length];
   float best_roulette_percentage=100.0;
   float fit_hold[test_no*max_val*array_length]={0.0}; int fitness_length;

   //Tournament array
   float tour[array_length][test_no];
   float tour_best[array_length];
   float tour_score[test_no]={0.0};
   float tour_best_score=10000.0;
   float tour_best_percentage=100;
   float tour_hold[array_length] = {0.0f};
   int final_gen = 0;


int loop=0;
while (loop<5)
{
    srand (time(NULL));
    loop++;

   //Initialization
    for (i=0;i<test_no;i++)
    {
        int j=0;
        for (j=0;j<array_length;j++)
        {
            elite[j][i]=rand()%int(sum_val)+min_val;
            roulette[j][i]=rand()%int(sum_val)+min_val;
            tour[j][i]=rand()%int(sum_val)+min_val;
        }
    }

    //Elite Test
    int n=0;
    while (final_choice_percent>target_score && n<max_gen || n<1) //Generations
    {
        best_fit=array_length*max_val; best_fit2=array_length*max_val;
        for (i=0;i<test_no;i++) //Each individual in the population
        {
            int j=0; fitness[i]=0;
            for(j=0;j<array_length;j++) //Each "allele" of the individual
            {
                if (i==test_no-1)
                {
                    elite[j][i]=rand()%int(sum_val);
                }
                else if (i==j)
                {
                    elite[j][i]=rand()%int(sum_val);
                    //elite[j][i]=elite[j][i]*((-1)^rand()%1);
                    //printf("%f\n",elite[j][i]);
                }
                fitness[i]+=fabs(elite[j][i]-perfect[j]);
            }
            if (fitness[i]<best_fit)
            {
                best_fit=fitness[i]; winner=i;
            }
            else if (fitness[i]<best_fit2 && fitness[i]>=best_fit2)
            {
                best_fit2=fitness[i]; winner2=i;
            }
        }
        int j=0;
        for (j=0;j<array_length;j++)
        {
            best_choice[j]=(elite[j][winner]+elite[j][winner2])/2.0;
        }
        if (best_fit<best_choice_val)
        {
            for (i=0;i<array_length;i++)
            {
                final_choice[i]=best_choice[i];
                //final_choice[i]=elite[j][winner];
            }
            best_choice_val=best_fit;
        }
        if (n<max_gen)
        {
            for (i=0;i<round(test_no/3);i++)
            {
                for (j=0;j<array_length;j++)
                {
                    elite[j][i]=best_choice[j];
                }
            }
            for (i=round(test_no/3)+1;i<round(2*test_no/3);i++)
            {
                for (j=0;j<array_length;j++)
                {
                    elite[j][i]=elite[j][winner];
                }
            }
            for (i=round(2*test_no/3)+1;i<test_no;i++)
            {
                for (j=0;j<array_length;j++)
                {
                    elite[j][i]=elite[j][winner2];
                }
            }
        }
    best_fit_percent=100*float(best_fit)/sum_val;
    final_choice_percent=100*float(best_choice_val)/sum_val;
    n++;
    }
    int elite_gens=n;
   //Ranking
   //Roulette wheel
    //for (n=0;n<max_gen;n++) //Generations
    n=0;
    while (best_roulette_percentage>target_score && n<max_gen) //Generations
    {
        roulette_pool=test_no;
        do {
        fitness_length=0; roulette_score=0;
        for (i=0;i<roulette_pool;i++) //Each individual in the population
        {
            int j=0; fitness[i]=0;
            for(j=0;j<array_length;j++) //Each "allele" of the individual
            {
                if (i==j)
                {
                    roulette[j][i]=rand()%int(sum_val);
                }
                fitness[i]+=fabs(roulette[j][i]-perfect[j]);
            }
            fitness_length+=(max_val*test_no-int(fitness[i]));
            for (j=fitness_length-(max_val*test_no-int(fitness[i]))-1;j<fitness_length;j++)
            {
                fit_hold[j]=i;
            }
        }
            check=rand()%(fitness_length-1);
            winner=fit_hold[check];
            int j;
            for (j=0;j<array_length;j++)
            {
                roulette_winner[j]=roulette[j][winner];
                roulette_score+=fabs(perfect[j]-roulette_winner[j]);
            }
            roulette_percentage=100*float(roulette_score)/sum_val;
        roulette_pool=1;
        for (i=0;i<test_no;i++)
        {
            if(fitness[i]<fitness[winner])
            {
                roulette_pool+=1;
            }
        }
        if (roulette_pool==1)
        {
            for (i=0;i<test_no;i++)
            {
                int j;
                for (j=0;j<array_length;j++)
                {
                    roulette[j][i]=roulette[j][winner];
                }
            }
            break;
        }
        }while(1);
        if (roulette_percentage<best_roulette_percentage)
        {
            int j;
            for (j=0;j<array_length;j++)
            {
                best_roulette_winner[j]=roulette_winner[j];
            }
            best_roulette_percentage=roulette_percentage;
        }
        n++;
    }
    int roulette_gens=n;


    //Tournament
    //for (n=0;n<max_gen;n++)
    n=0;
    int tour_gens = 0;
    while (tour_best_percentage>target_score && n<max_gen) //Generations
    {
        int k=rand()%(array_length-1)+1;
        //int k=3;
        tour_best_score=sum_val*array_length;
   //     printf("Test Best Score %f\n", test_best_score);
        int j=rand()%array_length;
        int k_found = 0;
        // Initial
          while(k_found<array_length)
          {
                j=j+k;
                if (j>=array_length)
                {
                    j=j-array_length;
                }
                tour_score[i]+=fabs(tour[j][i]-perfect[j]);
                tour_hold[k_found]=tour[j][i];
                k_found++;

           }
        for (i=0;i<test_no;i++)
        {
            int k_found=0;
            printf("Tour Test\n");
            printf("Tour Best Score: %f\n", tour_best_score);
            while(k_found != k)
            {
                if (tour_score[i]<tour_best_score)
                {
                    j=0;
                    for (j=0;j<array_length;j++)
                    {
                        tour_best[j]=tour_hold[j];
                        printf("\n Tour best: %f versus Tour Best Score %f\n",tour_best[j], tour_best_score);
                    }
                    tour_best_score=tour_score[i];
                   // k_found++;


                    printf("\n Testing %i %i %i\n\n",n,k,tour_best_score);
                }
                k_found++;
            }
            k_found = 0;
          //  else
           // {
                while(k_found<array_length)
                {
                    j=j+k;
                    if (j>=array_length)
                    {
                        j=j-array_length;
                    }
                    tour_score[i]+=(tour[j][i]-perfect[j]);
                    tour_hold[k_found]=tour[j][i];
                    k_found++;

                }


        }
        //printf("%i\t%i\t%i\n%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n",n,k,tour_score[i],tour_best[0],tour_best[1],tour_best[2],tour_best[3],tour_best[4],tour_best[5],tour_best[6],tour_best[7],tour_best[8],tour_best[9]);
//        int j;
        printf("Test number: %d", test_no);
        for (i=0;i<test_no;i++)
        {
            for (j=0;j<array_length;j++)
            {
                tour[j][i]=tour_best[j];
                if (i==j)
                {
                    tour[j][i]=float(rand()%int(sum_val));
                }
            }
        }
        tour_best_percentage=100*(float(tour_best_score)/sum_val);
        //n++;
        tour_gens++;
    }
    final_gen = tour_gens;

   //Print Out Values
    printf("\nGoal\n");

    int j=0;
    for (j=0;j<array_length;j++)
    {
        printf("%f ",perfect[j]);
    }
        best_fit_percent=100*float(best_fit)/(sum_val*float(array_length));
            printf("\n\nElite (%i Generations)\n",elite_gens);
            for (j=0;j<array_length;j++)
            {
                printf("%f ",final_choice[j]);
            }
        printf("\nBest Difference: %2.3f Percent\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",final_choice_percent);

        printf("\n\nRoulette (%i Generations)\n",roulette_gens);
            for (j=0;j<array_length;j++)
            {
                printf("%f ",best_roulette_winner[j]);
            }
        printf("\nBest Difference: %2.3f Percent\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",best_roulette_percentage);

        printf("\n\nTournament (%i Generations)\n",final_gen);
            for (j=0;j<array_length;j++)
            {
                printf("%2.3f ",tour_best[j]);
            }
        printf("\nBest Difference: %2.3f Percent\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",tour_best_percentage);

printf("\n%f\n",min_val);
}
printf("\n\nSIMULATION COMPLETE\n~~~~~~~~~~~~~~~~~~~\n");
printf("\nTry again? ");
   scanf("%s",&redo);
   if (!((redo=='y')||(redo=='Y')))
   {
       break;
   }
}while(1);
   return 0 ;
 }
