#include "checkers.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <windows.h>
#include <wincon.h>


using std::make_tuple;
using std::get;
using std::tie;
using std::remove;
using std::max;
using std::min;
using std::make_shared;
using std::cin;
using std::cout;
using std::endl;


using namespace checkersVals;


#define DEBUG_BOOL                  0   // If debugging, 1; Otherwise, 0
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

int states = 0;         // Used during debugging to check how many states minimax searched through

// If there is only one valid move, make it immediately
// Used during minimax search to check if there is a single move available
bool singleMove = true;



///////////////////////////////////// Piece /////////////////////////////////////

// Declaration constructor
board::piece::piece() {

}


// Regular constructor
board::piece::piece( bool color, int type ) {

    this->color = color;
    this->type = type;
    filler = false;

}


// Filler constructor
// Piece class for sections of board that are not supposed to be used
board::piece::piece( bool filler ) {

    this->filler = filler;

}


// Copy constructor
board::piece::piece( const piece &copyPiece ) {

    this->color = copyPiece.color;
    this->type = copyPiece.type;
    this->filler = copyPiece.filler;

    this->validMove = copyPiece.validMove;
    this->validJump = copyPiece.validJump;

    this->moves = copyPiece.moves;
    this->jumps = copyPiece.jumps;

    this->loc = copyPiece.loc;

}


// Updates piece counts
//      Increment = 1 for increaseCount
//      Increment = 0 for decreaseCount
void board::piece::updateCount( board &owner, bool increment ) {

    int change;
    if ( increment )
        change = 1;
    else
        change = -1;

    // Gets row and column of piece
    int row,col;
    tie( row, col ) = this->loc;

    // Updates counts
    if ( this->color == COLOR_RED_VAL ) {

        if ( this->type == TYPE_KING_VAL )
            owner.redKings += change;
        else
            owner.redMen += change;

    }
    else {

        if ( this->type == TYPE_KING_VAL )
            owner.whiteKings += change;
        else
            owner.whiteMen += change;

    }

}


// Removes piece from board vectors and decrements piece counts
void board::piece::clearPiece( board &owner ) {

    // Gets row and column of piece
    int row,col;
    tie( row, col ) = this->loc;

    // Removes piece from board move/jump sets
    if ( this->validMove ) {

        if ( this->color == COLOR_RED_VAL )
            owner.redMoves.erase( owner.gameboard[row][col] );
        else
            owner.whiteMoves.erase( owner.gameboard[row][col] );

    }

    if ( this->validJump ) {

        if( this->color == COLOR_RED_VAL )
            owner.redJumps.erase( owner.gameboard[row][col] );
        else
            owner.whiteJumps.erase( owner.gameboard[row][col] );

    }

    if ( this->color == COLOR_RED_VAL )
        owner.redPieces.erase( owner.gameboard[row][col] );
    else
        owner.whitePieces.erase( owner.gameboard[row][col] );

    // Decrements piece counts
    this->updateCount( owner, false );

}


// Resets a piece to default values
void board::piece::resetPiece() {

    this->validMove = false;
    this->validJump = false;

    this->moves.clear();
    this->jumps.clear();

}


// Checks if a piece should be promoted
bool board::piece::checkPromotion( board &owner ) {

    int row = get<0>(this->loc);
    bool promo = false;

    // Only continues if piece is a man
    if ( this->type == TYPE_MAN_VAL ) {

        // If piece should be promoted, update board counts
        if ( this->color == COLOR_WHITE_VAL && row == 0 ) {

            owner.whiteMen--;
            owner.whiteKings++;
            promo = true;

        }
        else if ( this->color == COLOR_RED_VAL && row == 7 ) {

            owner.redMen--;
            owner.redKings++;
            promo = true;

        }

    }

    // If valid promotion, piece becomes king
    if ( promo ) {

        this->type = TYPE_KING_VAL;
        return true;

    }
    else
        return false;

}


// Returns list of valid actions for a piece
list< tuple<int,int> >* board::piece::returnActions() {

    list< tuple<int,int> > *possibleActions;

    // If piece has a validJump, returns jump list
    if ( this->validJump )
        possibleActions = &jumps;
    // Else, returns move list
    else
        possibleActions = &moves;

    return possibleActions;

}


// Inserts piece into board move set
void board::piece::insertMove( board &owner ) {

    // Gets row and column of piece
    int row,col;
    tie( row, col ) = this->loc;

    this->validMove = true;

    if ( this->color == COLOR_RED_VAL )
        owner.redMoves.insert( owner.gameboard[row][col] );
    else
        owner.whiteMoves.insert( owner.gameboard[row][col] );

}


// Removes piece from board move set
void board::piece::removeMove( board &owner ) {

    // Gets row and column of piece
    int row,col;
    tie( row, col ) = this->loc;

    this->validMove = false;

    if ( this->color == COLOR_RED_VAL )
        owner.redMoves.erase( owner.gameboard[row][col] );
    else
        owner.whiteMoves.erase( owner.gameboard[row][col] );

}


// Inserts piece into board jump set
void board::piece::insertJump( board &owner ) {

    // Gets row and column of piece
    int row,col;
    tie( row, col ) = this->loc;

    this->validJump = true;

    if ( this->color == COLOR_RED_VAL )
        owner.redJumps.insert( owner.gameboard[row][col] );
    else
        owner.whiteJumps.insert( owner.gameboard[row][col] );

}


