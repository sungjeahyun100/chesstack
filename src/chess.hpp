#pragma once
#include <gameboard.hpp>
#include <piece.hpp>
#include <moves.hpp>
#include <enum.hpp>

// 기물 패턴 설정 함수
// 주어진 기물에 적절한 이동 패턴을 추가합니다.
inline void setupPiecePatterns(piece* p) {
    if (!p) return;
    
    pieceType type = p->getPieceType();
    
    switch(type) {
        case pieceType::KNIGHT:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::JUMP, KNIGHT_DIRECTIONS, 1));
            break;
        case pieceType::BISHOP:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, BISHOP_DIRECTIONS));
            break;
        case pieceType::ROOK:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, ROOK_DIRECTIONS));
            break;
        case pieceType::QUEEN:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, QUEEN_DIRECTIONS));
            break;
        case pieceType::KING:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KING_DIRECTIONS, 1));
            break;
        case pieceType::PWAN:
            // 폰: 전진 + 대각선 캡처 (색상에 따라 방향 달라짐)
            if(p->getColor() == colorType::WHITE) {
                // 백: +1 방향 (상향)
                std::vector<std::pair<int, int>> pawnForward = {{0, 1}};
                p->addMovePattern(legalMoveChunk(threatType::MOVE, moveType::RAY_FINITE, pawnForward, 1));
                std::vector<std::pair<int, int>> pawnCapture = {{-1, 1}, {1, 1}};
                p->addMovePattern(legalMoveChunk(threatType::TAKE, moveType::RAY_FINITE, pawnCapture, 1));
            } else {
                // 흑: -1 방향 (하향)
                std::vector<std::pair<int, int>> pawnForward = {{0, -1}};
                p->addMovePattern(legalMoveChunk(threatType::MOVE, moveType::RAY_FINITE, pawnForward, 1));
                std::vector<std::pair<int, int>> pawnCapture = {{-1, -1}, {1, -1}};
                p->addMovePattern(legalMoveChunk(threatType::TAKE, moveType::RAY_FINITE, pawnCapture, 1));
            }
            break;
        case pieceType::AMAZON:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::JUMP, KNIGHT_DIRECTIONS, 1));
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, QUEEN_DIRECTIONS));
            break;
        default:
            break;
    }
}

// 보드의 모든 기물에 대해 패턴을 설정합니다.
inline void setupAllPieces(bc_board* board) {
    if (!board) return;
    for(int f = 0; f < 8; f++) {
        for(int r = 0; r < 8; r++) {
            piece* p = board->getPiece(f, r);
            if(p) {
                setupPiecePatterns(p);
            }
        }
    }
}