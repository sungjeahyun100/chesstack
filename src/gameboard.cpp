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

// 스턴은 movePiece에서 직접 관리됨
void bc_board::decayStunAllAndChargeMoves() {
    // 아무것도 하지 않음
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
    
    // 수 카운트 증가 및 해당 색상 기물들의 스턴 스택 감소 (착수 전에 먼저 수행)
    if(color == colorType::WHITE) {
        whiteMoveCount++;
        applyStunTickForColor(colorType::WHITE);
    } else {
        blackMoveCount++;
        applyStunTickForColor(colorType::BLACK);
    }
    
    // pieces 컨테이너에 새로운 기물 추가
    pieces.emplace_back(type, color, file, rank, pieces.size());
    piece* placed = &pieces.back();

    // 초기 스턴 설정
    int initStun = computeInitialStun(type, color, rank);
    placed->setStun(initStun);
    
    // 보드에 포인터 저장
    board[file][rank] = placed;
    pocket[idx] -= 1;
    
    activePieceThisTurn = placed;
    performedActionThisTurn = true;
    
    // 착수 로그: @<좌표> 형식으로 기록 (0,0에서 목표로 이동으로 표현)
    log.push_back(PGN(0, 0, file, rank, type, color, false));
    
    std::cout << "Piece placed at (" << file << ", " << rank << ")" << std::endl;
    return true;
}

// 기물 이동
bool bc_board::movePiece(int fromFile, int fromRank, int toFile, int toRank) {
    if(!isValidPosition(fromFile, fromRank) || !isValidPosition(toFile, toRank)) {
        std::cerr << "Invalid position" << std::endl;
        return false;
    }
    
    piece* movingPiece = getPieceAt(fromFile, fromRank);
    if(movingPiece == nullptr) {
        std::cerr << "No piece at source position" << std::endl;
        return false;
    }

    if(movingPiece->getColor() != currentPlayerColor()) {
        std::cerr << "Not your turn" << std::endl;
        return false;
    }

    if(performedActionThisTurn && activePieceThisTurn != movingPiece) {
        std::cerr << "Another piece already acted this turn" << std::endl;
        return false;
    }

    // 스턴 확인: 스턴 상태이면 이 턴에서 움직일 수 없음
    if(movingPiece->isStunned()) {
        std::cerr << "Piece is stunned" << std::endl;
        return false;
    }
    
    // 합법 이동인지 확인
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
    
    // 수 카운트 증가 및 해당 색상 기물들의 스턴 스택 감소 (이동 전에 먼저 수행)
    colorType movingColor = movingPiece->getColor();
    if(movingColor == colorType::WHITE) {
        whiteMoveCount++;
        applyStunTickForColor(colorType::WHITE);
    } else {
        blackMoveCount++;
        applyStunTickForColor(colorType::BLACK);
    }
    
    // TAKEJUMP: 중간 기물도 캡처
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

    // 목표 위치의 기물 제거 (스턴 이전 후 포켓 적립)
    piece* targetPiece = getPieceAt(toFile, toRank);
    if(targetPiece != nullptr) {
        // 스턴 이전: 잡힌 기물의 스턴을 이동 기물에게 더한다
        movingPiece->addStun(targetPiece->getStunStack());
        
        // 잡힌 기물을 포켓에 추가
        pieceType capturedType = targetPiece->getPieceType();
        pocketIndex capturedPIdx = pieceTypeToPocketIndex(capturedType);
        auto& pocketCaptured = fullPocketForColor(movingColor);
        int capturedIdx = static_cast<int>(capturedPIdx);
        pocketCaptured[capturedIdx] += 1;
        
        removePiece(toFile, toRank);
    }
    
    // 기물 이동
    board[fromFile][fromRank] = nullptr;
    board[toFile][toRank] = movingPiece;
    movingPiece->setFile(toFile);
    movingPiece->setRank(toRank);
    activePieceThisTurn = movingPiece;
    performedActionThisTurn = true;
    
    std::cout << "Piece moved from (" << fromFile << ", " << fromRank << ") to (" 
              << toFile << ", " << toRank << ")" << std::endl;
    
    // 이동 로그 저장
    log.push_back(PGN(fromFile, fromRank, toFile, toRank, movingPiece->getPieceType(), movingPiece->getColor(), (targetPiece != nullptr)));
    
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
    bool pawnStunned = pawn->isStunned();
    
    // 폰을 포켓에 돌려놓음
    auto& pocket = fullPocketForColor(pawn->getColor());
    pocketIndex pawnIdx = pocketIndex::PAWN;
    pocket[static_cast<int>(pawnIdx)] += 1;
    
    // 새 기물 타입으로 변환
    pawn->setPieceType(promoteTo);
    pawn->setStun(pawnStun);
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
    // 스턴은 상대 기물에만 적용 (자신 기물은 허용하지 않음)
    if(target->getColor() == currentPlayerColor()) {
        std::cerr << "Cannot stun your own piece" << std::endl;
        return false;
    }
    if(target->getPieceType() == pieceType::KING) {
        std::cerr << "Cannot add stun to king" << std::endl;
        return false;
    }

    // 수 카운트 증가 및 해당 색상 기물들의 스턴 스택 감소 (스턴 추가 전에 먼저 수행)
    colorType currentColor = currentPlayerColor();
    if(currentColor == colorType::WHITE) {
        whiteMoveCount++;
        applyStunTickForColor(colorType::WHITE);
    } else {
        blackMoveCount++;
        applyStunTickForColor(colorType::BLACK);
    }
    
    target->addStun(delta);
    activePieceThisTurn = target;
    performedActionThisTurn = true;
    
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
    return true;
}

// 다음 턴
void bc_board::nextTurn() {
    // 합법 이동 재계산
    updateAllLegalMoves();
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
