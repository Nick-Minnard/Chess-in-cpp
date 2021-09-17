#include "static.h"
#include "move.h"
#include "board.h"

int main() {
  
  // initialize board object
  Board board = Board();

  cout << board.LegalPerft(1) << endl; // Possible unique games after the first move
  cout << board.LegalPerft(2) << endl; // Possible unique games after the second move
  cout << board.LegalPerft(3) << endl; // Possible unique games after the third move
  cout << board.LegalPerft(4) << endl; // Possible unique games after the fourth move

  // loads a specified chess posistion in the chess notation FEN
  board.loadPosistionFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  board.printPosistion();

  // loads the starting posistion of a chess board
  board.loadPosistionFromFen(board.startPosistion);
  board.printPosistion();

  // prints unrestricted sliding piece moves from a given square
  board.printRaysFrom(35);

  // prints unrestricted knight moves from a given square
  board.printKnightMovesFrom(35);

  // prints unrestricted king moves from a given square
  board.printKingMovesFrom(35);
 
  return 0;

}