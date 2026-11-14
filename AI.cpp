#include "AI.hpp"
#include "Check.hpp"
#include "Promotion.hpp"
#include <algorithm>

namespace
{
    constexpr int SearchDepth = 5;  // 5手
    constexpr int RootCandidates = 8;
    constexpr int ChildCandidates = 4;
    constexpr int NodeLimit = 3500;
    constexpr int MateScore = 1000000;
    constexpr int Infinity = 2000000;

    struct Position
    {
        Piece board[9][9]{};
        Array<String> senteHand;
        Array<String> goteHand;
    };

    struct AIMove
    {
        bool isDrop = false;
        int fromX = -1, fromY = -1;
        int toX = -1, toY = -1;
        int handIndex = -1;
        bool promote = false;
    };

    struct RankedMove
    {
        AIMove move;
        int priority = 0;
    };

    const Array<String>& GetHand(const Position& position, int player)
    {
        return (player == 1) ? position.senteHand : position.goteHand;
    }

    Array<String>& GetHand(Position& position, int player)
    {
        return (player == 1) ? position.senteHand : position.goteHand;
    }

    int PieceValue(const String& name)
    {
        if (name == U"歩") return 100;
        if (name == U"香") return 300;
        if (name == U"桂") return 320;
        if (name == U"銀") return 500;
        if (name == U"金") return 600;
        if (name == U"角") return 800;
        if (name == U"飛") return 1000;
        if (name == U"と" || name == U"成香" || name == U"成桂" || name == U"成銀") return 650;
        if (name == U"馬") return 1100;
        if (name == U"龍") return 1300;
        return 0; // 王・玉は点数ではなく王手・詰みで扱う
    }

    // player の立場から見た盤面評価（高いほど有利）
    int Evaluate(const Position& position, int player)
    {
        int score = 0;
        for (int y = 0; y < 9; ++y)
        {
            for (int x = 0; x < 9; ++x)
            {
                const Piece& p = position.board[y][x];
                if (p.player == 0) continue;

                int value = PieceValue(p.name);
                // 歩が前進したら少し加点
                if (p.name == U"歩")
                {
                    value += (p.player == 1 ? 8 - y : y) * 3;
                }
                score += (p.player == player ? value : -value);
            }
        }

        for (const String& name : GetHand(position, player))
        {
            score += PieceValue(name);
        }
        for (const String& name : GetHand(position, -player))
        {
            score -= PieceValue(name);
        }
        return score;
    }

    // AIで生成した合法手を、仮の局面に適用する
    bool ApplyMove(Position& position, const AIMove& m, int player)
    {
        if (m.toX < 0 || m.toX >= 9 || m.toY < 0 || m.toY >= 9)
        {
            return false;
        }
        if (m.isDrop)
        {
            auto& hand = GetHand(position, player);
            if (m.handIndex < 0 || m.handIndex >= static_cast<int>(hand.size()) ||
                position.board[m.toY][m.toX].player != 0)
            {
                return false;
            }
            const String name = hand[m.handIndex];
            position.board[m.toY][m.toX] = { name, player };
            hand.erase(hand.begin() + m.handIndex);
            return true;
        }

        if (m.fromX < 0 || m.fromX >= 9 || m.fromY < 0 || m.fromY >= 9 || position.board[m.fromY][m.fromX].player != player)
        {
            return false;
        }

        if (position.board[m.toY][m.toX].player == -player)
        {
            Piece captured = position.board[m.toY][m.toX];
            UnPromotePiece(captured);
            GetHand(position, player) << captured.name;
        }

        position.board[m.toY][m.toX] = position.board[m.fromY][m.fromX];
        position.board[m.fromY][m.fromX] = {};
        if (m.promote)
        {
            PromotePiece(position.board[m.toY][m.toX]);
        }
        return true;
    }

