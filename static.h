#include <iostream>
#include <chrono>
#include <bitset>
#include <vector>
#include <cmath>

typedef std::bitset<64> BitBoard;

using namespace std;

// WHITE CASTLING LOGIC WITH TERNERY MAYHEM
// if(getSquare(targetSquare) == 3) { setWhiteCastling(0); }
// if((posistion[4] << 1) + posistion[3] != 0) {
//   setWhiteCastling((posistion[4] << 1) + posistion[3] == 3 ? (startSquare == 7 ? 1 : (startSquare == 0 ? 2 : 3)) :
//   ((posistion[4] << 1) + posistion[3] == 2 ? (startSquare == 7 ? 0 : 2) : (startSquare == 0 ? 0 : 1))); }
// if((posistion[2] << 1) + posistion[1] != 0) {
// setBlackCastling((posistion[2] << 1) + posistion[1] == 3 ? (targetSquare == 63 ? 1 : (targetSquare == 56 ? 2 : 3)) :
// ((posistion[2] << 1) + posistion[1] == 2 ? (targetSquare == 63 ? 0 : 2) : (targetSquare == 56 ? 0 : 1))); }