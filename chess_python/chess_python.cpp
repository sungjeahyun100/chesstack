#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <chess.hpp>

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace py = pybind11;

namespace {

pieceType piece_type_from_str(const std::string &s) {
	if (s == "KING" || s == "king" || s == "K") return pieceType::KING;
	if (s == "QUEEN" || s == "queen" || s == "Q") return pieceType::QUEEN;
	if (s == "BISHOP" || s == "bishop" || s == "B") return pieceType::BISHOP;
	if (s == "KNIGHT" || s == "knight" || s == "N") return pieceType::KNIGHT;
	if (s == "ROOK" || s == "rook" || s == "R") return pieceType::ROOK;
	if (s == "PAWN" || s == "pawn" || s == "P" || s == "PWAN") return pieceType::PWAN;
	if (s == "AMAZON" || s == "amazon" || s == "A") return pieceType::AMAZON;
	if (s == "GRASSHOPPER" || s == "grasshopper" || s == "G") return pieceType::GRASSHOPPER;
	if (s == "KNIGHTRIDER" || s == "knightrider" || s == "Kr" || s == "KR") return pieceType::KNIGHTRIDER;
	if (s == "ARCHBISHOP" || s == "archbishop" || s == "W") return pieceType::ARCHBISHOP;
	if (s == "DABBABA" || s == "dabbaba" || s == "D") return pieceType::DABBABA;
	if (s == "ALFIL" || s == "alfil" || s == "L") return pieceType::ALFIL;
	if (s == "FERZ" || s == "ferz" || s == "F") return pieceType::FERZ;
	if (s == "CENTAUR" || s == "centaur" || s == "C") return pieceType::CENTAUR;
	if (s == "TESTROOK" || s == "testrook" || s == "Tr" || s == "TR") return pieceType::TESTROOK;
	if (s == "CAMEL" || s == "camel" || s == "Cl" || s == "CL") return pieceType::CAMEL;
	throw std::invalid_argument("invalid piece type: " + s);
}

colorType color_from_str(const std::string &s) {
	if (s == "WHITE" || s == "white" || s == "w") return colorType::WHITE;
	if (s == "BLACK" || s == "black" || s == "b") return colorType::BLACK;
	throw std::invalid_argument("invalid color: " + s);
}

std::string color_to_str(colorType c) {
	switch (c) {
		case colorType::WHITE: return "white";
		case colorType::BLACK: return "black";
		default: return "none";
	}
}

std::string piece_to_str(pieceType t) {
	switch (t) {
		case pieceType::KING: return "K";
		case pieceType::QUEEN: return "Q";
		case pieceType::BISHOP: return "B";
		case pieceType::KNIGHT: return "N";
		case pieceType::ROOK: return "R";
		case pieceType::PWAN: return "P";
		case pieceType::AMAZON: return "A";
		case pieceType::GRASSHOPPER: return "G";
		case pieceType::KNIGHTRIDER: return "Kr"; // KnightRider distinct code
		case pieceType::ARCHBISHOP: return "W";  // (Bishop+Knight variant)
		case pieceType::DABBABA: return "D";
		case pieceType::ALFIL: return "L";
		case pieceType::FERZ: return "F";
		case pieceType::CENTAUR: return "C";
		case pieceType::TESTROOK: return "Tr";
		case pieceType::CAMEL: return "Cl";
		default: return "?";
	}
}

colorType str_to_color(const std::string& s) {
	if (s == "white") return colorType::WHITE;
	if (s == "black") return colorType::BLACK;
	return colorType::NONE;
}

pieceType str_to_piece(const std::string& s) {
	if (s == "K") return pieceType::KING;
	if (s == "Q") return pieceType::QUEEN;
	if (s == "B") return pieceType::BISHOP;
	if (s == "N") return pieceType::KNIGHT;
	if (s == "R") return pieceType::ROOK;
	if (s == "P") return pieceType::PWAN;
	if (s == "A") return pieceType::AMAZON;
	if (s == "G") return pieceType::GRASSHOPPER;
	if (s == "Kr") return pieceType::KNIGHTRIDER;
	if (s == "W") return pieceType::ARCHBISHOP;
	if (s == "D") return pieceType::DABBABA;
	if (s == "L") return pieceType::ALFIL;
	if (s == "F") return pieceType::FERZ;
	if (s == "C") return pieceType::CENTAUR;
	if (s == "Tr") return pieceType::TESTROOK;
	if (s == "Cl") return pieceType::CAMEL;
	return pieceType::NONE;
}

py::dict pocket_to_dict(const std::array<int, POCKET_SIZE> &p) {
	py::dict d;
	// Follow pocketIndex ordering from enum.hpp
	d["K"] = p[0];
	d["Q"] = p[1];
	d["B"] = p[2];
	d["N"] = p[3];
	d["R"] = p[4];
	d["P"] = p[5];
	d["A"] = p[6];
	d["G"] = p[7];
	d["Kr"] = p[8];
	d["W"] = p[9];
	d["D"] = p[10];
	d["L"] = p[11];
	d["F"] = p[12];
	d["C"] = p[13];
	d["Tr"] = p[14];
	d["Cl"] = p[15];
	return d;
}

py::dict piece_to_dict(const piece *p) {
	py::dict d;
	d["file"] = p->getFile();
	d["rank"] = p->getRank();
	d["type"] = piece_to_str(p->getPieceType());
	d["color"] = color_to_str(p->getColor());
	d["stun"] = p->getStunStack();
	d["stunned"] = p->isStunned();
	d["move_stack"] = p->getMoveStack();
	d["royal"] = p->isRoyal();
	const auto disguise = p->getDisguisedAs();
	if (disguise == pieceType::NONE) {
		d["disguised_as"] = ""; // 빈 문자열이면 변장 없음
	} else {
		d["disguised_as"] = piece_to_str(disguise);
	}
	return d;
}

std::array<int, POCKET_SIZE> dict_to_pocket(const py::dict &d) {
	std::array<int, POCKET_SIZE> p{};
	// Defaults align with standard starting set; specials default to 0
	p[0] = d.contains("K") ? d["K"].cast<int>() : 1;
	p[1] = d.contains("Q") ? d["Q"].cast<int>() : 1;
	p[2] = d.contains("B") ? d["B"].cast<int>() : 2;
	p[3] = d.contains("N") ? d["N"].cast<int>() : 2;
	p[4] = d.contains("R") ? d["R"].cast<int>() : 2;
	p[5] = d.contains("P") ? d["P"].cast<int>() : 8;
	p[6] = d.contains("A") ? d["A"].cast<int>() : 0;
	p[7] = d.contains("G") ? d["G"].cast<int>() : 0;
	p[8] = d.contains("Kr") ? d["Kr"].cast<int>() : 0;
	p[9] = d.contains("W") ? d["W"].cast<int>() : 0;
	p[10] = d.contains("D") ? d["D"].cast<int>() : 0;
	p[11] = d.contains("L") ? d["L"].cast<int>() : 0;
	p[12] = d.contains("F") ? d["F"].cast<int>() : 0;
	p[13] = d.contains("C") ? d["C"].cast<int>() : 0;
	p[14] = d.contains("Tr") ? d["Tr"].cast<int>() : 0;
	p[15] = d.contains("Cl") ? d["Cl"].cast<int>() : 0;
	return p;
}

class PyBoard {
public:
	PyBoard() { board.initializeBoard(); }

