#include <array>
#include <iostream>
#include <vector>
#include <chess.hpp>

void printPockets(const bc_board& board) {
    auto wp = board.getPocketStock(colorType::WHITE);
    auto bp = board.getPocketStock(colorType::BLACK);
    auto printSide = [](const char* label, const std::array<int, POCKET_SIZE>& p) {
        std::cout << label << " [K Q B N R P A G Kr W D L F C Tr Cl] = "
                  << p[0] << " " << p[1] << " " << p[2] << " "
                  << p[3] << " " << p[4] << " " << p[5] << " " << p[6] << " "
                  << p[7] << " " << p[8] << " " << p[9] << " " << p[10] << " "
                  << p[11] << " " << p[12] << " " << p[13] << " " << p[14] << " "
                  << p[15] << std::endl;
    };
    printSide("White", wp);
    printSide("Black", bp);
}

void printStacks(const char* label, piece* p) {
    if(!p) return;
    std::cout << label << " @ " << char('a' + p->getFile()) << (p->getRank() + 1)
              << " stun=" << p->getStunStack()
              << std::endl;
}

int main(){
    bc_board board;
    board.initializeBoard();

    std::cout << "=== 스택/포켓 규칙 시연 ===" << std::endl;

    int turnLabel = 1;

    // 턴 1: 백 킹 e1
    std::cout << "\n[턴 " << turnLabel++ << "] 백: 킹 e1 착수" << std::endl;
    board.placePiece(pieceType::KING, colorType::WHITE, 4, 0);
    setupPiecePatterns(board.getPiece(4, 0));
    board.printBoard();
    printPockets(board);
    board.nextTurn();

    // 턴 2: 흑 킹 e8
    std::cout << "\n[턴 " << turnLabel++ << "] 흑: 킹 e8 착수" << std::endl;
    board.placePiece(pieceType::KING, colorType::BLACK, 4, 7);
    setupPiecePatterns(board.getPiece(4, 7));
    board.printBoard();
    printPockets(board);
    board.nextTurn();

    // 턴 3: 백 폰 e4 (스턴 3)
    std::cout << "\n[턴 " << turnLabel++ << "] 백: 폰 e4 착수 (초기 스턴=3)" << std::endl;
    board.placePiece(pieceType::PWAN, colorType::WHITE, 4, 3);
    setupPiecePatterns(board.getPiece(4, 3));
    printStacks("White pawn", board.getPiece(4, 3));
    board.printBoard();
    printPockets(board);
    board.nextTurn();
    std::cout << "  → 턴 종료 후 스턴 감소/이동 충전" << std::endl;
    printStacks("White pawn", board.getPiece(4, 3));

    // 턴 4: 흑 퀸 e7 (초기 스턴=1)
    std::cout << "\n[턴 " << turnLabel++ << "] 흑: 퀸 e7 착수 (초기 스턴=1)" << std::endl;
    board.placePiece(pieceType::QUEEN, colorType::BLACK, 4, 6);
    setupPiecePatterns(board.getPiece(4, 6));
    printStacks("Black queen", board.getPiece(4, 6));
    board.printBoard();
    printPockets(board);
    board.nextTurn();
    std::cout << "  → 턴 종료 후 스턴 감소/이동 충전" << std::endl;
    printStacks("Black queen", board.getPiece(4, 6));

    // 턴 5: 백 나이트 g1
    std::cout << "\n[턴 " << turnLabel++ << "] 백: 나이트 g1 착수" << std::endl;
    board.placePiece(pieceType::KNIGHT, colorType::WHITE, 6, 0);
    setupPiecePatterns(board.getPiece(6, 0));
    printStacks("White knight", board.getPiece(6, 0));
    board.printBoard();
    printPockets(board);
    board.nextTurn();
    std::cout << "  → 턴 종료 후 스턴 감소/이동 충전" << std::endl;
    printStacks("White knight", board.getPiece(6, 0));

    // 턴 6: 흑 퀸 e7→e4 (백 폰 캡처, 스턴 이전 + 포켓 적립)
    std::cout << "\n[턴 " << turnLabel++ << "] 흑: 퀸 e7→e4로 백 폰 캡처" << std::endl;
    board.movePiece(4, 6, 4, 3);
    printStacks("Black queen", board.getPiece(4, 3));
    board.printBoard();
    printPockets(board);
    board.nextTurn();
    std::cout << "  → 턴 종료 후 스턴 감소/이동 충전" << std::endl;
    printStacks("Black queen", board.getPiece(4, 3));

    // 턴 7: 백 나이트 g1→f3 (이동 스택 소비 확인), 같은 턴 2회 이동 시도 실패
    std::cout << "\n[턴 " << turnLabel++ << "] 백: 나이트 g1→f3 이동 (move 스택 소비)" << std::endl;
    board.movePiece(6, 0, 5, 2);
    bool secondMove = board.movePiece(5, 2, 4, 4); // 같은 턴 추가 이동 시도 (거절 예상)
    std::cout << "  두 번째 이동 시도 결과: " << (secondMove ? "성공" : "거절") << std::endl;
    printStacks("White knight", board.getPiece(5, 2));
    board.printBoard();
    printPockets(board);
    board.nextTurn();

    std::cout << "\n=== 시연 종료, 최종 보드 ===" << std::endl;
    board.printBoard();
    printPockets(board);
    std::cout << "누적 수: 백=" << board.getWhiteMoveCount() 
              << ", 흑=" << board.getBlackMoveCount() << std::endl;

    return 0;
}

