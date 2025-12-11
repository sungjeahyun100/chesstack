#include <iostream>
#include <moves.hpp>
#include <piece.hpp>

int main() {
    std::cout << "=== PGN 문자열 변환 테스트 ===" << std::endl;
    
    // 1. PGN -> 문자열 변환
    std::cout << "\n[PGN to String]" << std::endl;
    
    PGN move1(4, 1, 4, 3, pieceType::PWAN, colorType::WHITE, false);
    std::cout << "e2->e4: " << move1.toString() << std::endl;
    
    PGN move2(6, 0, 5, 2, pieceType::KNIGHT, colorType::WHITE, false);
    std::cout << "g1->f3 (Knight): " << move2.toString() << std::endl;
    
    PGN move3(4, 3, 3, 4, pieceType::PWAN, colorType::WHITE, true);
    std::cout << "e4xd5 (Pawn capture): " << move3.toString() << std::endl;
    
    PGN drop1(0, 0, 3, 3, pieceType::QUEEN, colorType::WHITE, false);
    drop1.isDrop = true;
    std::cout << "Q@d4 (Queen drop): " << drop1.toString() << std::endl;
    
    // 2. 문자열 -> PGN 변환
    std::cout << "\n[String to PGN]" << std::endl;
    
    PGN parsed1 = PGN::fromString("e4", colorType::WHITE);
    std::cout << "\"e4\" -> endFile=" << parsed1.endFile << ", endRank=" << parsed1.endRank 
              << ", pieceType=" << (int)parsed1.pT << std::endl;
    
    PGN parsed2 = PGN::fromString("Nf3", colorType::WHITE);
    std::cout << "\"Nf3\" -> endFile=" << parsed2.endFile << ", endRank=" << parsed2.endRank 
              << ", pieceType=" << (int)parsed2.pT << std::endl;
    
    PGN parsed3 = PGN::fromString("exd5", colorType::WHITE);
    std::cout << "\"exd5\" -> startFile=" << parsed3.startFile << ", endFile=" << parsed3.endFile 
              << ", endRank=" << parsed3.endRank << ", take=" << parsed3.take << std::endl;
    
    PGN parsed4 = PGN::fromString("Q@d4", colorType::WHITE);
    std::cout << "\"Q@d4\" -> endFile=" << parsed4.endFile << ", endRank=" << parsed4.endRank 
              << ", isDrop=" << parsed4.isDrop << ", pieceType=" << (int)parsed4.pT << std::endl;
    
    // 3. 왕복 변환 테스트
    std::cout << "\n[Round-trip Test]" << std::endl;
    std::string testMoves[] = {"e4", "Nf3", "Qd8", "Q@e5"};
    
    for(const auto& moveStr : testMoves) {
        PGN parsed = PGN::fromString(moveStr, colorType::WHITE);
        std::string back = parsed.toString();
        std::cout << moveStr << " -> " << back << std::endl;
    }
    
    return 0;
}