// Removes piece from board jump set
void board::piece::removeJump( board &owner ) {

    // Gets row and column of piece
    int row,col;
    tie( row, col ) = this->loc;

    this->validJump = false;

    if ( this->color == COLOR_RED_VAL )
        owner.redJumps.erase( owner.gameboard[row][col] );
    else
        owner.whiteJumps.erase( owner.gameboard[row][col] );

}


// Checks if piece can move in the direction given by rowOffset
// For white men:
//      rowOffset < 0 for move up
// For red men:
//      rowOffset > 0 for move down
bool board::piece::validDirection( int rowOffset ) {

    if ( this->type == TYPE_KING_VAL || ( this->color == COLOR_RED_VAL && rowOffset > 0 ) || ( this->color == COLOR_WHITE_VAL && rowOffset < 0 ) )
        return true;
    else
        return false;

}


///////////////////////////////////// Board /////////////////////////////////////

// Creates the default board
board::board() {

    // Board uses a single instance of emptyPiece and fillerPiece
    //      for all empty and filler locations in the board
    emptyPiece = make_shared<piece> ( piece(0,TYPE_EMPTY_VAL) );
    fillerPiece = make_shared<piece> ( piece(FILLER_TRUE) );

    tuple<int,int> tempLoc;

    // Loops through the entire board
    for ( int i=0; i<8; i++ ) {

        for ( int j=0; j<8; j++ ) {

            tempLoc = make_tuple(i,j);  // Stores location of piece

            // Red occupies first 3 rows
            if ( i <= 2 ) {

                // Checks if valid location
                if ( (i+j) % 2 ) {

                    // Initializes piece
                    gameboard[i][j] = make_shared<piece> ( piece(COLOR_RED_VAL,TYPE_MAN_VAL) ); // Red Man
                    gameboard[i][j]->loc = tempLoc;

                }
                else
                    gameboard[i][j] = fillerPiece; // Filler

            }
            // White occupies last 3 rows
            else if ( i >= 5 ) {

                // Checks if valid location
                if ( (i+j) % 2 ) {

                    // Initializes piece
                    gameboard[i][j] = make_shared<piece> ( piece(COLOR_WHITE_VAL,TYPE_MAN_VAL) ); // White Man
                    gameboard[i][j]->loc = tempLoc;
                    //whitePieces.insert( gameboard[i][j] );

                }
                else
                    gameboard[i][j] = fillerPiece; // Filler

            }
            // Middle 2 rows are empty
            else {

                // Checks if valid location
                if ( (i+j) % 2 )
                    gameboard[i][j] = emptyPiece;
                else
                    gameboard[i][j] = fillerPiece; // Filler

            }

        }

    }

}


// Used for testing
// Alters the initial board
void board::specialBoard() {
    /*
    // Empty Board
    for(int i=0; i<8; i++)
    {
        for(int j=0; j<8; j++)
        {
            if( (i+j) % 2 == 0 )
                gameboard[i][j] = fillerPiece; // Filler
            else
                gameboard[i][j] = emptyPiece;
        }
    }
    */

    // Computer make double jump choice
    gameboard[3][0] = gameboard[1][0];
    gameboard[3][0]->loc = make_tuple(3,0);
    gameboard[1][0] = emptyPiece;
    gameboard[4][5] = gameboard[2][3];
    gameboard[4][5]->loc = make_tuple(4,5);
    gameboard[2][3] = emptyPiece;

    gameboard[4][1] = gameboard[6][1];
    gameboard[4][1]->loc = make_tuple(4,1);
    gameboard[6][1] = emptyPiece;
    gameboard[4][7] = gameboard[6][5];
    gameboard[4][7]->loc = make_tuple(4,7);
    gameboard[6][5] = emptyPiece;

    /*
    // Double Jump
    gameboard[4][5] = gameboard[1][4];
    gameboard[4][5]->loc = make_tuple(4,5);
    gameboard[1][4] = emptyPiece;
    */
    /*
    // Two Possible Jumps
    gameboard[3][0] = gameboard[2][1];
    gameboard[3][0]->loc = make_tuple(3,0);
    //gameboard[3][4] = gameboard[2][5];
    gameboard[3][6] = gameboard[2][7];
    gameboard[3][6]->loc = make_tuple(3,6);
    gameboard[2][1] = emptyPiece;
    gameboard[1][2] = emptyPiece;
    gameboard[0][3] = emptyPiece;
    gameboard[1][4] = emptyPiece;
    //gameboard[2][5] = emptyPiece;
    gameboard[2][7] = emptyPiece;

    gameboard[4][3] = gameboard[5][2];
    gameboard[4][3]->loc = make_tuple(4,3);
    //gameboard[4][7] = gameboard[5][6];
    gameboard[5][2] = emptyPiece;
    //gameboard[5][6] = emptyPiece;
    gameboard[6][3] = emptyPiece;
    gameboard[6][7] = emptyPiece;
    */

}


// Plays the game
void board::playGame() {

    printStart();   // Prints the start menu

    // Infinite loop until an end state is reached
    while(1) {

        // If game is between 2 computers
        if ( AIvsAI )
            computerMove();
        // Game is between player and computer
        else {

            // Computer is red
            // If red's turn, computer moves
            if ( redTurn )
                computerMove();
            else
                playerMove();

        }

    }

}