	PyBoard(const py::dict &white_pocket, const py::dict &black_pocket) 
		: board(dict_to_pocket(white_pocket), dict_to_pocket(black_pocket)) {}

	void reset() { board.initializeBoard(); }

    void setupAllPieceMovePattern() { setupAllPieces(&board); }

	bool place_piece(const std::string &type, const std::string &color, int file, int rank) {
		return board.placePiece(piece_type_from_str(type), color_from_str(color), file, rank);
	}

	bool move_piece(int from_file, int from_rank, int to_file, int to_rank) {
		return board.movePiece(from_file, from_rank, to_file, to_rank);
	}

	bool remove_piece(int file, int rank) { return board.removePiece(file, rank); }

	void next_turn() { board.nextTurn(); }

	std::string turn_color() const { 
		return (board.getWhiteMoveCount() == board.getBlackMoveCount()) ? "white" : "black"; 
	}

	py::dict pocket(const std::string &color) const {
		return pocket_to_dict(board.getPocketStock(color_from_str(color)));
	}

	std::vector<py::dict> board_state() const {
		std::vector<py::dict> out;
		for (int f = 0; f < 8; ++f) {
			for (int r = 0; r < 8; ++r) {
				piece *p = board.getPiece(f, r);
				if (p) out.push_back(piece_to_dict(p));
			}
		}
		return out;
	}

