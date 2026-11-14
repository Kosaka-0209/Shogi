#include "Movement.hpp"

bool CanMove(Piece board[9][9], int fromX, int fromY, int toX, int toY)
{
	//盤外なら移動できない
	if (toX < 0 || toX >= 9 || toY < 0 || toY >= 9)
	{
		return false;
	}

	Piece piece = board[fromY][fromX];

	//駒がない場合
	if (piece.player == 0)
	{
		return false;
	}

	//自分の駒があるマスには移動できない
	if (board[toY][toX].player == piece.player)
	{
		return false;
	}

	int dx = toX - fromX;
	int dy = toY - fromY;
	int forward = -piece.player;

	bool canMove = false;
	int stepX = 0;
	int stepY = 0;

	if (piece.name == U"歩")
	{
		canMove = (dx == 0 && dy == forward);
	}
	else if (piece.name == U"金" || piece.name == U"と" || piece.name == U"成香" || piece.name == U"成桂" || piece.name == U"成銀")
	{
		canMove = (dy == forward && Abs(dx) <= 1) || (dy == 0 && Abs(dx) == 1) || (dy == -forward && dx == 0);
	}
	else if (piece.name == U"銀")
	{
		canMove = (dy == forward && dx >= -1 && dx <= 1) || (dy == -forward && (dx == -1 || dx == 1));
	}
	else if (piece.name == U"桂")
	{
		canMove = (dy == forward * 2) && (dx == -1 || dx == 1);
	}
	else if (piece.name == U"王" || piece.name == U"玉")
	{
		canMove = (Abs(dx) <= 1) && (Abs(dy) <= 1) && (dx != 0 || dy != 0);
	}
	else if (piece.name == U"香")
	{
		canMove = (dx == 0 && dy * forward > 0);
		if (canMove)
		{
			stepY = forward;
		}
	}
	else if (piece.name == U"飛")
	{
		canMove = (dx == 0 && dy != 0) || (dy == 0 && dx != 0);
		if (canMove)
		{
			stepX = (dx > 0) - (dx < 0);
			stepY = (dy > 0) - (dy < 0);
		}
	}
	else if (piece.name == U"角")
	{
		canMove = (Abs(dx) == Abs(dy)) && (dx != 0);
		if (canMove)
		{
			stepX = (dx > 0) - (dx < 0);
			stepY = (dy > 0) - (dy < 0);
		}
	}
	else if (piece.name == U"龍")
	{
		if ((dx == 0 && dy != 0) || (dy == 0 && dx != 0))
		{
			canMove = true;
			stepX = (dx > 0) - (dx < 0);
			stepY = (dy > 0) - (dy < 0);
		}
		else if (Abs(dx) == 1 && Abs(dy) == 1)
		{
			canMove = true;
		}
	}
	else if (piece.name == U"馬")
	{
		if (Abs(dx) == Abs(dy) && dx != 0)
		{
			canMove = true;
			stepX = (dx > 0) - (dx < 0);
			stepY = (dy > 0) - (dy < 0);
		}
		else if ((Abs(dx) == 1 && dy == 0) || (Abs(dy) == 1 && dx == 0))
		{
			canMove = true;
		}
	}

	if (canMove && (stepX != 0 || stepY != 0))
	{
		int checkX = fromX + stepX;
		int checkY = fromY + stepY;

		while (checkX != toX || checkY != toY)
		{
			//盤面の外に出ていないか確認
			if (checkX < 0 || checkX >= 9 || checkY < 0 || checkY >= 9)
			{
				return false;
			}
			//途中に駒があるか確認
			if (board[checkY][checkX].player != 0)
			{
				return false;
			}

			checkX += stepX;
			checkY += stepY;
		}
	}
	return canMove;
}