// Handles actions for computer
void board::computerMove() {

    cout << "Computer is thinking..." << "\n" << endl;

    // Stores bestMoves when a search to a depth has been fully completed
    list< tuple< tuple<int,int>, tuple<int,int> > > futureMoves;

    // Resets stored moves
    this->moves.clear();
    this->bestMoves.clear();

    // Variables for minimax search
    this->startTime = std::chrono::system_clock::now(); // Keeps track of elapsed time
    this->maxDepth = 1;
    float futureScore, tempScore = -12345;
    states = 0;
    singleMove = true;  // Updated during minimax search
                        // Do not need to reset to true during each iteration b/c
                        //      If false, will remain false for all future iterations
                        //      If true, will break out of iterative deepening

    // Iterative deepening
    while (1) {

        // Maximizing player if Red
        // Minimizing player if White
        tempScore = this->minimax( *this, 0, this->redTurn, VAL_MIN, VAL_MAX );

        // Stops iterative deepening if elapsed time becomes close to max time
        if ( tempScore == TIME_LIMIT_EXCEEDED )
            break;

        // Only updates if a search was fully completed
        futureMoves = this->bestMoves;
        futureScore = tempScore;    // Used for debugging

        // If there is only one move
        if( tempScore == SINGLE_MOVE )
            break;

        // If reached end of game
        if( terminalState( tempScore ) )
            break;

        this->maxDepth++;

    }

    // Used to calculate time taken
    this->endTime = std::chrono::system_clock::now();
    this->elapsed_seconds = this->endTime - this->startTime;

    this->bestMoves = futureMoves;

    // Used for debugging
    if ( DEBUG_BOOL ) {

        // Outputs a list of actions leading to optimal state
        for( auto iter : this->bestMoves )
            cout << char(get<0>(get<0>(iter))+97) << get<1>(get<0>(iter))+1 << " " << char(get<0>(get<1>(iter))+97) << get<1>(get<1>(iter))+1 << "\n";

        cout << "Future Score: " << futureScore << "\n"
             << "Number of States: " << states << "\n" << "\n";

    }

    bool multiJump = true;

    // Performs actions from list of best actions
    // Will loop if another jump is available
    //      E.g. If action is a single move,
    //           will not loop
    while ( multiJump ) {

        multiJump = moveResult( get<0>( this->bestMoves.front() ), get<1>( this->bestMoves.front() ) );
        this->bestMoves.pop_front(); // Removes action that was just performed
        printBoard();

    }

    // Required statistics
    cout << "Maximum Depth: " << this->maxDepth << "\n"
         << "Time Taken: " << ( this->elapsed_seconds ).count() << endl;


    endTurn();

}


// Handles player actions
void board::playerMove() {

    unordered_set< shared_ptr<piece> > possiblePieces;
    vector< tuple<int,int> > pieceLocs;
    shared_ptr<piece> tempPiece;
    int curRow, curCol, tempRow, tempCol, option;
    vector< tuple< tuple<int,int>, tuple<int,int> > > optionVec;
    tuple<int,int> oldLoc, newLoc;
    string input;

    bool multiJump = true;

    while ( multiJump ) {

        // Gets pieces that can take a valid action
        possiblePieces = *( this->returnPieces() );

        // Cleans up the list for the player
        for ( auto iter : possiblePieces )
            pieceLocs.push_back( iter->loc );
        sort( pieceLocs.begin(), pieceLocs.end() );

        bool validOption = false;

        // While player has not selected a valid action
        while ( !validOption ) {

            cout << "Select an option:" << "\n";

            int i=0;

            // Outputs the list
            for ( auto iter : pieceLocs ) {

                curRow = get<0>(iter);
                curCol = get<1>(iter);
                tempPiece = gameboard[ curRow ][ curCol ];

                for ( auto iter2 : *( tempPiece->returnActions() ) ) {

                    i++;
                    tempRow = get<0>(iter2);
                    tempCol = get<1>(iter2);

                    optionVec.push_back( make_tuple( make_tuple( curRow, curCol ), make_tuple( tempRow, tempCol ) ) );
                    cout << i << ": " << char( curRow+97 ) << curCol+1 << " -> " << char( tempRow+97 ) << tempCol+1 << "\n";

                }

            }

            cout << endl;

            // Waits for player input
            cin >> input;
            cout << endl;

            // Checks for valid input
            if ( validateInput() )
                continue;

            // Checks for specific commands
            if ( input == "help" ) {

                printHelp();
                continue;

            }
            else if ( input == "board" ) {

                printBoard();
                continue;

            }

            // Gets action from input
            try {

                option = std::stoi(input);

            } catch( ... ) {

                option = 0;

            }

            if ( 1 <= option && option <= i ) {

                tie( oldLoc,newLoc ) = optionVec[option-1];
                validOption = true;

            }

            if ( !validOption )
                printMoveError();

        }

        multiJump = moveResult( oldLoc, newLoc );
        printBoard();

        // Resets for multijump
        if ( multiJump ) {

            pieceLocs.clear();
            optionVec.clear();
            cout << "Another jump is available!" << endl;

        }

    }

    endTurn();

}


