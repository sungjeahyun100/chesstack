#pragma once
#include <iostream>
#include <vector>
#include <enum.hpp>
#include <moves.hpp>
#include <algorithm>

// 각 기물의 이동 방향 정보 (file, rank 오프셋)
// 나이트: JUMP 이동
const std::vector<std::pair<int, int>> KNIGHT_DIRECTIONS = {
    {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
    {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
};

// 비숍: RAY_INFINITE 대각선
const std::vector<std::pair<int, int>> BISHOP_DIRECTIONS = {
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
};

// 룩: RAY_INFINITE 수평/수직
const std::vector<std::pair<int, int>> ROOK_DIRECTIONS = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1}
};

// 퀸: RAY_INFINITE 모든 방향 (룩+비숍)
const std::vector<std::pair<int, int>> QUEEN_DIRECTIONS = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
};

// 킹: RAY_FINITE 모든 방향 (maxDistance=1)
const std::vector<std::pair<int, int>> KING_DIRECTIONS = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
};

// 폰 전진 방향: WHITE는 +1, BLACK은 -1 (별도 처리 필요)
// 폰 대각선 캡처 방향: TAKEMOVE 패턴으로 사용
const std::vector<std::pair<int, int>> PAWN_CAPTURE_DIRECTIONS = {
    {1, 1}, {-1, 1}  // 상대 진영 방향 대각선 (방향은 색깔에 따라 조정)
};

const std::vector<std::pair<int, int>> DABBABA_DIRECTIONS = {
    {2, 0}, {0, 2}, {-2, 0}, {0, -2}
};

const std::vector<std::pair<int, int>> ALFIL_DIRECTIONS = {
    {2, 2}, {2, -2}, {-2, 2}, {-2, -2}
};

const std::vector<std::pair<int, int>> CAMEL_DIRECTIONS = {
    {3, 1}, {3, -1}, {-3, 1}, {-3, -1},
    {1, 3}, {1, -3}, {-1, 3}, {-1, -3}
};

// 기물 점수 테이블 (스턴 스택 부여에 사용)
inline int pieceScore(pieceType t) {
    switch(t) {
        case pieceType::KING:        return 4;
        case pieceType::QUEEN:       return 9;
        case pieceType::ROOK:        return 5;
        case pieceType::BISHOP:      return 3;
        case pieceType::KNIGHT:      return 3;
        case pieceType::AMAZON:      return 13;
        case pieceType::GRASSHOPPER: return 4;
        case pieceType::KNIGHTRIDER: return 7;
        case pieceType::ARCHBISHOP:  return 6;
        case pieceType::DABBABA:     return 2;
        case pieceType::ALFIL:       return 2;
        case pieceType::FERZ:        return 1;
        case pieceType::CENTAUR:     return 5;
        case pieceType::TESTROOK:    return 6;
        case pieceType::CAMEL:       return 3;
        case pieceType::PWAN:        return 0; // 폰은 랭크 기반 별도 규칙
        default: return 1;
    }
}

class piece{
    private:
        int player_idx;
        int stun_stack; // 스턴 스택: 0 이상
        bool is_stunned; // 스턴 상태: true면 현재 스턴 중
        int move_stack; // 이동 스택: 한 턴에 여러 번 움직일 수 있는 횟수
        pieceType pT;
        int file, rank; // 보드 위치
        colorType cT;
        std::vector<legalMoveChunk> movePatterns; // 기물이 가질 수 있는 이동 패턴들
        std::vector<PGN> legal_move; // 계산된 합법 이동들
        bool is_royal; // 로얄 피스 여부
        pieceType disguised_as; // 변장 상태 (로얄 피스만 사용, NONE이면 변장 안 함)

    public:
        // 생성자
        piece();
        piece(pieceType type, colorType color, int f, int r, int idx);
        
        // getter
        pieceType getPieceType() const { return pT; }
        colorType getColor() const { return cT; }
        int getFile() const { return file; }
        int getRank() const { return rank; }
        int getPlayerIdx() const { return player_idx; }
        int getStunStack() const { return stun_stack; }
        bool isStunned() const { return is_stunned; }
        int getMoveStack() const { return move_stack; }
        const std::vector<PGN>& getLegalMoves() const { return legal_move; }
        const std::vector<legalMoveChunk>& getMovePatterns() const { return movePatterns; }
        bool isRoyal() const { return is_royal; }
        pieceType getDisguisedAs() const { return disguised_as; }
        
        // 이동 패턴 관리
        void addMovePattern(const legalMoveChunk& m);
        void clearMovePatterns();
        
        // 합법 이동 계산 및 업데이트
        void calculateAndUpdateLegalMoves(class bc_board* board);
        void updateLegalMoves(const std::vector<PGN>& moves);
        void addLegalMove(const PGN& move);
        void clearLegalMoves();
        
        // setter
        void setFile(int f) { file = f; }
        void setRank(int r) { rank = r; }
        void setPieceType(pieceType type) { pT = type; }
        void setRoyal(bool royal) { is_royal = royal; }
        void setDisguisedAs(pieceType type) { disguised_as = type; }
        // 스턴 조작
        void setStun(int s) { 
            stun_stack = std::max(0, s);
            is_stunned = (stun_stack > 0); // 스턴 스택이 0이면 스턴 해제
        }
        void addStun(int delta) { 
            stun_stack = std::max(0, stun_stack + delta);
            is_stunned = (stun_stack > 0); // 스턴 스택이 0이면 스턴 해제
        }
        void decrementStun() {
            if(stun_stack > 0) {
                stun_stack--;
                is_stunned = (stun_stack > 0); // 스턴 스택이 0이면 스턴 해제
            }
        }

        // 스택 틱: 각 플레이어가 수를 둘 때마다 스턴 스택 1 감소, 그러면 이동 스택 1 증가
        void applyStunTick() {
            if(stun_stack > 0) {
                stun_stack--;
                is_stunned = (stun_stack > 0);
                move_stack++; // 스턴 감소할 때마다 이동 스택 1 증가
            }
        }
        
        // 이동 스택 소비
        bool consumeMoveStack(int amount = 1) {
            if(move_stack >= amount) {
                move_stack -= amount;
                return true;
            }
            return false;
        }
        
        // 이동 스택 설정
        void setMoveStack(int m) {
            move_stack = std::max(0, m);
        }
        
        // 이동 스택 추가
        void addMoveStack(int amount) {
            move_stack = std::max(0, move_stack + amount);
        }
};
