#include <gameboard.hpp>
#include <cmath>
#include <algorithm>

// 생성자
bc_board::bc_board() : whiteMoveCount(0), blackMoveCount(0) {
    // 보드 초기화
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = nullptr;
        }
    }
    // 로얄 피스 벡터는 자동 초기화됨
    resetPockets();
}

// 포켓을 설정할 수 있는 생성자
bc_board::bc_board(const std::array<int, POCKET_SIZE>& whiteStock, const std::array<int, POCKET_SIZE>& blackStock) 
    : whiteMoveCount(0), blackMoveCount(0) {
    // 보드 초기화
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = nullptr;
        }
    }
    // 로얄 피스 벡터는 자동 초기화됨
    whitePocket = whiteStock;
    blackPocket = blackStock;
}

// 소멸자
bc_board::~bc_board() {
    // 모든 piece 포인터는 pieces 벡터에서 관리되므로 여기서 delete 필요 없음
}

// 보드 초기화
void bc_board::initializeBoard() {
    whiteMoveCount = 0;
    blackMoveCount = 0;
    pieces.clear();
    activePieceThisTurn = nullptr;
    performedActionThisTurn = false;
    resetPockets();
    
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = nullptr;
        }
    }
}

// 위치 유효성 검사
bool bc_board::isValidPosition(int file, int rank) const {
    return file >= 0 && file < BOARD_SIZE && rank >= 0 && rank < BOARD_SIZE;
}

// 현재 턴의 플레이어 색상 (백과 흑의 수 개수를 비교하여 결정)
colorType bc_board::currentPlayerColor() const {
    return (whiteMoveCount == blackMoveCount) ? colorType::WHITE : colorType::BLACK;
}

// pieceType -> 포켓 인덱스
pocketIndex bc_board::pieceTypeToPocketIndex(pieceType type) const {
    switch(type) {
        case pieceType::KING:         return pocketIndex::KING;
        case pieceType::QUEEN:        return pocketIndex::QUEEN;
        case pieceType::BISHOP:       return pocketIndex::BISHOP;
        case pieceType::KNIGHT:       return pocketIndex::KNIGHT;
        case pieceType::ROOK:         return pocketIndex::ROOK;
        case pieceType::PWAN:         return pocketIndex::PAWN;
        case pieceType::AMAZON:       return pocketIndex::AMAZON;
        case pieceType::GRASSHOPPER:  return pocketIndex::GRASSHOPPER;
        case pieceType::KNIGHTRIDER:  return pocketIndex::KNIGHTRIDER;
        case pieceType::ARCHBISHOP:   return pocketIndex::ARCHBISHOP;
        case pieceType::DABBABA:      return pocketIndex::DABBABA;
        case pieceType::ALFIL:        return pocketIndex::ALFIL;
        case pieceType::FERZ:         return pocketIndex::FERZ;
        case pieceType::CENTAUR:      return pocketIndex::CENTAUR;
        case pieceType::TESTROOK:   return pocketIndex::TESTROOK;
        default: return pocketIndex::NONE; // fallback (shouldn't happen)
    }
}

// 포켓 참조 반환 (일반 기물만, 하위 호환성용)
std::array<int, 6>& bc_board::pocketForColor(colorType color) {
    static std::array<int, 6> temp;
    auto& full = fullPocketForColor(color);
    std::copy_n(full.begin(), 6, temp.begin());
    return temp;
}

const std::array<int, 6>& bc_board::pocketForColor(colorType color) const {
    static std::array<int, 6> temp;
    const auto& full = fullPocketForColor(color);
    std::copy_n(full.begin(), 6, temp.begin());
    return temp;
}

// 전체 포켓 참조 반환 (일반 + 특수 기물)
std::array<int, POCKET_SIZE>& bc_board::fullPocketForColor(colorType color) {
    return (color == colorType::BLACK) ? blackPocket : whitePocket;
}

const std::array<int, POCKET_SIZE>& bc_board::fullPocketForColor(colorType color) const {
    return (color == colorType::BLACK) ? blackPocket : whitePocket;
}

std::array<int, POCKET_SIZE> bc_board::getPocketStock(colorType color) const {
    return fullPocketForColor(color);
}

int bc_board::getPocketCount(colorType color, pocketIndex idx) const {
    const auto& pocket = fullPocketForColor(color);
    int index = static_cast<int>(idx);
    if (index >= 0 && index < POCKET_SIZE) {
        return pocket[index];
    }
    return 0;
}

// 포켓 초기화 (초기 보유말)
void bc_board::resetPockets() {
    whitePocket = DEFAULT_POCKET_STOCK;
    blackPocket = DEFAULT_POCKET_STOCK;
}

