#include <gameboard.hpp>
#include <cmath>

// 생성자
bc_board::bc_board() : turn(0) {
    // 보드 초기화
    for(int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = nullptr;
        }
    }
    resetPockets();
}

// 소멸자
bc_board::~bc_board() {
    // 모든 piece 포인터는 pieces 벡터에서 관리되므로 여기서 delete 필요 없음
}

// 보드 초기화
void bc_board::initializeBoard() {
    turn = 0;
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

// 현재 턴의 플레이어 색상 (0턴=WHITE, 1턴=BLACK ...)
colorType bc_board::currentPlayerColor() const {
    return (turn % 2 == 0) ? colorType::WHITE : colorType::BLACK;
}

// pieceType -> 포켓 인덱스
int bc_board::pieceTypeToIndex(pieceType type) const {
    switch(type) {
        case pieceType::KING:   return 0;
        case pieceType::QUEEN:  return 1;
        case pieceType::BISHOP: return 2;
        case pieceType::KNIGHT: return 3;
        case pieceType::ROOK:   return 4;
        case pieceType::PWAN:   return 5;
        default: return -1;
    }
}

// 포켓 참조 반환
std::array<int, 6>& bc_board::pocketForColor(colorType color) {
    return (color == colorType::BLACK) ? blackPocket : whitePocket;
}

const std::array<int, 6>& bc_board::pocketForColor(colorType color) const {
    return (color == colorType::BLACK) ? blackPocket : whitePocket;
}

std::array<int, 6> bc_board::getPocketStock(colorType color) const {
    return pocketForColor(color);
}

// 포켓 초기화 (초기 보유말)
void bc_board::resetPockets() {
    whitePocket = DEFAULT_POCKET_STOCK;
    blackPocket = DEFAULT_POCKET_STOCK;
}

// 포켓 보유량 수동 설정 (색상별)
void bc_board::setPocketStock(colorType color, const std::array<int, 6>& stock) {
    pocketForColor(color) = stock;
}

// 포켓 보유량 수동 설정 (양쪽 한 번에)
void bc_board::setPocketStockBoth(const std::array<int, 6>& whiteStock, const std::array<int, 6>& blackStock) {
    whitePocket = whiteStock;
    blackPocket = blackStock;
}

// 특정 위치의 기물 가져오기
piece* bc_board::getPieceAt(int file, int rank) const {
    if(!isValidPosition(file, rank)) return nullptr;
    return board[file][rank];
}

// 착수 시 초기 스턴 계산
// README 규칙 15: 폰의 스턴은 착수 랭크에 따라 다름
// 백(WHITE) 기준: rank1=0, rank2=1, ..., rank7=6
// 흑(BLACK) 기준: rank8=0, rank7=1, ..., rank2=6
// 다른 기물: 모두 1
int bc_board::computeInitialStun(pieceType type, colorType color, int rank) const {
    if(type != pieceType::PWAN) {
        return 1; // 폰이 아니면 스턴 1
    }
    
    // 폰의 경우: 랭크에 따라 스턴 설정
    if(color == colorType::WHITE) {
        // 백: rank1(0)=0stun ~ rank7(6)=6stun
        return std::min(rank, 6); // rank는 0-7이므로 min으로 6 제한
    } else {
        // 흑: rank8(7)=0stun ~ rank2(1)=6stun
        return std::min(7 - rank, 6); // 7-rank 계산
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

// 수가 넘어갈 때 스턴 스택/상태 일관성 유지
void bc_board::applyStunTickAll() {
    for(auto& p : pieces) {
        p.applyStunTick();
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

    int pocketIdx = pieceTypeToIndex(type);
    if(pocketIdx < 0) {
        std::cerr << "Invalid piece type" << std::endl;
        return false;
    }
    auto& pocket = pocketForColor(color);
    if(pocket[pocketIdx] <= 0) {
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
    
    // 보드에 포인터 저장
    board[file][rank] = placed;
    pocket[pocketIdx] -= 1;
    
    // 새로운 기물의 합법 이동 계산 (스턴이 있으면 이동 불가지만 계산은 유지)
    updatePieceLegalMoves(placed);
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
    bool isLegalMove = false;
    
    for(const auto& moveData : legalMoves) {
        if(moveData.endFile == toFile && moveData.endRank == toRank) {
            isLegalMove = true;
            break;
        }
    }
    
    if(!isLegalMove) {
        std::cerr << "Illegal move" << std::endl;
        return false;
    }
    
    // 목표 위치의 기물 제거 (스턴 이전 후 포켓 적립)
    piece* targetPiece = getPieceAt(toFile, toRank);
    if(targetPiece != nullptr) {
        // 스턴 이전: 잡힌 기물의 스턴을 이동 기물에게 더한다
        movingPiece->addStun(targetPiece->getStunStack());
        int capturedIdx = pieceTypeToIndex(targetPiece->getPieceType());
        if(capturedIdx >= 0) {
            auto& pocketCaptured = pocketForColor(movingPiece->getColor());
            pocketCaptured[capturedIdx] += 1;
        }
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
    
    // 모든 기물의 합법 이동 재계산
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
    if(target->getColor() != currentPlayerColor()) {
        std::cerr << "Not your turn" << std::endl;
        return false;
    }
    if(target->getPieceType() == pieceType::KING) {
        std::cerr << "Cannot add stun to king" << std::endl;
        return false;
    }

    target->addStun(delta);
    activePieceThisTurn = target;
    performedActionThisTurn = true;
    nextTurn();
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

// 다음 턴 (한 수 = 백-흑 쌍마다 스턴 감소)
void bc_board::nextTurn() {
    if(turn % 2 == 1 && turn != 1) {
        applyStunTickAll();
    }

    // 합법 이동 재계산
    updateAllLegalMoves();
    resetTurnState();

    turn++;
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