	std::vector<py::dict> legal_moves(int file, int rank) const {
		std::vector<py::dict> out;
		piece *p = board.getPiece(file, rank);
		if (!p) return out;
		const auto &moves = p->getLegalMoves();
		for (const auto &mv : moves) {
			py::dict d;
			d["from_file"] = mv.startFile;
			d["from_rank"] = mv.startRank;
			d["to_file"] = mv.endFile;
			d["to_rank"] = mv.endRank;
			d["take"] = mv.take;
			d["is_drop"] = mv.isDrop;
			d["piece"] = piece_to_str(mv.pT);
			d["color"] = color_to_str(mv.cT);
			out.push_back(std::move(d));
		}
		return out;
	}

	bool add_stun(int file, int rank, int delta = 1) {
		return board.passAndAddStun(file, rank, delta);
	}

	bool promote(int file, int rank, const std::string &promoteTo) {
		return board.promote(file, rank, piece_type_from_str(promoteTo));
	}

	bool succeed_royal_piece(int file, int rank) {
		// file, rank에 있는 기물이 새 로얄 피스가 됨
		colorType currentColor = (board.getWhiteMoveCount() == board.getBlackMoveCount()) ? colorType::WHITE : colorType::BLACK;
		if (!board.hasRoyalPiece(currentColor)) {
			return false; // 현재 로얄 피스가 없으면 불가능
		}
		piece* successor = board.getPiece(file, rank);
		if (!successor || successor->getColor() != currentColor || successor->getPieceType() == pieceType::KING) {
			return false; // 기물이 없거나 왕이면 불가능
		}
		return board.succeedRoyalPiece(file, rank, currentColor);
	}

	bool disguise_piece(int file, int rank, const std::string &disguise_as) {
		// file, rank에 있는 왕(로얄 피스)이 disguise_as로 변장
		colorType currentColor = (board.getWhiteMoveCount() == board.getBlackMoveCount()) ? colorType::WHITE : colorType::BLACK;
		piece* royal = board.getPiece(file, rank);
		if (!royal || !royal->isRoyal() || royal->getColor() != currentColor) {
			return false; // 기물이 없거나 왕이 아니면 불가능
		}
		return board.disguisePiece(file, rank, piece_type_from_str(disguise_as));
	}

	int white_move_count() const { return board.getWhiteMoveCount(); }
	int black_move_count() const { return board.getBlackMoveCount(); }

	void print_board() const { board.printBoard(); }
	
