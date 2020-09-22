#include "checkers.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cmath>

#define DEBUG_BOOL                  0   // If debugging, 1; Otherwise, 0

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


// Custom sorting function used to organize vectors based on the first location of the first tuple
// Used in playerMove()
bool sortVecOfVecs( vector< tuple< tuple<int,int>, tuple<int,int> > > &, vector< tuple< tuple<int,int>, tuple<int,int> > > & );

unsigned int states = 0;    // Used to check how many states minimax searched through

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

    // Endgame
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

    gameboard[6][7] = make_shared<piece>( piece(COLOR_RED_VAL,TYPE_KING_VAL) );
    gameboard[6][7]->loc = make_tuple(6,7);
    gameboard[7][6] = make_shared<piece>( piece(COLOR_RED_VAL,TYPE_KING_VAL) );
    gameboard[7][6]->loc = make_tuple(7,6);
    gameboard[0][1] = make_shared<piece>( piece(COLOR_WHITE_VAL,TYPE_KING_VAL) );
    gameboard[0][1]->loc = make_tuple(0,1);


    /*
    // Computer pruning optimal player double jump at certain depth ( 9-11 ) but not others?
    gameboard[3][2] = gameboard[2][1];
    gameboard[3][2]->loc = make_tuple(3,2);
    gameboard[2][1] = emptyPiece;
    gameboard[1][2] = gameboard[5][0];
    gameboard[1][2]->loc = make_tuple(1,2);
    gameboard[5][0] = emptyPiece;
    this->redTurn = true;
    */
    /*
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
    */
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
    list< tuple< tuple<int,int>, tuple<int,int> > > futureMoves, tempMoves;

    // Variables for minimax search
    this->startTime = std::chrono::system_clock::now(); // Keeps track of elapsed time
    this->maxDepth = 1;
    float futureScore, tempScore = -12345;
    states = 0;

    // Check for single move
    vector< tuple< tuple<int,int>, tuple<int,int> > > curVecOfActions;
    this->getCurTurnActions( *this, curVecOfActions );

    // Copy single move
    if ( this->vecOfActions.size() == 1 )
        std::copy( this->vecOfActions.front().begin(), this->vecOfActions.front().end(), std::back_inserter( futureMoves ) );

    // Iterative deepening
    else {

        while (1) {

            // Maximizing player if Red
            // Minimizing player if White
            tie( tempScore, tempMoves ) = this->minimax( *this, 0, this->redTurn, VAL_MIN, VAL_MAX );


            // Used for debugging
            // Outputs a list of actions leading to the current state
            if ( DEBUG_BOOL ) {

                cout << "Depth: " << this->maxDepth << "\n"
                     << "Score: " << tempScore << "\n";

                for( auto iter : tempMoves )
                    cout << char(get<0>(get<0>(iter))+97) << get<1>(get<0>(iter))+1 << " " << char(get<0>(get<1>(iter))+97) << get<1>(get<1>(iter))+1 << "\n";
                cout << "\n";

            }
            // Stops iterative deepening if elapsed time becomes close to max time
            if ( tempScore == TIME_LIMIT_EXCEEDED )
                break;

            // Only updates if a search was fully completed
            futureMoves = tempMoves;
            futureScore = tempScore;    // Used for debugging

            // If reached end of game
            if( terminalState( tempScore ) )
                break;

            this->maxDepth++;

        }

    }

    // Used to calculate time taken
    this->endTime = std::chrono::system_clock::now();
    this->elapsed_seconds = this->endTime - this->startTime;

    // Used for debugging
    if ( DEBUG_BOOL ) {

        cout << "Best State: " << "\n";

        // Outputs a list of actions leading to optimal state
        for( auto iter : futureMoves )
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

        multiJump = this->moveResult( get<0>( futureMoves.front() ), get<1>( futureMoves.front() ) );
        cout << "Move taken: " << char(get<0>(get<0>( futureMoves.front() ))+97) << get<1>(get<0>(futureMoves.front()))+1 << " -> "
             << char(get<0>(get<1>(futureMoves.front()))+97) << get<1>(get<1>(futureMoves.front()))+1 << "\n" << endl;
        futureMoves.pop_front();    // Removes action that was just performed
        printBoard();

    }

    // Required statistics
    cout << "Maximum Depth: " << this->maxDepth-1 << "\n"
         << "Time Taken: " << ( this->elapsed_seconds ).count() << endl;

    endTurn();

}


