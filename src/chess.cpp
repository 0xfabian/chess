#include "chess.h"

Square::Square() {
    piece = EMPTY;
    color = WHITE;
}

void Square::SetSquare(Square* s) {
    piece = s->getPiece();
    color = s->getColor();
}

void Square::setEmpty() {
    piece = EMPTY;
    color = WHITE;
}

void Square::setPieceAndColor(Piece _piece, Color _color) {
    piece = _piece;
    color = _color;
}
Piece Square::getPiece() {
    return piece;
}

Color Square::getColor() {
    return color;
}

void Square::setPos(int _x, int _y) {
    x = _x;
    y = _y;
}

int Square::getX() {
    return x;
}

int Square::getY() {
    return y;
}

bool Square::isEmpty() {
    return piece == EMPTY;
}

Board::Board() {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) {
            square[i][j].setEmpty();
            square[i][j].setPos(j, i);
        }

    for (int i = 0; i < 8; i++) {
        square[1][i].setPieceAndColor(PAWN, WHITE);
        square[6][i].setPieceAndColor(PAWN, BLACK);
    }

    square[0][0].setPieceAndColor(ROOK, WHITE);
    square[0][1].setPieceAndColor(KNIGHT, WHITE);
    square[0][2].setPieceAndColor(BISHOP, WHITE);
    square[0][3].setPieceAndColor(QUEEN, WHITE);
    square[0][4].setPieceAndColor(KING, WHITE);
    square[0][5].setPieceAndColor(BISHOP, WHITE);
    square[0][6].setPieceAndColor(KNIGHT, WHITE);
    square[0][7].setPieceAndColor(ROOK, WHITE);

    square[7][0].setPieceAndColor(ROOK, BLACK);
    square[7][1].setPieceAndColor(KNIGHT, BLACK);
    square[7][2].setPieceAndColor(BISHOP, BLACK);
    square[7][3].setPieceAndColor(QUEEN, BLACK);
    square[7][4].setPieceAndColor(KING, BLACK);
    square[7][5].setPieceAndColor(BISHOP, BLACK);
    square[7][6].setPieceAndColor(KNIGHT, BLACK);
    square[7][7].setPieceAndColor(ROOK, BLACK);
}

Square* Board::getSelected() {
    return selected;
}

Square* Board::getSquare(int x, int y) {
    return &square[y][x];
}

void Board::clearSelected() {
    selected = nullptr;
    validMoves.clear();
}
vector<Square> Board::getValidMoves() {
    return validMoves;
}

void Board::setSquare(Square* s, int x, int y) {
    square[y][x].SetSquare(s);
}

GAME_STATE Board::getGameState() {
    return game_state;
}

vector<Piece> Board::getCapuredWhitePieces() {
    sort(capuredWhitePieces.begin(), capuredWhitePieces.end());
    return capuredWhitePieces;
}

vector<Piece> Board::getCapuredBlackPieces() {
    sort(capuredBlackPieces.begin(), capuredBlackPieces.end());
    return capuredBlackPieces;
}

void Board::printBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            switch (square[i][j].getPiece()) {
            case KING:
                if (square[i][j].getColor() == WHITE)
                    std::cout << "K ";
                else
                    std::cout << "k ";
                break;
            case QUEEN:
                if (square[i][j].getColor() == WHITE)
                    std::cout << "Q ";
                else
                    std::cout << "q ";
                break;
            case BISHOP:
                if (square[i][j].getColor() == WHITE)
                    std::cout << "B ";
                else
                    std::cout << "b ";
                break;
            case KNIGHT:
                if (square[i][j].getColor() == WHITE)
                    std::cout << "N ";
                else
                    std::cout << "n ";
                break;
            case ROOK:
                if (square[i][j].getColor() == WHITE)
                    std::cout << "R ";
                else
                    std::cout << "r ";
                break;
            case PAWN:
                if (square[i][j].getColor() == WHITE)
                    std::cout << "P ";
                else
                    std::cout << "p ";
                break;
            case EMPTY:
                std::cout << ". ";
                break;
            }
        }
        std::cout << std::endl;
    }
}

void Board::movePiece(Square* from, Square* to) {
    int x1 = from->getX();
    int y1 = from->getY();

    int x2 = to->getX();
    int y2 = to->getY();

    square[y2][x2].SetSquare(&square[y1][x1]);
    square[y1][x1].setEmpty();
}