	// 포지션 설정: 리스트 그대로 또는 {"turn": "white/black", "pieces": [...], "pockets": {"white": {...}, "black": {...}}}
	void setup_position(const py::object& position_obj) {
		py::list piece_list;
		colorType turn = colorType::WHITE;
		std::array<int, POCKET_SIZE> whitePocketOverride{};
		std::array<int, POCKET_SIZE> blackPocketOverride{};
		bool hasPocketOverride = false;

		if (py::isinstance<py::dict>(position_obj)) {
			py::dict d = position_obj.cast<py::dict>();
			if (d.contains("turn")) {
				turn = str_to_color(d["turn"].cast<std::string>());
			}
			if (d.contains("pieces")) {
				piece_list = d["pieces"].cast<py::list>();
			} else {
				piece_list = py::list(position_obj);
			}
			if (d.contains("pockets")) {
				py::dict pockets = d["pockets"].cast<py::dict>();
				if (pockets.contains("white")) {
					whitePocketOverride = dict_to_pocket(pockets["white"].cast<py::dict>());
					hasPocketOverride = true;
				}
				if (pockets.contains("black")) {
					blackPocketOverride = dict_to_pocket(pockets["black"].cast<py::dict>());
					hasPocketOverride = true;
				}
			}
		} else {
			piece_list = position_obj.cast<py::list>();
		}

		std::vector<std::tuple<pieceType, colorType, int, int, int, int>> pieces;
		for (const auto& item : piece_list) {
			py::dict d = item.cast<py::dict>();
			std::string type_str = d["type"].cast<std::string>();
			std::string color_str = d["color"].cast<std::string>();
			int file = d["file"].cast<int>();
			int rank = d["rank"].cast<int>();
			int stun = d["stun"].cast<int>();
			int move_stack = d["move_stack"].cast<int>();

			pieceType type = str_to_piece(type_str);
			colorType color = str_to_color(color_str);

			pieces.emplace_back(type, color, file, rank, stun, move_stack);
		}

		if (hasPocketOverride) {
			board.setupPosition(pieces, turn, &whitePocketOverride, &blackPocketOverride);
		} else {
			board.setupPosition(pieces, turn);
		}
	}

private:
	bc_board board;
};

} // namespace

PYBIND11_MODULE(chess_python, m) {
	m.doc() = "Python bindings for the 변형체스 engine";

	py::class_<PyBoard>(m, "Board")
		.def(py::init<>())
		.def(py::init<const py::dict &, const py::dict &>(), 
			py::arg("white_pocket"), py::arg("black_pocket"),
			"Create board with custom pocket configuration")
		.def("reset", &PyBoard::reset, "Reset board and pockets")
        .def("setupAllPieceMovePattern", &PyBoard::setupAllPieceMovePattern)
		.def("place_piece", &PyBoard::place_piece, py::arg("type"), py::arg("color"), py::arg("file"), py::arg("rank"))
		.def("move_piece", &PyBoard::move_piece, py::arg("from_file"), py::arg("from_rank"), py::arg("to_file"), py::arg("to_rank"))
		.def("remove_piece", &PyBoard::remove_piece, py::arg("file"), py::arg("rank"))
		.def("next_turn", &PyBoard::next_turn)
		.def("turn_color", &PyBoard::turn_color)
		.def("pocket", &PyBoard::pocket, py::arg("color"), "Get pocket counts as dict")
		.def("board_state", &PyBoard::board_state, "List of pieces with positions and stacks")
		.def("legal_moves", &PyBoard::legal_moves, py::arg("file"), py::arg("rank"), "Legal moves for a square")
		.def("add_stun", &PyBoard::add_stun, py::arg("file"), py::arg("rank"), py::arg("delta") = 1, "Pass turn and add stun to a non-king piece")
		.def("promote", &PyBoard::promote, py::arg("file"), py::arg("rank"), py::arg("promoteTo"), "Promote pawn to another piece")
		.def("succeed_royal_piece", &PyBoard::succeed_royal_piece, py::arg("file"), py::arg("rank"), "Make a piece the new royal piece")
		.def("disguise_piece", &PyBoard::disguise_piece, py::arg("file"), py::arg("rank"), py::arg("disguise_as"), "Disguise royal piece as another piece type")
		.def("white_move_count", &PyBoard::white_move_count, "Get white's move count")
		.def("black_move_count", &PyBoard::black_move_count, "Get black's move count")
		.def("setup_position", &PyBoard::setup_position, py::arg("piece_list"), "Setup custom position from list of pieces")
		.def("print_board", &PyBoard::print_board);
}