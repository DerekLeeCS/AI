#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <iomanip>


using std::string;
using std::ofstream;
using std::stringstream;
using std::cout;
using std::cin;


#define NUM_INPUTS 9
#define NUM_OUTPUTS 1


void trimPrecision( ofstream &, double );


int main() {

    string outFile;
    int numHidNodes;

    cout << "Enter the name of the output file: ";
    cin >> outFile;
    ofstream output( outFile );

    cout << "Enter the number of hidden nodes: ";
    cin >> numHidNodes;

    output << NUM_INPUTS << " " << numHidNodes << " " << NUM_OUTPUTS << "\n";

    // Randomly generate weights
    std::default_random_engine generator;
    std::uniform_real_distribution<double> unif( 0, 1 );
    double randNum;

    for ( int i=0; i<numHidNodes; i++ ) {

        for ( int j=0; j<NUM_INPUTS+1; j++ ) {

            randNum = unif( generator );
            trimPrecision( output, randNum );

        }

        output << "\n";

    }

    for ( int i=0; i<NUM_OUTPUTS; i++ ) {

        for ( int j=0; j<numHidNodes+1; j++ ) {

            randNum = unif( generator );
            trimPrecision( output, randNum );

        }

        output << "\n";

    }

    output.close();
    return 0;

}


// Trims floating-points to 3 digits after the decimal
void trimPrecision( ofstream &output, double number ) {

    stringstream stream;
    stream << std::fixed << std::setprecision(3) << number;
    output << stream.str() << " ";

}