// 포켓 보유량 수동 설정 (색상별)
void bc_board::setPocketStock(colorType color, const std::array<int, POCKET_SIZE>& stock) {
    fullPocketForColor(color) = stock;
}

// 포켓 보유량 수동 설정 (양쪽 한 번에)
void bc_board::setPocketStockBoth(const std::array<int, POCKET_SIZE>& whiteStock, const std::array<int, POCKET_SIZE>& blackStock) {
    whitePocket = whiteStock;
    blackPocket = blackStock;
}

// 특정 위치의 기물 가져오기
piece* bc_board::getPieceAt(int file, int rank) const {
    if(!isValidPosition(file, rank)) return nullptr;
    return board[file][rank];
}

// 착수 시 초기 스턴 계산: 폰은 랭크에 따라, 다른 기물은 스턴 스택 1
int bc_board::computeInitialStun(pieceType type, colorType color, int rank) const {
    if(type != pieceType::PWAN) {
        return 1; // 폰이 아니면 스턴 1
    }
    
    // 폰의 경우: 랭크에 따라 스턴 설정
    if(color == colorType::WHITE) {
        // 백: rank1(0)=0stun ~ rank7(6)=6stun
        return std::min(rank, 6);
    } else {
        // 흑: rank8(7)=0stun ~ rank2(1)=6stun
        return std::min(7 - rank, 6);
    }
}

// 턴 상태 초기화
void bc_board::resetTurnState() {
    activePieceThisTurn = nullptr;
    performedActionThisTurn = false;
}

// 특정 기물의 합법 이동 업데이트
void bc_board::updatePieceLegalMoves(piece* p) {
    if(p == nullptr) return;
    p->calculateAndUpdateLegalMoves(this);
}

// 모든 기물의 합법 이동 업데이트
void bc_board::updateAllLegalMoves() {
    for(auto& p : pieces) {
        updatePieceLegalMoves(&p);
    }
}

// 특정 색상 기물들의 스턴 스택 감소 (해당 플레이어가 수를 둘 때 호출)
void bc_board::applyStunTickAll() {
    for(auto& p : pieces) {
        p.applyStunTick();
    }
}

// 특정 색상의 기물들만 스턴 틱 적용
void bc_board::applyStunTickForColor(colorType color) {
    for(auto& p : pieces) {
        if(p.getColor() == color) {
            p.applyStunTick();
        }
    }
}

// 기물 착수
bool bc_board::placePiece(pieceType type, colorType color, int file, int rank) {
    if(!isValidPosition(file, rank)) {
        std::cerr << "Invalid position: (" << file << ", " << rank << ")" << std::endl;
        return false;
    }

    if(performedActionThisTurn) {
        std::cerr << "Action already performed this turn" << std::endl;
        return false;
    }
    
    if(type == pieceType::PWAN) {
        // 폰은 상대 진영 최종 랭크에 착수 불가
        if((color == colorType::WHITE && rank == BOARD_SIZE - 1) ||
           (color == colorType::BLACK && rank == 0)) {
            std::cerr << "Pawn cannot be placed on final rank" << std::endl;
            return false;
        }
    }

    if(color != currentPlayerColor()) {
        std::cerr << "Not your turn" << std::endl;
        return false;
    }

    // 포켓 인덱스 확인
    pocketIndex pIdx = pieceTypeToPocketIndex(type);
    auto& pocket = fullPocketForColor(color);
    int idx = static_cast<int>(pIdx);
    
    if (pocket[idx] <= 0) {
        std::cerr << "No remaining pieces of this type to drop" << std::endl;
        return false;
    }
    
    if(board[file][rank] != nullptr) {
        std::cerr << "Position already occupied: (" << file << ", " << rank << ")" << std::endl;
        return false;
    }
    
    // pieces 컨테이너에 새로운 기물 추가
    pieces.emplace_back(type, color, file, rank, pieces.size());
    piece* placed = &pieces.back();

    // 초기 스턴 설정
    int initStun = computeInitialStun(type, color, rank);
    placed->setStun(initStun);
    placed->setMoveStack(0); // 착수 후 이동 스택은 0부터 시작
    
    // 보드에 포인터 저장
    board[file][rank] = placed;
    pocket[idx] -= 1;
    
    activePieceThisTurn = placed;
    performedActionThisTurn = true;
    
    // 착수 로그: @<좌표> 형식으로 기록 (0,0에서 목표로 이동으로 표현)
    log.push_back(PGN(0, 0, file, rank, type, color, false));
    
    // 킹 착수 시 자동으로 로얄 피스 설정
    if(type == pieceType::KING) {
        pieces.back().setRoyal(true);
    }
    
    std::cout << "Piece placed at (" << file << ", " << rank << ")" << std::endl;
    // 합법수 재계산
    for (auto& p : pieces) {
        p.clearMovePatterns();
        setupPiecePatterns(&p);
    }
    updateAllLegalMoves();
    return true;
}

