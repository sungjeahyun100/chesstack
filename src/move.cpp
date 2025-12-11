#include <moves.hpp>
#include <gameboard.hpp>

// 생성자들
legalMoveChunk::legalMoveChunk() 
    : tT(threatType::NONE), mT(moveType::NONE), maxDistance(0) {}

legalMoveChunk::legalMoveChunk(threatType t, moveType m)
    : tT(t), mT(m), maxDistance(0) {}

legalMoveChunk::legalMoveChunk(threatType t, moveType m, const std::vector<std::pair<int, int>>& dirs)
    : tT(t), mT(m), directions(dirs), maxDistance(0) {}

legalMoveChunk::legalMoveChunk(threatType t, moveType m, const std::vector<std::pair<int, int>>& dirs, int maxDist)
    : tT(t), mT(m), directions(dirs), maxDistance(maxDist) {}

// threatType에 따른 목표 유효성 검사
bool legalMoveChunk::isValidTarget(bc_board* board, int targetFile, int targetRank, colorType cT) const {
    if(board == nullptr) return false;
    
    piece* targetPiece = board->getPiece(targetFile, targetRank);
    
    switch(tT) {
        case threatType::CATCH:
            // 적 기물만 캡처 가능
            return targetPiece != nullptr && targetPiece->getColor() != cT;
            
        case threatType::TAKE:
            // 이동하려면 적 기물이 있어야 함
            return targetPiece != nullptr && targetPiece->getColor() != cT;
            
        case threatType::MOVE:
            // 빈 공간에만 이동 가능
            return targetPiece == nullptr;
            
        case threatType::TAKEMOVE:
            // 빈 공간이거나 적 기물
            return targetPiece == nullptr || targetPiece->getColor() != cT;
            
        default:
            return false;
    }
}

// Ray 기반 이동 계산 (RAY_INFINITE, RAY_FINITE)
std::vector<PGN> legalMoveChunk::calculateRayMoves(int startFile, int startRank, pieceType pT, colorType cT, bc_board* board) const {
    std::vector<PGN> moves;
    
    if(board == nullptr) return moves;
    
    for(const auto& dir : directions) {
        int fileDir = dir.first;
        int rankDir = dir.second;
        
        int maxDist = (mT == moveType::RAY_INFINITE) ? 8 : maxDistance;
        
        for(int dist = 1; dist <= maxDist; dist++) {
            int newFile = startFile + fileDir * dist;
            int newRank = startRank + rankDir * dist;
            
            if(!board->isValidPosition(newFile, newRank)) break;
            
            piece* targetPiece = board->getPiece(newFile, newRank);
            
            if(targetPiece == nullptr) {
                // 빈 공간
                if(tT == threatType::MOVE || tT == threatType::TAKEMOVE) {
                    moves.push_back(PGN(startFile, startRank, newFile, newRank, pT, cT, false));
                }
            } else {
                // 기물이 있음
                if(targetPiece->getColor() != cT) {
                    // 적 기물
                    if(tT == threatType::CATCH) {
                        // CATCH는 이동하지 않음
                        // 이 경우 이동 리스트에 추가하지 않음
                    } else if(tT == threatType::TAKE || tT == threatType::TAKEMOVE) {
                        moves.push_back(PGN(startFile, startRank, newFile, newRank, pT, cT, true));
                    }
                }
                break; // 경로 중단
            }
        }
    }
    
    return moves;
}

// Jump 기반 이동 계산
std::vector<PGN> legalMoveChunk::calculateJumpMoves(int startFile, int startRank, pieceType pT, 
                                          colorType cT, bc_board* board) const {
    std::vector<PGN> moves;
    
    if(board == nullptr) return moves;
    
    for(const auto& dir : directions) {
        int fileDir = dir.first;
        int rankDir = dir.second;
        
        // Jump는 최대 거리만큼만 한 번에 이동
        int jumpFile = startFile + fileDir * maxDistance;
        int jumpRank = startRank + rankDir * maxDistance;
        
        if(!board->isValidPosition(jumpFile, jumpRank)) continue;
        
        // Jump는 경로 상의 기물 무시, 목표 위치만 확인
        piece* targetPiece = board->getPiece(jumpFile, jumpRank);
        
        if(targetPiece == nullptr) {
            // 빈 공간
            if(tT == threatType::MOVE || tT == threatType::TAKEMOVE) {
                moves.push_back(PGN(startFile, startRank, jumpFile, jumpRank, pT, cT, false));
            }
        } else {
            // 기물이 있음
            if(targetPiece->getColor() != cT) {
                // 적 기물
                if(tT == threatType::CATCH) {
                    // CATCH는 이동하지 않음
                } else if(tT == threatType::TAKE || tT == threatType::TAKEMOVE) {
                    moves.push_back(PGN(startFile, startRank, jumpFile, jumpRank, pT, cT, true));
                }
            }
        }
    }
    
    return moves;
}

// 메인 이동 계산 함수
std::vector<PGN> legalMoveChunk::calculateMoves(int startFile, int startRank, pieceType pT, 
                                       colorType cT, bc_board* board) const {
    std::vector<PGN> moves;
    
    if(board == nullptr) return moves;
    
    switch(mT) {
        case moveType::RAY_INFINITE:
        case moveType::RAY_FINITE:
            moves = calculateRayMoves(startFile, startRank, pT, cT, board);
            break;
            
        case moveType::JUMP:
            moves = calculateJumpMoves(startFile, startRank, pT, cT, board);
            break;
            
        default:
            break;
    }
    
    return moves;
}
