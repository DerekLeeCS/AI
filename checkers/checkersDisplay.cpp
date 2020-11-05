#include "checkers.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <wincon.h>

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::make_shared;
using std::make_tuple;

using namespace checkersVals;

#define FOREGROUND_CYAN		        (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_MAGENTA		    (FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_WHITE   	        (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)

///////////////////////////////////// Colored Output /////////////////////////////////////

// Code from:
// https://stackoverflow.com/questions/25559077/how-to-get-background-color-back-to-previous-color-after-use-of-std-handle
void SetConsoleColour( WORD* Attributes, DWORD Colour ) {

    CONSOLE_SCREEN_BUFFER_INFO Info;
    HANDLE hStdout = GetStdHandle( STD_OUTPUT_HANDLE );
    GetConsoleScreenBufferInfo( hStdout, &Info );
    *Attributes = Info.wAttributes;
    SetConsoleTextAttribute( hStdout, Colour );

}

void ResetConsoleColour( WORD Attributes ) {

    SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE), Attributes );

}

// Loads a board from a specified file
void board::loadBoard() {

    string fileName;
    bool validName = false;

    while ( !validName ) {

        cout << "Enter the name of the file containing the board: ";
        cin >> fileName;
        cout << endl;

        if ( this->validateInput() )
            continue;
        else
            validName = true;

    }

    ifstream input( fileName );
    int pieceNum, row = 0, col = 1;
    tuple<int,int> tempLoc;
    shared_ptr<piece> tempPiece;

    // Load the board
    while ( input >> pieceNum ) {

        tempLoc = make_tuple( row, col );

        switch ( pieceNum ) {

        // Empty Piece
        case 0:

            tempPiece = this->emptyPiece;
            break;

        // Regular Piece
        default:

            tempPiece = make_shared<piece>( piece( pieceNum<=2, pieceNum%2 ) );
            tempPiece->loc = tempLoc;

        }


        // Ensure row and col are still valid
        if ( validLoc( row ) && validLoc( col ) )
            this->gameboard[ row ][ col ] = tempPiece;
        else {

            std::cerr << "Specified board is invalid. Current location: " << row;
            exit( EXIT_FAILURE );

        }

        // Update next square
        if ( col != 6 && col != 7 )
            col += 2;
        else {

            row++;
            if ( row % 2 )
                col = 0;
            else
                col = 1;

        }

    }

}

///////////////////////////////////// Display Functions /////////////////////////////////////

// Prints a victory message
void board::printVictory( bool color, bool noMoves ) {

    string colorText, oppositeColorText;

    if ( color == COLOR_RED_VAL ) {

        colorText = "Red";
        oppositeColorText = "White";

    }
    else {

        colorText = "White";
        oppositeColorText = "Red";

    }

    if ( noMoves )
        cout << "There are no remaining moves for " << oppositeColorText << "." << "\n";
    cout << colorText << " Wins!" << endl;

    exit( EXIT_SUCCESS );

}


// Prints the start menu
void board::printStart() {

    bool start = false;
    bool validOption;
    int option;

    cout << "\n";
    cout << "******************** Checkers ********************" << "\n" << "\n";
    cout << "Welcome!" << "\n";
    cout << "You play White." << "\n" << "\n";

    while ( !start ) {

        cout << "1 = Start" << "\n";
        cout << "2 = Settings" << "\n";
        cout << "3 = Quit Game" << "\n" << endl;

        validOption = false;
        option = 0;

        while ( !validOption ) {

            cin >> option;
            cout << endl;

            if ( validateInput() )
                continue;

            if ( 1 <= option && option <= 3 )
                validOption = true;

            if ( !validOption )
                printError();

        }

        switch ( option ) {

        case 1:

            start = true;
            break;

        case 2:

            printSettings();
            break;

        case 3:

            cout << "Bye!" << endl;
            exit( EXIT_SUCCESS );

        }

    }

    printTimeSettings();

    // Load piece counts
    for ( int i=0; i<8; i++ ) {

        for ( int j=0; j<8; j++ ) {

            if ( gameboard[i][j]->filler == FILLER_FALSE && gameboard[i][j]->type != TYPE_EMPTY_VAL ) {

                checkMoves( gameboard[i][j] );
                gameboard[i][j]->updateCount( *this, true );
                if( gameboard[i][j]->color == COLOR_RED_VAL )
                    redPieces.insert(gameboard[i][j]);
                else
                    whitePieces.insert(gameboard[i][j]);

            }

        }

    }

    cout << "------------------- Game Begin -------------------" << "\n" << endl;
    printBoard();

    this->heuristic();
    if ( this->terminalState( this->score ) )
        endTurn();

}


