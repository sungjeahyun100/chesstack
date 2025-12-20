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
        
        // 턴 진행용 함수.
        void nextTurn(); // 턴을 종료했을 때 호출하는 함수로 이 타이밍에 스턴-이동 스택에 대한 연산을 수행한다.
        
        // 포지션 설정/불러오기
        void clearBoard(); // 보드 초기화 (기물만 제거, 포켓/턴 유지)
        void setupPosition(
            const std::vector<std::tuple<pieceType, colorType, int, int, int, int>>& pieces,
            colorType turn = colorType::WHITE,
            const std::array<int, POCKET_SIZE>* whitePocketOverride = nullptr,
            const std::array<int, POCKET_SIZE>* blackPocketOverride = nullptr
        ); // (type, color, file, rank, stun, moveStack)
        void setTurn(colorType turn);

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
        
        // FEN 변환
        std::string getBoardAsFEN() const;
        
        // 로얄 피스 관련
        bool hasRoyalPiece(colorType color) const;
        bool isRoyalPieceInCheck(colorType color) const;
        bool disguisePiece(int file, int rank, pieceType disguiseAs); // 로얄 피스 변장
        bool succeedRoyalPiece(int file, int rank, colorType color); // 로얄 피스 승격
};

// 기물 패턴 설정 함수
// 주어진 기물에 적절한 이동 패턴을 추가합니다.
inline void setupPiecePatterns(piece* p) {
    if (!p) return;
    
    pieceType type = p->getPieceType();
    
    switch(type) {
        case pieceType::KNIGHT:
            // RAY_FINITE로 구현: 최대 2칸까지 나이트 방향으로 이동 (경로상 기물에 영향)
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KNIGHT_DIRECTIONS, 1));
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
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KNIGHT_DIRECTIONS, 1));
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, QUEEN_DIRECTIONS));
            break;
        case pieceType::GRASSHOPPER:
            p->addMovePattern(legalMoveChunk(threatType::MOVEJUMP, moveType::RAY_INFINITE, QUEEN_DIRECTIONS));
            break;
        case pieceType::KNIGHTRIDER:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, KNIGHT_DIRECTIONS));
            break;
        case pieceType::ARCHBISHOP:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KNIGHT_DIRECTIONS, 1));
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_INFINITE, BISHOP_DIRECTIONS));
            break;
        case pieceType::DABBABA:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, DABBABA_DIRECTIONS, 1));
            break;
        case pieceType::ALFIL:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, ALFIL_DIRECTIONS, 1));
            break;
        case pieceType::FERZ:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, BISHOP_DIRECTIONS, 1));
            break;
        case pieceType::CENTAUR:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KING_DIRECTIONS, 1));
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, KNIGHT_DIRECTIONS, 1));
            break;
        case pieceType::TESTROOK:
            p->addMovePattern(legalMoveChunk(threatType::TAKEJUMP, moveType::RAY_INFINITE, ROOK_DIRECTIONS));
            break;
        case pieceType::CAMEL:
            p->addMovePattern(legalMoveChunk(threatType::TAKEMOVE, moveType::RAY_FINITE, CAMEL_DIRECTIONS, 1));
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
