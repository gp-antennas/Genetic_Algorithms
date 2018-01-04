#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <random>
#include <iostream>
using namespace std;

int main(int arg_no, char *arg_typ[])
{
    /** READ DATA FROM FILE **/
    int roul_x_no=atoi(arg_typ[1]);
    int roul_mut_no=atoi(arg_typ[2]);
    int tour_x_no=atoi(arg_typ[3]);
    int tour_mut_no=atoi(arg_typ[4]);
    int roul_no=roul_x_no+roul_mut_no; int tour_no=tour_x_no+tour_mut_no;
    int child_no=tour_no+roul_no;
    printf("\nRoulette Crossbreeds: %i\tRoulette Mutations: %i\nTournament Crossbreeds: %i\tTournament Mutations: %i\n\n",roul_x_no,roul_mut_no,tour_x_no,tour_mut_no);
    char dir[200]="C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\";

    //Read file into array
    /*
    buffer = holds lines as they are brought in
    line = holds each line as it is read in
    word = holds each word as it is read in
    val = holds each word as a float
    child_no = holds integer number of children to be created
    parent_no = hold integer number of parents brought in
    column_no = index for theta gain and phi gain values
    row_no = index for frequency, theta, and phi values
    gain_input = holds gain values that change between generations
    phase_input = holds phase values that change between generations
    max_val = maximum value of either theta gain or phi gain of parent arrays
    min_val = minimum value of either theta gain or phi gain of parent arrays
    gain_output = holds gain_output phi gain and theta gain values
    */
    char buffer[1000]; char *word, *line; float val;    //Variable for extracting data
    int neu_loc = 160147;

    //Find Files
    int file_cnt=0;
    while(1)
    {
        FILE *fptr;
        char filename[100]; char file_end[30];
        sprintf(filename,dir); sprintf(file_end,"bicone_NEC+_%i.txt",file_cnt);
        strcat(filename,file_end);
        if (!(fptr=fopen(filename,"r")))
        {
            break;
        }
        file_cnt++;
    }
    int parent_no=file_cnt;
    float neutrino[parent_no]={0};

    for (int k=0;k<parent_no;k++)
    {
        FILE *fptr;
        char filename[100]; char file_end[30];
        sprintf(filename,dir); sprintf(file_end,"bicone_NEC+_%i.txt",k);
        strcat(filename,file_end);
        if (fptr=fopen(filename,"r"))
        {
            int j=0; float neu=-1;
            while((line=fgets(buffer,sizeof(buffer),fptr)) != NULL)   //Runs down lines
           {
            //if (j>=neu_loc)
            if (j==neu_loc)
            {
                int i=0;
                word = strtok(line," \t");    //Separate lines into words
                while(word != NULL)
                {
                    val=strtof(word,NULL);  //Grabs word as a floating-point value
                    if (i==3)
                    {
                        neu=val;
                        neutrino[k]=val;
                        break;
                    }
                    i++;
                    word = strtok(NULL," \t");
                }
            }
            j++;
           }
        }
        else
        {
            break;
        }
    }

    //int parent_no=4;
    int freq_cnt=60; int theta_step_no=37;
    int column_no=13; int row_no=60; int StartLine=160022;
    float gain_input[column_no][row_no][parent_no]={{0}};
    float phase_input[column_no][row_no][parent_no]={{0}};
    float gain_output[column_no][row_no][child_no]={{0}};
    float phase_output[column_no][row_no][child_no]={{0}};
    int max_neu_pos[parent_no]={0};
    float neutrino_total=0;

    //Pull data from input ("parent") files
    for (int k=0;k<parent_no;k++)
    {
        neutrino_total=neutrino_total+neutrino[k];
        char filename[100]; char file_end[30];
        sprintf(filename,dir); sprintf(file_end,"bicone_NEC+_%i.txt",k);
        strcat(filename,file_end);;  //Check next file
        FILE *fptr=fopen(filename,"r");    //Begin taking data from gain_input file
        int j=-1;
    while((line=fgets(buffer,sizeof(buffer),fptr)) != NULL)   //Runs down lines
       {
        if (j>=StartLine)
        {
            int i=0;
            word = strtok(line," \t");    //Separate lines into words
            while(word != NULL && j>=StartLine)
            {
                val=strtof(word,NULL);  //Grabs word as a floating-point value
                if (j>=StartLine && j<StartLine+row_no)
                {
                    gain_input[i][j-StartLine][k]=val;
                }
                else if (j>=StartLine+row_no+3 && j<StartLine+2*row_no+3)
                {
                    phase_input[i][j-StartLine-row_no-3][k]=val;
                }
                i++;
                word = strtok(NULL," \t");
                if (i>=column_no)
                {
                    break;
                }
            }
        }
        j++;
        if (j>=StartLine+2*row_no+3)
        {
            break;
        }
       }
    fclose(fptr);   //Stop taking data from input file
    }

    float sum_val[column_no] = {0};
    float mean_val[column_no] = {0};

   /** CALCULATE MINIMUM AND MAXIMUM VALUES **/
   float min_val[column_no]={0.0f}; float max_val[column_no]={-1000.0f};
   for (int n=0;n<parent_no;n++)
   {
       for (int t=0;t<column_no;t++)
       {
           for (int u=0;u<row_no;u++)
           {
               if (gain_input[t][u][n]<min_val[n])
               {
                   min_val[t]=gain_input[t][u][n];
               }
               if (gain_input[t][u][n]>max_val[n])
               {
                   max_val[t]=fabs(gain_input[t][u][n]);
               }
           }
       }
       for (int t=0;t<column_no;t++)
       {
           for (int u=0;u<row_no;u++)
           {
               sum_val[t]+=fabs(gain_input[t][u][n]);
           }
           mean_val[t]=sum_val[t]/(column_no*row_no);
       }
    }

    srand (time(NULL));

    int count_no[parent_no]={0};
    int count_total = 0;
    /** ROULETTE WHEEL SELECTION **/
        for (int k=0;k<roul_no;k++) //Each individual in the population
        {
            int parents[2]={0};
            for (int m=0;m<2;m++)
            {
                float r=rand()%100;
                int neutrino_sum_temp=0;
                for (int n=0;n<parent_no;n++)
                {
                    neutrino_sum_temp+=neutrino[n];
                    if (n<parent_no-1 && r<100*neutrino_sum_temp/neutrino_total)
                    {
                        parents[m]=n; count_no[n]++;
                        break;
                    }
                    else if (n==parent_no-1)
                    {
                        parents[m]=n; count_no[n]++;
                        break;
                    }
                }
            }
            printf("\nRoul: %i\t%i",parents[0],parents[1]);
            if (k<=roul_x_no)
            {
                //Crossbreeds
                for(int i=0;i<column_no;i++) //Each "allele" of the individual
                {
                    for(int j=0;j<row_no;j++)
                    {
                        int temp1=round(rand()%1);
                        int temp2=parents[temp1];
                        gain_output[i][j][k]=gain_input[i][j][temp2];
                        phase_output[i][j][k]=phase_input[i][j][temp2];
                    }
                }
                for (int n=0;n<parent_no;n++)
                {
                    count_total=count_total+count_no[n];
                }
            }
            else
            {
                //Mutations
                //This distribution needs to be reprogrammed for spherical harmonics
                std::default_random_engine generator;
                int temp = 100 + rand()%(-100); printf("\t%i",temp);
                int p[21]={0};
                    /** MUTATIONS **/
                    //How do we prevent mutations from producing an unrealistic radiation pattern?
                        for(int i=0;i<column_no;i++) //Each "allele" of the individual
                        {
                //std::default_random_engine generator;
                //std::normal_distribution <float> distribution(mean_val[i],mean_val[i]/10);
                std::normal_distribution <float> distribution(mean_val[i],mean_val[i]/10+mean_val[i]*temp/1000);
                            for(int j=0;j<row_no;j++)
                            {
                                float r=distribution(generator);
                                int t=round(10*r)+10;
                                if (r>=-1 && r<=1)
                                {
                                    p[t]++;
                                }
                                gain_output[i][j][k]=r+gain_input[i][j][rand()%parent_no];
                                r=distribution(generator);
                                t=round(10*r)+10;
                                if (r>=-1 && r<=1)
                                {
                                    p[t]++;
                                }
                                phase_output[i][j][k]=r+phase_input[i][j][rand()%parent_no];
                            }
                        }
                        int cnt;
                        for (cnt=0;cnt<21;cnt++)
                        {
                            printf("\nBin: %d\t", cnt);
                            for (int temp=0;temp<round(p[cnt]/10);temp++) //each star represent 10 values
                            {
                                printf("*");
                            }
                        }
            }
        }
    /** TOURNAMENT SELECTION **/
    for (int k=roul_no;k<child_no;k++)
    {
        int mat_size=ceil(parent_no/2); int max_neu=-1; int max_neu2=-1;
        int mat_loc[mat_size]={0};
        for (int n=0;n<mat_size;n++)
        {
            mat_loc[n]=rand()%parent_no;
            if (max_neu==-1)
            {
                max_neu=mat_loc[n];
            }
            else if (max_neu2==-1)
            {
                max_neu2=mat_loc[n];
            }
            else if (neutrino[mat_loc[n]]>neutrino[max_neu])
            {
                max_neu=mat_loc[n];
            }
            else if (neutrino[mat_loc[n]]>neutrino[max_neu2])
            {
                max_neu2=mat_loc[n];
            }
        }
        printf("\nTour: %i\t%i",max_neu,max_neu2);
        count_no[max_neu]++; count_no[max_neu2]++; count_total=count_total+2;
        int parents[2]={max_neu,max_neu2};
        if (k<=tour_x_no)
        {
            for (int i=0;i<column_no;i++)
            {
                int temp = 100 + rand()%(-100); printf("\t%i",temp);
                for (int j=0;j<row_no;j++)
                {
                int temp1=round(rand()%1);
                int temp2=parents[temp1];
                gain_output[i][j][k]=gain_input[i][j][temp2];
                phase_output[i][j][k]=phase_input[i][j][temp2];
                }
            }
        }
        else
        {
            //Mutations
                //This distribution needs to be reprogrammed for spherical harmonics
                std::default_random_engine generator;
                int temp = 100 + rand()%(-100); printf("\t%i",temp);
                int p[21]={0};
                    /** MUTATIONS **/
                    //How do we prevent mutations from producing an unrealistic radiation pattern?
                        for(int i=0;i<column_no;i++) //Each "allele" of the individual
                        {
                //std::default_random_engine generator;
                //std::normal_distribution <float> distribution(mean_val[i],mean_val[i]/10);
                std::normal_distribution <float> distribution(mean_val[i],mean_val[i]/10+mean_val[i]*temp/1000);
                            for(int j=0;j<row_no;j++)
                            {
                                float r=distribution(generator);
                                int t=round(10*r)+10;
                                if (r>=-1 && r<=1)
                                {
                                    p[t]++;
                                }
                                gain_output[i][j][k]=r+gain_input[i][j][rand()%parent_no];
                                r=distribution(generator);
                                t=round(10*r)+10;
                                if (r>=-1 && r<=1)
                                {
                                    p[t]++;
                                }
                                phase_output[i][j][k]=r+phase_input[i][j][rand()%parent_no];
                            }
                        }
                        int cnt;
                        for (cnt=0;cnt<21;cnt++)
                        {
                            printf("\nBin: %d\t", cnt);
                            for (int temp=0;temp<round(p[cnt]/10);temp++) //each star represent 10 values
                            {
                                printf("*");
                            }
                        }
        }
    }

/** WRITE VALUES INTO OUTPUT FILES **/
char writename2[200]; sprintf(writename2,dir); strcat(writename2,"INPUT_CHECK_TEST.txt");

for (int k=0;k<child_no;k++)
{
//Create new file
char writename[200]; sprintf(writename,dir);
char writename_end[30]="SphereHarm.txt";
strcat(writename,writename_end);
FILE *fptr;

//Give file a unique name
int file_cnt=1;
while(1)
{
    if ((fptr = fopen(writename, "r")) && k>0 )
    {
        fclose(fptr);
        sprintf(writename,dir);
        sprintf(writename_end,"SphereHarm-%i.txt",file_cnt);
        strcat(writename,writename_end);
    }
    else
    {
        break;
    }
    file_cnt++;
}

//Open said file
fptr=fopen(writename,"w");
    for (int n=0;n<2;n++)
    {
        if (n==0)
        {
            fprintf(fptr,"\nGain Coefficients\nFit Type: Poly\n");
        }
        else
        {
            fprintf(fptr,"\nPhase Coefficients\nFit Type: Poly\n");
        }
    for (int j=0;j<row_no;j++)
    {
        for (int i=0;i<column_no;i++)
        {
            if (n==0)
            {
                fprintf(fptr,"%E",gain_output[i][j][k]);
            }
            else
            {
                fprintf(fptr,"%E",phase_output[i][j][k]);
            }
            if (i<column_no-1)
            {
                fprintf(fptr," ");
            }
        }
        fprintf(fptr,"\n");
    }
}
fclose(fptr);
}

/** REWRITE INPUT FILES TO CHECK **/
FILE *fptr2=fopen(writename2,"w");
for (int n=0;n<2;n++)
{
    if (n==0)
    {
        fprintf(fptr2,"\nGain Coefficients\nFit Type: Poly\n");
    }
    else
    {
        fprintf(fptr2,"\nPhase Coefficients\nFit Type: Poly\n");
    }
for (int k=0;k<parent_no;k++)
{
    for (int j=0;j<row_no;j++)
    {
        for (int i=0;i<column_no;i++)
        {
            if (n==0)
            {
                fprintf(fptr2,"%E",gain_input[i][j][k]);
            }
            else
            {
                fprintf(fptr2,"%E",phase_input[i][j][k]);
            }
            if (i<column_no-1)
            {
                fprintf(fptr2," ");
            }
        }
        fprintf(fptr2,"\n");
    }
}
}
fclose(fptr2);

printf("\n\nNeutrino Percentages\n");
for (int n=0;n<parent_no;n++)
    {
        printf("%f%\t",100*neutrino[n]/neutrino_total);
    }
    printf("\n");
for (int n=0;n<parent_no;n++)
    {
        printf("%f%\t",100*count_no[n]/count_total);
    }

printf("\n\nTotal Neutrinos:%f\n\nSIMULATION COMPLETE\n~~~~~~~~~~~~~~~~~~~\n",neutrino_total);
//int status = system("SphereHarm_to_NEC+.exe"); //Call program to continue loop
   return 0 ;
 }