// 기물 이동
bool bc_board::movePiece(int fromFile, int fromRank, int toFile, int toRank) {
    // 1) 입력 좌표 유효성 검사
    if(!isValidPosition(fromFile, fromRank) || !isValidPosition(toFile, toRank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }
    
    // 2) 출발지에 기물이 있는지 확인
    piece* movingPiece = getPieceAt(fromFile, fromRank);
    if(movingPiece == nullptr) {
        std::cerr << "No piece at source position" << std::endl;
        return false;
    }

    // 3) 턴 소유 확인
    if(movingPiece->getColor() != currentPlayerColor()) {
        std::cerr << "Not your turn" << std::endl;
        return false;
    }

    // 4) 한 턴 한 액션 제한
    if(performedActionThisTurn && activePieceThisTurn != movingPiece) {
        std::cerr << "Another piece already acted this turn" << std::endl;
        return false;
    }

    // 스턴 확인: 스턴 상태이면 이 턴에서 움직일 수 없음
    // 5) 스턴 상태면 이동 불가
    if(movingPiece->isStunned()) {
        std::cerr << "Piece is stunned" << std::endl;
        return false;
    }
    
    // 이동 스택 확인: 이동 스택이 있어야 이동 가능
    // 6) 이동 스택 소비 (없으면 이동 불가)
    if(!movingPiece->consumeMoveStack(1)) {
        std::cerr << "No move stack available" << std::endl;
        return false;
    }
    
    // 7) 요청된 목적지가 합법수인지 확인
    const auto& legalMoves = movingPiece->getLegalMoves();
    const PGN* selectedMove = nullptr;
    
    for(const auto& moveData : legalMoves) {
        if(moveData.endFile == toFile && moveData.endRank == toRank) {
            selectedMove = &moveData;
            break;
        }
    }
    
    if(!selectedMove) {
        std::cerr << "Illegal move" << std::endl;
        return false;
    }
    
    // 8) 이동 전에 해당 색상의 모든 기물 스턴 틱 감소
    colorType movingColor = movingPiece->getColor();
    applyStunTickForColor(movingColor);
    
    // 9) TAKEJUMP: 중간 기물도 캡처
    if(selectedMove->captureJumped && selectedMove->jumpedFile >=0 && selectedMove->jumpedRank >=0) {
        piece* midPiece = getPieceAt(selectedMove->jumpedFile, selectedMove->jumpedRank);
        if(midPiece != nullptr) {
            movingPiece->addStun(midPiece->getStunStack());
            pieceType capturedType = midPiece->getPieceType();
            pocketIndex capturedPIdx = pieceTypeToPocketIndex(capturedType);
            auto& pocketCaptured = fullPocketForColor(movingColor);
            int capturedIdx = static_cast<int>(capturedPIdx);
            pocketCaptured[capturedIdx] += 1;
            removePiece(selectedMove->jumpedFile, selectedMove->jumpedRank);
        }
    }

    // 10) 도착지에 기물이 있으면 스턴 이전 후 포켓 적립
    piece* targetPiece = getPieceAt(toFile, toRank);
    if(targetPiece != nullptr) {
        const int capturedStun = targetPiece->getStunStack(); // 로얄 스턴 분배 전에 캡처 피스 스턴을 저장

        // 로얄 피스 캡처 시: 같은 색 모든 기물에 스턴 +3 (로얄 피스 자신 포함)
        if(targetPiece->isRoyal()) {
            colorType targetColor = targetPiece->getColor();
            for(auto& p : pieces) {
                if(p.getColor() == targetColor) {
                    p.addStun(3);
                }
            }
        }

        // 스턴 이전: 잡힌 기물의 (로얄 보정 전) 스턴을 이동 기물에게 더한다
        movingPiece->addStun(capturedStun);
        
        // 잡힌 기물을 포켓에 추가
        pieceType capturedType = targetPiece->getPieceType();
        pocketIndex capturedPIdx = pieceTypeToPocketIndex(capturedType);
        auto& pocketCaptured = fullPocketForColor(movingColor);
        int capturedIdx = static_cast<int>(capturedPIdx);
        pocketCaptured[capturedIdx] += 1;
        
        removePiece(toFile, toRank);
    }
    
    // 11) 기물 위치 갱신 및 턴 상태 플래그 업데이트
    board[fromFile][fromRank] = nullptr;
    board[toFile][toRank] = movingPiece;
    movingPiece->setFile(toFile);
    movingPiece->setRank(toRank);
    activePieceThisTurn = movingPiece;
    performedActionThisTurn = true;
    
    std::cout << "Piece moved from (" << fromFile << ", " << fromRank << ") to (" 
              << toFile << ", " << toRank << ")" << std::endl;
    
    // 12) 이동 로그 저장
    log.push_back(PGN(fromFile, fromRank, toFile, toRank, movingPiece->getPieceType(), movingPiece->getColor(), (targetPiece != nullptr)));

    // 합법수 재계산
    for (auto& p : pieces) {
        p.clearMovePatterns();
        setupPiecePatterns(&p);
    }
    updateAllLegalMoves();
    return true;
}

// 기물 제거
bool bc_board::removePiece(int file, int rank) {
    if(!isValidPosition(file, rank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }
    
    piece* targetPiece = getPieceAt(file, rank);
    if(targetPiece == nullptr) {
        std::cerr << "No piece at position" << std::endl;
        return false;
    }
    
    board[file][rank] = nullptr;
    
    // pieces 벡터에서도 제거
    for(auto it = pieces.begin(); it != pieces.end(); ++it) {
        if(&(*it) == targetPiece) {
            pieces.erase(it);
            break;
        }
    }
    
    std::cout << "Piece removed from (" << file << ", " << rank << ")" << std::endl;
    // 합법수 재계산
    for (auto& p : pieces) {
        p.clearMovePatterns();
        setupPiecePatterns(&p);
    }
    updateAllLegalMoves();
    return true;
}


// 폰 프로모션: 특정 기물을 다른 기물로 변환
bool bc_board::promote(int file, int rank, pieceType promoteTo) {
    if(!isValidPosition(file, rank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }
    
    piece* pawn = getPieceAt(file, rank);
    if(pawn == nullptr) {
        std::cerr << "No piece at position" << std::endl;
        return false;
    }
    
    if(pawn->getPieceType() != pieceType::PWAN) {
        std::cerr << "Piece is not a pawn" << std::endl;
        return false;
    }
    
    // 폰이 프로모션 가능한 위치에 있는지 확인
    if(!((pawn->getColor() == colorType::WHITE && rank == BOARD_SIZE - 1) ||
         (pawn->getColor() == colorType::BLACK && rank == 0))) {
        std::cerr << "Pawn is not at promotion rank" << std::endl;
        return false;
    }
    
    // 변환할 기물이 킹이나 폰이면 안 됨
    if(promoteTo == pieceType::KING || promoteTo == pieceType::PWAN) {
        std::cerr << "Cannot promote to king or pawn" << std::endl;
        return false;
    }
    
    // 폰의 스턴을 새 기물에게 이전
    int pawnStun = pawn->getStunStack();
    int pawnMoveStack = pawn->getMoveStack(); // 이동 스택도 함께 이전
    bool pawnStunned = pawn->isStunned();
    
    // 폰을 포켓에 돌려놓음
    auto& pocket = fullPocketForColor(pawn->getColor());
    pocketIndex pawnIdx = pocketIndex::PAWN;
    pocket[static_cast<int>(pawnIdx)] += 1;
    
    // 새 기물 타입으로 변환
    pawn->setPieceType(promoteTo);
    pawn->setStun(pawnStun);
    pawn->setMoveStack(pawnMoveStack); // 이동 스택도 설정
    if(pawnStunned) {
        pawn->setStun(pawnStun);  // 스턴 상태는 setStun에서 자동 처리
    }
    
    // 변환된 기물을 포켓에서 빼기 (착수한 것으로 간주)
    pocketIndex newIdx = pieceTypeToPocketIndex(promoteTo);
    int newIdxInt = static_cast<int>(newIdx);
    if(newIdxInt >= 0 && newIdxInt < POCKET_SIZE) {
        pocket[newIdxInt] = std::max(0, pocket[newIdxInt] - 1);
    }
    
    std::cout << "Pawn promoted at (" << file << ", " << rank << ")" << std::endl;
    // 합법수 재계산
    for (auto& p : pieces) {
        p.clearMovePatterns();
        setupPiecePatterns(&p);
    }
    updateAllLegalMoves();
    return true;
}

// 턴을 넘기며 특정 기물의 스턴 스택을 추가 (킹 제외)
bool bc_board::passAndAddStun(int file, int rank, int delta) {
    if(!isValidPosition(file, rank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }

    if(performedActionThisTurn) {
        std::cerr << "Action already performed this turn" << std::endl;
        return false;
    }

    piece* target = getPieceAt(file, rank);
    if(target == nullptr) {
        std::cerr << "No piece at position" << std::endl;
        return false;
    }
    
    target->addStun(delta);
    activePieceThisTurn = target;
    performedActionThisTurn = true;
    // 합법수 재계산
    for (auto& p : pieces) {
        p.clearMovePatterns();
        setupPiecePatterns(&p);
    }
    updateAllLegalMoves();
    return true;
}

// 다음 턴
void bc_board::nextTurn() {
    // 현재 플레이어의 수를 마무리하며 턴을 넘긴다
    colorType current = currentPlayerColor();
    if(current == colorType::WHITE) {
        whiteMoveCount++;
    } else {
        blackMoveCount++;
    }

    applyStunTickForColor(current);

    resetTurnState();
}

// 특정 위치의 기물 포인터 가져오기 (public)
piece* bc_board::getPiece(int file, int rank) const {
    return getPieceAt(file, rank);
}

// 보드 출력
void bc_board::printBoard() const {
    std::cout << "\n  ";
    for(int f = 0; f < BOARD_SIZE; f++) {
        std::cout << "  " << char('a' + f) << " ";
    }
    std::cout << "\n";
    
    for(int r = BOARD_SIZE - 1; r >= 0; r--) {
        std::cout << "  +";
        for(int f = 0; f < BOARD_SIZE; f++) {
            std::cout << "---+";
        }
        std::cout << "\n" << (r + 1) << " |";
        
        for(int f = 0; f < BOARD_SIZE; f++) {
            piece* p = board[f][r];
            if(p == nullptr) {
                std::cout << "   |";
            } else {
                // 색상 표시: 백=대문자, 흑=소문자
                char symbol = ' ';
                switch(p->getPieceType()) {
                    case pieceType::KING:   symbol = 'K'; break;
                    case pieceType::QUEEN:  symbol = 'Q'; break;
                    case pieceType::ROOK:   symbol = 'R'; break;
                    case pieceType::BISHOP: symbol = 'B'; break;
                    case pieceType::KNIGHT: symbol = 'N'; break;
                    case pieceType::PWAN:   symbol = 'P'; break;
                    default: symbol = '?'; break;
                }
                
                if(p->getColor() == colorType::BLACK) {
                    symbol = tolower(symbol);
                }
                
                // 스턴 상태 표시: * 추가
                char stunMarker = p->isStunned() ? '*' : ' ';
                std::cout << " " << symbol << stunMarker << "|";
            }
        }
        std::cout << " " << (r + 1) << "\n";
    }
    
    std::cout << "  +";
    for(int f = 0; f < BOARD_SIZE; f++) {
        std::cout << "---+";
    }
    std::cout << "\n  ";
    for(int f = 0; f < BOARD_SIZE; f++) {
        std::cout << "  " << char('a' + f) << " ";
    }
    std::cout << "\n\n";
}

// 기보 출력
void bc_board::printGameLog() const {
    if(log.empty()) {
        std::cout << "아직 수가 없습니다." << std::endl;
        return;
    }
    
    std::cout << "기보 (PGN):" << std::endl;
    int moveNumber = 1;
    
    for(size_t i = 0; i < log.size(); i++) {
        const auto& move = log[i];
        bool isSpecialMove = false;
        
        // 변장과 승격은 별도 처리
        if(move.isDisguise) {
            // 변장: f1=Q 형식
            char disguiseSymbol = ' ';
            switch(move.disguiseAs) {
                case pieceType::QUEEN:      disguiseSymbol = 'Q'; break;
                case pieceType::ROOK:       disguiseSymbol = 'R'; break;
                case pieceType::BISHOP:     disguiseSymbol = 'B'; break;
                case pieceType::KNIGHT:     disguiseSymbol = 'N'; break;
                case pieceType::AMAZON:     disguiseSymbol = 'A'; break;
                case pieceType::GRASSHOPPER: disguiseSymbol = 'G'; break;
                case pieceType::KNIGHTRIDER: disguiseSymbol = 'H'; break;
                case pieceType::ARCHBISHOP: disguiseSymbol = 'W'; break;
                case pieceType::DABBABA:    disguiseSymbol = 'D'; break;
                case pieceType::ALFIL:      disguiseSymbol = 'L'; break;
                case pieceType::FERZ:       disguiseSymbol = 'F'; break;
                case pieceType::CENTAUR:    disguiseSymbol = 'C'; break;
                case pieceType::TESTROOK:   disguiseSymbol = 'T'; break;
                case pieceType::CAMEL:      disguiseSymbol = 'M'; break;
                default: disguiseSymbol = '?'; break;
            }
            std::cout << char('a' + move.startFile) << (move.startRank + 1) 
                      << "=" << disguiseSymbol;
            isSpecialMove = true;
        } else if(move.isSuccession) {
            // 로얄 피스 승격: suc e5 형식
            std::cout << "suc " << char('a' + move.startFile) << (move.startRank + 1);
            isSpecialMove = true;
        }
        
        if(!isSpecialMove) {
            // 백의 수일 때만 이동 번호 출력
            if(i % 2 == 0) {
                std::cout << moveNumber << ". ";
            }
            
            // 기물 표시
            char pieceName = ' ';
            switch(move.pT) {
                case pieceType::KING:   pieceName = 'K'; break;
                case pieceType::QUEEN:  pieceName = 'Q'; break;
                case pieceType::ROOK:   pieceName = 'R'; break;
                case pieceType::BISHOP: pieceName = 'B'; break;
                case pieceType::KNIGHT: pieceName = 'N'; break;
                case pieceType::PWAN:   pieceName = 'P'; break;
                default: pieceName = ' '; break;
            }
            
            std::cout << pieceName;
            
            // 착수 체크 (startFile == 0 && startRank == 0)
            if(move.startFile == 0 && move.startRank == 0) {
                // 착수: @<목표좌표> 형식
                std::cout << "@" << char('a' + move.endFile) << (move.endRank + 1);
            } else {
                // 일반 이동: <시작좌표>-<목표좌표> 형식
                std::cout << char('a' + move.startFile) << (move.startRank + 1)
                          << (move.take ? "x" : "-")
                          << char('a' + move.endFile) << (move.endRank + 1);
            }
        }

        // 각 수 사이에 공백, 흑의 수 이후 줄바꿈
        if(i % 2 == 0) {
            std::cout << " ";
        } else {
            std::cout << std::endl;
            moveNumber++;
        }
    }
    
    // 마지막 수가 백의 수인 경우 줄바꿈
    if(log.size() % 2 == 1) {
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// 보드 클리어 (기물만 제거, 포켓/턴 유지)
void bc_board::clearBoard() {
    pieces.clear();
    activePieceThisTurn = nullptr;
    performedActionThisTurn = false;
    
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = nullptr;
        }
    }
}

// 포지션 설정 (type, color, file, rank, stun, moveStack)
void bc_board::setupPosition(
    const std::vector<std::tuple<pieceType, colorType, int, int, int, int>>& pieceList,
    colorType turn,
    const std::array<int, POCKET_SIZE>* whitePocketOverride,
    const std::array<int, POCKET_SIZE>* blackPocketOverride
) {
    clearBoard();

    // 포켓 설정: 기본값으로 초기화 후 오버라이드가 있으면 적용
    resetPockets();
    if (whitePocketOverride) {
        setPocketStock(colorType::WHITE, *whitePocketOverride);
    }
    if (blackPocketOverride) {
        setPocketStock(colorType::BLACK, *blackPocketOverride);
    }
    
    for(const auto& [type, color, file, rank, stun, moveStack] : pieceList) {
        if(!isValidPosition(file, rank)) continue;
        if(board[file][rank] != nullptr) continue; // 이미 기물이 있으면 스킵
        
        // 새 기물 추가
        pieces.emplace_back(type, color, file, rank, pieces.size());
        piece* p = &pieces.back();
        
        // 스턴과 이동 스택 설정
        p->setStun(stun);
        p->setMoveStack(moveStack);

        // 미리 배치된 킹은 로얄 피스로 취급해야 특수 액션(위장/계승)이 동작한다.
        if (type == pieceType::KING) {
            p->setRoyal(true);
        }
        
        // 보드에 배치
        board[file][rank] = p;
    }
    
    // 턴 설정 (기본: 백)
    setTurn(turn);

    // 합법수 재계산
    for (auto& p : pieces) {
        setupPiecePatterns(&p);
    }
    updateAllLegalMoves();
}

// 턴 설정: whiteMoveCount/blackMoveCount로 현재 턴 결정 (whiteMoveCount==blackMoveCount -> 백 차례)
void bc_board::setTurn(colorType turn) {
    whiteMoveCount = 0;
    blackMoveCount = 0;
    if(turn == colorType::BLACK) {
        whiteMoveCount = 1; // 백이 한 번 둔 것으로 취급하여 흑 차례로 설정
    }
}

// 보드 상태를 간단한 FEN 형식으로 변환 (기물 배치만, 캐슬링/앙파상 표기 제외)
std::string bc_board::getBoardAsFEN() const {
    std::string fen;
    
    // 보드를 rank 7부터 0까지 순회 (위에서 아래로)
    for(int rank = BOARD_SIZE - 1; rank >= 0; --rank) {
        int emptyCount = 0;
        
        // 각 파일(column) 순회
        for(int file = 0; file < BOARD_SIZE; ++file) {
            piece* p = board[file][rank];
            
            if(p == nullptr) {
                // 빈 칸 카운트
                emptyCount++;
            } else {
                // 빈 칸이 있으면 먼저 숫자로 표기
                if(emptyCount > 0) {
                    fen += char('0' + emptyCount);
                    emptyCount = 0;
                }
                
                // 기물 표기
                char symbol = ' ';
                switch(p->getPieceType()) {
                    case pieceType::KING:       symbol = 'K'; break;
                    case pieceType::QUEEN:      symbol = 'Q'; break;
                    case pieceType::ROOK:       symbol = 'R'; break;
                    case pieceType::BISHOP:     symbol = 'B'; break;
                    case pieceType::KNIGHT:     symbol = 'N'; break;
                    case pieceType::PWAN:       symbol = 'P'; break;
                    case pieceType::AMAZON:     symbol = 'A'; break;
                    case pieceType::GRASSHOPPER: symbol = 'G'; break;
                    case pieceType::KNIGHTRIDER: symbol = 'H'; break;
                    case pieceType::ARCHBISHOP: symbol = 'W'; break;
                    case pieceType::DABBABA:    symbol = 'D'; break;
                    case pieceType::ALFIL:      symbol = 'L'; break;
                    case pieceType::FERZ:       symbol = 'F'; break;
                    case pieceType::CENTAUR:    symbol = 'C'; break;
                    case pieceType::TESTROOK:   symbol = 'T'; break;
                    case pieceType::CAMEL:      symbol = 'M'; break;
                    default: symbol = '?'; break;
                }
                
                // 백 = 대문자, 흑 = 소문자
                if(p->getColor() == colorType::WHITE) {
                    fen += toupper(symbol);
                } else {
                    fen += tolower(symbol);
                }
            }
        }
        
        // 줄의 끝에서 남은 빈 칸 표기
        if(emptyCount > 0) {
            fen += char('0' + emptyCount);
        }
        
        // rank 구분 (마지막 rank 제외)
        if(rank > 0) {
            fen += '/';
        }
    }
    
    return fen;
}

// 로얄 피스 존재 여부
bool bc_board::hasRoyalPiece(colorType color) const {
    for (const auto& p : pieces) {
        if (p.getColor() == color && p.isRoyal()) return true;
    }
    return false;
}

// 로얄 피스가 체크 상태인지 확인 (로얄 피스 중 하나라도 체크되면 true)
bool bc_board::isRoyalPieceInCheck(colorType color) const {
    colorType enemyColor = (color == colorType::WHITE) ? colorType::BLACK : colorType::WHITE;

    // 적 기물이 공격하는 모든 칸을 미리 마킹
    std::array<std::array<bool, BOARD_SIZE>, BOARD_SIZE> attacked{};
    for (auto& col : attacked) col.fill(false);

    for (const auto& p : pieces) {
        if (p.getColor() != enemyColor) continue;
        for (const auto& mv : p.getLegalMoves()) {
            int ef = mv.endFile;
            int er = mv.endRank;
            if (isValidPosition(ef, er)) {
                attacked[ef][er] = true;
            }
        }
    }

    // 로얄 피스 중 하나라도 공격받으면 체크
    for (const auto& r : pieces) {
        if (!r.isRoyal() || r.getColor() != color) continue;
        int f = r.getFile();
        int rk = r.getRank();
        if (isValidPosition(f, rk) && attacked[f][rk]) return true;
    }

    return false;
}

// 로얄 피스 변장 (다른 기물로 위장)
bool bc_board::disguisePiece(int file, int rank, pieceType disguiseAs) {
    if(!isValidPosition(file, rank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }

    // 참고: disguisePiece는 독립적인 로얄 피스 액션이므로 performedActionThisTurn 체크 안 함

    piece* p = getPieceAt(file, rank);
    if(p == nullptr) {
        std::cerr << "No piece at position" << std::endl;
        return false;
    }

    if(!p->isRoyal()) {
        std::cerr << "Piece is not a royal piece" << std::endl;
        return false;
    }

    if(p->getColor() != currentPlayerColor()) {
        std::cerr << "Not your turn" << std::endl;
        return false;
    }

    if(disguiseAs == pieceType::KING || disguiseAs == pieceType::PWAN || disguiseAs == pieceType::NONE) {
        std::cerr << "Cannot disguise as king, pawn, or none" << std::endl;
        return false;
    }

    // 변장 설정: 실제 피스타입도 변장 타입으로 교체해 이동/표기 모두 변함
    p->setDisguisedAs(disguiseAs);
    p->setPieceType(disguiseAs);
    activePieceThisTurn = p;
    performedActionThisTurn = true;
    // 참고: disguisePiece는 특수 행마이므로 performedActionThisTurn 플래그를 설정하지 않음
    // (move/drop과는 별개의 로얄 피스 액션)

    // 노테이션: f1=Q (파일f 랭크1에서 퀸으로 변장)
    char pieceName = ' ';
    switch(disguiseAs) {
        case pieceType::QUEEN:      pieceName = 'Q'; break;
        case pieceType::ROOK:       pieceName = 'R'; break;
        case pieceType::BISHOP:     pieceName = 'B'; break;
        case pieceType::KNIGHT:     pieceName = 'N'; break;
        case pieceType::AMAZON:     pieceName = 'A'; break;
        case pieceType::GRASSHOPPER: pieceName = 'G'; break;
        case pieceType::KNIGHTRIDER: pieceName = 'H'; break;
        case pieceType::ARCHBISHOP: pieceName = 'W'; break;
        case pieceType::DABBABA:    pieceName = 'D'; break;
        case pieceType::ALFIL:      pieceName = 'L'; break;
        case pieceType::FERZ:       pieceName = 'F'; break;
        case pieceType::CENTAUR:    pieceName = 'C'; break;
        case pieceType::TESTROOK:   pieceName = 'T'; break;
        case pieceType::CAMEL:      pieceName = 'M'; break;
        default: pieceName = '?'; break;
    }

    PGN disguiseLog;
    disguiseLog.startFile = file;
    disguiseLog.startRank = rank;
    disguiseLog.pT = p->getPieceType();
    disguiseLog.cT = p->getColor();
    disguiseLog.isDisguise = true;
    disguiseLog.disguiseAs = disguiseAs;
    log.push_back(disguiseLog);

    std::cout << "Piece disguised at (" << file << ", " << rank << ") as " << pieceName << std::endl;

    // 합법수 재계산
    for (auto& piece : pieces) {
        piece.clearMovePatterns();
        setupPiecePatterns(&piece);
    }
    updateAllLegalMoves();
    return true;
}

// 로얄 피스 승격 (다른 기물을 새 로얄 피스로 승격)
bool bc_board::succeedRoyalPiece(int file, int rank, colorType color) {
    if(!isValidPosition(file, rank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }

    // 참고: succeedRoyalPiece는 독립적인 로얄 피스 액션이므로 performedActionThisTurn 체크 안 함

    if(color != currentPlayerColor()) {
        std::cerr << "Not your turn" << std::endl;
        return false;
    }

    piece* targetPiece = getPieceAt(file, rank);
    if(targetPiece == nullptr) {
        std::cerr << "No piece at position" << std::endl;
        return false;
    }

    if(targetPiece->getColor() != color) {
        std::cerr << "Piece is not your color" << std::endl;
        return false;
    }

    if(targetPiece->isRoyal()) {
        std::cerr << "Piece is already royal" << std::endl;
        return false;
    }

    // 새로운 로얄 피스로 지정 (기존 로얄 유지)
    targetPiece->setRoyal(true);
    activePieceThisTurn = targetPiece;
    performedActionThisTurn = true;
    // 참고: succeedRoyalPiece는 특수 행마이므로 performedActionThisTurn 플래그를 설정하지 않음
    // (move/drop과는 별개의 로얄 피스 액션)

    // 노테이션: suc e5 (e5의 기물을 로얄 피스로 승격)
    PGN successionLog;
    successionLog.startFile = file;
    successionLog.startRank = rank;
    successionLog.pT = targetPiece->getPieceType();
    successionLog.cT = color;
    successionLog.isSuccession = true;
    log.push_back(successionLog);

    std::cout << "Piece succeeded as royal at (" << file << ", " << rank << ")" << std::endl;

    // 합법수 재계산
    for (auto& piece : pieces) {
        piece.clearMovePatterns();
        setupPiecePatterns(&piece);
    }
    updateAllLegalMoves();
    return true;
}
