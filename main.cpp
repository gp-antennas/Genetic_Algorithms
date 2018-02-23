#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <math.h>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <math.h>

using namespace std;


enum DataType {
    gain,
    phase,
    poly,
    sphHarm
};

enum InputFlag {
    err = -1,
    parentFlag,
    algoFlag
};

InputFlag mapInputFlag (std::string const& inString) {
    if (inString == "-p") return parentFlag;
    if (inString == "-gp") return algoFlag;
    return err;
}

int readCoeff(string filePath, DataType dataType, vector<vector<float>> &dataVector );
int readVeff(string filePath, float &Veff);
int parseArgs(int argc, char const *argv[], vector<vector<vector<float>>> &gainVector, vector<vector<vector<float>>> &phaseVector, vector<float> &Veff, int &parent_no, int &roul_x_no, int &roul_mut_no, int &tour_x_no, int &tour_mut_no);
int writeCoeff(ofstream &outFile, DataType dataType, vector<vector<float>> &gainVector, vector<vector<float>> &phaseVector);
int writeInfo(ofstream &outFile, float Veff, int runNum, int iterNum);
int writeRadPattern(ofstream &outFile, DataType dataType, vector<vector<float>> &gainVector, vector<vector<float>> &phaseVector);
int necWrite(DataType dataType, int numChildren, vector<vector<vector<float>>> &gainVector, vector<vector<vector<float>>> &phaseVector, vector<float> &Veff);
int writeHist(ofstream &histFile, vector<float> &Veff);

const float PI = 3.14159265359;