// Prints the settings menu
void board::printSettings() {

    bool validOption = false;
    int option = 0;

    cout << "1 = Change player settings" << "\n";
    cout << "2 = Change starting board" << "\n";
    cout << "3 = Back" << "\n" << endl;

    while ( !validOption ) {

        cin >> option;
        cout << endl;

        if ( validateInput() )
            continue;

        if ( 1 <= option && option <= 3 )
            validOption = true;

        if ( !validOption )
            printError();

    }

    switch ( option ) {

    case 1:

        printPlayerSettings();
        break;

    case 2:

        printPieceSettings();
        break;

    case 3:

        return;

    }

}


// Prints the player settings menu
void board::printPlayerSettings() {

    bool validOption;
    int option;

    while (1) {

        validOption = false;
        option = 0;

        cout << "1 = Red Goes First" << "\n";
        cout << "2 = White Goes First" << "\n";
        cout << "3 = Player vs. Computer" << "\n";
        cout << "4 = Computer vs. Computer" << "\n";
        cout << "5 = Back" << "\n" << "\n";

        cout << "Current Settings: ";
        if ( redTurn )
            cout << "1";
        else
            cout << "2";

        cout << " & ";

        if ( AIvsAI )
            cout << "4";
        else
            cout << "3";

        cout << "\n" << endl;

        while ( !validOption ) {

            cin >> option;
            cout << endl;

            if ( validateInput() )
                continue;

            if ( 1 <= option && option <= 5 )
                validOption = true;

            if ( !validOption )
                printError();

        }

        switch ( option ) {

        case 1:

            this->redTurn = true;
            break;

        case 2:

            this->redTurn = false;
            break;

        case 3:

            this->AIvsAI = false;
            break;

        case 4:

            this->AIvsAI = true;
            break;

        case 5:

            return;

        }

    }

}


// Change starting board
void board::printPieceSettings() {

    bool validOption;
    int option;

    while (1) {

        validOption = false;
        option = 0;

        printBoard();
        cout << "1 = Add a piece" << "\n"
             << "2 = Remove a piece" << "\n"
             << "3 = Load a board from a file" << "\n"
             << "4 = Back" << "\n" << endl;

        while ( !validOption ) {

            cin >> option;
            cout << endl;

            if ( validateInput() )
                continue;

            if ( 1 <= option && option <= 4 )
                validOption = true;

            if ( !validOption )
                printError();

        }

        switch ( option ) {

        case 1:

            printAddPiece();
            break;

        case 2:

            printRemovePiece();
            break;

        case 3:

            loadBoard();
            break;

        case 4:

            return;

        }

    }

}


// Handles adding a piece
void board::printAddPiece() {

    int row,col;
    string loc;
    bool validSq = false;

    cout << "Select an empty square: " << "\n";
    cout << "(Any square with a '.')" << "\n";
    cout << "(Format should be ColumnRow, e.g. a1)" << "\n" << endl;

    while ( !validSq ) {

        cin >> loc;
        cout << endl;

        if ( validateInput() )
            continue;

        row = int(loc[0]) - 97;
        col = loc[1] - '1';

        if ( 0 <= col && col <= 7 && 0 <= row && row <= 7 ) {

            if( gameboard[row][col]->filler == FILLER_FALSE && gameboard[row][col]->type == TYPE_EMPTY_VAL )
                validSq = true;
            else {

                cout << "Error: Invalid Square" << "\n" << endl;
                continue;

            }

        }

        if( !validSq )
            printError();

    }

    int pieceType = 0;
    bool validType = false;

    cout << "Select a piece to add: " << "\n";
    cout << "1 = White King" << "\n";
    cout << "2 = White Man" << "\n";
    cout << "3 = Red King" << "\n";
    cout << "4 = Red Man" << "\n";
    cout << "5 = Back" << "\n" << endl;

    while ( !validType ) {

        cin >> pieceType;
        cout << endl;

        if ( validateInput() )
            continue;

        if ( 1 <= pieceType && pieceType <= 4 ) {

            validType = true;
            gameboard[row][col] = make_shared<piece> ( piece(pieceType<=2, pieceType%2) );
            gameboard[row][col]->loc = make_tuple( row, col );

        }
        else if ( pieceType == 5 )
            return;

    }

}