// Performs a set of actions at the end of the turn
void board::endTurn() {

    heuristic();    // Calculates score for the current state

    // Checks if score represents a terminal state
    if ( terminalState(this->score) ) {

        // Outputs terminal state message
        if ( this->score == VICTORY_RED_MOVE )
            printVictory( COLOR_RED_VAL, true );
        else if ( this->score == VICTORY_RED_PIECE )
            printVictory( COLOR_RED_VAL, false );
        else if ( this->score == VICTORY_WHITE_MOVE )
            printVictory( COLOR_WHITE_VAL, true );
        else if ( this->score == VICTORY_WHITE_PIECE )
            printVictory( COLOR_WHITE_VAL, false );

    }

    // Updates turn
    redTurn = !redTurn;
    turnCount++;

    // Prints statistics for board
    // Used for debugging
    if ( DEBUG_BOOL ) {

        cout << "Score (for Red): " << this->score << endl;
        cout << "Red Men: " << redMen << "\n";
        cout << "Red King: " << redKings << "\n";
        cout << "Red Last: " << redLast << "\n";
        cout << "Red Pieces: " << redPieces.size() << "\n";
        cout << "White Men: " << whiteMen << "\n";
        cout << "White King: " << whiteKings << "\n";
        cout << "White Last: " << whiteLast << "\n";
        cout << "White Pieces: " << whitePieces.size() << "\n";

    }

    string color;
    if ( redTurn )
        color = "Red";
    else
        color = "White";

    cout << "--------------------------------------------------" << "\n" << "\n";
    cout << "Turn " << turnCount << ": " << color << " to move" << endl;

}


// Handles alpha-beta pruning minimax search
float board::minimax( board &originalBoard, int depth, bool maxPlayer, float alpha, float beta ) {

    // Used for debugging (and because I was curious)
    // Counts number of states visited
    if ( DEBUG_BOOL )
        states++;

    // Updates elapsed time and returns if time limit is exceeded
    this->endTime = std::chrono::system_clock::now();
    elapsed_seconds = this->endTime - this->startTime;

    if ( this->computerTime - elapsed_seconds.count() < REMAINING_TIME_LIMIT )
        return TIME_LIMIT_EXCEEDED;

    // Reached max depth and starts returning from recursion
    if ( depth == originalBoard.maxDepth ) {

        originalBoard.heuristic();                      // Calculates score for current state
        originalBoard.bestMoves = originalBoard.moves;  // Sets bestMoves equal to moves taken to reach current state

        return originalBoard.score;  // Returns score for alpha-beta pruning

    }

    // Makes a copy of the parent board
    board tempBoard = originalBoard;
    unordered_set< shared_ptr<piece> > posMoves = *( tempBoard.returnPieces() );
    list< tuple<int,int> > *possibleActions;

    bool multiJump,newPath;
    float val,bestVal;
    int randNum;

    if ( maxPlayer )
        bestVal = VAL_MIN;
    else
        bestVal = VAL_MAX;

    // First check for singleMove
    if ( depth == 0 ) {

        if( posMoves.size() != 1 )
            singleMove = false;

    }

    for ( auto iter : posMoves ) {

        possibleActions = iter->returnActions();

        // Second check for singleMove
        if ( depth == 0 ) {

            if( possibleActions->size() != 1 )
                singleMove = false;

        }

        for ( auto iter2 : *possibleActions ) {

            // Because board class contains pointers to pieces, copying board class copies the pointers
            // Does not make copies of pieces, so pointers will still point to original pieces
            // Need to "isolate" tempBoard from originalBoard because operations on tempBoard will
            //      affect pieces of originalBoard through pointers
            tempBoard.isolateBoard( iter->loc, iter2 );

            // Creates a tuple containing piece's old location and new location
            // Adds to moves taken to reach current state
            tempBoard.moves.push_back( make_tuple( iter->loc, iter2 ) );
            multiJump = tempBoard.moveResult( iter->loc, iter2 );

            // Final check for singleMove
            //      Only applies if it is Depth 0 and there is no additional jump available
            if ( depth == 0 ) {

                if( singleMove && !multiJump ) {

                    originalBoard.bestMoves = tempBoard.moves;
                    return SINGLE_MOVE;

                }

            }

            if ( multiJump )
                val = tempBoard.minimax( tempBoard, depth, maxPlayer, alpha, beta );    // Same player as now
            else {

                tempBoard.redTurn = !(tempBoard.redTurn);
                val = tempBoard.minimax( tempBoard, depth+1, !maxPlayer, alpha, beta ); // Switch players

            }

            // Returns from depth if the time limited is exceeded
            if ( val == TIME_LIMIT_EXCEEDED )
                return val;

            // Alpha-beta Pruning
            newPath = false;

            if ( maxPlayer ) {

                if ( bestVal <= val ) {

                    bestVal = val;
                    newPath = true;

                }

                if ( alpha < bestVal )
                    alpha = bestVal;
                else if ( alpha == bestVal ) {

                    randNum = rand()%2; // Choose randomly if two positions are equivalent
                    if ( randNum )
                        newPath = false;

                }
                else
                    newPath = false;

            }
            else {

                if ( bestVal >= val ) {

                    bestVal = val;
                    newPath = true;

                }

                if ( beta > bestVal )
                    beta = bestVal;
                else if( beta == bestVal ) {

                    randNum = rand()%2; // Choose randomly if two positions are equivalent
                    if ( randNum )
                        newPath = false;

                }
                else
                    newPath = false;

            }

            if ( beta <= alpha )
                goto prune;

            if ( newPath )
                originalBoard.bestMoves = tempBoard.bestMoves;

            // Reset tempBoard
            tempBoard = originalBoard;

        }

    }

prune:
    return bestVal;

}


