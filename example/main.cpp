#include <iostream>
#include <vector>
#include <gameboard.hpp>
#include <piece.hpp>
#include <moves.hpp>

// 간단 데모: 기사 이동 패턴을 수동으로 추가하고 합법 수를 출력
int main() {
    bc_board board;
    board.initializeBoard();

    std::cout << "=== 초기 빈 보드 ===" << std::endl;
    board.printBoard();

    // b1에 백 나이트 착수
    board.placePiece(pieceType::KNIGHT, colorType::WHITE, 1, 0);

    // c3에 흑 폰 배치(포획 대상), c2에 백 폰 배치(아군으로 점프 도착 칸 차단 예시)
    board.placePiece(pieceType::PWAN, colorType::BLACK, 2, 2);
    board.placePiece(pieceType::PWAN, colorType::WHITE, 2, 1);
    
    // d4에 백 킹, e5에 흑 퀸 추가
    board.placePiece(pieceType::KING, colorType::WHITE, 3, 3);
    board.placePiece(pieceType::QUEEN, colorType::BLACK, 4, 4);

    std::cout << "=== 기물 배치 후 ===" << std::endl;
    board.printBoard();

    // b1의 기물 포인터 가져오기
    piece* kn = board.getPiece(1, 0);
    if (!kn) {
        std::cerr << "Failed to fetch knight at b1" << std::endl;
        return 1;
    }

    // 나이트 이동 패턴 추가: 전역 상수 KNIGHT_DIRECTIONS 사용
    kn->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::JUMP, KNIGHT_DIRECTIONS, 1));

    // 킹 이동 패턴 추가
    piece* king = board.getPiece(3, 3);
    if (king) {
        king->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KING_DIRECTIONS, 1));
        king->calculateAndUpdateLegalMoves(&board);
        std::cout << "\nKing at d4 legal moves:" << std::endl;
        for (const auto& m : king->getLegalMoves()) {
            std::cout << " - " << char('a' + m.startFile) << (m.startRank + 1)
                      << " -> " << char('a' + m.endFile) << (m.endRank + 1)
                      << (m.take ? " (capture)" : "") << std::endl;
        }
    }

    // 퀸 이동 패턴 추가
    piece* queen = board.getPiece(4, 4);
    if (queen) {
        queen->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, QUEEN_DIRECTIONS));
        queen->calculateAndUpdateLegalMoves(&board);
        std::cout << "\nQueen at e5 legal moves:" << std::endl;
        for (const auto& m : queen->getLegalMoves()) {
            std::cout << " - " << char('a' + m.startFile) << (m.startRank + 1)
                      << " -> " << char('a' + m.endFile) << (m.endRank + 1)
                      << (m.take ? " (capture)" : "") << std::endl;
        }
    }

    // 합법 이동 계산
    kn->calculateAndUpdateLegalMoves(&board);

    // 결과 출력
    const auto& moves = kn->getLegalMoves();
    std::cout << "\nKnight at b1 legal moves:" << std::endl;
    for (const auto& m : moves) {
        std::cout << " - " << char('a' + m.startFile) << (m.startRank + 1)
                  << " -> " << char('a' + m.endFile) << (m.endRank + 1)
                  << (m.take ? " (capture)" : "") << std::endl;
    }

    return 0;
}