    // 合法手を集め、駒取り・成りを優先して探索順を決める
    Array<RankedMove> GenerateMoves(Position& position, int player)
    {
        Array<RankedMove> result;
        for (int fromY = 0; fromY < 9; ++fromY)
        {
            for (int fromX = 0; fromX < 9; ++fromX)
            {
                const Piece piece = position.board[fromY][fromX];
                if (piece.player != player) continue;

                for (int toY = 0; toY < 9; ++toY)
                {
                    for (int toX = 0; toX < 9; ++toX)
                    {
                        if (!IsLegalMove(position.board, fromX, fromY, toX, toY))
                        {
                            continue;
                        }

                        // 同点の手でも、中央や前進を少し優先する
                        int distanceFromCenter = (toX > 4) ? toX - 4 : 4 - toX;
                        int forwardProgress = (player == 1) ? fromY - toY : toY - fromY;
                        int priority = (4 - distanceFromCenter) * 5 + forwardProgress * 4;
                        if (position.board[toY][toX].player == -player)
                        {
                            priority += PieceValue(position.board[toY][toX].name) * 4;
                        }

                        const bool canPromote = CanPromote(piece, fromY, toY);
                        const bool mustPromote = MustPromote(piece, toY);

                        if (!mustPromote)
                        {
                            result << RankedMove{{ false, fromX, fromY, toX, toY, -1, false }, priority};
                        }
                        if (canPromote || mustPromote)
                        {
                            result << RankedMove{{ false, fromX, fromY, toX, toY, -1, true }, priority + 250};
                        }
                    }
                }
            }
        }

        const auto& hand = GetHand(position, player);
        for (int i = 0; i < static_cast<int>(hand.size()); ++i)
        {
            // 同じ種類の持ち駒を重複して調べない
            bool duplicate = false;
            for (int j = 0; j < i; ++j)
            {
                if (hand[j] == hand[i])
                {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) continue;

            for (int y = 0; y < 9; ++y)
            {
                for (int x = 0; x < 9; ++x)
                {
                    if (IsLegalDrop(position.board, hand[i], player, x, y))
                    {
                        result << RankedMove{{ true, -1, -1, x, y, i, false },-20};
                    }
                }
            }
        }

        std::sort(result.begin(), result.end(), [](const RankedMove& a, const RankedMove& b)
            {
                return a.priority > b.priority;
            });
        return result;
    }

    // ネガマックス法 + アルファベータ枝刈り
    int Negamax(Position& position, int player, int depth,int alpha, int beta, int ply, int& visited)
    {
        ++visited;
        if (visited > NodeLimit)
        {
            return Evaluate(position, player);
        }

        if (depth == 0)
        {
            // 最終手での詰みも検出する
            if (IsKingInCheck(position.board, player))
            {
                if (GenerateMoves(position, player).isEmpty())
                {
                    return -MateScore + ply;
                }
            }
            return Evaluate(position, player);
        }

        auto moves = GenerateMoves(position, player);
        if (moves.isEmpty())
        {
            return -MateScore + ply;
        }

        int best = -Infinity;
        const int count = std::min(ChildCandidates, static_cast<int>(moves.size()));

        for (int i = 0; i < count; ++i)
        {
            Position next = position;
            if (!ApplyMove(next, moves[i].move, player)) continue;

            const int score = -Negamax(next, -player, depth - 1,-beta, -alpha, ply + 1, visited);
            if (score > best) best = score;
            if (score > alpha) alpha = score;
            if (alpha >= beta) break;
        }
        return (best == -Infinity) ? Evaluate(position, player) : best;
    }
}

bool PlayCPU(Piece board[9][9],Array<String>& goteHand, Array<String>& senteHand)
{
    Position position;
    for (int y = 0; y < 9; ++y)
    {
        for (int x = 0; x < 9; ++x)
        {
            position.board[y][x] = board[y][x];
        }
    }
    position.senteHand = senteHand;
    position.goteHand = goteHand;

    auto moves = GenerateMoves(position, -1);
    if (moves.isEmpty()) return false;

    const int count = std::min(RootCandidates, static_cast<int>(moves.size()));

    int bestScore = -Infinity;
    int bestIndex = -1;
    int alpha = -Infinity;
    const int beta = Infinity;
    int visited = 0;

    for (int i = 0; i < count; ++i)
    {
        Position next = position;
        if (!ApplyMove(next, moves[i].move, -1)) continue;

        const int score = -Negamax(next, 1, SearchDepth - 1,-beta, -alpha, 1, visited);
        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = i;
        }
        if (score > alpha) alpha = score;
    }

    if (bestIndex < 0) return false;
    if (!ApplyMove(position, moves[bestIndex].move, -1)) return false;

    for (int y = 0; y < 9; ++y)
    {
        for (int x = 0; x < 9; ++x)
        {
            board[y][x] = position.board[y][x];
        }
    }
    senteHand = position.senteHand;
    goteHand = position.goteHand;
    return true;
}
