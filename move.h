struct Move {
  short start; short target; short flags; short pieceType; short cpieceType;
  short castlingRights; short enPassantSquare;
  Move(short s, short t, short f, short p, short c, short r, short e) {
    start = s; target = t; flags = f; pieceType = p; cpieceType = c;
    castlingRights = r; enPassantSquare = e;
  }
};