// Creates a copy of pieces potentially affected by an action from start to destination
void board::isolateBoard( tuple<int,int> start, tuple<int,int> destination ) {

    int row,col;
    list< shared_ptr<board::piece> > pieceList;
    shared_ptr<piece> tempPiece;

    // Gets a list of potential affected pieces
    pieceList = affectedPieces(start,destination);

    for ( auto iter : pieceList ) {

        tie( row, col ) = iter->loc;

        // Deletes pointers to old pieces
        iter->clearPiece(*this);
        iter->updateCount(*this,true);

        // Makes a copy of the piece
        gameboard[row][col] = make_shared<piece>( piece(*iter) );

        // Replaces with pointers to new pieces
        if ( iter->validMove ) {

            if ( iter->color == COLOR_RED_VAL )
                redMoves.insert( gameboard[row][col] );
            else
                whiteMoves.insert( gameboard[row][col] );

        }

        if ( iter->validJump ) {

            if ( iter->color == COLOR_RED_VAL )
                redJumps.insert( gameboard[row][col] );
            else
                whiteJumps.insert( gameboard[row][col] );

        }

    }

}


// Calculates pieces potentially affected by action
    // Start tuple represents original location of piece
    // End tuple represents new location after move
list< shared_ptr<board::piece> > board::affectedPieces( tuple<int,int> start, tuple<int,int> destination ) {

    list< shared_ptr<board::piece> > pieceList;
    int oldRow,oldCol,newRow,newCol,tempRow,tempCol;
    tie( oldRow, oldCol ) = start;
    tie( newRow, newCol ) = destination;

    int rowOffset = newRow - oldRow;
    int colOffset = newCol - oldCol;

    if ( rowOffset > 0 )
        rowOffset = 1;
    else
        rowOffset = -1;

    if ( colOffset > 0 )
        colOffset = 1;
    else
        colOffset = -1;

    tempRow = oldRow - 2*rowOffset;
    tempCol = oldCol - 2*colOffset;
    int endRow = newRow + 2*rowOffset;
    int endCol = newCol + 2*colOffset;

    // Inserts pieces on same diagonal
    while ( tempRow != endRow+rowOffset && tempCol != endCol+colOffset ) {

        if ( validLoc(tempRow) && validLoc(tempCol) ) {

            if( gameboard[ tempRow ][ tempCol ]->type != TYPE_EMPTY_VAL )
                pieceList.push_back(gameboard[ tempRow ][ tempCol ]);

        }

        tempRow += rowOffset;
        tempCol += colOffset;

    }

    int perpRowOffset = rowOffset;
    int perpColOffset = colOffset * -1;

    tempRow = oldRow + perpRowOffset;
    tempCol = oldCol + perpColOffset;
    endRow = newRow + perpRowOffset;
    endCol = newCol + perpColOffset;

    for ( int i=0; i<2; i++ ) {

        for( int j=0; j<2; j++ ) {

            while ( tempRow != endRow+rowOffset && tempCol != endCol+colOffset ) {

                if ( validLoc(tempRow) && validLoc(tempCol) ) {

                    if ( gameboard[tempRow][tempCol]->type != TYPE_EMPTY_VAL )
                        pieceList.push_back(gameboard[tempRow][tempCol]);

                }

                tempRow += rowOffset;
                tempCol += colOffset;

            }

            tempRow = oldRow + 2*perpRowOffset;
            tempCol = oldCol + 2*perpColOffset;
            endRow = newRow + 2*perpRowOffset;
            endCol = newCol + 2*perpColOffset;

        }

        perpRowOffset *= -1;
        perpColOffset *= -1;
        tempRow = oldRow + perpRowOffset;
        tempCol = oldCol + perpColOffset;
        endRow = newRow + perpRowOffset;
        endCol = newCol + perpColOffset;

    }

    return pieceList;

}


// Returns set of pieces that can take an action
unordered_set< shared_ptr<board::piece> >* board::returnPieces() {

    int jumpLen;
    unordered_set< shared_ptr<board::piece> > *possibleMoves;

    // A multiJump is when a jump took place and the same piece is available for another jump
    // Stores a pointer to that piece
    // Should contain a piece only if previous action was a jump and piece has oppoprtunity for another jump
    jumpLen = multiJumps.size();

    // If multiJump is not available
    if ( jumpLen == 0 ) {

        // Checks for board jump set size
        // If board jump set is empty, no valid jumps
            // Will return board move set
        // If board jump set is not empty, valid jumps
            // Will return board jump set
        if ( redTurn ) {

            jumpLen = redJumps.size();
            if( jumpLen == 0 )
                possibleMoves = &redMoves;
            else
                possibleMoves = &redJumps;

        }
        else {

            jumpLen = whiteJumps.size();
            if( jumpLen == 0 )
                possibleMoves = &whiteMoves;
            else
                possibleMoves = &whiteJumps;

        }

    }

    // If multiJump is available
    else
        possibleMoves = &multiJumps;

    return possibleMoves;

}


// Performs a specified action
    // Start tuple represents original location of piece
    // End tuple represents new location after move
