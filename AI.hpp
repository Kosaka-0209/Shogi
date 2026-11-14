#pragma once
#include <Siv3D.hpp>
#include "Piece.hpp"

// 後手CPUが1手指す。先手・後手の持ち駒も探索に使う。
bool PlayCPU(
    Piece board[9][9],
    Array<String>& goteHand,
    Array<String>& senteHand
);
