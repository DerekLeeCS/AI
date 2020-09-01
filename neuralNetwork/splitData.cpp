#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <cstdlib>

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;

#define PERCENT 25
#define NUM_INPUTS 9
#define NUM_OUTPUTS 1

template <class T>
void checkValidStream( T &, string );
void addToFile( ofstream &, string, int &, int, bool & );

int main() {

    string inFile, trainFile, testFile;
    vector<string> fileContents;

    cout << "Enter the name of the Input file: ";
    cin >> inFile;
    ifstream input( inFile );
    checkValidStream( input, inFile );


    cout << "Enter the name of the Training file: ";
    cin >> trainFile;
    ofstream outputTrain( trainFile );
    checkValidStream( outputTrain, trainFile );


    cout << "Enter the name of the Test file: ";
    cin >> testFile;
    ofstream outputTest( testFile );
    checkValidStream( outputTest, testFile );

    string line;

    // Counts number of lines
    while ( std::getline(input,line) ) {

        if ( line.length() > 0 )
            fileContents.push_back( line );

    }

    input.close();

    int lineCount = fileContents.size();

    // Randomize the data
    std::default_random_engine generator;
    std::shuffle( fileContents.begin(), fileContents.end(), generator );

    // Number of data for Test set
    int testNum = lineCount * PERCENT / 100;

    // Number of data for Train set
    int trainNum = lineCount - testNum;

    // Count of number of data currently in each set
    int testCount=0, trainCount=0;

    // Keeps track of whether or not the line was added to a file yet
    bool added;

    // Outputs number of examples, number of input nodes, number of output nodes
    outputTrain << trainNum << " " << NUM_INPUTS << " " << NUM_OUTPUTS << "\n";
    outputTest << testNum << " " << NUM_INPUTS << " " << NUM_OUTPUTS << "\n";

    for ( auto iter : fileContents ) {

        line = iter;

        // Skips blank lines
        if(line.length() == 0)
            continue;

        added = false;

        addToFile(outputTrain,line,trainCount,trainNum,added);  // Training Dataset
        addToFile(outputTest,line,testCount,testNum,added);     // Testing Dataset

    }

    /*
    cout << trainCount << " " << trainNum << "\n"
         << testCount << " " << testNum << endl;
    */
    input.close();
    outputTest.close();
    outputTrain.close();

    return 0;

}


template <class T>
void checkValidStream( T &stream, string fileName ) {

    if ( !stream ) {

        cerr << "Error: could not open " << fileName << "\n";
        exit( EXIT_FAILURE );

    }

}


void addToFile( ofstream &stream, string line, int &curLines, int maxLines, bool &added ) {

    if ( added )
        return;

    if ( curLines != maxLines ) {

        stream << line << endl;
        curLines++;
        added = true;

    }
    else
        added = false;

}
