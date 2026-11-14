#include "Promotion.hpp"

//成ることができるかどうか
bool CanPromote(Piece piece, int fromY, int toY)
{
	if (piece.name != U"歩" && piece.name != U"香" && piece.name != U"桂" && piece.name != U"銀" && piece.name != U"角" && piece.name != U"飛")
	{
		return false;
	}

	if ((piece.player == 1))//先手
	{
		return (fromY <= 2 || toY <= 2);
	}

	if(piece.player == -1) //後手
	{
		return (fromY >= 6 || toY >= 6);
	}

	return false;
}

//強制成
bool MustPromote(Piece piece, int toY)
{
	if (piece.name == U"歩" || piece.name == U"香")
	{
		return (piece.player == 1 && toY == 0) || (piece.player == -1 && toY == 8);
	}

	if (piece.name == U"桂")
	{
		return (piece.player == 1 && toY <= 1) || (piece.player == -1 && toY >= 7);
	}

	return false;

}

//駒成る
void PromotePiece(Piece& piece)
{
	if (piece.name == U"歩")
	{
		piece.name = U"と";
	}
	else if (piece.name == U"香")
	{
		piece.name = U"成香";
	}
	else if (piece.name == U"桂")
	{
		piece.name = U"成桂";
	}
	else if (piece.name == U"銀")
	{
		piece.name = U"成銀";
	}
	else if (piece.name == U"飛")
	{
		piece.name = U"龍";
	}
	else if (piece.name == U"角")
	{
		piece.name = U"馬";
	}

}

//成駒を戻す
void UnPromotePiece(Piece& piece)
{
	if (piece.name == U"と")
	{
		piece.name = U"歩";
	}
	else if (piece.name == U"成香")
	{
		piece.name = U"香";
	}
	else if (piece.name == U"成桂")
	{
		piece.name = U"桂";
	}
	else if (piece.name == U"成銀")
	{
		piece.name = U"銀";
	}
	else if (piece.name == U"龍")
	{
		piece.name = U"飛";
	}
	else if (piece.name == U"馬")
	{
		piece.name = U"角";
	}

}
