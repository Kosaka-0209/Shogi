#include "Check.hpp"
#include "Movement.hpp"

bool IsKingInCheck(Piece board[9][9], int player)
{
	int kingX = -1;
	int kingY = -1;

	//自分の王将を探す
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			if (board[y][x].player == player && (board[y][x].name == U"王" || board[y][x].name == U"玉"))
			{
				kingX = x;
				kingY = y;
			}
		}
	}

	if (kingX == -1)return false;//王将が見つからなかったら

	//相手の駒が王将を狙っているか
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			if (board[y][x].player == -player)
			{
				if (CanMove(board, x, y, kingX, kingY))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool IsLegalMove(Piece board[9][9], int fromX, int fromY, int toX, int toY)
{
	//座標チェック
	if (fromX < 0 || fromX >= 9 || fromY < 0 || fromY >= 9 || toX < 0 || toX >= 9 || toY < 0 || toY >= 9)
	{
		return false;
	}

	int player = board[fromY][fromX].player;
	if (player == 0)return false;
	if (!CanMove(board, fromX, fromY, toX, toY))return false;
	if (board[toY][toX].name == U"王" || board[toY][toX].name == U"玉") return false;

	//盤面をコピー
	Piece tempBoard[9][9];

	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			tempBoard[y][x] = board[y][x];
		}
	}

	//仮に駒を移動
	tempBoard[toY][toX] = tempBoard[fromY][fromX];
	tempBoard[fromY][fromX] = {};

	//自分の王将が王手されるなら移動禁止
	if (IsKingInCheck(tempBoard, player))
	{
		return false;
	}
	return true;
}

bool IsLegalDrop(Piece board[9][9], const String& pieceName, int player, int x, int y)
{
	if (x < 0 || x >= 9 || y < 0 || y >= 9)return false;

	if (player != 1 && player != -1)return false;

	if (board[y][x].player != 0)return false;

	//二歩の判定
	if (pieceName == U"歩")
	{
		for (int i = 0; i < 9; ++i)
		{
			//同じ列に自分の歩があるか？
			if (board[i][x].player == player && board[i][x].name == U"歩")
			{
				return false;
			}
		}
	}
	//歩と香車は最終段に打てない
	if (pieceName == U"歩" || pieceName == U"香")
	{
		if ((player == 1 && y == 0) || (player == -1 && y == 8))
		{
			return false;
		}
	}

	//桂馬は奥の二段に打てない
	if (pieceName == U"桂")
	{
		if ((player == 1 && y <= 1) || (player == -1 && y >= 7))
		{
			return false;
		}
	}

	//盤面をコピー
	Piece tempBoard[9][9];

	for (int row = 0; row < 9; ++row)
	{
		for (int col = 0; col < 9; ++col)
		{
			tempBoard[row][col] = board[row][col];
		}
	}

	//仮に持ち駒を打つ
	tempBoard[y][x] = { pieceName,player };

	//王手が残るなら打てない
	return !IsKingInCheck(tempBoard, player);

}

bool IsCheckmate(Piece board[9][9], const Array<String>& hand, int player)
{
	if (!IsKingInCheck(board, player)) return false;

	//盤上の駒で王手を防げるかを調べる
	for (int fromY = 0; fromY < 9; ++fromY)
	{
		for (int fromX = 0; fromX < 9; ++fromX)
		{
			if (board[fromY][fromX].player != player)
			{
				continue;
			}
			for (int toY = 0; toY < 9; ++toY)
			{
				for (int toX = 0; toX < 9; ++toX)
				{
					if (IsLegalMove(board, fromX, fromY, toX, toY))
					{
						return false;
					}
				}
			}
		}
	}

	//持ち駒で王手を防げるかを調べる
	for (const String& name : hand)
	{
		for (int y = 0; y < 9; ++y)
		{
			for (int x = 0; x < 9; ++x)
			{
				if (IsLegalDrop(board, name, player, x, y))
				{
					return false;
				}
			}
		}
	}
	return true;
}
