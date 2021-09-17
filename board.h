struct Board {

  string const startPosistion {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
  char const files[8] {'a','b','c','d','e','f','g','h'};
  char const ranks[8] {'1','2','3','4','5','6','7','8'};
  short const directionOffsets[8] {8,-8,-1,1,7,-7,9,-9};
  short const knightOffsets[8] {15,17,6,10,-10,-6,-15,-17};
  short const pieceValues[5] {100,300,300,500,900};

  // GAME STATE VARS
  short enPassantSquare {0}; short castlingRights {15}; bool turn {true};
  bool generatingLegalMoves {false}; vector<Move> moves;

  // PRECOMPUTED MOVE DATA
  BitBoard KINGMOVES[64]; BitBoard KNIGHTMOVES[64];
  short numSquaresToEdge[64][8]; BitBoard RAYS[8][64];

  // COMBINATION BOARDS
  BitBoard white_pieces_board; BitBoard black_pieces_board; BitBoard all_pieces_board;

  // TEMPARARY BOARDS
  BitBoard movedPiece; BitBoard takenPiece; BitBoard specialCase; BitBoard opponentAttacks;

  // PIECE BOARDS
  BitBoard white_king_board;BitBoard black_king_board;
  BitBoard white_pawn_board;BitBoard black_pawn_board;
  BitBoard white_knight_board;BitBoard black_knight_board;
  BitBoard white_bishop_board;BitBoard black_bishop_board;
  BitBoard white_rook_board;BitBoard black_rook_board;
  BitBoard white_queen_board;BitBoard black_queen_board;

  // BOARD POINTER ARRAYS
  BitBoard * white_boards[6] {&white_king_board, &white_pawn_board, &white_knight_board,
  &white_bishop_board, &white_rook_board, &white_queen_board};
  BitBoard * black_boards[6] {&black_king_board, &black_pawn_board, &black_knight_board,
  &black_bishop_board, &black_rook_board, &black_queen_board};

  // CONSTRUCTOR
  Board() {
    generateKingMoves(); generateKnightMoves();
    generateSlideDistances(); generateRays();
    loadPosistionFromFen(startPosistion);
  }

// ******************** FEN SUPPORT ******************** //

  void loadPosistionFromFen(string fen) {
    fullReset();
    vector<string> segments;
    string segment;
    for(int i = 0; i < fen.size(); i++) {
      if(fen[i] == ' ') {
        segments.push_back(segment);
        segment = "";
      } else { segment += fen[i]; }
    } segments.push_back(segment);
    turn = segments[1] == "w" ? true : false;
    castlingRights = 0;
    if(segments[2] != "-") {
      for(int i = 0; i < segments[2].size(); i++) {
        switch(segments[2][i]) {
          case 'K': castlingRights |= 8; break;
          case 'Q': castlingRights |= 4; break;
          case 'k': castlingRights |= 2; break;
          case 'q': castlingRights |= 1; break;
          default: break; } } }
    if(segments[3] != "-") { enPassantSquare = SQ2IDX(segments[3]); }
    int file {0}; int rank {7};
    for(int i = 0; i < segments[0].size(); i++) {
      if(segments[0][i] == '/') { file = 0; rank--; }
      else {
        if(isdigit(segments[0][i])) { file += (stoi(to_string(segments[0][i])) - 48); }
        else {
          switch(segments[0][i]) {
            case 'K': white_king_board[63 - (rank * 8 + file)] = 1; break;
            case 'P': white_pawn_board[63 - (rank * 8 + file)] = 1; break;
            case 'N': white_knight_board[63 - (rank * 8 + file)] = 1; break;
            case 'B': white_bishop_board[63 - (rank * 8 + file)] = 1; break;
            case 'R': white_rook_board[63 - (rank * 8 + file)] = 1; break;
            case 'Q': white_queen_board[63 - (rank * 8 + file)] = 1; break;
            case 'k': black_king_board[63 - (rank * 8 + file)] = 1; break;
            case 'p': black_pawn_board[63 - (rank * 8 + file)] = 1; break;
            case 'n': black_knight_board[63 - (rank * 8 + file)] = 1; break;
            case 'b': black_bishop_board[63 - (rank * 8 + file)] = 1; break;
            case 'r': black_rook_board[63 - (rank * 8 + file)] = 1; break;
            case 'q': black_queen_board[63 - (rank * 8 + file)] = 1; break;
            default: break; }
            file++; } } }
  }

// ******************** INITIALIZATION FUNCTIONS ******************** //

  void generateKingMoves() {
    for(int rank = 0; rank < 8; rank++) {
      for(int file = 0; file < 8; file++) {
        short squareIndex = rank * 8 + file;
        for(int dir = 0; dir < 8; dir++) {
          short target = squareIndex + directionOffsets[dir];
          if(-1 < target && target < 64) {
            if(!((file == 0 && target % 8 == 7) ||
            (file == 7 && target % 8 == 0))) {
              KINGMOVES[squareIndex][target] = 1; } } } } }
  }

  void generateKnightMoves() {
    for(int rank = 0; rank < 8; rank++) {
      for(int file = 0; file < 8; file++) {
        short squareIndex = rank * 8 + file;
        for(int dir = 0; dir < 8; dir++) {
          short target = squareIndex + knightOffsets[dir];
          if(-1 < target && target < 64) {
            if(!((file > 5 && target % 8 < 2) ||
            (file < 2 && target % 8 > 5))) {
              KNIGHTMOVES[squareIndex][target] = 1; } } } } }
  }

  void generateSlideDistances() {
    for(int rank = 0; rank < 8; rank++) {
      for(int file = 0; file < 8; file++) {
        short numNorth = 7 - rank;
        short numSouth = rank;
        short numWest = file;
        short numEast = 7 - file;
        short squareIndex = rank * 8 + file;
        numSquaresToEdge[squareIndex][0] = numNorth;
        numSquaresToEdge[squareIndex][1] = numSouth;
        numSquaresToEdge[squareIndex][2] = numWest;
        numSquaresToEdge[squareIndex][3] = numEast;
        numSquaresToEdge[squareIndex][4] = min(numNorth, numWest);
        numSquaresToEdge[squareIndex][5] = min(numSouth, numEast);
        numSquaresToEdge[squareIndex][6] = min(numNorth, numEast);
        numSquaresToEdge[squareIndex][7] = min(numSouth, numWest); } }
  }

  void generateRays() {
    for(int rank = 0; rank < 8; rank++) {
      for(int file = 0; file < 8; file++) {
        int squareIndex = rank * 8 + file;
        for(int directionIndex = 0; directionIndex < 8; directionIndex++) {
          for(int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++) {
            short targetSquare = squareIndex + directionOffsets[directionIndex] * (n + 1);
            if(-1 < targetSquare && targetSquare < 64) { RAYS[directionIndex][squareIndex][63 - targetSquare] = 1; } } } } }
  }

// ******************** BITBOARD HANDLERS ******************** //

  void resetTempararyBoards() {
    movedPiece.reset();
    takenPiece.reset();
    specialCase.reset();
  }

  void resetCombinationBoards() {
    white_pieces_board.reset();
    black_pieces_board.reset();
    all_pieces_board.reset();
  }

  void fullReset() {
    enPassantSquare = 0; castlingRights = 15; turn = true; moves.clear();
    resetTempararyBoards(); resetCombinationBoards(); opponentAttacks.reset();
    white_king_board.reset(); white_pawn_board.reset(); white_knight_board.reset();
    white_bishop_board.reset(); white_rook_board.reset(); white_queen_board.reset();
    black_king_board.reset(); black_pawn_board.reset(); black_knight_board.reset();
    black_bishop_board.reset(); black_rook_board.reset(); black_queen_board.reset();
  }

  void updateCombinationBoards() {
    white_pieces_board = white_king_board |
    white_pawn_board | white_knight_board |
    white_bishop_board | white_rook_board | white_queen_board;

    black_pieces_board = black_king_board |
    black_pawn_board | black_knight_board |
    black_bishop_board | black_rook_board | black_queen_board;

    all_pieces_board = white_pieces_board | black_pieces_board;
  }  

  short getWhitePieceOn(short index) {
    if(white_king_board[63 - index]) { return 2; }
    else if(white_pawn_board[63 - index]) { return 4; }
    else if(white_knight_board[63 - index]) { return 6; }
    else if(white_bishop_board[63 - index]) { return 8; }
    else if(white_rook_board[63 - index]) { return 10; }
    else if(white_queen_board[63 - index]) { return 12; }
    else { return 0; }
  }

  short getBlackPieceOn(short index) {
    if(black_king_board[63 - index]) { return 2; }
    else if(black_pawn_board[63 - index]) { return 4; }
    else if(black_knight_board[63 - index]) { return 6; }
    else if(black_bishop_board[63 - index]) { return 8; }
    else if(black_rook_board[63 - index]) { return 10; }
    else if(black_queen_board[63 - index]) { return 12; }
    else { return 0; }
  }

// ******************** MOVE FUNCTIONS ******************** //

  void makeMove(Move move) {

    resetTempararyBoards();
    enPassantSquare = 0;

    if(turn) {

      if(move.cpieceType != 0) {
        takenPiece[63 - move.target] = 1;
        *black_boards[(move.cpieceType / 2) - 1] ^= takenPiece; }
      movedPiece[63 - move.start] = 1; movedPiece[63 - move.target] = 1;
      *white_boards[(move.pieceType / 2) - 1] ^= movedPiece;

      if(move.pieceType == 4) { if(move.target - 16 == move.start) { enPassantSquare = move.target - 8; } }

      short whiteCastlingRights = castlingRights >> 2;
      short blackCastlingRights = castlingRights & 3;
      if(whiteCastlingRights != 0) {
        if(move.pieceType == 2) { castlingRights &= 3; }
        else if(whiteCastlingRights == 3) {
          if(move.start == 7) { castlingRights &= 7; }
          else if(move.start == 0) { castlingRights &= 11; } }
        else if(whiteCastlingRights == 2) {
          if(move.start == 7) { castlingRights &= 3; } }
        else if(whiteCastlingRights == 1) {
          if(move.start == 0) { castlingRights &= 3; } } }
      if(blackCastlingRights != 0) {
        if(blackCastlingRights == 3) {
          if(move.target == 63) { castlingRights &= 13; }
          else if(move.target == 56) { castlingRights &= 14; } }
        else if(blackCastlingRights == 2) {
          if(move.target == 63) { castlingRights &= 12; } }
        else if(blackCastlingRights == 1) {
          if(move.target == 56) { castlingRights &= 12; } } }

      switch(move.flags) {
        case 1: specialCase[63 - (move.target - 8)] = 1; black_pawn_board ^= specialCase; break;
        case 2: specialCase[58] = 1; specialCase[56] = 1; white_rook_board ^= specialCase; break;
        case 3: specialCase[60] = 1; specialCase[63] = 1; white_rook_board ^= specialCase; break;
        case 4: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_knight_board ^= specialCase; break;
        case 5: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_bishop_board ^= specialCase; break;
        case 6: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_rook_board ^= specialCase; break;
        case 7: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_queen_board ^= specialCase; break;
        default: break; } }

    else {

      if(move.cpieceType != 0) {
        takenPiece[63 - move.target] = 1;
        *white_boards[(move.cpieceType / 2) - 1] ^= takenPiece; }
      movedPiece[63 - move.start] = 1; movedPiece[63 - move.target] = 1;
      *black_boards[(move.pieceType / 2) - 1] ^= movedPiece;

      if(move.pieceType == 4) { if(move.target + 16 == move.start) { enPassantSquare = move.target + 8; } }

      short blackCastlingRights = castlingRights & 3;
      short whiteCastlingRights = castlingRights >> 2;
      if(blackCastlingRights != 0) {
        if(move.pieceType == 2) { castlingRights &= 12; }
        else if(blackCastlingRights == 3) {
          if(move.start == 63) { castlingRights &= 13; }
          else if(move.start == 56) { castlingRights &= 14; } }
        else if(blackCastlingRights == 2) {
          if(move.start == 63) { castlingRights &= 12; } }
        else if(blackCastlingRights == 1) {
          if(move.start == 56) { castlingRights &= 12; } } }
      if(whiteCastlingRights != 0) {
        if(whiteCastlingRights == 3) {
          if(move.target == 7) { castlingRights &= 7; }
          else if(move.target == 0) { castlingRights &= 11; } }
        else if(whiteCastlingRights == 2) {
          if(move.target == 7) { castlingRights &= 3; } }
        else if(whiteCastlingRights == 1) {
          if(move.target == 0) { castlingRights &= 3; } } }

      switch(move.flags) {
        case 1: specialCase[63 - (move.target + 8)] = 1; white_pawn_board ^= specialCase; break;
        case 2: specialCase[2] = 1; specialCase[0] = 1; black_rook_board ^= specialCase; break;
        case 3: specialCase[7] = 1; specialCase[4] = 1; black_rook_board ^= specialCase; break;
        case 4: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_knight_board ^= specialCase; break;
        case 5: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_bishop_board ^= specialCase; break;
        case 6: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_rook_board ^= specialCase; break;
        case 7: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_queen_board ^= specialCase; break;
        default: break; } }

    turn ^= 1;

  }

  void unmakeMove(Move move) {

    resetTempararyBoards();
    enPassantSquare = move.enPassantSquare;
    castlingRights = move.castlingRights;

    if(turn) {

      if(move.cpieceType != 0) {
        takenPiece[63 - move.target] = 1;
        *white_boards[move.cpieceType / 2 - 1] ^= takenPiece; }
      movedPiece[63 - move.start] = 1; movedPiece[63 - move.target] = 1;
      *black_boards[move.pieceType / 2 - 1] ^= movedPiece;

      switch(move.flags) {
        case 1: specialCase[63 - (move.target + 8)] = 1; white_pawn_board ^= specialCase; break;
        case 2: specialCase[2] = 1; specialCase[0] = 1; black_rook_board ^= specialCase; break;
        case 3: specialCase[7] = 1; specialCase[4] = 1; black_rook_board ^= specialCase; break;
        case 4: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_knight_board ^= specialCase; break;
        case 5: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_bishop_board ^= specialCase; break;
        case 6: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_rook_board ^= specialCase; break;
        case 7: specialCase[63 - move.target] = 1; black_pawn_board ^= specialCase; black_queen_board ^= specialCase; break;
        default: break; } }

    else {

      if(move.cpieceType != 0) {
        takenPiece[63 - move.target] = 1;
        *black_boards[move.cpieceType / 2 - 1] ^= takenPiece; }
      movedPiece[63 - move.start] = 1; movedPiece[63 - move.target] = 1;
      *white_boards[move.pieceType / 2 - 1] ^= movedPiece;

      switch(move.flags) {
        case 1: specialCase[63 - (move.target - 8)] = 1; black_pawn_board ^= specialCase; break;
        case 2: specialCase[58] = 1; specialCase[56] = 1; white_rook_board ^= specialCase; break;
        case 3: specialCase[60] = 1; specialCase[63] = 1; white_rook_board ^= specialCase; break;
        case 4: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_knight_board ^= specialCase; break;
        case 5: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_bishop_board ^= specialCase; break;
        case 6: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_rook_board ^= specialCase; break;
        case 7: specialCase[63 - move.target] = 1; white_pawn_board ^= specialCase; white_queen_board ^= specialCase; break;
        default: break; } }

    turn ^= 1;

  }

// ******************** EVALUATION FUNCTION ******************** //

  short getEvaluation(bool perspective) {
    return perspective *
    (((white_pawn_board.count() - black_pawn_board.count()) * pieceValues[0]) +
    ((white_knight_board.count() - black_knight_board.count()) * pieceValues[1]) +
    ((white_bishop_board.count() - black_bishop_board.count()) * pieceValues[2]) +
    ((white_rook_board.count() - black_rook_board.count()) * pieceValues[3]) +
    ((white_queen_board.count() - black_queen_board.count()) * pieceValues[4]));
  }

// ******************** MOVE GENERATION MAIN ******************** //

  vector<Move> getPseudoLegalMoves() {
    moves.clear();
    updateCombinationBoards();
    if(turn) { generateWhiteMoves(); }
    else { generateBlackMoves(); }
    return moves;
  }

  vector<Move> getLegalMoves() {
    generatingLegalMoves = true;
    opponentAttacks.reset();
    updateCombinationBoards();
    if(turn) { hashBlackAttacks(); }
    else { hashWhiteAttacks(); }
    vector<Move> legalMoves;
    vector<Move> moves = getPseudoLegalMoves();
    for(int i = 0; i < moves.size(); i++) {
      makeMove(moves[i]);
      vector<Move> newMoves = getPseudoLegalMoves();
      bool illegal = false;
      for(int j = 0; j < newMoves.size(); j++) {
        if(newMoves[j].cpieceType == 2) {
          illegal = true; break; } }
      if(!illegal) { legalMoves.push_back(moves[i]); }
      unmakeMove(moves[i]);
    } generatingLegalMoves = false;
    return legalMoves;
  }

// ******************** ATTACK SQUARE GENERATION ******************** //

  void hashWhiteAttacks() {
    for(int i = 0; i < 64; i++) {
      if(white_king_board[63 - i]) { hashKingAttacks(i); }
      else if(white_pawn_board[63 - i]) { hashWhitePawnAttacks(i); }
      else if(white_knight_board[63 - i]) { hashKnightAttacks(i); }
      else if(white_bishop_board[63 - i]) { hashSliderAttacks(i, 8); }
      else if(white_rook_board[63 - i]) { hashSliderAttacks(i, 10); }
      else if(white_queen_board[63 - i]) { hashSliderAttacks(i, 12); }
    }
  }

  void hashBlackAttacks() {
    for(int i = 0; i < 64; i++) {
      if(black_king_board[63 - i]) { hashKingAttacks(i); }
      else if(black_pawn_board[63 - i]) { hashBlackPawnAttacks(i); }
      else if(black_knight_board[63 - i]) { hashKnightAttacks(i); }
      else if(black_bishop_board[63 - i]) { hashSliderAttacks(i, 8); }
      else if(black_rook_board[63 - i]) { hashSliderAttacks(i, 10); }
      else if(black_queen_board[63 - i]) { hashSliderAttacks(i, 12); }
    }
  }

  void hashKingAttacks(short index) {
    opponentAttacks |= KINGMOVES[63 - index];
  }

  void hashWhitePawnAttacks(short index) {
    opponentAttacks[63 - (index + 7)] = 1;
    opponentAttacks[63 - (index + 9)] = 1;
  }

  void hashBlackPawnAttacks(short index) {
    opponentAttacks[63 - (index - 7)] = 1;
    opponentAttacks[63 - (index - 9)] = 1;
  }

  void hashKnightAttacks(short index) {
    opponentAttacks |= KNIGHTMOVES[63 - index];
  }

  void hashSliderAttacks(short startSquare, short piece) {
    int start = (piece == 8) ? 4 : 0;
    int end = (piece == 10) ? 4 : 8;
    for(int dir = start; dir < end; dir++) {
      for(int n = 0; n < numSquaresToEdge[startSquare][dir]; n++) {

        int targetSquare = startSquare + directionOffsets[dir] * (n + 1);

        opponentAttacks[63 - targetSquare] = 1;

        if(white_pieces_board[63 - targetSquare] || black_pieces_board[63 - targetSquare]) { break; }

      }
    }
  }

// ******************** MOVE GENERATORS ******************** //

  void generateWhiteMoves() {
    for(int i = 0; i < 64; i++) {
      if(white_king_board[63 - i]) { addWhiteKingMoves(i); }
      else if(white_pawn_board[63 - i]) { addWhitePawnMoves(i); }
      else if(white_knight_board[63 - i]) { addWhiteKnightMoves(i); }
      else if(white_bishop_board[63 - i]) { addWhiteSliderMoves(i, 8); }
      else if(white_rook_board[63 - i]) { addWhiteSliderMoves(i, 10); }
      else if(white_queen_board[63 - i]) { addWhiteSliderMoves(i, 12); }
    }
  }

  void generateBlackMoves() {
    for(int i = 0; i < 64; i++) {
      if(black_king_board[63 - i]) { addBlackKingMoves(i); }
      else if(black_pawn_board[63 - i]) { addBlackPawnMoves(i); }
      else if(black_knight_board[63 - i]) { addBlackKnightMoves(i); }
      else if(black_bishop_board[63 - i]) { addBlackSliderMoves(i, 8); }
      else if(black_rook_board[63 - i]) { addBlackSliderMoves(i, 10); }
      else if(black_queen_board[63 - i]) { addBlackSliderMoves(i, 12); }
    }
  }

  void addWhiteKingMoves(short index) {
    for(int i = 0; i < 64; i++) {
      if(KINGMOVES[63 - index][63 - i]) {
        if(!white_pieces_board[63 - i]) {
          moves.push_back(Move(index, i, 0, 2, getBlackPieceOn(i), castlingRights, enPassantSquare)); } } }
    if(castlingRights & 8) {
      if(generatingLegalMoves) {
        if(!opponentAttacks[57] && !opponentAttacks[58] && !opponentAttacks[59]) {
          if(!all_pieces_board[57] && !all_pieces_board[58]) {
            moves.push_back(Move(index, 6, 2, 2, 0, castlingRights, enPassantSquare)); } }
      } else { if(!all_pieces_board[57] && !all_pieces_board[58]) {
          moves.push_back(Move(index, 6, 2, 2, 0, castlingRights, enPassantSquare)); } } }
    if(castlingRights & 4) {
      if(generatingLegalMoves) {
        if(!opponentAttacks[59] && !opponentAttacks[60] && !opponentAttacks[61]) {
          if(!all_pieces_board[60] && !all_pieces_board[61] && !all_pieces_board[62]) {
            moves.push_back(Move(index, 2, 3, 2, 0, castlingRights, enPassantSquare)); } }
      } else { if(!all_pieces_board[60] && !all_pieces_board[61] && !all_pieces_board[62]) {
          moves.push_back(Move(index, 2, 3, 2, 0, castlingRights, enPassantSquare)); } } }
      
  }

  void addBlackKingMoves(short index) {
    for(int i = 0; i < 64; i++) {
      if(KINGMOVES[63 - index][63 - i]) {
        if(!black_pieces_board[63 - i]) {
          moves.push_back(Move(index, i, 0, 2, getWhitePieceOn(i), castlingRights, enPassantSquare)); } } }
    if(castlingRights & 2) {
      if(generatingLegalMoves) {
        if(!opponentAttacks[1] && !opponentAttacks[2] && !opponentAttacks[3]) {
          moves.push_back(Move(index, 62, 2, 2, 0, castlingRights, enPassantSquare)); }
      } else { if(!all_pieces_board[1] && !all_pieces_board[2]) {
          moves.push_back(Move(index, 62, 2, 2, 0, castlingRights, enPassantSquare)); } } }
    if(castlingRights & 1) {
      if(generatingLegalMoves) {
        if(!opponentAttacks[3] && !opponentAttacks[4] && !opponentAttacks[5]) {
          if(!all_pieces_board[4] && !all_pieces_board[5] && !all_pieces_board[6]) {
            moves.push_back(Move(index, 58, 3, 2, 0, castlingRights, enPassantSquare)); } }
      } else { if(!all_pieces_board[4] && !all_pieces_board[5] && !all_pieces_board[6]) {
          moves.push_back(Move(index, 58, 3, 2, 0, castlingRights, enPassantSquare)); } } }
  }

  void addWhitePawnMoves(short index) {
    if(!all_pieces_board[63 - (index + 8)]) {
      if(index > 47 && index < 56) {
        addPawnPromotionMoves(index, index + 8, 0);
      } else { moves.push_back(Move(index, index + 8, 0, 4, 0, castlingRights, enPassantSquare)); }
      if(index > 7 && index < 16) {
        if(!all_pieces_board[63 - (index + 16)]) {
          moves.push_back(Move(index, index + 16, 0, 4, 0, castlingRights, enPassantSquare)); } } }
    if(index % 8 != 0) {
      if(black_pieces_board[63 - (index + 7)]) {
        if(index > 47 && index < 56) {
          addPawnPromotionMoves(index, index + 7, getBlackPieceOn(index + 7));
        } else { moves.push_back(Move(index, index + 7, 0, 4, getBlackPieceOn(index + 7), castlingRights, enPassantSquare)); } }
      else if(enPassantSquare == index + 7) {
        moves.push_back(Move(index, index + 7, 1, 4, 0, castlingRights, enPassantSquare)); } }
    if(index % 8 != 7) {
      if(black_pieces_board[63 - (index + 9)]) {
        if(index > 47 && index < 56) {
          addPawnPromotionMoves(index, index + 9, getBlackPieceOn(index + 9));
        } else { moves.push_back(Move(index, index + 9, 0, 4, getBlackPieceOn(index + 9), castlingRights, enPassantSquare)); } }
      else if(enPassantSquare == index + 9) {
        moves.push_back(Move(index, index + 9, 1, 4, 0, castlingRights, enPassantSquare)); } }
  }

  void addBlackPawnMoves(short index) {
    if(!all_pieces_board[63 - (index - 8)]) {
      if(index > 7 && index < 16) {
        addPawnPromotionMoves(index, index - 8, 0);
      } else { moves.push_back(Move(index, index - 8, 0, 4, 0, castlingRights, enPassantSquare)); }
      if(index > 47 && index < 56) {
        if(!all_pieces_board[63 - (index - 16)]) {
          moves.push_back(Move(index, index - 16, 0, 4, 0, castlingRights, enPassantSquare)); } } }
    if(index % 8 != 0) {
      if(white_pieces_board[63 - (index - 9)]) {
        if(index > 7 && index < 16) {
          addPawnPromotionMoves(index, index - 9, getWhitePieceOn(index - 9));
        } else { moves.push_back(Move(index, index - 9, 0, 4, getWhitePieceOn(index - 9), castlingRights, enPassantSquare)); } }
      else if(enPassantSquare == index - 9) {
        moves.push_back(Move(index, index - 9, 1, 4, 0, castlingRights, enPassantSquare)); } }
    if(index % 8 != 7) {
      if(white_pieces_board[63 - (index - 7)]) {
        if(index > 7 && index < 16) {
          addPawnPromotionMoves(index, index - 7, getWhitePieceOn(index - 7));
        } else { moves.push_back(Move(index, index - 7, 0, 4, getWhitePieceOn(index - 7), castlingRights, enPassantSquare)); } }
      else if(enPassantSquare == index - 7) {
        moves.push_back(Move(index, index - 7, 1, 4, 0, castlingRights, enPassantSquare)); } }
  }

  void addPawnPromotionMoves(short start, short target, short cpiece) {
    moves.push_back(Move(start, target, 4, 4, cpiece, castlingRights, enPassantSquare));
    moves.push_back(Move(start, target, 5, 4, cpiece, castlingRights, enPassantSquare));
    moves.push_back(Move(start, target, 6, 4, cpiece, castlingRights, enPassantSquare));
    moves.push_back(Move(start, target, 7, 4, cpiece, castlingRights, enPassantSquare)); 
  }

  void addWhiteKnightMoves(short index) {
    for(int i = 0; i < 64; i++) {
      if(KNIGHTMOVES[63 - index][63 - i]) {
        if(!white_pieces_board[63 - i]) {
          moves.push_back(Move(index, i, 0, 6, getBlackPieceOn(i), castlingRights, enPassantSquare)); } } }
  }

  void addBlackKnightMoves(short index) {
    for(int i = 0; i < 64; i++) {
      if(KNIGHTMOVES[63 - index][63 - i]) {
        if(!black_pieces_board[63 - i]) {
          moves.push_back(Move(index, i, 0, 6, getWhitePieceOn(i), castlingRights, enPassantSquare)); } } }
  }

  void addWhiteSliderMoves(short startSquare, short piece) {
    int start = (piece == 8) ? 4 : 0;
    int end = (piece == 10) ? 4 : 8;
    for(int dir = start; dir < end; dir++) {
      for(int n = 0; n < numSquaresToEdge[startSquare][dir]; n++) {

        int targetSquare = startSquare + directionOffsets[dir] * (n + 1);

        if(white_pieces_board[63 - targetSquare]) { break; }

        moves.push_back(Move(startSquare, targetSquare, 0, piece, getBlackPieceOn(targetSquare), castlingRights, enPassantSquare));

        if(black_pieces_board[63 - targetSquare]) { break; }

      }
    }
  }

  void addBlackSliderMoves(short startSquare, short piece) {
    int start = (piece == 8) ? 4 : 0;
    int end = (piece == 10) ? 4 : 8;
    for(int dir = start; dir < end; dir++) {
      for(int n = 0; n < numSquaresToEdge[startSquare][dir]; n++) {

        int targetSquare = startSquare + directionOffsets[dir] * (n + 1);

        if(black_pieces_board[63 - targetSquare]) { break; }

        moves.push_back(Move(startSquare, targetSquare, 0, piece, getWhitePieceOn(targetSquare), castlingRights, enPassantSquare));

        if(white_pieces_board[63 - targetSquare]) { break; }

      }
    }
  }


// ******************** UTILITY FUNCTIONS ******************** //

  void printKingMovesFrom(short index) {
    for(int i = 7; i > -1; i--) {
      for(int j = 0; j < 8; j++) {
        unsigned short squareIndex = j + i * 8;
        if(squareIndex == index) { cout << "🟨"; }
        else { cout << (KINGMOVES[index][squareIndex] ? "🟥" : "⬜"); }
      } cout << endl; }
  }

  void printKnightMovesFrom(short index) {
    for(int i = 7; i > -1; i--) {
      for(int j = 0; j < 8; j++) {
        unsigned short squareIndex = j + i * 8;
        if(squareIndex == index) { cout << "🟨"; }
        else { cout << (KNIGHTMOVES[index][squareIndex] ? "🟥" : "⬜"); }
      } cout << endl; }
  }

  void printRaysFrom(short index) {
    BitBoard collection = RAYS[0][index] | RAYS[1][index] | RAYS[2][index] |
    RAYS[3][index] | RAYS[4][index] | RAYS[5][index] | RAYS[6][index] | RAYS[7][index];
    for(int i = 7; i > -1; i--) {
      for(int j = 0; j < 8; j++) {
        unsigned short squareIndex = j + i * 8;
        if(squareIndex == index) { cout << "🟨"; }
        else { cout << (collection[63 - squareIndex] ? "🟥" : "⬜"); }
      } cout << endl; }
  }


  Move constructMove(string s, string t, short f, short p, short c) {
    return Move(SQ2IDX(s), SQ2IDX(t), f, p, c, castlingRights, enPassantSquare);
  }

  string IDX2SQ(short index) {
    string rank = to_string((index / 8) + 1);
    string file = "";
    for(int i = 0; i < 8; i++) {
      if(i == index % 8) { file = files[i]; break; }
    } return file + rank;
  }

  short SQ2IDX(string square) {
    short index = (stoi(square.substr(1)) - 1) * 8;
    for(int i = 0; i < 8; i++) {
      if(files[i] == square[0]) { index += i; break; }
    } return index;
  }

  string convertMoveToString(Move move) {
    string square1 = IDX2SQ(move.start);
    string square2 = IDX2SQ(move.target);
    return square1 + square2;
  }

  void printPosistion() {
    cout << endl << endl << "------------------------" << endl;
    for(int i = 7; i > -1; i--) {
      for(int j = 0; j < 8; j++) {
        unsigned short index = 63 - (j + i * 8);
        if(white_king_board[index]) { cout << " K "; }
        else if(white_pawn_board[index]) { cout << " P "; }
        else if(white_knight_board[index]) { cout << " N "; }
        else if(white_bishop_board[index]) { cout << " B "; }
        else if(white_rook_board[index]) { cout << " R "; }
        else if(white_queen_board[index]) { cout << " Q "; }
        else if(black_king_board[index]) { cout << " k "; }
        else if(black_pawn_board[index]) { cout << " p "; }
        else if(black_knight_board[index]) { cout << " n "; }
        else if(black_bishop_board[index]) { cout << " b "; }
        else if(black_rook_board[index]) { cout << " r "; }
        else if(black_queen_board[index]) { cout << " q "; }
        else { cout << " - "; } } cout << endl;
    } cout << "------------------------" << endl;
    string ctom = turn ? "White" : "Black";
    string epsq = IDX2SQ(enPassantSquare);
    if(epsq == "a1") { epsq = "None"; }
    string crs = "";
    if(castlingRights & 8) { crs += "K"; }
    if(castlingRights & 4) { crs += "Q"; }
    if(castlingRights & 2) { crs += "k"; }
    if(castlingRights & 1) { crs += "q"; }
    cout << "Color To Move: " + ctom << endl;
    cout << "Castling Rights: " + crs << endl;
    cout << "En Passant Square: " + epsq << endl;
    cout << "------------------------" << endl << endl;
  }

// ******************** TESTING FUNCTIONS ******************** //

  void e4MakeUnmakeTest(int sets, int repetitions) {
    vector<double> times;
    for(int i = 0; i < sets; i++) {

      chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

      int j = 0;
      while(j < repetitions) {
        Move move = Move(12, 28, 0, 4, 0, castlingRights, enPassantSquare);
        makeMove(move);
        unmakeMove(move); j++; }

      chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
      chrono::duration<double, std::milli> required = (end - start);
      float time = required.count();

      cout << "Milliseconds: " << time << endl; times.push_back(time); }

    double average {0};
    for(int i = 0; i < sets; i++) { average += times[i]; }
    cout << endl << "Average: " << average / sets << endl;
  }

  void PseudoLegalMoveGenerationTimeTest() {
    cout << endl; vector<double> times;
    int perftNum {PseudoLegalPerft(3)};
    for(int i = 0; i < 10; i++) {

      chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

      PseudoLegalPerft(3);

      chrono::high_resolution_clock::time_point end   = chrono::high_resolution_clock::now();
      chrono::duration<double, std::milli> required = (end - start);
      float time = required.count();
      times.push_back(time); }

    double average {0};
    for(int i = 0; i < 10; i++) { average += times[i]; }
    long result = perftNum / (average / 10000);
    cout << result << " pseudo legal moves per second" << endl << endl;

  }

  void LegalMoveGenerationTimeTest() {
    cout << endl; vector<double> times;
    int perftNum {LegalPerft(3)};
    for(int i = 0; i < 10; i++) {

      chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

      LegalPerft(3);

      chrono::high_resolution_clock::time_point end   = chrono::high_resolution_clock::now();
      chrono::duration<double, std::milli> required = (end - start);
      float time = required.count();
      times.push_back(time); }

    double average {0};
    for(int i = 0; i < 10; i++) { average += times[i]; }
    cout << perftNum / (average / 10000) << " legal moves per second" << endl << endl;;

  }

  int PseudoLegalPerft(int depth) {
    if(depth == 0) { return 1; }

    vector<Move> moves = getPseudoLegalMoves();
    int numPosistions = 0;

    for(int i = 0; i < moves.size(); i++) {
      makeMove(moves[i]);
      numPosistions += PseudoLegalPerft(depth - 1);
      unmakeMove(moves[i]); }

    return numPosistions;
  }

  int LegalPerft(int depth) {
    if(depth == 0) { return 1; }

    vector<Move> moves = getLegalMoves();
    int numPosistions = 0;

    for(int i = 0; i < moves.size(); i++) {
      makeMove(moves[i]);
      numPosistions += LegalPerft(depth - 1);
      unmakeMove(moves[i]); }

    return numPosistions;
  }

  bool perftSweet() {
    long total {0};
    for(int i = 1; i < 4; i++) {
      total += LegalPerft(i); }
    loadPosistionFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    for(int i = 1; i < 4; i++) {
      total += LegalPerft(i); }
    loadPosistionFromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    for(int i = 1; i < 5; i++) {
      total += LegalPerft(i); }
    loadPosistionFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    for(int i = 1; i < 4; i++) {
      total += LegalPerft(i); }
    loadPosistionFromFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    for(int i = 1; i < 4; i++) {
      total += LegalPerft(i); }
    loadPosistionFromFen(startPosistion);
    if(total == 229172) { return true; } return false;
  }

};