bool inside(int x, int y)
{
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

void Board::generateMovesDir(int x, int y, int dx, int dy, vector<Square>& moves)
{
    for (int i = 1; i < 8; i++) {
        int mx = x + dx * i;
        int my = y + dy * i;
        if (!inside(mx, my))
            break;

        if (square[my][mx].isEmpty())
            moves.push_back(square[my][mx]);
        else {
            if (square[my][mx].getColor() != square[y][x].getColor())
                moves.push_back(square[my][mx]);
            break;
        }
    }
}

vector<Square> Board::generateKingMoves(int x, int y)
{
    vector<Square> moves;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int mx = x + j;
            int my = y + i;
            if (!inside(mx, my))
                continue;
            if (square[my][mx].isEmpty() || square[my][mx].getColor() != square[y][x].getColor())
                moves.push_back(square[my][mx]);
        }
    }
    return moves;
}

vector<Square> Board::generateQueenMoves(int x, int y)
{
    vector<Square> moves;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0)
                continue;
            generateMovesDir(x, y, j, i, moves);
        }
    }
    return moves;

}

vector<Square> Board::generateBishopMoves(int x, int y)
{
    vector<Square> moves;
    generateMovesDir(x, y, 1, 1, moves);
    generateMovesDir(x, y, -1, -1, moves);
    generateMovesDir(x, y, 1, -1, moves);
    generateMovesDir(x, y, -1, 1, moves);
    return moves;
}

vector<Square> Board::generateKnightMoves(int x, int y)
{
    vector<Square> moves;
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            if (abs(i) + abs(j) != 3)
                continue;
            int mx = x + j;
            int my = y + i;
            if (!inside(mx, my))
                continue;
            if (square[my][mx].isEmpty() || square[my][mx].getColor() != square[y][x].getColor())
                moves.push_back(square[my][mx]);
        }
    }
    return moves;
}

vector<Square> Board::generateRookMoves(int x, int y)
{
    vector<Square> moves;
    generateMovesDir(x, y, 1, 0, moves);
    generateMovesDir(x, y, -1, 0, moves);
    generateMovesDir(x, y, 0, 1, moves);
    generateMovesDir(x, y, 0, -1, moves);
    return moves;
}

vector<Square> Board::generatePawnMoves(int x, int y)
{
    vector<Square> moves;
    Color color = square[y][x].getColor();
    int dir = (color == WHITE) ? 1 : -1;

    if (inside(x, y + dir) && square[y + dir][x].isEmpty())
        moves.push_back(square[y + dir][x]);

    if ((y == 1 || y == 6) && square[y + 2 * dir][x].isEmpty())
        moves.push_back(square[y + 2 * dir][x]);

    if (inside(x + 1, y + dir) && square[y + dir][x + 1].getColor() != color && !square[y + dir][x + 1].isEmpty())
        moves.push_back(square[y + dir][x + 1]);

    if (inside(x - 1, y + dir) && square[y + dir][x - 1].getColor() != color && !square[y + dir][x - 1].isEmpty())
        moves.push_back(square[y + dir][x - 1]);

    return moves;
}
vector<Square> Board::generateMoves(Square* s) {
    int x = s->getX();
    int y = s->getY();
    switch (s->getPiece())
    {
    case KING:
        return generateKingMoves(x, y);

    case QUEEN:
        return generateQueenMoves(x, y);

    case BISHOP:
        return generateBishopMoves(x, y);

    case KNIGHT:
        return generateKnightMoves(x, y);

    case ROOK:
        return generateRookMoves(x, y);

    case PAWN:
        return generatePawnMoves(x, y);

    default:
        return {};
    }
}

bool Board::isCheck(Color color) {
    int x, y;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (square[i][j].getPiece() == KING && square[i][j].getColor() == color) {
                x = j;
                y = i;
            }

    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (!square[i][j].isEmpty() && square[i][j].getColor() != color) {
                vector<Square> moves = generateMoves(&square[i][j]);
                for (auto& move : moves)
                    if (move.getX() == x && move.getY() == y)
                        return true;
            }
    return false;
}

bool Board::squareAttacked(int x, int y, Color color) {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (!square[i][j].isEmpty() && square[i][j].getColor() != color) {
                vector<Square> moves = generateMoves(&square[i][j]);
                for (auto& move : moves)
                    if (move.getX() == x && move.getY() == y)
                        return true;
            }
    return false;
}

bool Board::hasAnyValidMove(Color color) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (square[i][j].isEmpty() || square[i][j].getColor() != color)
                continue;
            validMoves.clear();
            generateValidMoves(&square[i][j]);
            if (!validMoves.empty()) {
                clearSelected();
                return true;
            }
        }
    }
    return false;
}

