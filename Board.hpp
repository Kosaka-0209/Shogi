#pragma once
#include <Siv3D.hpp>
#include "Piece.hpp"

//将棋盤
void DrawBoard(int startX, int startY, int cellSize, int selectedX, int selectedY);

//駒
void DrawPiece(Piece board[9][9], const Font& pieceFont, int startX, int startY, int cellSize);

void DrawMoveHints(Piece board[9][9], int selectedX, int selectedY, int startX, int startY, int cellSize);
void DrawDropHints(Piece board[9][9], const String& pieceName, int turn, int startX, int startY, int cellSize);