// If there is another valid jump available, return true; otherwise, return false
bool board::moveResult( tuple<int,int> start, tuple<int,int> destination ) {

    int oldRow,oldCol,newRow,newCol;
    tie( oldRow, oldCol ) = start;
    tie( newRow, newCol ) = destination;

    // Checks if piece is making a jump
    bool jump,tempBool;
    jump = gameboard[oldRow][oldCol]->validJump;

    // Moves the piece pointer from original location to new location
    gameboard[ newRow ][ newCol ] = gameboard[ oldRow ][ oldCol ];

    // Updates the location of piece
    gameboard[ newRow ][ newCol ]->loc = destination;
    tempBool = gameboard[ newRow ][ newCol ]->checkPromotion(*this);

    // Reset piece and calculate valid actions in new location
    gameboard[ newRow ][ newCol ]->resetPiece();

    // Piece is no longer in original location, so replace with empty piece
    gameboard[ oldRow ][ oldCol ] = emptyPiece;

    // If piece made a jump, remove captured piece
    if ( jump ) {

        // Calculate location of captured piece
        int jumpRow = oldRow + (newRow - oldRow)/2;
        int jumpCol = oldCol + (newCol - oldCol)/2;

        // Remove piece from board sets and decrement counts
        gameboard[ jumpRow ][ jumpCol ]->clearPiece(*this);

        // Replace captured piece pointer with empty piece pointer
        gameboard[ jumpRow ][ jumpCol ] = emptyPiece;
    }

    // Checks if any diagonal pieces were affected by action taken
        // E.g. If a piece was captured, a piece diagonal to it
        //      may be able to move to the captured piece's location
    checkDiagMoves( gameboard[ newRow ][ newCol ], start, jump );

    // Checks actions for piece in new location
    checkMoves( gameboard[newRow][newCol] );

    // Empty the multiJump set after every move
    multiJumps.clear();

    // Ends turn after promotion
    if ( tempBool )
        return false;
    // Ends turn if piece did not jump
    if ( !jump )
        return false;

    // Continue turn if piece has another jump available
    if ( gameboard[ newRow ][ newCol ]->validJump ) {

        multiJumps.insert( gameboard[ newRow ][ newCol ] );
        return true;

    }
    // End turn otherwise
    else
        return false;

}


// Checks actions of a specific piece
void board::checkMoves( shared_ptr<piece> &curPiece ) {

    int row,col;
    tie( row, col ) = curPiece->loc;

    shared_ptr<piece> tempPiece;
    tuple<int,int> tempTuple;

    int rowOffset = 1;
    int colOffset = 1;

    int newRow,newCol,jumpRow,jumpCol;
    bool canMove = false;
    bool canJump = false;

    // Loops through all 4 diagonal directions of a piece
    for ( int i=0; i<2; i++ ) {

        rowOffset *= -1;
        for ( int j=0; j<2; j++ ) {

            colOffset *= -1;
            // Location of piece after potential move
            newRow = row + rowOffset;
            newCol = col + colOffset;

            // Checks if newRow & newCol are on the board
            if ( validLoc(newRow) && validLoc(newCol) ) {

                tempPiece = gameboard[newRow][newCol];

                // Checks if curPiece can move in the direction of rowOffset
                if ( curPiece->validDirection(rowOffset) ) {

                    // Checks for moves
                    // Only possible if tempPiece is an empty piece
                    if ( tempPiece->type == TYPE_EMPTY_VAL ) {

                        tempTuple = make_tuple(newRow,newCol);
                        curPiece->moves.push_back(tempTuple);
                        canMove = true;

                    }
                    // Checks for jumps
                    // Only possible if curPiece and tempPiece are different colors
                    else if ( curPiece->color != tempPiece->color ) {

                        // Location of piece after potential jump
                        jumpRow = row + 2*rowOffset;
                        jumpCol = col + 2*colOffset;

                        // Checks if jumpRow & jumpCol are on the board
                        if ( validLoc(jumpRow) && validLoc(jumpCol) ) {

                            tempPiece = gameboard[jumpRow][jumpCol];

                            // Jump is only possible if location after potential jump is empty
                            if ( tempPiece->type == TYPE_EMPTY_VAL ) {

                                tempTuple = make_tuple( jumpRow, jumpCol );
                                curPiece->jumps.push_back(tempTuple);
                                canJump = true;

                            }

                        }

                    }

                }

            }

        }

    }

    if ( canMove ) {

        if( curPiece->validMove == false ) // If not in Move vector
            curPiece->insertMove(*this);

    }
    else
        curPiece->removeMove(*this);

    if ( canJump ) {

        if( curPiece->validJump == false ) // If not in Jump vector
            curPiece->insertJump(*this);

    }
    else
        curPiece->removeJump(*this);

}


