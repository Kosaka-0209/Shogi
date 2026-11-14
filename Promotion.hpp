#pragma once
#include "Piece.hpp"

//成ることができるかどうか
bool CanPromote(Piece piece, int fromY, int toY);

//強制成
bool MustPromote(Piece piece, int toY);

//駒を成らせる
void PromotePiece(Piece& piece);

//成駒をもとの駒に戻す
void UnPromotePiece(Piece& piece);
