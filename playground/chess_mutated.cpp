#include <iostream>
#include <string>
#include <array>
#include <sstream>
#include <chess.hpp>

struct Command {
	std::string op;
	std::string arg1;
	std::string arg2;
};

std::pair<int, int> toSquare(const std::string& s) {
	if(s.size() != 2) return {-1, -1};
	int f = s[0] - 'a';
	int r = s[1] - '1';
	if(f < 0 || f >= 8 || r < 0 || r >= 8) return {-1, -1};
	return {f, r};
}

pieceType toPieceType(char c) {
	switch(std::toupper(c)) {
		case 'K': return pieceType::KING;
		case 'Q': return pieceType::QUEEN;
		case 'B': return pieceType::BISHOP;
		case 'N': return pieceType::KNIGHT;
		case 'R': return pieceType::ROOK;
		case 'P': return pieceType::PWAN;
		default:  return pieceType::NONE;
	}
}

void printPockets(const bc_board& board) {
	auto wp = board.getPocketStock(colorType::WHITE);
	auto bp = board.getPocketStock(colorType::BLACK);
	auto printSide = [](const char* label, const std::array<int, 15>& p) {
		std::cout << label << " [K Q B N R P A G Kr W D L F C Tr] = "
				  << p[0] << " " << p[1] << " " << p[2] << " "
				  << p[3] << " " << p[4] << " " << p[5] << " " << p[6] << " "
				  << p[7] << " " << p[8] << " " << p[9] << " " << p[10] << " "
				  << p[11] << " " << p[12] << " " << p[13] << " " << p[14] << std::endl;
	};
	printSide("White", wp);
	printSide("Black", bp);
}

void printHelp() {
	std::cout << "명령어: drop <K/Q/B/N/R/P> <e4>, move <e2> <e4>, pass <e4>, board, pocket, stun <e4>, legal <e4>, moves, pgn, help, quit" << std::endl;
	std::cout << "규칙: 한 턴에 하나의 기물만 행동, 스턴>0이면 이동 불가, 이동 시 이동스택 1 소모, 캡처 시 스턴 이전+포켓 적립" << std::endl;
	std::cout << "첫 턴: 각 색 킹을 반드시 한 번은 착수해야 합니다." << std::endl;
	std::cout << "  - stun <e4>: 기물의 스턴값/이동스택 확인" << std::endl;
	std::cout << "  - legal <e4>: 기물의 가능한 이동 표시" << std::endl;
	std::cout << "  - moves: 현재 보드의 모든 합법 수 출력" << std::endl;
	std::cout << "  - pgn: 현재까지의 수순 기보 출력" << std::endl;
}

Command parseCommand(const std::string& line) {
	Command cmd;
	std::istringstream iss(line);
	iss >> cmd.op >> cmd.arg1 >> cmd.arg2;
	return cmd;
}

