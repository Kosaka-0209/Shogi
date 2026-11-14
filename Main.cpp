#include <Siv3d.hpp>
#include "Piece.hpp"
#include "Movement.hpp"
#include "Promotion.hpp"
#include "Board.hpp"
#include "Check.hpp"
#include "AI.hpp"
#include "Title.hpp"

struct MoveEffect : IEffect
{
	Vec2 pos;
	explicit MoveEffect(const Vec2& p) : pos(p) {}

	bool update(double t) override
	{
		if (t >= 0.6)
		{
			return false;

		}

		double rate = t / 0.6;

		//時間とともに広がる輪
		Circle{ pos,10 + rate * 45 }.drawFrame(5, ColorF{ 1.0,0.85,0.2,1.0 - rate });
		return true;
	}
};

void Main()
{
	Window::Resize(800, 800);

	const int cellSize = 60;//マスの大きさ

	//盤面上の左上の座標
	const int startX = 130;
	const int startY = 140;

	const Font font{ 36,Typeface::Bold };
	//将棋駒専用フォント
	const Font pieceFont{ FontMethod::MSDF,48,U"font/Kouzan.ttf" };


	Piece board[9][9] = {};

	Array<String> senteHand;//先手
	Array<String> goteHand;//後手

	//将棋の駒を管理する配列
	String back[9] =
	{
		U"香", U"桂", U"銀", U"金", U"王", U"金", U"銀", U"桂", U"香"
	};
	//先手と後手の一段目
	for (int x = 0; x < 9; ++x)
	{
		board[0][x] = { back[x], -1 };
		board[8][x] = { back[x],  1 };
	}

	//歩を配置
	for (int x = 0; x < 9; ++x)
	{
		board[2][x] = { U"歩",-1 };
		board[6][x] = { U"歩", 1 };
	}

	board[1][1] = { U"飛",-1 };
	board[1][7] = { U"角",-1 };

	board[7][7] = { U"飛", 1 };
	board[7][1] = { U"角", 1 };

	board[0][4] = { U"王", -1 };
	board[8][4] = { U"玉",  1 };

	int selectedX = -1;
	int selectedY = -1;

	int selectedHand = -1;

	int selectedHandPlayer = 0;

	int turn = 1;

	bool promotionPending = false;

	int promotionX = -1;
	int promotionY = -1;

	bool gameOver = false;
	int winner = 0;

	double cpuWaitTime = 0.0;

	bool title = true;

	bool startMessage = false;
	double startMessageTime = 0.0;

	const Audio startVoice{ U"sound/startVoice.wav" };
	const Audio titleBGM{ Audio::Stream, U"sound/titleBGM.mp3",Loop::Yes };
	const Audio gameBGM{ Audio::Stream,U"sound/gameBGM.mp3" ,Loop::Yes};
	const Audio moveSE{ U"sound/move.mp3" };
	const Audio checkSE{ U"sound/check.mp3" };
	startVoice.setVolume(1.0);
	titleBGM.setVolume(0.35);
	gameBGM.setVolume(0.2);
	moveSE.setVolume(0.7);
	checkSE.setVolume(0.9);

	titleBGM.play(1s);

	auto EndTurn = [&]()
		{
			if (IsKingInCheck(board, -turn))
			{
				checkSE.playOneShot();
			}
			else
			{
				moveSE.playOneShot();
			}

			turn = -turn;

			//次のプレイヤーの持ち駒を取得
			const Array<String>& hand = (turn == 1) ? senteHand : goteHand;

			//詰みか判定
			if (IsCheckmate(board, hand, turn))
			{
				gameOver = true;
				winner = -turn;//詰まされた側の相手が勝者
			}
		};

	Effect effects;

	Piece initialBoard[9][9];

	for (int y = 0; y < 9; ++y)
	{
		for (int x = 0; x < 9; ++x)
		{
			initialBoard[y][x] = board[y][x];
		}
	}

	const auto initialSenteHand = senteHand;
	const auto initialGoteHand = goteHand;

	while (System::Update())
	{
		//背景色
		Scene::SetBackground(ColorF{ 0.2,0.25,0.2 });

		//将棋盤
		DrawBoard(startX, startY, cellSize, selectedX, selectedY);
		DrawMoveHints(board, selectedX, selectedY, startX, startY, cellSize);
		//持ち駒を選択している場合
		if (selectedHand != -1 && selectedHandPlayer == turn)
		{
			Array<String>& hand = (turn == 1) ? senteHand : goteHand;
			if (selectedHand >= 0 && selectedHand < static_cast<int>(hand.size()))
			{
				DrawDropHints(board, hand[selectedHand], turn, startX, startY, cellSize);
			}
		}
		DrawPiece(board, pieceFont, startX, startY, cellSize);
		for (int y = 0; y < 9; ++y)
		{
			for (int x = 0; x < 9; ++x)
			{
				//先手の持ち駒処理
				font(U"先手の持ち駒").draw(20, 700, Palette::White);
				for (int i = 0; i < static_cast<int>(senteHand.size()); ++i)
				{
					int handX = 20 + i * 45;
					int handY = 750;

					Rect handRect{ handX,handY,40,45 };

					//選択中の持ち駒を黄色にする
					if (selectedHand == i && selectedHandPlayer == 1)
					{
						handRect.draw(ColorF{ 1.0,0.9,0.3 });
					}
					pieceFont(senteHand[i]).draw(handX, handY, Palette::White);

					//持ち駒をクリック
					if (handRect.leftClicked() && turn == 1 && !promotionPending)
					{
						if (selectedHand == i && selectedHandPlayer == 1)
						{
							selectedHand = -1;
							selectedHandPlayer = 0;
						}
						else
						{
							selectedHand = i;
							selectedHandPlayer = 1;

							selectedX = -1;
							selectedY = -1;
						}
					}
				}

				//後手の持ち駒処理
				font(U"後手の持ち駒").draw(20, 20, Palette::White);
				for (int i = 0; i < static_cast<int>(goteHand.size()); ++i)
				{
					int handX = 20 + i * 45;
					int handY = 70;

					Rect handRect{ handX,handY,40,45 };

					//選択中の持ち駒を黄色にする
					if (selectedHand == i && selectedHandPlayer == -1)
					{
						handRect.draw(ColorF{ 1.0,0.9,0.3 });
					}

					//後手の駒を逆さまに表示
					const Vec2 center = handRect.center();
					{
						const Transformer2D transformer{ Mat3x2::Rotate(Math::Pi,center) };
						pieceFont(goteHand[i]).drawAt(center, Palette::White);
					}

					//持ち駒をクリック
					if (handRect.leftClicked() && turn == -1 && !promotionPending)
					{
						if (selectedHand == i && selectedHandPlayer == -1)
						{
							//同じ持ち駒なら選択解除
							selectedHand = -1;
							selectedHandPlayer = 0;
						}
						else
						{
							selectedHand = i;
							selectedHandPlayer = -1;

							selectedX = -1;
							selectedY = -1;
						}
					}
				}

				if (turn == 1)
				{
					font(U"先").draw(40, 150, Palette::White);
					font(U"手").draw(40, 200, Palette::White);
					font(U"の").draw(40, 250, Palette::White);
					font(U"番").draw(40, 300, Palette::White);
				}
				else
				{
					font(U"後").draw(40, 150, Palette::White);
					font(U"手").draw(40, 200, Palette::White);
					font(U"の").draw(40, 250, Palette::White);
					font(U"番").draw(40, 300, Palette::White);

				}

				if (promotionPending)
				{
					Rect promoteButton{ 270,300,150,60 };
					Rect cancelButton{ 460,300,150,60 };

					promoteButton.draw(Palette::Lightgreen);
					cancelButton.draw(Palette::Lightgray);

					font(U"成る").drawAt(promoteButton.center(), Palette::Black);
					font(U"成らない").drawAt(cancelButton.center(), Palette::Black);

					if (promoteButton.leftClicked())
					{
						PromotePiece(board[promotionY][promotionX]);
						promotionPending = false;
						EndTurn();
					}
					else if (cancelButton.leftClicked())
					{
						promotionPending = false;
						EndTurn();
					}
				}

				//CPU
				if (turn == -1 && !promotionPending && !gameOver && !title)
				{
					cpuWaitTime += Scene::DeltaTime();
					if (cpuWaitTime >= 60.0)
					{
						cpuWaitTime = 0.0;
						Piece oldBoard[9][9];
						for (int y = 0; y < 9; ++y)
						{
							for (int x = 0; x < 9; ++x)
							{
								oldBoard[y][x] = board[y][x];
							}
						}

						if (PlayCPU(board, goteHand, senteHand))
						{
							for (int y = 0; y < 9; ++y)
							{
								for (int x = 0; x < 9; ++x)
								{
									if (board[y][x].player == -1 && oldBoard[y][x].player != -1)
									{
										Vec2 effectPos{ startX + x * cellSize + cellSize / 2.0, startY + y * cellSize + cellSize / 2.0 };
										effects.add<MoveEffect>(effectPos);
									}
								}
							}
							EndTurn();
						}
					}
				}
				else
				{
					cpuWaitTime = 0.0;
				}

				if (IsKingInCheck(board, turn))
				{
					font(U"王手").drawAt(Vec2{ 450,25 }, Palette::Red);
				}

				effects.update();

				// 対局終了画面
				if (gameOver)
				{

					// 詰み
					font(U"詰み！").drawAt(Vec2{ 400, 330 },Palette::Yellow);

					// 先手が勝った場合
					if (winner == 1)
					{
						font(U"先手の勝利！").drawAt(Vec2{ 400, 400 },Palette::White);
					}
					// 後手が勝った場合
					else if (winner == -1)
					{
						font(U"後手の勝利！").drawAt(Vec2{ 400, 400 },Palette::White);
					}

					Rect titleButton{ 260, 470, 280, 60 };

					titleButton.draw(titleButton.mouseOver() ? ColorF{ 0.85, 0.70, 0.4 } : ColorF{ 0.65, 0.50, 0.3 });

					titleButton.drawFrame(2, Palette::Black);

					font(U"タイトルに戻る").drawAt(titleButton.center(), Palette::Black);

					// ボタンを押したら
					if (titleButton.leftClicked())
					{
						// 盤面を初期状態に戻す
						for (int y = 0; y < 9; ++y)
						{
							for (int x = 0; x < 9; ++x)
							{
								board[y][x] = initialBoard[y][x];
							}
						}

						// 持ち駒をリセット
						senteHand = initialSenteHand;
						goteHand = initialGoteHand;

						// 対局状態をリセット
						turn = 1;
						winner = 0;
						gameOver = false;
						promotionPending = false;
						cpuWaitTime = 0.0;

						// タイトル画面へ
						title = true;
						startMessage = false;
						startMessageTime = 0.0;

						gameBGM.stop(0.5s);
						titleBGM.play(1s);

						continue;
					}
				}
			}
		}

		//マウス
		if (MouseL.down() && turn == 1 && !promotionPending && !gameOver && !title)
		{
			const Point mousePos = Cursor::Pos();

			int x = (mousePos.x - startX) / cellSize;
			int y = (mousePos.y - startY) / cellSize;
			if (0 <= x && x < 9 && 0 <= y && y < 9)
			{
				if (selectedHand != -1 && selectedHandPlayer == turn)
				{
					//選択中のプレイヤーの持ち駒
					Array<String>& hand = (turn == 1) ? senteHand : goteHand;

					bool nifu = false;

					//歩を打つときだけ二歩を調べる
					if (hand[selectedHand] == U"歩")
					{
						for (int i = 0; i < 9; ++i)
						{
							//同じ列に自分の歩があるか？
							if (board[i][x].name == U"歩" && board[i][x].player == turn)
							{
								nifu = true;
								break;
							}
						}
					}

					if (IsLegalDrop(board,hand[selectedHand],turn,x,y))
					{
						//空いているマスかつ二歩でなければ打てる
						if (board[y][x].player == 0 && !nifu)
						{
							board[y][x] = { hand[selectedHand],turn };

							//使用した持ち駒を削除
							hand.erase(hand.begin() + selectedHand);
							selectedHand = -1;
							selectedHandPlayer = 0;
							EndTurn();
						}
					}
				}
				else if (selectedX == -1)
				{
					//駒を選択
					if (board[y][x].player == turn)
					{
						selectedX = x;
						selectedY = y;
					}
				}
				else
				{
					Piece piece = board[selectedY][selectedX];
					//空いているマスに移動
					if (board[y][x].player != piece.player)
					{
						bool canMove = IsLegalMove(board, selectedX, selectedY, x, y);

						if (canMove)
						{
							//移動先に相手の駒がある場合
							if (board[y][x].player != 0)
							{
								Piece capturedPiece= board[y][x];
								UnPromotePiece(capturedPiece);
								//先手がとった場合
								if (piece.player == 1)
								{
									senteHand << capturedPiece.name;
								}
								else
								{
									goteHand << capturedPiece.name;
								}
							}

							bool canPromote = CanPromote(piece, selectedY, y);
							bool mustPromote = MustPromote(piece, y);

	                        //通常の移動処理
							board[y][x] = piece;
							board[selectedY][selectedX] = {};

							Vec2 effectPos{ startX + x * cellSize + cellSize / 2.0, startY + y * cellSize + cellSize / 2.0 };
							effects.add<MoveEffect>(effectPos);

							//強制成
							if (mustPromote)
							{
								PromotePiece(board[y][x]);
								EndTurn();
							}
							//成るか成らないか
							else if (canPromote)
							{
								promotionPending = true;
								promotionX = x;
								promotionY = y;
							}
							else
							{
								EndTurn();
							}
						}
					}



					//選択を解除
					selectedX = -1;
					selectedY = -1;
				}
			}
		}

		if (title)
		{
			if (ShowTitle(pieceFont, font))
			{
				title = false;
				startMessage = true;
				startMessageTime = 0.0;
				titleBGM.stop(0.4s);
				startVoice.playOneShot();
			}
			continue;
		}

		if (startMessage)
		{
			Scene::SetBackground(ColorF{ 0.07,0.13,0.11 });
			startMessageTime += Scene::DeltaTime();
			double alpha = 1.0;

			if (startMessageTime < 0.3)
			{
				alpha = startMessageTime / 0.3;
			}
			else if (startMessageTime > 1.2)
			{
				alpha = (1.5 - startMessageTime) / 0.3;
			}

			pieceFont(U"対局開始").drawAt(64, Scene::Center(), ColorF{0.0,0.0,0.0,alpha});

			if (startMessageTime >= 1.5)
			{
				startMessage = false;
				gameBGM.play(1s);
			}
			continue;
		}
	}
}
