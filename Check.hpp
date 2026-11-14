#pragma once
#include "Piece.hpp"

//プレイヤーが王手されているか
bool IsKingInCheck(Piece board[9][9], int player);

//王手放置せずに移動できるか
bool IsLegalMove(Piece board[9][9], int fromX, int fromY, int toX, int toY);

//王手放置せずに持ち駒を打てるか
bool IsLegalDrop(Piece board[9][9], const String& pieceName, int player, int x, int y);

//詰みかどうか判定
bool IsCheckmate(Piece board[9][9], const Array<String>& hand, int player);
