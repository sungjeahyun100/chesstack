#include <piece.hpp>
#include <gameboard.hpp>

// 기본 생성자
piece::piece() 
        : player_idx(0), stun_stack(0), is_stunned(false), move_stack(0), pT(pieceType::NONE), 
            file(0), rank(0), cT(colorType::NONE), is_royal(false), disguised_as(pieceType::NONE) {}

// 매개변수 생성자
piece::piece(pieceType type, colorType color, int f, int r, int idx)
        : player_idx(idx), stun_stack(0), is_stunned(false), move_stack(0), pT(type), 
      file(f), rank(r), cT(color), is_royal(false), disguised_as(pieceType::NONE) {}

// 이동 패턴 추가
void piece::addMovePattern(const legalMoveChunk& m) {
    movePatterns.push_back(m);
}

// 이동 패턴 초기화
void piece::clearMovePatterns() {
    movePatterns.clear();
}

// 합법 이동 계산 및 업데이트
// 모든 movePattern에서 이동들을 계산하여 합산한다
void piece::calculateAndUpdateLegalMoves(bc_board* board) {
    clearLegalMoves();
    
    if(board == nullptr) return;
    
    // 스턴 상태이면 이동 불가 (합법 수 없음)
    if(is_stunned) {
        return;
    }
    
    // 각 이동 패턴에서 이동을 계산하고 합산
    for(const auto& pattern : movePatterns) {
        std::vector<PGN> moves = pattern.calculateMoves(file, rank, pT, cT, board);
        for(const auto& m : moves) {
            addLegalMove(m);
        }
    }
}

// 합법 이동 업데이트
void piece::updateLegalMoves(const std::vector<PGN>& moves) {
    legal_move = moves;
}

// 합법 이동 추가
void piece::addLegalMove(const PGN& move) {
    legal_move.push_back(move);
}

// 합법 이동 초기화
void piece::clearLegalMoves() {
    legal_move.clear();
}