// Checks actions of pieces affected by curPiece's move
void board::checkDiagMoves( shared_ptr<piece> &curPiece, tuple<int,int> oldLoc, bool jump ) {

    int oldRow,oldCol,newRow,newCol,tempRow,tempCol,row,col,jumpRow,jumpCol;
    tie( oldRow, oldCol ) = oldLoc;
    tuple<int,int> newLoc = curPiece->loc;
    tie( newRow, newCol ) = newLoc;

    shared_ptr<piece> tempPiece,jumpPiece;
    tuple<int,int> curLoc = oldLoc;

    int rowOffset = 1;
    int colOffset = 1;

    row = oldRow;
    col = oldCol;

    // Updates pieces around old square
    for ( int loop=0; loop<(1+jump); loop++ ) {

        for(int i=0; i<2; i++) {

            rowOffset *= -1;
            for(int j=0; j<2; j++) {

                colOffset *= -1;
                tempRow = row + rowOffset;
                tempCol = col + colOffset;

                if ( validLoc(tempRow) && validLoc(tempCol) ) {

                    if ( tempRow == newRow && tempCol == newCol ) // Ensure no duplicate moves
                        continue;

                    tempPiece = gameboard[ tempRow ][ tempCol ];

                    if ( tempPiece->type == TYPE_EMPTY_VAL )
                        continue;

                    if ( tempPiece->validDirection(rowOffset*-1) ) {

                        tempPiece->moves.push_back(curLoc);
                        if ( tempPiece->validMove == false )
                            tempPiece->insertMove(*this);

                        if ( tempPiece->validJump == true ) {

                            tuple<int,int> tempTuple = make_tuple(row+(rowOffset*-1),col+(colOffset*-1));
                            tempPiece->jumps.remove(tempTuple);
                            if ( tempPiece->jumps.size() == 0 )
                                tempPiece->removeJump(*this);

                        }

                    }

                    jumpRow = row + 2*rowOffset;
                    jumpCol = col + 2*colOffset;

                    if ( validLoc(jumpRow) && validLoc(jumpCol) ) {

                        if ( jumpRow == newRow && jumpCol == newCol ) // Ensure no duplicate moves
                            continue;

                        jumpPiece = gameboard[ jumpRow ][ jumpCol ];

                        if ( jumpPiece->type == TYPE_EMPTY_VAL )
                            continue;

                        if ( jumpPiece->validDirection(rowOffset*-1) ) {

                            if ( jumpPiece->color != tempPiece->color ) {

                                if ( jumpPiece->validJump == false )
                                    jumpPiece->insertJump(*this);
                                jumpPiece->jumps.push_back( curLoc );

                            }

                        }

                    }

                }

            }

        }

        if ( jump ) {

            row += ( newRow - oldRow )/2;
            col += ( newCol - oldCol )/2;
            curLoc = make_tuple( row, col );

        }

    }

    // Update pieces around new square
    rowOffset = 1;
    colOffset = 1;

    row = newRow;
    col = newCol;
    curLoc = newLoc;

    for ( int i=0; i<2; i++ ) {

        rowOffset *= -1;
        for ( int j=0; j<2; j++ ) {

            colOffset *= -1;
            tempRow = row + rowOffset;
            tempCol = col + colOffset;

            if ( validLoc(tempRow) && validLoc(tempCol) ) {

                tempPiece = gameboard[ tempRow ][ tempCol ];

                if ( tempPiece->type == TYPE_EMPTY_VAL )
                    continue;

                if ( tempPiece->validDirection(rowOffset*-1) ) {

                    if ( tempPiece->validMove == true ) {

                        auto tempIter = find( tempPiece->moves.begin(), tempPiece->moves.end(), curLoc );

                        if ( tempIter != tempPiece->moves.end() ) { // If tempPiece has a move to curLoc

                            tempPiece->moves.erase( tempIter );
                            if ( tempPiece->moves.empty() )
                                tempPiece->removeMove(*this);

                        }

                    }

                    if ( tempPiece->color != curPiece->color ) {

                        jumpRow = row + (-1*rowOffset);
                        jumpCol = col + (-1*colOffset);

                        if ( validLoc(jumpRow) && validLoc(jumpCol) ) {

                            if ( gameboard[jumpRow][jumpCol]->type == TYPE_EMPTY_VAL ) {

                                if ( tempPiece->validJump == false )
                                    tempPiece->insertJump(*this);
                                tempPiece->jumps.push_back( make_tuple( jumpRow, jumpCol ) );

                            }

                        }

                    }

                }

                jumpRow = row + 2*rowOffset;
                jumpCol = col + 2*colOffset;

                if ( validLoc(jumpRow) && validLoc(jumpCol) ) {

                    jumpPiece = gameboard[ jumpRow ][ jumpCol ];

                    if ( jumpPiece->type == TYPE_EMPTY_VAL )
                        continue;

                    if ( jumpPiece->validDirection(rowOffset*-1) ) {

                        if ( jumpPiece->color != tempPiece->color ) {

                            jumpPiece->jumps.remove( curLoc );
                            if ( jumpPiece->jumps.empty() )
                                jumpPiece->removeJump(*this);

                        }

                    }

                }

            }

        }

    }

}


