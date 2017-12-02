#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <random>

int main()
{
    /** READ DATA FROM FILE **/
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

    int child_no=10; int parent_no=2;
    int freq_cnt=60; int theta_step_no=37;
    int out_cnt=8;
    int column_no=13; int row_no=60; int StartLine=160022;
    float gain_input[column_no][row_no][parent_no]={{0}};
    float phase_input[column_no][row_no][parent_no]={{0}};
    float gain_output[column_no][row_no][child_no]={{0}};
    float phase_output[column_no][row_no][child_no]={{0}};
    float neutrino[parent_no]={0};
    int max_neu_pos[parent_no]={0};
    float neutrino_total=10;

    //Find Files
    int file_cnt=0;
    while(1)
    {
        FILE *fptr;
        char filename[100];
        sprintf(filename,"C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\bicone_NEC+_%i.txt",file_cnt);
        if (fptr=fopen(filename,"r"))
        {
            int j=0; float neu=-1;
            while((line=fgets(buffer,sizeof(buffer),fptr)) != NULL)   //Runs down lines
           {
            if (j>=160147)
            {
                int i=0;
                word = strtok(line," \t");    //Separate lines into words
                while(word != NULL)
                {
                    val=strtof(word,NULL);  //Grabs word as a floating-point value
                    if (i==3)
                    {
                        neu=val; break;
                    }
                    i++;
                    word = strtok(NULL," \t");
                }
            }
            j++;
           }
           if (neu>neutrino[0])
           {
               neutrino[1]=neutrino[0]; max_neu_pos[1]=max_neu_pos[0];
               neutrino[0]=neu; max_neu_pos[0]=file_cnt;
           }
           else if ( (neu>neutrino[1]) && (neu<neutrino[0]) )
           {
               neutrino[1]=neu; max_neu_pos[1]=file_cnt;
           }
        }
        else
        {
            neutrino_total=neutrino[0]+neutrino[1];
            break;
        }
        file_cnt++;
    }

    //Pull data from input ("parent") files
    for (int k=0;k<parent_no;k++)
    {
        char filename_name[200];
        sprintf(filename_name,"C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\bicone_NEC+_%i.txt",k);  //Check next file
        FILE *fptr=fopen(filename_name,"r");    //Begin taking data from gain_input file
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

    float sum_val = 0;
    //float f_max_val = 100.0;

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
    }

   for (int t=0;t<column_no;t++)
   {
       for (int u=0;u<row_no;u++)
       {
           sum_val+=fabs(gain_input[t][u][0]);
       }
   }

    srand (time(NULL));

    /** RANDOM CROSSBREED **/
 /*       for (k=0;k<out_cnt;k++) //Each individual in the population
        {
            int i;
            for(i=0;i<column_no;i++) //Each "allele" of the individual
            {
                int j;
                for(j=0;j<row_no;j++)
                {
                    gain_output[i][j][k]=gain_input[i][j][rand()%parent_no];  //Randomly pick one parent from whom to steal an "allele"
                }
            }
        }
*/
    int count_0=0;
    int count_1 = 0;
    int count_total = 0;
    /** WEIGHTED RANDOM CROSSBREED **/
        for (int k=0;k<out_cnt;k++) //Each individual in the population
        {
            for(int i=0;i<column_no;i++) //Each "allele" of the individual
            {
                for(int j=0;j<row_no;j++)
                {
                    int r=rand()%100;
                    if (r <= 100*neutrino[0]/neutrino_total && r > 15)
                    {
                        gain_output[i][j][k]=gain_input[i][j][0];
                        count_0++;
                    }
                    else
                    {
                        gain_output[i][j][k]=gain_input[i][j][1];
                        count_1++;
                    }
                    r=rand()%100;
                    if (r <= 100*neutrino[0]/neutrino_total && r > 15)
                    {
                        phase_output[i][j][k]=phase_input[i][j][0];
                        count_0++;
                    }
                    else
                    {
                        phase_output[i][j][k]=phase_input[i][j][1];
                        count_1++;
                    }
                }
            }
            count_total = count_0+count_1;
            float count_0_percent = 100*count_0/count_total;
            float count_1_percent = 100*count_1/count_total;
            printf("Parent 0 %f\t%i\t Parent 1 %f\t%i\n", count_0_percent,count_0, count_1_percent,count_1 );
            count_0=0; count_1=0;
        }

//This distribution needs to be reprogrammed for spherical harmonics
std::default_random_engine generator;
std::normal_distribution <float> distribution(0.0,1.0);
int p[21]={0};

    /** MUTATIONS **/
    //How do we prevent mutations from producing an unrealistic radiation pattern?
        for (int k=out_cnt;k<child_no;k++) //Each individual in the population
        {
            for(int i=0;i<column_no;i++) //Each "allele" of the individual
            {
                for(int j=0;j<row_no;j++)
                {
                    float r=distribution(generator);
                    int t=round(10*r)+10;
                    if (r>=-1 && r<=1)
                    {
                        p[t]++;
                    }
                    gain_output[i][j][k]=r*gain_input[i][j][j%parent_no];
                    r=distribution(generator);
                    t=round(10*r)+10;
                    if (r>=-1 && r<=1)
                    {
                        p[t]++;
                    }
                    phase_output[i][j][k]=r*phase_input[i][j][j%parent_no];
                }
            }
        }
int cnt;
for (cnt=0;cnt<21;cnt++)
{
    printf("Bin: %d\t", cnt);
    for (int what=0;what<round(p[cnt]/10);what++) //each star represent 10 values
    {
        printf("*");
    }
    printf("\n");
}
/** WRITE VALUES INTO OUTPUT FILES **/
//char writename[200]="C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\SphereHarm.txt";
char writename2[200]="C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\INPUT_CHECK_TEST.txt";

for (int k=0;k<child_no;k++)
{
//Create new file
char writename[200]="C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\SphereHarm.txt";
FILE *fptr;

//Give file a unique name
int file_cnt=1;
while(1)
{
    if ((fptr = fopen(writename, "r")) && k>0 )
    {
        fclose(fptr);
        sprintf(writename,"C:\\Users\\Jack\\Documents\\CalPoly\\NuRad\\SphereHarm-%i.txt",file_cnt);
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

printf("\n%f\t%f\n%i\t%i\n",neutrino[0],neutrino[1],max_neu_pos[0],max_neu_pos[1]);
printf("\n\nSIMULATION COMPLETE\n~~~~~~~~~~~~~~~~~~~\n");
//int status = system("SphereHarm_to_NEC+.exe"); //Call program to continue loop
   return 0 ;
 }
