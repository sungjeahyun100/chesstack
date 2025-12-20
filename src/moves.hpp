#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <enum.hpp>

struct PGN{
    public:
        int startFile, startRank, endFile, endRank;
        pieceType pT;
        colorType cT;
        bool take;
        bool isDrop; // 착수 여부
        bool captureJumped; // 중간에 뛰어넘은 기물도 잡는지 (TAKEJUMP)
        int jumpedFile;
        int jumpedRank;
        bool isDisguise; // 기물 변장 여부
        pieceType disguiseAs; // 변장할 기물 타입
        bool isSuccession; // 로얄 피스 승격 여부
        
        // 생성자
        PGN() : startFile(0), startRank(0), endFile(0), endRank(0), 
            pT(pieceType::NONE), cT(colorType::NONE), take(false), isDrop(false),
            captureJumped(false), jumpedFile(-1), jumpedRank(-1), 
            isDisguise(false), disguiseAs(pieceType::NONE), isSuccession(false) {}
        
        PGN(int sF, int sR, int eF, int eR, pieceType p, colorType c, bool t)
                : startFile(sF), startRank(sR), endFile(eF), endRank(eR),
                    pT(p), cT(c), take(t), isDrop(false),
                    captureJumped(false), jumpedFile(-1), jumpedRank(-1),
                    isDisguise(false), disguiseAs(pieceType::NONE), isSuccession(false) {}
              
        // PGN 문자열로 변환 (예: "Nf3", "exd5", "Q@d4")
        std::string toString() const;
        
        // 문자열에서 PGN 파싱 (예: "e4", "Nf3", "exd5", "Q@d4")
        static PGN fromString(const std::string& str, colorType c);
};

struct moveLog{
    private:
        int move_idx; // move_idx.move.first move.second ex)1.e4 e5
        std::pair<PGN, PGN> move_log;
};

/* move 클래스: moveType과 threatType에 따라 개별 이동을 계산한다.
   예를 들어, (RAY_INFINITE, TAKEMOVE)는 무한한 방향으로 이동 가능하며, 적 기물을 잡을 수 있다.
*/
class legalMoveChunk{
    private:
        threatType tT;
        moveType mT;
        std::vector<std::pair<int, int>> directions; // 이동 방향 (file 변화, rank 변화)
        int maxDistance; // RAY_FINITE의 경우 최대 거리
        
    public:
        // 생성자
        legalMoveChunk();
        legalMoveChunk(threatType t, moveType m);
        legalMoveChunk(threatType t, moveType m, const std::vector<std::pair<int, int>>& dirs);
        legalMoveChunk(threatType t, moveType m, const std::vector<std::pair<int, int>>& dirs, int maxDist);
        
        // getter
        threatType getThreatType() const { return tT; }
        moveType getMoveType() const { return mT; }
        const std::vector<std::pair<int, int>>& getDirections() const { return directions; }
        int getMaxDistance() const { return maxDistance; }
        
        // 이동 계산 함수
        std::vector<PGN> calculateMoves(int startFile, int startRank, pieceType pT, colorType cT, 
                                        class bc_board* board) const;
        
    private:
        // 개별 이동 계산 헬퍼 함수들
        std::vector<PGN> calculateRayMoves(int startFile, int startRank, pieceType pT, colorType cT, 
                                          class bc_board* board) const;
        
        // threatType에 따른 필터링
        bool isValidTarget(class bc_board* board, int targetFile, int targetRank, colorType cT) const;
};