// Handles player actions
void board::playerMove() {

    // Get vector of jumps
    vector< tuple< tuple<int,int>, tuple<int,int> > > curVecOfActions, vecChosenActions;
    this->getCurTurnActions( *this, curVecOfActions );

    // Cleans up the vector for the player
    sort( this->vecOfActions.begin(), this->vecOfActions.end(), sortVecOfVecs );

    bool validOption = false;
    int i, option;
    string input;

    // While player has not selected a valid action
    while ( !validOption ) {

        cout << "Select an option:" << "\n";

        i=0;

        // Iterates through sequences of actions
        for ( auto iter : this->vecOfActions ) {

            // Prints starting piece
            cout << i+1 << ": " << char( get<0>( get<0>( iter.front() ) )+97 ) << get<1>( get<0>( iter.front() ) )+1;

            // Prints following pieces
            for( auto iter2 : iter )
                cout << " -> " << char( get<0>( get<1>( iter2 ) )+97 ) << get<1>( get<1>( iter2 ) )+1;

            cout << "\n";
            i++;

        }

        cout << endl;

        // Waits for player input
        cin >> input;
        cout << endl;

        // Checks for valid input
        if ( this->validateInput() )
            continue;

        // Checks for specific commands
        if ( input == "help" ) {

            this->printHelp();
            continue;

        }
        else if ( input == "board" ) {

            this->printBoard();
            continue;

        }

        // Gets action from input
        try {

            option = std::stoi(input);

        } catch( ... ) {

            option = 0;

        }

        if ( 1 <= option && option <= i ) {

            vecChosenActions = this->vecOfActions[ option-1 ];
            validOption = true;

        }

        if ( !validOption )
            this->printMoveError();

    }

    for ( unsigned int j=0; j<vecChosenActions.size(); j++ ) {

        this->moveResult( get<0>( vecChosenActions[j] ), get<1>( vecChosenActions[j] ) );
        this->printBoard();

    }

    endTurn();

}


