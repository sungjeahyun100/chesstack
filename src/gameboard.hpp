#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <moves.hpp>
#include <piece.hpp>

class bc_board{
    private:
        static constexpr int BOARD_SIZE = 8; // 8x8 체스보드
        int turn;
        std::vector<PGN> log; // 이동 로그
        std::array<std::array<piece*, BOARD_SIZE>, BOARD_SIZE> board; // 2D 보드
        std::list<piece> pieces; // 모든 기물 관리(주소 안정성 보장)
        piece* activePieceThisTurn = nullptr; // 한 턴에 움직인 기물
        bool performedActionThisTurn = false; // 드롭/이동 중복 방지
        inline static constexpr std::array<int, 6> DEFAULT_POCKET_STOCK = {1, 1, 2, 2, 2, 8};
        std::array<int, 6> whitePocket{}; // KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN 보유량
        std::array<int, 6> blackPocket{};
        
        piece* getPieceAt(int file, int rank) const;
        int computeInitialStun(pieceType type, colorType color, int rank) const;
        void decayStunAllAndChargeMoves();
        void resetTurnState();
        void resetPockets();
        void setPocketStock(colorType color, const std::array<int, 6>& stock);
        void setPocketStockBoth(const std::array<int, 6>& whiteStock, const std::array<int, 6>& blackStock);
        int pieceTypeToIndex(pieceType type) const;
        std::array<int, 6>& pocketForColor(colorType color);
        const std::array<int, 6>& pocketForColor(colorType color) const;
        colorType currentPlayerColor() const;

    public:
        // 생성자/소멸자
        bc_board();
        ~bc_board();
        
        // 보드 초기화
        void initializeBoard();
        
        // 모든 기물의 합법 이동 업데이트
        void updateAllLegalMoves();
        void updatePieceLegalMoves(piece* p);
        void applyStunTickAll();
        
        // 기물 착수 관련 함수
        bool placePiece(pieceType type, colorType color, int file, int rank);
        bool movePiece(int fromFile, int fromRank, int toFile, int toRank);
        bool removePiece(int file, int rank);
        bool passAndAddStun(int file, int rank, int delta = 1); // 킹 제외 스턴 +1
        
        // 게임 진행 함수
        void nextTurn(); // 이동 후 호출 (스턴 감소)

        // 포켓 조회
        std::array<int, 6> getPocketStock(colorType color) const;
        
        // getter (move 클래스에서 접근 가능하도록 public으로)
        bool isValidPosition(int file, int rank) const;
        piece* getPiece(int file, int rank) const;
        int getTurn() const { return turn; }
        
        // 보드 출력
        void printBoard() const;
        
        // 기보 출력
        void printGameLog() const;
};