// Calculates score for current board state
void board::heuristic() {

    int row,col;
    int whiteCount = whiteMen + whiteKings;
    int redCount = redMen + redKings;

    ////////// Terminal State Check //////////
    // No pieces remaining
    if ( redCount == 0 ) {

        this->score = VICTORY_WHITE_PIECE;  // White Victory
        return;

    }
    else if ( whiteCount == 0 ) {

        this->score = VICTORY_RED_PIECE;    // Red Victory
        return;

    }

    // No moves remaining
    if ( redTurn ) {

        if( redMoves.size() + redJumps.size() == 0 ) {

            this->score = VICTORY_WHITE_MOVE;   // White Victory
            return;

        }

    }
    else {

        if ( whiteMoves.size() + whiteJumps.size() == 0 ) {

            this->score = VICTORY_RED_MOVE;     // Red Victory
            return;

        }

    }


    ////////// Score Calculation //////////
    // Scores
    float whiteScore = 0;
    float redScore = 0;
    float menValue = 30;
    float kingValue = 50;
    float lastRowVal = 5;   // Favors keeping men on the last row (to block opponent's pieces)
    float cornerScore = 50; // Additional score added for king corner pieces

    // Score for kings based on how close they are to enemy pieces
    //      Only awarded to the player with piece advantage
    float whiteClosest = 0;
    float redClosest = 0;

    // Number of kings in a double corner
    int whiteCorner = 0;
    int redCorner = 0;

    this->whiteLast = 0;
    this->redLast = 0;

    whiteScore += whiteMen * menValue;
    whiteScore += whiteKings * kingValue;
    redScore += redMen * menValue;
    redScore += redKings * kingValue;

    // Calculates Corner, Last, and Closest
    for ( auto iter : whitePieces ) {

        tie( row, col ) = iter->loc;
        if ( iter->type == TYPE_MAN_VAL ) {

            whiteScore += pow((float(7 - row)/2), 2)/2;
            if( row == 7 )
                whiteLast++;

        }
        else {

            // Factorial-like function that gives a smaller bonus as king gets closer to a piece
            for ( int i=kingDistance(iter); i<=6; i++ )
                whiteClosest += i;
            if ( row+col == 1 || row+col == 13 )
                whiteCorner++;

        }

    }

    for ( auto iter : redPieces ) {

        tie( row, col ) = iter->loc;
        if ( iter->type == TYPE_MAN_VAL ) {

            redScore += pow( (float(row)/2), 2 )/2;
            if ( row == 0 )
                redLast++;

        }
        else {

            // Factorial-like function that gives a smaller bonus as king gets closer to a piece
            for ( int i=kingDistance(iter); i<=6; i++ )
                redClosest += i;
            if ( row+col == 1 || row+col == 13)
                redCorner++;

        }

    }

    // Only favors having last row men if there are at least 8 pieces left
    if ( whiteCount >= 8 )
        whiteScore += whiteLast * lastRowVal;
    if ( redCount >= 8 )
        redScore += redLast * lastRowVal;

    // Favors:
    //      Fewer pieces if in the lead
    //      King getting closer to enemy pieces if in the lead
    if ( whiteCount > 0 && redCount > 0 ) {

        if ( whiteCount > redCount ) { // White advantage

          whiteScore += pow( 2*(whiteCount/redCount),2 );
          whiteScore += whiteClosest;
          redScore += redCorner * cornerScore;

        }
        else if ( redCount > whiteCount ) { // Red Advantage

          redScore += pow( 2*(redCount/whiteCount), 2 );
          redScore += redClosest;
          whiteScore += whiteCorner * cornerScore;

        }
        else { // Even game

          redScore += redClosest;
          whiteScore += whiteClosest;

        }

    }

    this->score = redScore - whiteScore;

}


// Calculates distance of closest piece from king
// Returns int representing how far away the closest piece is
// Smaller int = closer
int board::kingDistance( shared_ptr<piece> &curPiece ) {

    unordered_set< shared_ptr<piece> > *pieceSet;
    int curRow,curCol,tempRow,tempCol,tempMin,rowDiff,colDiff;
    int minDistance = 8;

    tie( curRow, curCol ) = curPiece->loc;

    // Gets set of pieces of opposite color
    if ( curPiece->color == COLOR_RED_VAL )
        pieceSet = &whitePieces;
    else
        pieceSet = &redPieces;

    // Iterates through all pieces to find the closest piece
    for ( auto iter : *pieceSet ) {

        tie( tempRow, tempCol ) = iter->loc;
        rowDiff = abs( tempRow - curRow );
        colDiff = abs( tempCol - curCol );
        tempMin = max( rowDiff, colDiff );

        if ( tempMin < minDistance )
            minDistance = tempMin;
        // Smallest possible distance
        if ( minDistance == 1 )
            break;

    }

    return minDistance-1;

}


// Checks if score represents a terminal state
bool board::terminalState( float tempScore ) {

    if ( tempScore == VICTORY_RED_MOVE || tempScore == VICTORY_RED_PIECE || tempScore == VICTORY_WHITE_MOVE || tempScore == VICTORY_WHITE_PIECE )
        return true;
    else
        return false;

}


// Checks if a row/column is within the board
// If valid, returns true; otherwise, returns false
bool board::validLoc( int loc ) {

    if ( 0 <= loc && loc <= 7 )
        return true;
    else
        return false;

}


// Prints a victory message
void board::printVictory( bool color, bool noMoves ) {

    string colorText;

    if ( color == COLOR_RED_VAL )
        colorText = "Red";
    else
        colorText = "White";

    if ( noMoves )
        cout << "There are no remaining moves for " << colorText << "." << "\n";
    cout << colorText << " Wins!" << endl;

    exit(EXIT_SUCCESS);

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

        if ( option == 1 )
            start = true;
        else if ( option == 2 )
            printSettings();
        else if ( option == 3 ) {

            cout << "Bye!" << endl;
            exit( EXIT_SUCCESS );

        }

    }

    printTimeSettings();

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

    if ( option == 1 )
        printPlayerSettings();
    else if ( option == 2 )
        printPieceSettings();
    else if ( option == 3 )
        return;

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

        if ( option == 1 )
            redTurn = true;
        else if ( option == 2 )
            redTurn = false;
        else if ( option == 3 )
            AIvsAI = false;
        else if ( option == 4 )
            AIvsAI = true;
        else if ( option == 5 )
            return;

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
        cout << "1 = Add a piece" << "\n";
        cout << "2 = Remove a piece" << "\n";
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

        if ( option == 1 )
            printAddPiece();
        else if ( option == 2 )
            printRemovePiece();
        else if ( option == 3 )
            return;

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


// Ensures player input is a valid type
// If invalid, returns true; otherwise, returns false
bool board::validateInput() {

    if ( cin.fail() ) {

        cin.clear();
        cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
        printError();
        return true;

    }
    else
        return false;

}

