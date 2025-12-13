#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <moves.hpp>
#include <piece.hpp>

inline static constexpr int POCKET_SIZE = 16;

class bc_board{
    private:
        static constexpr int BOARD_SIZE = 8; // 8x8 체스보드
        int whiteMoveCount; // 백이 둔 수의 개수
        int blackMoveCount; // 흑이 둔 수의 개수
        std::vector<PGN> log; // 이동 로그
        std::array<std::array<piece*, BOARD_SIZE>, BOARD_SIZE> board; // 2D 보드
        std::list<piece> pieces; // 모든 기물 관리(주소 안정성 보장)
        piece* activePieceThisTurn = nullptr; // 한 턴에 움직인 기물
        bool performedActionThisTurn = false; // 드롭/이동 중복 방지
        inline static constexpr std::array<int, POCKET_SIZE> DEFAULT_POCKET_STOCK = {
            1, 1, 2, 2, 2, 8, // K,Q,B,N,R,P
            0, // A (Amazon)
            0, // G (Grasshopper)
            0, // Kr (KnightRider)
            0, // W (Archbishop)
            0, // D (Dabbaba)
            0, // L (Alfil)
            0, // F (Ferz)
            0, // C (Centaur)
            0,  // Tr (TesterRook)
            0 //Cl(Camel)
        };
        std::array<int, POCKET_SIZE> whitePocket{}; // 통합 포켓 (일반 기물 + 특수 기물)
        std::array<int, POCKET_SIZE> blackPocket{};
        
        piece* getPieceAt(int file, int rank) const;
        int computeInitialStun(pieceType type, colorType color, int rank) const;
        void decayStunAllAndChargeMoves();
        void resetTurnState();
        void resetPockets();
        void setPocketStock(colorType color, const std::array<int, POCKET_SIZE>& stock);
        void setPocketStockBoth(const std::array<int, POCKET_SIZE>& whiteStock, const std::array<int, POCKET_SIZE>& blackStock);
        pocketIndex pieceTypeToPocketIndex(pieceType type) const;
        std::array<int, 6>& pocketForColor(colorType color);
        const std::array<int, 6>& pocketForColor(colorType color) const;
        std::array<int, POCKET_SIZE>& fullPocketForColor(colorType color);
        const std::array<int, POCKET_SIZE>& fullPocketForColor(colorType color) const;
        colorType currentPlayerColor() const;

    public:
        // 생성자/소멸자
        bc_board();
        bc_board(const std::array<int, POCKET_SIZE>& whiteStock, const std::array<int, POCKET_SIZE>& blackStock);
        ~bc_board();
        
        // 보드 초기화
        void initializeBoard();
        
        // 모든 기물의 합법 이동 업데이트
        void updateAllLegalMoves();
        void updatePieceLegalMoves(piece* p);
        void applyStunTickAll();
        void applyStunTickForColor(colorType color);
        
        // 기물 착수 관련 함수
        bool placePiece(pieceType type, colorType color, int file, int rank);
        bool movePiece(int fromFile, int fromRank, int toFile, int toRank);
        bool removePiece(int file, int rank);
        bool passAndAddStun(int file, int rank, int delta = 1); // 킹 제외 스턴 +1
        bool promote(int file, int rank, pieceType promoteTo); // 폰 프로모션
        
        // 게임 진행 함수
        void nextTurn(); // 이동 후 호출 (스턴 감소)

        // 포켓 조회
        std::array<int, POCKET_SIZE> getPocketStock(colorType color) const;
        int getPocketCount(colorType color, pocketIndex idx) const;
        
        // getter (move 클래스에서 접근 가능하도록 public으로)
        bool isValidPosition(int file, int rank) const;
        piece* getPiece(int file, int rank) const;
        int getWhiteMoveCount() const { return whiteMoveCount; }
        int getBlackMoveCount() const { return blackMoveCount; }
        
        // 보드 출력
        void printBoard() const;
        
        // 기보 출력
        void printGameLog() const;
};
