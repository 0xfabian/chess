#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

enum Piece { KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN, EMPTY };
enum Color { BLACK, WHITE };
enum GAME_STATE { CHECKMATE, STALEMATE, ONGOING };

class Square
{
    Piece piece;
    Color color;

    int x, y;

public:
    void SetSquare(Square*);
    void setEmpty();
    void setPieceAndColor(Piece, Color);
    Piece getPiece();
    Color getColor();
    void setPos(int _x, int _y);
    int getX();
    int getY();
    bool isEmpty();
    Square();
};

class Board
{
    Square square[8][8];
    Color turn = WHITE;
    Square* selected = nullptr;
    vector <Square> validMoves;
    GAME_STATE game_state = ONGOING;
    Square* enPassant = nullptr;
    vector<Piece> capuredWhitePieces;
    vector<Piece> capuredBlackPieces;
    bool castlingWhiteKing = true;
    bool castlingWhiteQueen = true;
    bool castlingBlackKing = true;
    bool castlingBlackQueen = true;

public:
    Board();
    GAME_STATE getGameState();
    Square* getSquare(int x, int y);
    void setSquare(Square* square, int x, int y);
    void printBoard();
    void movePiece(Square* from, Square* to);
    vector<Square> generateMoves(Square*);
    bool isCheck(Color color);
    bool squareAttacked(int x, int y, Color color);
    bool hasAnyValidMove(Color color);
    void generateValidMoves(Square*);
    void addSpecialMoves(Square* s);
    void checkSpecialMoves(Square* move);
    void click(int x, int y);
    Square* getSelected();
    vector<Square> getValidMoves();
    void clearSelected();
    void generateMovesDir(int x, int y, int dx, int dy, vector<Square>& moves);
    vector<Square> generateKingMoves(int x, int y);
    vector<Square> generateQueenMoves(int x, int y);
    vector<Square> generateBishopMoves(int x, int y);
    vector<Square> generateKnightMoves(int x, int y);
    vector<Square> generateRookMoves(int x, int y);
    vector<Square> generatePawnMoves(int x, int y);
    vector<Piece> getCapuredWhitePieces();
    vector<Piece> getCapuredBlackPieces();
};