int main(int argc, char const *argv[])
{
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

    int rc = 0;
    int roul_x_no = 4;//=atoi(argv[1]);
    int roul_mut_no = 1;//=atoi(argv[2]);
    int tour_x_no = 4;//=atoi(argv[3]);
    int tour_mut_no = 1;//=atoi(argv[4]);
    int parent_no;
    char buffer[1000]; char *word, *line; float val;    //Variable for extracting data
    int neu_loc = 160147;

    cout << endl;

    //int parent_no=4;
    int freq_cnt=60; int theta_step_no=37;
    const int column_no=13; const int row_no=60; int StartLine=160022;

    // Create vectors for parsing in fit coefficients
    vector<vector<vector<float>>> gain_input (1,vector<vector<float> >(60,vector <float>(13, 0)));
    vector<vector<vector<float>>> phase_input (1,vector<vector<float> >(60,vector <float>(13, 0)));
    vector<float> Veff (1, 0);

    // Create vectors that will hold modified output coefficents
    vector<vector<vector<float>>> gain_output (1,vector<vector<float> >(60,vector <float>(13, 0)));
    vector<vector<vector<float>>> phase_output (1,vector<vector<float> >(60,vector <float>(13, 0)));

    // Read in input arguments and parse in data from files
    rc = parseArgs(argc, argv, gain_input, phase_input, Veff, parent_no, roul_x_no, roul_mut_no, tour_x_no, tour_mut_no);

    // Check for error
    if ( rc == 1 ){
        cout << "ERROR PARSING INPUT ARGUMENTS, PROGRAM CANNOT EXECUTE...";
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "\n";
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "PROGRAM EXITING\n";

        exit(EXIT_FAILURE);
    }

    // Write Veff to history files
    if(parent_no > 1) {
        ofstream histFile;
        histFile.open("history.txt", ios::out | ios::app);
        writeHist(histFile, Veff);
    }

    // Determine number of children from given GA inputs
    int roul_no=roul_x_no+roul_mut_no;
    int tour_no=tour_x_no+tour_mut_no;

    int child_no=tour_no+roul_no;

    // Push back child_no onto output vectors to increase size for number of children
    for (int i = 0; i < child_no; i++)
    {
        gain_output.push_back(vector<vector<float> >(60,vector <float>(13, 0)));
        phase_output.push_back(vector<vector<float> >(60,vector <float>(13, 0)));
    }

    vector<int> max_neu_pos(parent_no, 0);
    //int max_neu_pos[parent_no]={0};

    float Veff_total=0;



    float sum_val[column_no] = {0};
    float mean_val[column_no] = {0};

   /** CALCULATE MINIMUM AND MAXIMUM VALUES **/
   float min_val[column_no]={0.0f}; float max_val[column_no]={-1000.0f};
   for (int t=0;t<parent_no;t++)
   {
       for (int n=0;n<column_no;n++)
       {
           for (int u=0;u<row_no;u++)
           {
               if (gain_input[t][u][n]<min_val[n])
               {
                   min_val[n]=gain_input[t][u][n];
               }
               if (gain_input[t][u][n]>max_val[n])
               {
                   max_val[n]=fabs(gain_input[t][u][n]);
               }
           }
       }
       for (int n=0;n<column_no;n++)
       {
           for (int u=0;u<row_no;u++)
           {
               sum_val[n]+=fabs(gain_input[t][u][n]);
           }
           mean_val[n]=sum_val[t]/(column_no*row_no);
       }
    }

    srand (time(NULL));

    vector<int> count_no(parent_no, 0);
    int count_total = 0;
    /** ROULETTE WHEEL SELECTION **/
        for (int k=0;k<roul_no;k++) //Each individual in the population
        {
            int parents[2]={0};
            for (int m=0;m<2;m++)
            {
                float r=rand()%100;
                int Veff_sum_temp=0;
                for (int n=0;n<parent_no;n++)
                {
                    Veff_sum_temp+=Veff[n];
                    if (n<parent_no-1 && r<100*Veff_sum_temp/Veff_total)
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
            if (k<=roul_x_no - 1)
            {
                //Crossbreeds
                for(int i=0;i<column_no;i++) //Each "allele" of the individual
                {
                    for(int j=0;j<row_no;j++)
                    {
                        int temp1=round(rand()%1);
                        int temp2=parents[temp1];
                        gain_output[k][j][i]=gain_input[temp2][j][i];
                        phase_output[k][j][i]=phase_input[temp2][j][i];
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
                                gain_output[k][j][i]=r+gain_input[rand()%parent_no][j][i];
                                r=distribution(generator);
                                t=round(10*r)+10;
                                if (r>=-1 && r<=1)
                                {
                                    p[t]++;
                                }
                                phase_output[k][j][i]=r+phase_input[rand()%parent_no][j][i];
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
        vector<int> mat_loc(mat_size, 0);

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
            else if (Veff[mat_loc[n]]>Veff[max_neu])
            {
                max_neu=mat_loc[n];
            }
            else if (Veff[mat_loc[n]]>Veff[max_neu2])
            {
                max_neu2=mat_loc[n];
            }
        }
        printf("\nTour: %i\t%i",max_neu,max_neu2);
        count_no[max_neu]++; count_no[max_neu2]++; count_total=count_total+2;
        int parents[2]={max_neu,max_neu2};
        if (k<=tour_x_no - 1)
        {
            for (int i=0;i<column_no;i++)
            {
                int temp = 100 + rand()%(-100); printf("\t%i",temp);
                for (int j=0;j<row_no;j++)
                {
                int temp1=round(rand()%1);
                int temp2=parents[temp1];
                gain_output[k][j][i]=gain_input[temp2][j][i];
                phase_output[k][j][i]=phase_input[temp2][j][i];
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
                                gain_output[k][j][i]=r+gain_input[rand()%parent_no][j][i];
                                r=distribution(generator);
                                t=round(10*r)+10;
                                if (r>=-1 && r<=1)
                                {
                                    p[t]++;
                                }
                                phase_output[k][j][i]=r+phase_input[rand()%parent_no][j][i];
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

    for(int i = 0; i < child_no; i++) {
        for(int j = 0; j < 60; j++) {
                gain_output[i][j][0] = 3.5449;
        }
    }


    /** WRITE VALUES INTO OUTPUT FILES **/
    necWrite(DataType::sphHarm, child_no, gain_output, phase_output, Veff);

    cout << "\nGA execution complete... Exiting\n" << endl;
    exit(EXIT_SUCCESS);

    return 0;

 }


int parseArgs(int argc, char const *argv[], vector<vector<vector<float>>> &gainVector, vector<vector<vector<float>>> &phaseVector, vector<float> &Veff, int &parent_no, int &roul_x_no, int &roul_mut_no, int &tour_x_no, int &tour_mut_no)
{

    // Parse input arguments from the command-line
    // returns 0 on success
    // returns 1 if an error or unexpected input occurs

    /*
        INPUT ARGUMENTS :

        argc = number of arguments given on the command-line
        argv[] = array of arguments passed on the command line
        gainVector = vector read gain coefficient values will be stored to
        phaseVector = vector read phase coefficent values will be stored to
        Veff = vector read Veff (fitness score) values will be written to
        parent_no = variable that number of parents read-in will be written to
        roul_x_no = variable that number of roullete crossbreeds will be written to
        roul_mut_no = variable that number of roulette mutations will be written to
        tour_x_no = variable that number of tournament crossbreeds will be written to
        tour_mut_no = variable that number of tournament mutations will be written to
    */



    vector<int> flagIndices(1, 0);
    int numFlags = 0;

    // Find location of input flags (if any) passed on the command line
    for (int i = 1; i < argc; i++)
    {
        // Use '-' as a flag indicator
        if (argv[i][0] == '-')
        {
            //Store found location in a vector for future reference
            flagIndices[numFlags] = i;
            flagIndices.push_back(0);
            numFlags++;
        }
    }

    // Begin parsing in inputs by looking at provided flags
    int numParents;
    for (int i = 0; i < flagIndices.size(); i++)
    {
        cout << i << endl;
        if(flagIndices[i] != 0){

            // For current flag, switch through possible flags
            int argIndex = flagIndices[i];
            switch ( mapInputFlag(argv[argIndex]) )
            {
                // Parent flag is found
                case parentFlag:

                    cout << "Parent Flag Read" << endl;

                    // Next argv is the number of parents being input
                    numParents = stoi(argv[argIndex + 1]);
                    parent_no = numParents;

                    // Push back input vectors to adapt to the number of parents being used
                    for (int j = 0; j < numParents-1; j++)
                    {
                        gainVector.push_back(vector<vector<float>> (60,vector <float>(13, 0)));
                        phaseVector.push_back(vector<vector<float>> (60,vector <float>(13, 0)));
                        Veff.push_back(0.0f);
                    }

                    cout << endl;
                    // Read coefficients and Veff of each parent
                    for (int j = 0; j < numParents; j++)
                    {
                        cout << "Reading parent " << j+1 << "/" << numParents << endl;

                        // Read gain coefficients of parent
                        cout << "Reading gain coeff... " << endl;
                        readCoeff(argv[argIndex + j + 2], DataType::gain, gainVector[j]);

                        // Read phase coefficients of parent
                        cout << "Reading phase coeff... " << endl;
                        readCoeff(argv[argIndex + j + 2], DataType::phase, phaseVector[j]);

                        // Read Veff of parent
                        cout << "Reading Veff..." << endl;
                        readVeff(argv[argIndex + j + 2], Veff[j]);
                        cout << "Finished parent \n" << endl;
                    }
                    cout << endl;

                    break;


                // Algorithm parameter flag found
                case algoFlag:

                    cout << "Algorithm Flag Read" << endl;

                    // Parse algorithm parameters from argv array
                    roul_x_no = atoi(argv[argIndex + 1]);
                    roul_mut_no = atoi(argv[argIndex + +2]);
                    tour_x_no = atoi(argv[argIndex + 3]);
                    tour_mut_no = atoi(argv[argIndex + 4]);

                    cout << "\nRoulette Crossbreeds: " << roul_x_no << "\nRoulette Mutations:" << roul_mut_no << endl;
                    cout << "\nTournament Crossbreeds: " << tour_x_no << "\nTournament Mutations:" << tour_mut_no << endl;
                    cout << "\nTotal Children: " << roul_x_no + roul_mut_no + tour_mut_no + tour_x_no << "\n\n" << endl;
                    break;

                // A bad flag was saved to the array
                default:
                    cout << "Input Flag Error, IGNORING FLAG : " << argv[flagIndices[i]] << endl;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                    break;
            }
        }

    }

    cout << "Finished parsing inputs..." << endl;
    return 0;
}


int readCoeff(string filePath, DataType dataType, vector<vector<float>> &dataVector )
{

    // Read and parse coefficients from nec+ file at filePath
    // returns 0 on success
    // returns 1 if an error occurs

    /*
        INPUT ARGUMENTS :

        filePath = pathway to file being read
        dataType =  type of cofficient data being read
        dataVector = vector to store coefficent data

    */

    // NEC+ file parameters
    const int NUM_COlUMNS = 13;
    const int NUM_ROWS = 60;
    int START_LINE = 160021;

    // Starting point for gain coefficents
    if(dataType == gain){

        START_LINE = 160021;

    // Starting point for phase coefficents
    } else if (dataType == phase){

        START_LINE = 160085;

    // No data type given, return error
    }else{

        return 1;

    }


    int lineNum, rowCount, columnCount;
    string line, word;

    lineNum = 0;
    rowCount = 0;

    ifstream parentFile;

    // Open file and begin reading
    parentFile.open(filePath);
    if ( parentFile.is_open() ){

        // Read until end of file
        while(!(parentFile.eof())){

            // Get the current line
            string line;
            getline(parentFile, line);

            // If lineNum is at START_LINE and less than 60 rows have been read, save input data
            if( (lineNum >= START_LINE) && (rowCount < 60) ){

                if( (lineNum > (START_LINE + 1)) && (lineNum < (START_LINE + NUM_ROWS + 2)) ){

                    columnCount = 0;
                    istringstream iss(line);
                    string element;

                    // Delimit data by ','
                    while(getline(iss, element, ',')){

                        string :: size_type sz;

                        // Store a data element in the dataVector
                        dataVector[rowCount][columnCount] = stof(element, &sz);

                        // Increment column index
                        columnCount++;

                    }

                    // Increment row index
                    rowCount++;
                }

            }

            // Increment line number
            lineNum++;
        }

        // Close file when done
        parentFile.close();

    // If the file is not open, return an error
    } else {

        cout << "Problem opening file at : " << filePath << endl;
        return 1;

    }


    return 0;

}



int readVeff(string filePath, float &Veff)
{

    // Read and parse Veff from NEC+ file found at filePath
    // returns 0 on success
    // returns 1 if an error occurs

    /*
        INPUT ARGUMENTS :

        filePath = pathway to file being read
        Veff =  variable to save the Veff score to

    */

    // Line number for Veff in a NEC+ file
    const int VEFF_LINE = 160149;

    int lineNum;
    string line, word;

    lineNum = 1;

    // Open file and begin reading
    ifstream parentFile;
    parentFile.open(filePath);

    if ( parentFile.is_open() ){

        while(!(parentFile.eof())){

            string line;
            getline(parentFile, line);

            // lineNum is the Veff data line
            if( lineNum == VEFF_LINE ){

                istringstream iss(line);
                string element;

                int count = 0;
                while( getline(iss, element, ':') ) {

                    string :: size_type sz;
                    element.erase(0, 1);

                    // Veff score follows a ':'
                    if (count == 1) {
                        Veff = stof(element, &sz);
                    }

                    count++;

                }
            }
            lineNum++;
        }

        parentFile.close();

    // If the file is not open, return an error
    } else {

        cout << "Problem opening file at : " << filePath << endl;
        return 1;

    }

    return 0;
}


int writeCoeff(ofstream &outFile, DataType dataType, vector<vector<float>> &gainVector, vector<vector<float>> &phaseVector)
{

    // Write coefficients found in gainVector and phaseVector of data-type dataType to ofstream object outFile
    // returns 0 on success
    // returns 1 if an error occurs

    /*

        INPUT ARGUMENTS :

        outFile = ofstream object of output file being written to
        dataType = either poly or sphHarm, for the dataType being used
        gainVector = gain coefficients to write to file
        phaseVector = phase coefficients to write to file

    */

    // Write gain coefficent header
    outFile << "Gain Coefficients" << endl;
    outFile << "Fit Type : ";

    // Write fit type
    if(dataType == DataType::poly){

        outFile << "Poly" << endl;

    } else if(dataType == DataType::sphHarm) {

        outFile << "Sph Harm" << endl;

    }

    // Begin writing coefficients, comma and line delimitted
    for (int i = 0; i < 60; i++)
    {
        for (int j = 0; j < 13; j++)
        {

            if(j == 12){
                outFile << gainVector[i][j] << "\n";
            }else{
                outFile << gainVector[i][j] << ",";
            }

        }
    }

    // Write phase coefficent header
    outFile << "\n\nPhase Coefficients" << endl;
    outFile << "Fit Type : ";

    // Write fit type
    if(dataType == DataType::poly){

        outFile << "Poly" << endl;

    } else if(dataType == DataType::sphHarm) {

        outFile << "Sph Harm" << endl;

    }

    // Begin writing coefficents, comma and line delimitted
    for (int i = 0; i < 60; i++)
    {
        for (int j = 0; j < 13; j++)
        {

            if(j == 12){
                outFile << phaseVector[i][j] << "\n";
            }else{
                outFile << phaseVector[i][j] << ",";
            }

        }
    }

    outFile << "\n";

    return 0;

}

int writeInfo(ofstream &outFile, float Veff , int runNum, int iterNum)
{
    // Write extra info to ofstream object outFile
    // returns 0 on success
    // returns 1 if an error occurs

    /*

        INPUT ARGUMENTS :

        outFile = ofstream object of output file being written to
        Veff = effective volume associated with data
        runNum = generation of GA to write to NEC+
        iterNum = iteration of GA to write to NEC+

    */

    outFile << "test Veff : " << Veff << endl << endl;
    outFile << "Iter : " << iterNum << endl << endl;
    outFile << "Run : " << runNum << endl << endl;

    return 0;
}

int writeRadPattern(ofstream &outFile, DataType dataType, vector<vector<float>> &gainVector, vector<vector<float>> &phaseVector)
{

    // Compute and write the radiation pattern defined by gainVector and phaseVector to ofstream object outFile
    // returns 0 on success
    // returns 1 if an error occurs

    /*

        INPUT ARGUMENTS :

        outFile = ofstream object of output file being written to
        dataType = data-type (poly or sph. harm.) that will be used to reconstruct radiation pattern
        gainVector = gain coefficients to use for reconstruction
        phaseVector = phase coefficients to use for reconstruction

    */


    // Populate theta and phi arrays
    double theta[37];
    double phi[72];
    int freq[60];

    for (int th = 0; th < 37; th++)
    {
        theta[th] = (double) th*5.0f;
    }

    for (int p = 0; p < 72; p++)
    {
        phi[p] = (double) p*5.0f;
    }

    // Create an populate gain and phase arrays that define an antenna radiation pattern
    double gain[60][72][37] = {{{0}}};
    double gainDB[60][72][37] = {{{0}}};
    double phase[60][72][37] = {{{0}}};
    for (int f = 0; f < 60; f++)
    {
        outFile << "freq : " << 83.33 + 16.67*f << " MHz" << endl;
        //SWR will not be accurate so give it an impossible value to avoid confusion
        outFile << " MHz\rSWR : 0.0\n";
        outFile << " Theta \t Phi \t Gain(dB)   \t   Gain   \t   Phase(deg)\n";


        for (int p = 0; p < 72; p++)
        {
            for (int th = 0; th < 37; th++)
            {

                // Populate the gain and phase vectors for a polynomial fit
                if(dataType == DataType::poly) {


                    gain[f][p][th] = gainVector[f][0]*pow(theta[th]*PI/180.0, 0) +
                                     gainVector[f][1]*pow(theta[th]*PI/180.0, 1) +
                                     gainVector[f][2]*pow(theta[th]*PI/180.0, 2) +
                                     gainVector[f][3]*pow(theta[th]*PI/180.0, 3) +
                                     gainVector[f][4]*pow(theta[th]*PI/180.0, 4) +
                                     gainVector[f][5]*pow(theta[th]*PI/180.0, 5) +
                                     gainVector[f][6]*pow(theta[th]*PI/180.0, 6) +
                                     gainVector[f][7]*pow(theta[th]*PI/180.0, 7) +
                                     gainVector[f][8]*pow(theta[th]*PI/180.0, 8) +
                                     gainVector[f][9]*pow(theta[th]*PI/180.0, 9) +
                                     gainVector[f][10]*pow(theta[th]*PI/180.0, 10) +
                                     gainVector[f][11]*pow(theta[th]*PI/180.0, 11) +
                                     gainVector[f][12]*pow(theta[th]*PI/180.0, 12);

                    if (gain[f][p][th] > 0) {

                        gainDB[f][p][th] = 10*log10(gain[f][p][th]);

                    } else {

                        gainDB[f][p][th] = -80.000;

                    }


                    phase[f][p][th] = fmod((phaseVector[f][0]*pow(theta[th]*PI/180.0f, 0) +
                                  phaseVector[f][1]*pow(theta[th]*PI/180.0f, 1) +
                                  phaseVector[f][2]*pow(theta[th]*PI/180.0f, 2) +
                                  phaseVector[f][3]*pow(theta[th]*PI/180.0f, 3) +
                                  phaseVector[f][4]*pow(theta[th]*PI/180.0f, 4) +
                                  phaseVector[f][5]*pow(theta[th]*PI/180.0f, 5) +
                                  phaseVector[f][6]*pow(theta[th]*PI/180.0f, 6) +
                                  phaseVector[f][7]*pow(theta[th]*PI/180.0f, 7) +
                                  phaseVector[f][8]*pow(theta[th]*PI/180.0f, 8) +
                                  phaseVector[f][9]*pow(theta[th]*PI/180.0f, 9) +
                                  phaseVector[f][10]*pow(theta[th]*PI/180.0f, 10) +
                                  phaseVector[f][11]*pow(theta[th]*PI/180.0f, 11) +
                                  phaseVector[f][12]*pow(theta[th]*PI/180.0f, 12))*180/PI, 180);

                    outFile  << theta[th] << " \t " << phi[p] << " \t " << gainDB[f][p][th] << "     \t   " << gain[f][p][th] << "     \t    " << phase[f][p][th] << endl;

                }

                // Populate the gain and phase vectors for a spherical harmonic fit
                else if(dataType == DataType::sphHarm) {

                    gain[f][p][th] = gainVector[f][0]*(1/2.0)*(1/sqrt(PI)) +
                                     gainVector[f][1]*(1/2.0)*sqrt(3/PI)*cos(theta[th]*PI/180.0f) +
                                     gainVector[f][2]*(1/4.0)*sqrt(5/PI)*(3*pow(cos(theta[th]*PI/180.0f), 2)- 1) +
                                     gainVector[f][3]*(1/4.0)*sqrt(7/PI)*(5*pow(cos(theta[th]*PI/180.0f),3)- 3*cos(theta[th]*PI/180.0f)) +
                                     gainVector[f][4]*(3/16.0)*sqrt(1/PI)*(35*pow(cos(theta[th]*PI/180.0f),4) - 30*pow(cos(theta[th]*PI/180.0f),2)+3) +
                                     gainVector[f][5]*(1/16.0)*sqrt(11/PI)*(15*cos(theta[th]*PI/180.0f) - 70*pow(cos(theta[th]*PI/180.0f),3)+63*pow(cos(theta[th]*PI/180.0f),5)) +
                                     gainVector[f][6]*(1/32.0)*sqrt(13/PI)*(-5 + 105*pow(cos(theta[th]*PI/180.0f),2)-315*pow(cos(theta[th]*PI/180.0f),4) + 231*pow(cos(theta[th]*PI/180.0f),6)) +
                                     gainVector[f][7]*(1/32.0)*sqrt(15/PI)*(-35*cos(theta[th]*PI/180.0f)+ 315*pow(cos(theta[th]*PI/180.0f),3) -693*pow(cos(theta[th]*PI/180.0f),5) + 429*pow(cos(theta[th]*PI/180.0f),7)) +
                                     gainVector[f][8]*(1/256.0)*sqrt(17/PI)*(35 - 1260*pow(cos(theta[th]*PI/180.0f),2) + 6930*pow(cos(theta[th]*PI/180.0f),4) - 12012*pow(cos(theta[th]*PI/180.0f),6) + 6435*pow((cos(theta[th]*PI/180.0f)),8)) +
                                     gainVector[f][9]*(1/256.0)*sqrt(19/PI)*(315*cos(theta[th]*PI/180.0f)- 4620*pow(cos(theta[th]*PI/180.0f),3) + 18018*pow(cos(theta[th]*PI/180.0f),5) - 25740*pow(cos(theta[th]*PI/180.0f),7) + 12155*pow((cos(theta[th]*PI/180.0f)),9)) +
                                     gainVector[f][10]*(1/512.0)*sqrt(21/PI)*(-63 +3465*pow(cos(theta[th]*PI/180.0f),2) - 30030*pow(cos(theta[th]*PI/180.0f),4) + 90090*pow(cos(theta[th]*PI/180.0f),6) -109395*pow((cos(theta[th]*PI/180.0f)),8)+46189*pow(cos(theta[th]*PI/180.0f),10)) +
                                     gainVector[f][11]*(1/512.0)*sqrt(23/PI)*(-693*pow(cos(theta[th]*PI/180.0f),1) +15015*pow(cos(theta[th]*PI/180.0f),3) - 90090*pow(cos(theta[th]*PI/180.0f),5) +218790*pow((cos(theta[th]*PI/180.0f)),7)-230945*pow(cos(theta[th]*PI/180.0f),9)+88179*pow(cos(theta[th]*PI/180.0f),11)) +
                                     gainVector[f][12]*(1/2048.0)*sqrt(25/PI)*(231 -18018*pow(cos(theta[th]*PI/180.0f),2) +225225*pow(cos(theta[th]*PI/180.0f),4) - 1021020*pow(cos(theta[th]*PI/180.0f),6) +2078505*pow((cos(theta[th]*PI/180.0f)),8)-1939938*pow(cos(theta[th]*PI/180.0f),10)+676039*pow(cos(theta[th]*PI/180.0f),12));

                    if (gain[f][p][th] > 0) {

                        gainDB[f][p][th] = 10*log10(gain[f][p][th]);

                    } else {

                        gainDB[f][p][th] = -80.000;

                    }


                    phase[f][p][th] = fmod(gainVector[f][0]*(1/2.0)*(1/sqrt(PI)) +
                                     gainVector[f][1]*(1/2.0)*sqrt(3/PI)*cos(theta[th]*PI/180.0f) +
                                     gainVector[f][2]*(1/4.0)*sqrt(5/PI)*(3*pow(cos(theta[th]*PI/180.0f), 2) - 1) +
                                     gainVector[f][3]*(1/4.0)*sqrt(7/PI)*(5*pow(cos(theta[th]*PI/180.0f),3)- 3*cos(theta[th]*PI/180.0f)) +
                                     gainVector[f][4]*(3/16.0)*sqrt(1/PI)*(35*pow(cos(theta[th]*PI/180.0f),4) - 30*pow(cos(theta[th]*PI/180.0f),2)+3) +
                                     gainVector[f][5]*(1/16.0)*sqrt(11/PI)*(15*cos(theta[th]*PI/180.0f) - 70*pow(cos(theta[th]*PI/180.0f),3)+63*pow(cos(theta[th]*PI/180.0f),5)) +
                                     gainVector[f][6]*(1/32.0)*sqrt(13/PI)*(-5 + 105*pow(cos(theta[th]*PI/180.0f),2)-315*pow(cos(theta[th]*PI/180.0f),4) + 231*pow(cos(theta[th]*PI/180.0f),6)) +
                                     gainVector[f][7]*(1/32.0)*sqrt(15/PI)*(-35*cos(theta[th]*PI/180.0f)+ 315*pow(cos(theta[th]*PI/180.0f),3) -693*pow(cos(theta[th]*PI/180.0f),5) + 429*pow(cos(theta[th]*PI/180.0f),7)) +
                                     gainVector[f][8]*(1/256.0)*sqrt(17/PI)*(35 - 1260*pow(cos(theta[th]*PI/180.0f),2) + 6930*pow(cos(theta[th]*PI/180.0f),4) - 12012*pow(cos(theta[th]*PI/180.0f),6) + 6435*pow((cos(theta[th]*PI/180.0f)),8)) +
                                     gainVector[f][9]*(1/256.0)*sqrt(19/PI)*(315*cos(theta[th]*PI/180.0f)- 4620*pow(cos(theta[th]*PI/180.0f),3) + 18018*pow(cos(theta[th]*PI/180.0f),5) - 25740*pow(cos(theta[th]*PI/180.0f),7) + 12155*pow((cos(theta[th]*PI/180.0f)),9)) +
                                     gainVector[f][10]*(1/512.0)*sqrt(21/PI)*(-63 +3465*pow(cos(theta[th]*PI/180.0f),2) - 30030*pow(cos(theta[th]*PI/180.0f),4) + 90090*pow(cos(theta[th]*PI/180.0f),6) -109395*pow((cos(theta[th]*PI/180.0f)),8)+46189*pow(cos(theta[th]*PI/180.0f),10)) +
                                     gainVector[f][11]*(1/512.0)*sqrt(23/PI)*(-693*pow(cos(theta[th]*PI/180.0f),1) +15015*pow(cos(theta[th]*PI/180.0f),3) - 90090*pow(cos(theta[th]*PI/180.0f),5) +218790*pow((cos(theta[th]*PI/180.0f)),7)-230945*pow(cos(theta[th]*PI/180.0f),9)+88179*pow(cos(theta[th]*PI/180.0f),11)) +
                                     gainVector[f][12]*(1/2048.0)*sqrt(25/PI)*(231 -18018*pow(cos(theta[th]*PI/180.0f),2) +225225*pow(cos(theta[th]*PI/180.0f),4) - 1021020*pow(cos(theta[th]*PI/180.0f),6) +2078505*pow((cos(theta[th]*PI/180.0f)),8)-1939938*pow(cos(theta[th]*PI/180.0f),10)+676039*pow(cos(theta[th]*PI/180.0f),12))*180/PI,180);

                    outFile  << theta[th] << " \t " << phi[p] << " \t " << gainDB[f][p][th] << "     \t   " << gain[f][p][th] << "     \t    " << phase[f][p][th] << endl;

                } else {

                    return 1;

                }

            }
        }

        // float max = 0.0;
        // for(int i = 0; i < 37; i++){
        //     if(gain[f][0][i] > max){
        //         max = gain[f][0][i];
        //     }

        // }

        // double sum = 0.0;
        // for(int i = 0; i < 37; i++){
        //     sum = sum + (gain[f][0][i]*sin(theta[i]*PI/180.0f)/max);
        //     if(gain[f][0][i] > max){
        //         max = gain[f][0][i];
        //     }

        // }

        // float checkHPBW = 2*PI*sum*(5*PI/180)*max;
        // cout << "Max : " << max << "\tHPBW Check : "<< checkHPBW / (4*PI) << endl;

    }

    outFile << endl;

    return 0;
}

int necWrite(DataType dataType, int numChildren, vector<vector<vector<float>>> &gainVector, vector<vector<vector<float>>> &phaseVector, vector<float> &Veff)
{
    // Write all data to a unique chile NEC+ file
    // returns 0 on success
    // returns 1 if an error occurs

    /*

        INPUT ARGUMENTS :

        dataType = data-type (poly or sph. harm.) that will be used to reconstruct radiation pattern
        gainVector = gain coefficients to write
        phaseVector = phase coefficients to write
        Veff = effective volume to write

    */

    string txt = ".txt";
    string base;

    cout << "\n\nFinished GA, writing to output files\n\n";

    for(int i = 0; i < numChildren; i++) {

        base = "child_";

        ofstream outFile;
        base.append(to_string(i));
        base.append(txt);
        outFile.open(base);

        writeRadPattern(outFile, dataType, gainVector[i], phaseVector[i]);
        writeCoeff(outFile, dataType, gainVector[i], phaseVector[i]);
        writeInfo(outFile, Veff[i], 0, i);

        outFile.close();
        cout << "Child " << i+1 << "/" << numChildren << " complete" << endl;
    }

    return 0;
}

int writeHist(ofstream &histFile, vector<float> &Veff)
{
    for(int i = 0; i < Veff.size(); i++){
        if(i < Veff.size()-1){
            histFile << Veff[i] << ",";
        } else {
            histFile << Veff[i];
        }
    }


    histFile << endl;

    return 0;

}