int main() {
	bc_board board;
	board.initializeBoard();

	bool kingPlacedWhite = false;
	bool kingPlacedBlack = false;

	printHelp();
	board.printBoard();
	printPockets(board);

	while(true) {
		int totalMoves = board.getWhiteMoveCount() + board.getBlackMoveCount();
		bool isWhiteTurn = (board.getWhiteMoveCount() == board.getBlackMoveCount());
		std::cout << "[턴 " << totalMoves + 1 << "] "
				  << (isWhiteTurn ? "백" : "흑")
				  << " > ";
		std::string line;
		if(!std::getline(std::cin, line)) break;
		if(line.empty()) continue;

		Command cmd = parseCommand(line);
		for(auto& ch : cmd.op) ch = std::tolower(ch);

		if(cmd.op == "quit" || cmd.op == "exit") {
			break;
		} else if(cmd.op == "help") {
			printHelp();
			continue;
		} else if(cmd.op == "board") {
			board.printBoard();
			continue;
		} else if(cmd.op == "pocket") {
			printPockets(board);
			continue;
		} else if(cmd.op == "drop") {
			if(cmd.arg1.empty() || cmd.arg2.empty()) {
				std::cout << "형식: drop <K/Q/B/N/R/P> <e4>" << std::endl;
				continue;
			}
			pieceType pt = toPieceType(cmd.arg1[0]);
			auto [f, r] = toSquare(cmd.arg2);
			if(pt == pieceType::NONE || f < 0) {
				std::cout << "잘못된 입력" << std::endl;
				continue;
			}

			colorType turnColor = (board.getWhiteMoveCount() == board.getBlackMoveCount()) 
									? colorType::WHITE : colorType::BLACK;
			if(pt != pieceType::KING) {
				if(turnColor == colorType::WHITE && !kingPlacedWhite) {
					std::cout << "백 킹을 먼저 착수해야 합니다." << std::endl;
					continue;
				}
				if(turnColor == colorType::BLACK && !kingPlacedBlack) {
					std::cout << "흑 킹을 먼저 착수해야 합니다." << std::endl;
					continue;
				}
			}

			if(board.placePiece(pt, turnColor, f, r)) {
				piece* placed = board.getPiece(f, r);
				if(placed) {
					setupPiecePatterns(placed);
					placed->calculateAndUpdateLegalMoves(&board);
				}
				if(pt == pieceType::KING) {
					if(turnColor == colorType::WHITE) kingPlacedWhite = true; else kingPlacedBlack = true;
				}
				board.nextTurn();
			}
		} else if(cmd.op == "move") {
			if(cmd.arg1.empty() || cmd.arg2.empty()) {
				std::cout << "형식: move <e2> <e4>" << std::endl;
				continue;
			}
			auto [f1, r1] = toSquare(cmd.arg1);
			auto [f2, r2] = toSquare(cmd.arg2);
			if(f1 < 0 || f2 < 0) {
				std::cout << "잘못된 좌표" << std::endl;
				continue;
			}
			piece* p = board.getPiece(f1, r1);
			if(!p) {
				std::cout << "해당 위치에 기물이 없습니다." << std::endl;
				continue;
			}
			// 패턴이 없다면 자동 할당
			if(p->getMovePatterns().empty()) {
				setupPiecePatterns(p);
			}
			p->calculateAndUpdateLegalMoves(&board);
			
			if(board.movePiece(f1, r1, f2, r2)) {
				board.nextTurn();
			}
		} else if(cmd.op == "pass") {
			if(cmd.arg1.empty()) {
				std::cout << "형식: pass <e4> (킹 제외)" << std::endl;
				continue;
			}
			auto [f, r] = toSquare(cmd.arg1);
			if(f < 0) {
				std::cout << "잘못된 좌표" << std::endl;
				continue;
			}
			if(board.passAndAddStun(f, r, 1)) {
				// passAndAddStun 내부에서 nextTurn 호출
			}
		} else if(cmd.op == "stun") {
			if(cmd.arg1.empty()) {
				std::cout << "형식: stun <e4>" << std::endl;
				continue;
			}
			auto [f, r] = toSquare(cmd.arg1);
			if(f < 0) {
				std::cout << "잘못된 좌표" << std::endl;
				continue;
			}
			piece* p = board.getPiece(f, r);
			if(!p) {
				std::cout << "해당 위치에 기물이 없습니다." << std::endl;
				continue;
			}
			std::cout << char('a' + f) << (r + 1) << " 기물: ";
			switch(p->getPieceType()) {
				case pieceType::KING:   std::cout << "킹"; break;
				case pieceType::QUEEN:  std::cout << "퀸"; break;
				case pieceType::BISHOP: std::cout << "비숍"; break;
				case pieceType::KNIGHT: std::cout << "나이트"; break;
				case pieceType::ROOK:   std::cout << "룩"; break;
				case pieceType::PWAN:   std::cout << "폰"; break;
				default: std::cout << "?"; break;
			}
			std::cout << " (" << (p->getColor() == colorType::WHITE ? "백" : "흑") << ")" << std::endl;
			std::cout << "  스턴: " << p->getStunStack() << std::endl;
		} else if(cmd.op == "legal") {
			if(cmd.arg1.empty()) {
				std::cout << "형식: legal <e4>" << std::endl;
				continue;
			}
			auto [f, r] = toSquare(cmd.arg1);
			if(f < 0) {
				std::cout << "잘못된 좌표" << std::endl;
				continue;
			}
			piece* p = board.getPiece(f, r);
			if(!p) {
				std::cout << "해당 위치에 기물이 없습니다." << std::endl;
				continue;
			}
			if(p->getMovePatterns().empty()) {
				setupPiecePatterns(p);
			}
			p->calculateAndUpdateLegalMoves(&board);
			const auto& moves = p->getLegalMoves();
			std::cout << "가능한 이동 (" << moves.size() << "개):" << std::endl;
			for(const auto& m : moves) {
				std::cout << "  " << char('a' + m.startFile) << (m.startRank + 1)
				          << " -> " << char('a' + m.endFile) << (m.endRank + 1);
				if(m.take) std::cout << " (캡처)";
				std::cout << std::endl;
			}
		} else if(cmd.op == "moves") {
			int totalMoves = 0;
			std::cout << "현재 모든 합법 수:" << std::endl;
			for(int f = 0; f < 8; f++) {
				for(int r = 0; r < 8; r++) {
					piece* p = board.getPiece(f, r);
					if(!p) continue;
					if(p->getMovePatterns().empty()) {
						setupPiecePatterns(p);
					}
					p->calculateAndUpdateLegalMoves(&board);
					const auto& pieceMoves = p->getLegalMoves();
					if(pieceMoves.empty()) continue;
					std::cout << char('a' + f) << (r + 1) << " 기물 (" << pieceMoves.size() << "개): ";
					for(const auto& m : pieceMoves) {
						std::cout << char('a' + m.endFile) << (m.endRank + 1) << " ";
					}
					std::cout << std::endl;
					totalMoves += pieceMoves.size();
				}
			}
			std::cout << "총 합법 수: " << totalMoves << "개" << std::endl;
		} else if(cmd.op == "pgn") {
			board.printGameLog();
		} else {
			std::cout << "알 수 없는 명령. help 를 입력하세요." << std::endl;
		}
	}

	std::cout << "게임을 종료합니다." << std::endl;
	return 0;
}
