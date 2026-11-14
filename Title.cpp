#include "Title.hpp"

bool ShowTitle(const Font& titleFont, const Font& uiFont)
{
	const int x = Scene::Width() / 2;
	const int y = Scene::Height() / 2;

	Rect{ 0,0,Scene::Width(),Scene::Height() }.draw(ColorF{ 0.07,0.13,0.11 });

	Rect panel{ x - 290,y - 280,580,560 };
	panel.draw(ColorF{ 0.1,0.18,0.15 });
	panel.drawFrame(3, ColorF{ 0.7,0.53,0.25 });

	uiFont(U"JAPANESE CHESS").drawAt(20, Vec2{ x,y - 220 }, ColorF{ 0.8,0.7,0.5 });

	titleFont(U"将棋").drawAt(90, Vec2{ x,y - 125 }, ColorF{ 0.95,0.8,0.55 });
	uiFont(U"先手:プレイヤー 後手:CPU").drawAt(18, Vec2{ x,y + 40 }, ColorF{ 0.75,0.8,0.75 });
	const Rect startButton{ x - 155,y + 95,310,65 };

	if (startButton.mouseOver())
	{
		startButton.draw(ColorF{ 0.92,0.75,0.38 });
	}
	else
	{
		startButton.draw(ColorF{ 0.72,0.55,0.26 });
	}

	startButton.drawFrame(2, ColorF{ 0.96,0.85,0.56 });
	uiFont(U"対局開始").drawAt(28, startButton.center(), ColorF{ 0.12,0.16,0.11 });
	uiFont(U"ENTERキーで開始").drawAt(17, Vec2{ x,y + 210 }, ColorF{ 0.65,0.7,0.65 });
	return startButton.leftClicked() || KeyEnter.down();
}