// Performs a set of actions at the end of the turn
void board::endTurn() {

    heuristic();    // Calculates score for the current state

    // Checks if score represents a terminal state
    if ( terminalState( this->score ) ) {

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

    // Resets stored moves
    this->moves.clear();
    this->vecOfActions.clear();

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


// Updates vecOfActions, representing available actions during the current turn, including multi-jumps
void board::getCurTurnActions( board &originalBoard, vector< tuple< tuple<int,int>, tuple<int,int> > > curVecOfJumps ) {

    unordered_set< shared_ptr<piece> > possiblePieces = *( originalBoard.returnPieces() );
    list< tuple<int,int> > *pieceActions;
    vector< tuple< tuple<int,int>, tuple<int,int> > > tempVec;

    bool multiJump;
    board tempBoard;

    // Iterate through all pieces available to perform an action
    for ( auto iter : possiblePieces ) {

        pieceActions = iter->returnActions();

        // Iterate through all actions available for the piece
        for( auto iter2 : *pieceActions ) {

            // Copy originalBoard
            tempBoard = originalBoard;
            tempBoard.isolateBoard( iter->loc, iter2 );

            // Copy actions leading to originalBoard
            tempVec = curVecOfJumps;

            // Add current action to vector
            tempVec.push_back( make_tuple( iter->loc, iter2 ) );

            // Apply action
            multiJump = tempBoard.moveResult( iter->loc, iter2 );

            if ( multiJump )
                getCurTurnActions( tempBoard, tempVec );
            else
                tempBoard.vecOfActions.push_back( tempVec );


            // Copy tempBoard vector of vectors of jumps
            //      Should have one additional vector of jumps
            originalBoard.vecOfActions = tempBoard.vecOfActions;

        }

    }

}


// Handles alpha-beta pruning minimax search
// Returns a score and a list of moves to reach the state with that score
tuple< float, list< tuple< tuple<int,int>, tuple<int,int> > > > board::minimax( board &originalBoard, int depth, bool maxPlayer, float alpha, float beta ) {

    // Counts number of states visited (because I was curious)
    states++;

    // Updates elapsed time and returns if time limit is exceeded
    this->endTime = std::chrono::system_clock::now();
    this->elapsed_seconds = this->endTime - this->startTime;

    if ( this->computerTime - elapsed_seconds.count() < REMAINING_TIME_LIMIT )
        return make_tuple( TIME_LIMIT_EXCEEDED, originalBoard.moves );

    // Reached max depth and starts returning from recursion
    if ( depth == originalBoard.maxDepth ) {

        originalBoard.heuristic();                                      // Calculates score for current state
        //originalBoard.printBoard();
        //cout << whitePieces.size();
        //auto iter = whitePieces.begin();
        //piece tempPiece = **iter;
        //cout << char(get<0>( tempPiece.loc )+97) << get<1>(tempPiece.loc)+1 << "\n";
        //cout << originalBoard.score << "\n";
        return make_tuple( originalBoard.score, originalBoard.moves );  // Returns score for alpha-beta pruning

    }

    // Makes a copy of the parent board
    board tempBoard;
    unordered_set< shared_ptr<piece> > *possibleMoves = originalBoard.returnPieces();
    list< tuple<int,int> > *possibleActions;

    bool multiJump;
    tuple< float, list< tuple< tuple<int,int>, tuple<int,int> > > > val, bestVal;

    if ( maxPlayer )
        bestVal = make_tuple( VAL_MIN, originalBoard.moves );
    else
        bestVal = make_tuple( VAL_MAX, originalBoard.moves );

    // Iterate through all actions
    for ( auto iter : *possibleMoves ) {

        possibleActions = iter->returnActions();

        for ( auto iter2 : *possibleActions ) {

            // Because board class contains pointers to pieces, copying board class copies the pointers
            // Does not make copies of pieces, so pointers will still point to original pieces
            // Need to "isolate" tempBoard from originalBoard because operations on tempBoard will
            //      affect pieces of originalBoard through pointers
            tempBoard = originalBoard;
            tempBoard.isolateBoard( iter->loc, iter2 );

            // Creates a tuple containing piece's old location and new location
            // Adds to moves taken to reach current state
            tempBoard.moves.push_back( make_tuple( iter->loc, iter2 ) );
            multiJump = tempBoard.moveResult( iter->loc, iter2 );

            if ( multiJump )
                val = tempBoard.minimax( tempBoard, depth, maxPlayer, alpha, beta );    // Same player as now
            else {

                tempBoard.turnCount++;
                tempBoard.redTurn = !(tempBoard.redTurn);
                val = tempBoard.minimax( tempBoard, depth+1, !maxPlayer, alpha, beta ); // Switch players

            }

            // Returns from depth if the time limited is exceeded
            if ( get<0>( val ) == TIME_LIMIT_EXCEEDED )
                return val;

            // Alpha-beta Pruning
            if ( maxPlayer ) {

                // Get maximum of bestVal & val
                if ( get<0>( bestVal ) < get<0>( val ) )
                    bestVal = val;

                // Randomly choose if 2 states are equivalent
                else if ( get<0>( bestVal ) == get<0>( val ) ) {

                    if ( rand() % 2 )
                        bestVal = val;

                }

                // Pruning
                // Returns bestVal+1 so subtree is pruned
                if ( get<0>( bestVal ) >= beta )
                    return make_tuple( get<0>( bestVal )+1, get<1>( bestVal ) );

                // Update alpha
                alpha = max( alpha, get<0>( bestVal ) );

            }
            else {

                // Get minimum of bestVal & val
                if ( get<0>( bestVal ) > get<0>( val ) )
                    bestVal = val;

                // Randomly choose if 2 states are equivalent
                else if ( get<0>( bestVal ) == get<0>( val ) ) {

                   if ( rand() % 2 )
                        bestVal = val;

                }

                // Pruning
                // Returns bestVal-1 so subtree is pruned
                if ( get<0>( bestVal ) <= alpha )
                    return make_tuple( get<0>( bestVal )-1, get<1>( bestVal ) );

                // Update beta
                beta = min( beta, get<0>( bestVal ) );

            }

        }

    }

    return bestVal;

}


// Creates a copy of pieces potentially affected by an action from start to destination
void board::isolateBoard( tuple<int,int> start, tuple<int,int> destination ) {

    int row,col;
    list< shared_ptr<board::piece> > pieceList;
    shared_ptr<piece> tempPiece;

    // Gets a list of potential affected pieces
    pieceList = affectedPieces( start, destination );

    for ( auto iter : pieceList ) {

        tie( row, col ) = iter->loc;

        // Deletes pointers to old piece
        iter->clearPiece(*this);
        iter->updateCount(*this,true);

        // Makes a copy of the piece
        gameboard[row][col] = make_shared<piece>( piece(*iter) );

        // Replaces with pointers to new piece
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

        if ( iter->color == COLOR_RED_VAL )
            redPieces.insert( gameboard[row][col] );
        else
            whitePieces.insert( gameboard[row][col] );

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
    jump = gameboard[ oldRow ][ oldCol ]->validJump;

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
    checkMoves( gameboard[ newRow ][ newCol ] );

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

                tempPiece = gameboard[ newRow ][ newCol ];

                // Checks if curPiece can move in the direction of rowOffset
                if ( curPiece->validDirection( rowOffset ) ) {

                    // Checks for moves
                    // Only possible if tempPiece is an empty piece
                    if ( tempPiece->type == TYPE_EMPTY_VAL ) {

                        tempTuple = make_tuple( newRow, newCol );
                        curPiece->moves.push_back( tempTuple );
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

                            tempPiece = gameboard[ jumpRow ][ jumpCol ];

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
    float whiteClosest = -1;
    float redClosest = -1;

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

            // Adds a score corresponding to how close the farthest king is
            whiteClosest += addKingDist( iter );
            /*
            if ( whiteClosest == -1 )
                whiteClosest = addKingDist( iter );
            else
                whiteClosest = min( whiteClosest, addKingDist( iter ) );
            */

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

            // Adds a score corresponding to how close the farthest king is
            redClosest += addKingDist( iter );
            /*
            if ( redClosest == -1 )
                redClosest = addKingDist( iter );
            else
                redClosest = min( redClosest, addKingDist( iter ) );
            */

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

            whiteScore += pow( 2*(whiteCount/redCount), 2 );
            whiteScore += whiteClosest;
            redScore += redCorner * cornerScore;
            //whiteScore *= float( 1/ float( 1 + exp( float( -turnCount ) ) / 5 ) + 0.5 ); // Encourage winning faster

        }
        else if ( redCount > whiteCount ) { // Red Advantage

            redScore += pow( 2*(redCount/whiteCount), 2 );
            redScore += redClosest;
            whiteScore += whiteCorner * cornerScore;
            //redScore *= float( 1/ float( 1 + exp( float( -turnCount ) ) / 5 ) + 0.5 ); // Encourage winning faster

        }
        else { // Even game

          redScore += redClosest;
          whiteScore += whiteClosest;

        }

    }

    this->score = redScore - whiteScore;

}


float board::addKingDist( shared_ptr<piece> &curPiece ) {

    float score = 0;

    // Factorial-like function that gives a smaller bonus as king gets closer to a piece
    for ( int i=kingDistance( curPiece ); i<=6; i++ )
        score += float(i) / 2;

    return score;

}


// Calculates distance of closest piece from king
// Returns int representing how far away the closest piece is
// Smaller int = closer
int board::kingDistance( shared_ptr<piece> &curPiece ) {

    unordered_set< shared_ptr<piece> > *pieceSet;
    int curRow,curCol,tempRow,tempCol,tempMin,rowDiff,colDiff;
    int minDistance = 6;

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
        //tempMin = min( rowDiff, colDiff );
        tempMin = (rowDiff + colDiff)/2;
        if ( minDistance > tempMin )
            minDistance = tempMin;
        // Smallest possible distance
        if ( minDistance <= 2 )
            return 2;

    }

    return minDistance;

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


// Custom sorting function used to organize vectors based on the first location of the first tuple
bool sortVecOfVecs( vector< tuple< tuple<int,int>, tuple<int,int> > > &vecA, vector< tuple< tuple<int,int>, tuple<int,int> > > &vecB ) {

    int rowA, colA, rowB, colB;
    tie( rowA, colA ) = get<0>( vecA[0] );
    tie( rowB, colB ) = get<0>( vecB[0] );

    if ( rowA < rowB )
        return true;
    else if ( rowA == rowB )
        return colA < colB;
    else
        return false;

}