// Handles removing a piece
void board::printRemovePiece() {

    int row,column;
    string loc;
    bool validSq = false;

    cout << "Select an occupied square:" << "\n";
    cout << "(Any square with a number)" << "\n";
    cout << "(Format should be ColumnRow, e.g. b1)" << "\n" << endl;

    while ( !validSq ) {

        cin >> loc;
        cout << endl;

        if ( validateInput() )
            continue;

        row = int(loc[0]) - 97;
        column = loc[1] - '1';

        if ( 0 <= column && column <= 7 && 0 <= row && row <= 7 ) {

            if ( gameboard[row][column]->filler == FILLER_FALSE && gameboard[row][column]->type != TYPE_EMPTY_VAL ) {

                gameboard[row][column] = emptyPiece;
                validSq = true;

            }
            else {

                cout << "Error: Invalid Square" << "\n" << endl;
                continue;

            }

        }

        if ( !validSq )
            printError();

    }

}


// Change computing time
void board::printTimeSettings() {

    int inputTime = 0;

    cout << "Please enter a valid time in seconds:" << "\n";
    cout << "(Only a postive integer greater than or equal to 5 will be accepted)" << "\n" << endl;

    while ( inputTime < 5 ) {

        cin >> inputTime;
        cout << endl;

        if ( validateInput() )
            continue;

        if ( inputTime < 5 )
            printError();
        else
          computerTime = inputTime;

    }

}


// Prints the current board
void board::printBoard() {

    string man = "1";
    string king = "2";

    WORD Attributes = 0;

    // Prints column labels
    cout << "  " << "   ";
    for ( int i=0; i<8; i++ ) {

        SetConsoleColour(&Attributes, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
        cout << i+1 << "  ";
        ResetConsoleColour(Attributes);

    }

    cout << endl;

    piece tempPiece;

    for ( int i=0; i<8; i++ ) {

        // Prints row labels
        SetConsoleColour( &Attributes, FOREGROUND_INTENSITY | FOREGROUND_GREEN );
        cout << "  " << char(97+i) << "  ";
        ResetConsoleColour( Attributes );

        for ( int j=0; j<8; j++ ) {

            tempPiece = *gameboard[i][j];
            if ( tempPiece.filler == FILLER_TRUE )
                cout << "   ";
            else {

                if ( tempPiece.type == TYPE_MAN_VAL ) { // Man

                    if ( tempPiece.color == COLOR_RED_VAL ) // Red
                        SetConsoleColour( &Attributes, FOREGROUND_INTENSITY | FOREGROUND_RED );
                    else if ( tempPiece.color == COLOR_WHITE_VAL ) // White
                        SetConsoleColour(&Attributes, FOREGROUND_WHITE);

                    cout << man << "  ";

                }
                else if ( tempPiece.type == TYPE_KING_VAL ) { // King

                    if ( tempPiece.color == COLOR_RED_VAL ) // Red
                        SetConsoleColour( &Attributes, FOREGROUND_INTENSITY | FOREGROUND_RED );
                    else if ( tempPiece.color == COLOR_WHITE_VAL ) // White
                        SetConsoleColour( &Attributes, FOREGROUND_WHITE );

                    cout << king << "  ";

                }
                if ( tempPiece.type == TYPE_EMPTY_VAL ) { // Empty

                    SetConsoleColour( &Attributes, FOREGROUND_INTENSITY | FOREGROUND_CYAN );
                    cout << "." << "  ";

                }

                ResetConsoleColour( Attributes );

            }

        }

        cout << "\n";

    }

    cout << endl;

}


// Prints a list of commands available
void board::printHelp() {

    cout << "###################### Help ######################" << "\n"
         << "Game Pieces:" << "\n"
         << "1 = Man" << "\n"
         << "2 = King" << "\n" << "\n";

    cout << "Move Descriptions:" << "\n"
         << "1 = Move Up Left" << "\n"
         << "2 = Move Up Right" << "\n"
         << "3 = Move Down Left" << "\n"
         << "4 = Move Down Right" << "\n"
         << "5 = Jump Up Left" << "\n"
         << "6 = Jump Up Right" << "\n"
         << "7 = Jump Down Left" << "\n"
         << "8 = Jump Down Right" << "\n"
         << "##################################################" << "\n"
         << "\n" << endl;

}


// Prints an error message
void board::printError() {

    cout << "Error: Invalid Input" << "\n" << endl;

}


// Prints an error message with help
void board::printMoveError() {

    cout << "Error: Invalid Input" << "\n";
    cout << "Type 'help' to bring up the help menu." << "\n";
    cout << "Type 'board' to see the board again." << "\n" << endl;

}
