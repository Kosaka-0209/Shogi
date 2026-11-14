#include "Board.hpp"
#include "Movement.hpp"
#include "Check.hpp"

//将棋盤
void DrawBoard(int startX, int startY, int cellSize, int selectedX, int selectedY)
{
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			int px = startX + x * cellSize;
			int py = startY + y * cellSize;

			if (x == selectedX && y == selectedY)
			{
				//選択中のマス
				Rect{ px,py,cellSize,cellSize }.draw(Palette::Yellow);
			}
			else
			{
				Rect{ px,py,cellSize,cellSize }.draw(ColorF{ 0.85,0.65,0.35 });
			}
			Rect{ px,py,cellSize,cellSize }.drawFrame(1, Palette::Black);
		}
	}
}

//駒
void DrawPiece(Piece board[9][9], const Font& pieceFont, int startX, int startY, int cellSize)
{
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			if (board[y][x].player == 0)
			{
				continue;
			}

			String name = board[y][x].name;

			bool isPromoted = (name == U"と" || name == U"成香" || name == U"成桂" || name == U"成銀" || name == U"馬" || name == U"龍");

			const Vec2 center{ (startX + x * cellSize) + cellSize / 2.0 ,(startY + y * cellSize) + cellSize / 2.0 };

			//成駒は赤色
			ColorF color = isPromoted ? ColorF{ 0.75,0.1,0.1 } : ColorF{ 0.1,0.1,0.1 };

			//後手の駒を180回転
			const Transformer2D transformer{ Mat3x2::Rotate(board[y][x].player == -1 ? Math::Pi : 0.0,center) };

			if (name == U"成香" || name == U"成桂" || name == U"成銀")
			{
				//上側の文字
				pieceFont(U"成").drawAt(22, Vec2{ center.x,center.y - 16 }, color);
				//下側の文字
				String bottom = name.substr(1);
				pieceFont(bottom).drawAt(22, Vec2{ center.x,center.y + 16 }, color);
			}
			else
			{
				pieceFont(name).drawAt(38, Vec2{ center.x,center.y }, color);
			}
		}
	}
}

void DrawMoveHints(Piece board[9][9], int selectedX, int selectedY, int startX, int startY, int cellSize)
{
	if (selectedX == -1 || selectedY == -1)
	{
		return;
	}

	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			if (IsLegalMove(board, selectedX, selectedY, x, y))
			{
				Rect{ startX + x * cellSize,startY + y * cellSize,cellSize,cellSize }.draw(ColorF{ 0.2,1.0,0.3,0.35 });
			}
		}
	}
}

void DrawDropHints(Piece board[9][9], const String& pieceName, int turn, int startX, int startY, int cellSize)
{
	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			bool canDrop = IsLegalDrop(board, pieceName, turn, x, y);
			//二歩の判定
			if (pieceName == U"歩")
			{
				for (int i = 0; i < 9; ++i)
				{
					//同じ列に自分の歩があるか？
					if (board[i][x].name == U"歩" && board[i][x].player == turn)
					{
						canDrop = false;
						break;
					}
				}
			}

			//歩と香車は最終段に打てない
			if (pieceName == U"歩" || pieceName == U"香")
			{
				if ((turn == 1 && y == 0) || (turn == -1 && y == 8))
				{
					canDrop = false;
				}
			}

			//桂馬は奥の二段に打てない
			if (pieceName == U"桂")
			{
				if ((turn == 1 && y <= 1) || (turn == -1 && y >= 7))
				{
					canDrop = false;
				}
			}
			//打てるマスを緑色にする
			if (canDrop)
			{
				Rect{ startX + x * cellSize,startY + y * cellSize,cellSize,cellSize }.draw(ColorF{ 0.2,1.0,0.3,0.35 });
			}
		}
	}
}