void Board::generateValidMoves(Square* s) {
    
    validMoves.clear();
    vector<Square> moves = generateMoves(s);
    for (auto& move : moves) {
        Square copySquare = move;
        movePiece(s, &move);
        if (!isCheck(turn))
            validMoves.push_back(move);
        movePiece(&move, s);
        setSquare(&copySquare, move.getX(), move.getY());
    }
}

void Board::addSpecialMoves(Square* s)
{
    int x = s->getX();
    int y = s->getY();
    int dir = (turn == WHITE) ? 1 : -1;
    if(s->getPiece() == KING)
    {
        if(square[y][x+1].isEmpty() && square[y][x+2].isEmpty() && !isCheck(turn) && !squareAttacked(x+1, y, turn) && !squareAttacked(x+2, y, turn))
            if((castlingWhiteKing && turn == WHITE) || (castlingBlackKing && turn == BLACK))
                validMoves.push_back(square[y][x + 2]);
        if(square[y][x-1].isEmpty() && square[y][x-2].isEmpty() && square[y][x-3].isEmpty() && !isCheck(turn)  && !squareAttacked(x-1, y, turn) && !squareAttacked(x-2, y, turn))
            if((castlingWhiteQueen && turn == WHITE) || (castlingBlackQueen && turn == BLACK))
                validMoves.push_back(square[y][x - 2]);
    }

    if(s->getPiece() == PAWN)
    {
        if (enPassant != nullptr && enPassant->getX() == x + 1 && enPassant->getY() == y && s->getColor() != enPassant->getColor())
            validMoves.push_back(square[y + dir][x + 1]);
        if (enPassant != nullptr && enPassant->getX() == x - 1 && enPassant->getY() == y && s->getColor() != enPassant->getColor())
            validMoves.push_back(square[y + dir][x - 1]);
    }
}

void Board::checkSpecialMoves(Square* move)
{
    int dir = (turn == WHITE) ? 1 : -1;

    if(enPassant != nullptr && selected->getPiece() == PAWN && move->getX() == enPassant->getX() && move->getY() == enPassant->getY() + dir)
        square[enPassant->getY()][enPassant->getX()].setEmpty();

    if(selected->getPiece() == PAWN && abs(selected->getY() - move->getY()) == 2)
        enPassant = &square[move->getY()][move->getX()];
    else
        enPassant = nullptr;

    if (selected->getPiece() == KING) {
        if (move->getX() == selected->getX() + 2)
            movePiece(&square[selected->getY()][7], &square[selected->getY()][5]);

        if (move->getX() == selected->getX() - 2) 
            movePiece(&square[selected->getY()][0], &square[selected->getY()][3]);

        if (turn == WHITE) {
            castlingWhiteKing = false;
            castlingWhiteQueen = false;
        }
        else
        {
            castlingBlackKing = false;
            castlingBlackQueen = false;
        }
    }

    if (selected->getPiece() == ROOK) {
        if (turn == WHITE && selected->getX() == 0)
            castlingWhiteQueen = false;
        if (turn == WHITE && selected->getX() == 7)
            castlingWhiteKing = false;
        if (turn == BLACK && selected->getX() == 0)
            castlingBlackQueen = false;
        if (turn == BLACK && selected->getX() == 7)
            castlingBlackKing = false;
    }
}

// bool operator<(const Piece& a, const Piece& b)
// {
//     return a < b;
// }
void Board::click(int x, int y) {  
    Square* s = &square[y][x];
    if (selected == nullptr)
    {
        if (!s->isEmpty() && s->getColor() == turn) {
            selected = s;
            generateValidMoves(selected);
            addSpecialMoves(selected);
        }
    }
    else
    {
        for (auto& move : validMoves)
            if (move.getX() == x && move.getY() == y)
            {
                checkSpecialMoves(&move);
                if(move.getPiece() != EMPTY)
                {
                    if(move.getColor() == WHITE)
                        capuredWhitePieces.push_back(move.getPiece());
                    else
                        capuredBlackPieces.push_back(move.getPiece());

                }
                movePiece(selected, &move);
                clearSelected();
                turn = (turn == WHITE) ? BLACK : WHITE;
                if (!hasAnyValidMove(turn)) {
                    if (isCheck(turn))
                        game_state = CHECKMATE;
                    else
                        game_state = STALEMATE;
                }
                return;
            }

        if (!s->isEmpty() && s->getColor() == turn) {
            selected = s;
            generateValidMoves(selected);
            addSpecialMoves(selected);
        }
        else
            clearSelected();
    }
}