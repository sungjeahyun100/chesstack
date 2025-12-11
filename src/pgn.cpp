#include <moves.hpp>
#include <cctype>
#include <sstream>

// PGN을 문자열로 변환
std::string PGN::toString() const {
    std::stringstream ss;
    
    // 기물 타입 (폰은 생략)
    if(pT != pieceType::PWAN && pT != pieceType::NONE) {
        switch(pT) {
            case pieceType::KING:   ss << 'K'; break;
            case pieceType::QUEEN:  ss << 'Q'; break;
            case pieceType::ROOK:   ss << 'R'; break;
            case pieceType::BISHOP: ss << 'B'; break;
            case pieceType::KNIGHT: ss << 'N'; break;
            default: break;
        }
    }
    
    // 착수인 경우 @ 표기
    if(isDrop) {
        ss << '@';
        ss << char('a' + endFile) << (endRank + 1);
        return ss.str();
    }
    
    // 출발 파일 (모호성 제거용, 필요시)
    // 지금은 단순화: 폰의 캡처만 출발 파일 표시
    if(pT == pieceType::PWAN && take) {
        ss << char('a' + startFile);
    }
    
    // 캡처 표시
    if(take) {
        ss << 'x';
    }
    
    // 도착 위치
    ss << char('a' + endFile) << (endRank + 1);
    
    return ss.str();
}

// 문자열에서 PGN 파싱
PGN PGN::fromString(const std::string& str, colorType c) {
    if(str.empty()) {
        return PGN();
    }
    
    PGN result;
    result.cT = c;
    result.take = false;
    result.isDrop = false;
    
    size_t idx = 0;
    
    // 1. 기물 타입 파싱 (대문자로 시작하면 기물, 아니면 폰)
    if(std::isupper(str[idx])) {
        switch(str[idx]) {
            case 'K': result.pT = pieceType::KING; break;
            case 'Q': result.pT = pieceType::QUEEN; break;
            case 'R': result.pT = pieceType::ROOK; break;
            case 'B': result.pT = pieceType::BISHOP; break;
            case 'N': result.pT = pieceType::KNIGHT; break;
            default: result.pT = pieceType::NONE; break;
        }
        idx++;
    } else {
        result.pT = pieceType::PWAN;
    }
    
    // 2. 착수 표기 (@) 확인
    if(idx < str.length() && str[idx] == '@') {
        result.isDrop = true;
        idx++;
        
        // 도착 위치만 파싱
        if(idx + 1 < str.length()) {
            result.endFile = str[idx] - 'a';
            result.endRank = str[idx + 1] - '1';
            result.startFile = result.endFile;
            result.startRank = result.endRank;
        }
        return result;
    }
    
    // 3. 출발 파일 (폰 캡처의 경우)
    if(idx < str.length() && str[idx] >= 'a' && str[idx] <= 'h') {
        // 다음이 'x'이거나 다른 파일이면 출발 파일
        if(idx + 1 < str.length() && (str[idx + 1] == 'x' || std::isdigit(str[idx + 1]))) {
            if(std::isdigit(str[idx + 1])) {
                // 단순 이동 (e4)
                result.endFile = str[idx] - 'a';
                result.endRank = str[idx + 1] - '1';
                result.startFile = result.endFile; // 출발은 나중에 추론
                result.startRank = result.endRank;
                return result;
            }
        }
        result.startFile = str[idx] - 'a';
        idx++;
    }
    
    // 4. 캡처 표시 (x)
    if(idx < str.length() && str[idx] == 'x') {
        result.take = true;
        idx++;
    }
    
    // 5. 도착 위치 (파일+랭크)
    if(idx + 1 < str.length()) {
        result.endFile = str[idx] - 'a';
        result.endRank = str[idx + 1] - '1';
    }
    
    // 출발 위치는 기본적으로 도착과 동일 (나중에 보드에서 실제 이동 찾아야 함)
    if(result.startFile == 0 && result.startRank == 0 && !result.take) {
        result.startFile = result.endFile;
        result.startRank = result.endRank;
    }
    
    return result;
}
