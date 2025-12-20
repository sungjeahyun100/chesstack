#!/usr/bin/env python3
"""Pygame UI driven by the C++ engine (chess_python).

- 드롭/이동/스턴 모두 pybind11 바인딩을 통해 수행합니다.
- 합법 수는 엔진의 legal_moves를 사용합니다.
- 기본 조작: D=드롭, M=이동, S=스턴, 턴 종료는 END TURN 버튼 클릭, R=리셋, Q/Esc=종료, 숫자키 1~6으로 드롭 기물 선택.
- L+숫자: 테스트 포지션 로드 (L1~L9)
"""

from __future__ import annotations

import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import pygame
import test_positions

# Ensure the locally built extension (build/chess_python*.so) is importable.
_ROOT = Path(__file__).resolve().parent
_BUILD = _ROOT / "build"
_BUILD_RELEASE = _BUILD / "Release"
_BUILD_DEBUG = _BUILD / "Debug"

for p in [
    _BUILD,
    _BUILD_RELEASE,
    _BUILD_DEBUG,
]:
    if p.exists():
        sp = str(p)
        if sp not in sys.path:
            sys.path.insert(0, sp)

try:
    import chess_python  # pybind11 확장 모듈
    # 디버그: 로드된 모듈의 실제 파일 위치를 출력
    print(f"Loaded chess_python from: {getattr(chess_python, '__file__', 'unknown')}")
except ImportError:
    print("chess_python 모듈을 찾을 수 없습니다. 빌드 후 다시 실행하세요.")
    print(f"sys.path head: {sys.path[:5]}")
    sys.exit(1)

BOARD_SIZE = 8
SQUARE_SIZE = 80
BOARD_PX = BOARD_SIZE * SQUARE_SIZE
INFO_WIDTH = 340
WINDOW_SIZE = (BOARD_PX + INFO_WIDTH, BOARD_PX)
DEBUG_EXTRA_WIDTH = 260  # 추가 디버그 패널 폭

LIGHT = (238, 238, 210)
DARK = (118, 150, 86)
SELECTED = (186, 202, 68)
TARGET = (244, 247, 116)
LAST = (246, 246, 105)
TEXT = (20, 20, 20)
PANEL_BG = (32, 32, 32)
PANEL_TEXT = (235, 235, 235)
STUN_TEXT = (200, 50, 50)

PIECE_ORDER = [
    "P", "N", "B", "R", "Q", "K",
    "A", "G", "Kr", "W", "D", "L", "F", "C", "Tr", "Cl"
]
PIECE_NAMES = {
    "P": "Pawn",
    "N": "Knight",
    "B": "Bishop",
    "R": "Rook",
    "Q": "Queen",
    "A": "Amazon",
    "K": "King",
    "G": "Grasshopper",
    "Kr": "KnightRider",
    "W": "Archbishop",
    "D": "Dabbaba",
    "L": "Alfil",
    "F": "Ferz",
    "C": "Centaur",
    "Tr": "TestRook",
    "Cl": "Camel",
}

# Button layout will be computed dynamically based on panel width

# Glyphs for nicer board display (white pieces shown; black lowercased where applicable)
PIECE_GLYPH = {
    "K": "K",
    "Q": "Q",
    "R": "R",
    "B": "B",
    "N": "N",
    "P": "P",
    # Fairy pieces use simple ASCII codes
    "A": "A",    # Amazon
    "G": "G",    # Grasshopper
    "Kr": "Kr",  # KnightRider
    "W": "W",    # Archbishop
    "D": "D",    # Dabbaba
    "L": "L",    # Alfil
    "F": "F",    # Ferz
    "C": "C",    # Centaur
    "Tr": "Tr",  # TesterRook
    "Cl": "Cl",
}

white_pocket = {'K': 1, 'Q': 1, 'B': 2, 'N': 2, 'R': 2, 'P': 8, 'Cl': 1}
black_pocket = {'K': 1, 'Q': 1, 'B': 2, 'N': 2, 'R': 2, 'P': 8, 'Cl': 1}

class EngineState:
    """Bridge between UI state and the C++ engine."""

    def __init__(self) -> None:
        self.engine = chess_python.Board(white_pocket, black_pocket) # type: ignore
        self.mode = "drop"  # move | drop | stun | promote | succession | disguise
        self.drop_kind = "K"
        self.selected: Optional[Tuple[int, int]] = None
        self.targets: List[Tuple[int, int]] = []
        self.last_move: Tuple[Tuple[int, int], Tuple[int, int]] = ((-1, -1), (-1, -1))
        self.status = "Drop mode"
        self.game_over = False
        self.promoting_pos: Optional[Tuple[int, int]] = None  # 프로모션 대기 위치
        self.promoted_piece: Optional[str] = None  # 프로모션된 기물 표시
        self.debug: bool = True  # 기본값을 확장 뷰로
        self.hovered: Optional[Tuple[int, int]] = None
        self.refresh()

    def refresh(self) -> None:
        self.turn = self.engine.turn_color()
        self.pieces: Dict[Tuple[int, int], Dict[str, object]] = {}
        for p in self.engine.board_state():
            self.pieces[(p["file"], p["rank"])] = p
        self.pockets = {
            "white": self.engine.pocket("white"),
            "black": self.engine.pocket("black"),
        }
        self._check_royal_elimination()

    def _check_royal_elimination(self) -> None:
        """If a side has no royal piece after the opening turn, end the game."""
        total_moves = self.engine.white_move_count() + self.engine.black_move_count()
        if total_moves < 2:  # 첫 턴(아직 턴 종료 전)에는 판정하지 않음
            return

        has_white_royal = any(p.get("royal") and p.get("color") == "white" for p in self.pieces.values())
        has_black_royal = any(p.get("royal") and p.get("color") == "black" for p in self.pieces.values())

        if not has_white_royal and not has_black_royal:
            self.status = "Draw: no royals"  # 이론상 동시 소멸 방지
            self.game_over = True
        elif not has_white_royal:
            self.status = "Black wins (white lost royal)"
            self.game_over = True
        elif not has_black_royal:
            self.status = "White wins (black lost royal)"
            self.game_over = True

    def setupMovePattern(self) -> None:
        self.engine.setupAllPieceMovePattern()

    def piece_at(self, x: int, y: int) -> Optional[Dict[str, object]]:
        return self.pieces.get((x, y))

    def legal_moves(self, x: int, y: int) -> List[Tuple[int, int]]:
        out = []
        for mv in self.engine.legal_moves(x, y):
            out.append((mv["to_file"], mv["to_rank"]))
        return out

    def try_drop(self, kind: str, x: int, y: int) -> bool:
        ok = self.engine.place_piece(kind, self.turn, x, y)
        if ok:
            self.last_move = ((-1, -1), (x, y))
            self.setupMovePattern()
            self.refresh()
        return ok

    def try_move(self, src: Tuple[int, int], dst: Tuple[int, int]) -> bool:
        sx, sy = src
        dx, dy = dst
        ok = self.engine.move_piece(sx, sy, dx, dy)
        if ok:
            self.last_move = (src, dst)
            self.setupMovePattern()
            # 폰 프로모션 체크: 백 rank7, 흑 rank0
            piece_at_dst = self.engine.getPiece(dx, dy) if hasattr(self.engine, 'getPiece') else None
            board_state = self.engine.board_state()
            for p in board_state:
                if p["file"] == dx and p["rank"] == dy:
                    if p["type"] == "P" and ((self.turn == "white" and dy == 7) or (self.turn == "black" and dy == 0)):
                        self.promoting_pos = (dx, dy)
                        self.mode = "promote"
                        self.status = "Choose promotion (Tab cycle)"
                        self.refresh()
                        return True
                    break
            self.refresh()
        return ok

    def try_promote(self, x: int, y: int, piece_type: str) -> bool:
        ok = self.engine.promote(x, y, piece_type)
        if ok:
            self.promoted_piece = piece_type  # 승격된 기물 저장
            self.status = f"Promoted pawn to {piece_type}"
            self.promoting_pos = None
            self.mode = "drop"
            self.refresh()
        return ok

    def try_stun(self, x: int, y: int) -> bool:
        ok = self.engine.add_stun(x, y, 1)
        if ok:
            self.last_move = ((-1, -1), (x, y))
            self.refresh()
        return ok

    def try_succession(self, x: int, y: int) -> bool:
        """왕을 새로운 기물로 대체"""
        try:
            ok = self.engine.succeed_royal_piece(x, y)
            if ok:
                self.last_move = ((-1, -1), (x, y))
                self.refresh()
            return ok
        except AttributeError:
            # Python binding이 아직 없으면 실패
            return False

    def try_disguise(self, x: int, y: int, piece_type: str) -> bool:
        """왕을 다른 기물로 변장"""
        try:
            ok = self.engine.disguise_piece(x, y, piece_type)
            if ok:
                self.last_move = ((-1, -1), (x, y))
                self.refresh()
            return ok
        except AttributeError:
            # Python binding이 아직 없으면 실패
            return False

    def load_position(self, position_name: str) -> bool:
        """테스트 포지션 로드"""
        pos_data = test_positions.get_position(position_name)
        if not pos_data:
            self.status = f"Position '{position_name}' not found"
            return False
        
        try:
            self.engine.setup_position(pos_data)
            self.setupMovePattern()
            self.refresh()
            self.status = f"pos:{position_name}"
            return True
        except Exception as e:
            self.status = f"Err loading position: {e}"
            return False


def board_from_mouse(pos: Tuple[int, int]) -> Optional[Tuple[int, int]]:
    mx, my = pos
    if mx >= BOARD_PX:
        return None
    x = mx // SQUARE_SIZE
    y = BOARD_SIZE - 1 - (my // SQUARE_SIZE)  # y=0 bottom
    return x, y


def layout_panel_width(gs: EngineState) -> int:
    # 항상 확장 폭 유지 (디버그 토글과 무관)
    return INFO_WIDTH + DEBUG_EXTRA_WIDTH


def layout_buttons(gs: EngineState) -> Tuple[pygame.Rect, pygame.Rect]:
    panel_width = layout_panel_width(gs)
    debug_btn = pygame.Rect(BOARD_PX + 10, BOARD_PX - 120, panel_width - 20, 50)
    end_btn = pygame.Rect(BOARD_PX + 10, BOARD_PX - 60, panel_width - 20, 50)
    return debug_btn, end_btn


def draw(gs: EngineState, screen, font, info_font) -> None:
    panel_width = layout_panel_width(gs)
    debug_btn, end_btn = layout_buttons(gs)
    debug_panel = gs.debug
    base_x = BOARD_PX + 10
    debug_x = BOARD_PX + INFO_WIDTH + 10 if debug_panel else base_x

    # Board squares
    for file in range(BOARD_SIZE):
        for rank in range(BOARD_SIZE):
            x = file * SQUARE_SIZE
            y = (BOARD_SIZE - 1 - rank) * SQUARE_SIZE
            base = LIGHT if (file + rank) % 2 == 0 else DARK
            if (file, rank) in gs.last_move:
                base = LAST
            if gs.selected == (file, rank):
                base = SELECTED
            elif (file, rank) in gs.targets:
                base = TARGET
            elif gs.debug and gs.hovered == (file, rank):
                base = tuple(min(255, c + 25) for c in base)
            # 프로모션 모드: 프로모션 위치 색상 반전
            if gs.mode == "promote" and gs.promoting_pos == (file, rank):
                base = (255 - base[0], 255 - base[1], 255 - base[2])
            pygame.draw.rect(screen, base, (x, y, SQUARE_SIZE, SQUARE_SIZE))

            p = gs.piece_at(file, rank)
            if p:
                symbol = str(p["type"])
                display = PIECE_GLYPH.get(symbol, symbol)
                # For black, use lowercase letters; unicode glyphs stay as-is
                if p["color"] == "black" and display.isalpha():
                    display = display.lower()
                text = font.render(display, True, TEXT)
                rect = text.get_rect(center=(x + SQUARE_SIZE // 2, y + SQUARE_SIZE // 2))
                screen.blit(text, rect)

                # 이동 스택을 좌상단에 파란색으로 표시 (0이면 생략)
                if p["move_stack"] > 0: # type: ignore
                    move_surf = info_font.render(str(p["move_stack"]), True, (100, 150, 255))  # 파란색
                    move_rect = move_surf.get_rect()
                    move_rect.topleft = (x + 4, y + 4)
                    screen.blit(move_surf, move_rect)

                # 스턴 스택을 우상단에 표시 (0이면 생략)
                if p["stun"] > 0: # type: ignore
                    stun_surf = info_font.render(str(p["stun"]), True, STUN_TEXT)
                    stun_rect = stun_surf.get_rect()
                    stun_rect.topright = (x + SQUARE_SIZE - 6, y + 4)
                    screen.blit(stun_surf, stun_rect)

                # 스턴 상태 마커를 우하단에 '*'로 표시
                if p["stunned"]:
                    mark = info_font.render("*", True, STUN_TEXT)
                    mark_rect = mark.get_rect()
                    mark_rect.bottomright = (x + SQUARE_SIZE - 6, y + SQUARE_SIZE - 4)
                    screen.blit(mark, mark_rect)

    # Info panel
    panel = pygame.Rect(BOARD_PX, 0, panel_width, BOARD_PX)
    pygame.draw.rect(screen, PANEL_BG, panel)

    lines = [
        f"Turn: {gs.turn}",
        f"White moves: {gs.engine.white_move_count()}",
        f"Black moves: {gs.engine.black_move_count()}",
        f"Mode: {gs.mode}",
        f"Drop: {PIECE_NAMES.get(gs.drop_kind, '')} ({gs.drop_kind})" if gs.mode != "promote" else f"Promote to: {PIECE_NAMES.get(gs.drop_kind, '')} ({gs.drop_kind})",
        f"Status: {gs.status}",
        f"Debug: {'ON' if gs.debug else 'OFF'}",
    ]
    # 프로모션된 기물 표시
    if gs.promoted_piece:
        lines.append(f"✓ Promoted: {PIECE_NAMES.get(gs.promoted_piece, gs.promoted_piece)} ({gs.promoted_piece})")
    lines.extend([
        "",
        "Reserves (White | Black):",
    ])
    for k in PIECE_ORDER:
        white_count = gs.pockets['white'].get(k, 0)
        black_count = gs.pockets['black'].get(k, 0)
        # 양쪽 다 0이면 표시하지 않음
        if white_count > 0 or black_count > 0:
            lines.append(f"  {k}: {white_count:2d} | {black_count:2d}")
    lines.extend(
        [
            "",
            "Controls:",
            "  M move | D drop | S stun",
            "  U succession | V disguise",
            "  Tab/Shift+Tab cycle pieces",
            "  Click END TURN to finish turn",
            "  R reset | Q/Esc quit",
            "  ←/→ [,] also cycle",
        ]
    )

    y = 10
    for line in lines:
        surf = info_font.render(line, True, PANEL_TEXT)
        screen.blit(surf, (base_x, y))
        y += 22

    # End Turn button
    pygame.draw.rect(screen, (50, 100, 50), end_btn)
    pygame.draw.rect(screen, (100, 200, 100), end_btn, 2)
    btn_text = info_font.render("END TURN", True, (255, 255, 255))
    btn_rect = btn_text.get_rect(center=end_btn.center)
    screen.blit(btn_text, btn_rect)

    # Debug toggle button
    dbg_color = (80, 80, 120) if gs.debug else (60, 60, 60)
    pygame.draw.rect(screen, dbg_color, debug_btn)
    pygame.draw.rect(screen, (140, 140, 200), debug_btn, 2)
    dbg_text = info_font.render("DEBUG OVERLAY", True, (255, 255, 255))
    dbg_rect = dbg_text.get_rect(center=debug_btn.center)
    screen.blit(dbg_text, dbg_rect)

    # Selected piece detail (defer rendering; place in debug panel if present)
    selected_lines: List[str] = []
    if gs.selected:
        sx, sy = gs.selected
        p = gs.piece_at(sx, sy)
        if p:
            selected_lines = [
                f"Selected: {p['type']}{'*' if p['color']=='black' else ''} @ {sx},{sy}",
                f"Stun: {p['stun']}",
                "Targets: " + (" ".join([f"{tx},{ty}" for tx, ty in gs.targets]) or "none"),
            ]

    # Debug panel content (uses extra width area)
    if debug_panel:
        dy = 10
        # Hover detail
        if gs.hovered:
            hp = gs.piece_at(*gs.hovered)
            label = f"Hover: {gs.hovered[0]},{gs.hovered[1]}" if hp else "Hover: empty"
            surf = info_font.render(label, True, (200, 220, 200))
            screen.blit(surf, (debug_x, dy)); dy += 20
            if hp:
                dbg_lines = [
                    f"type={hp['type']} color={hp['color']}",
                    f"stun={hp['stun']} stunned={hp['stunned']}",
                    f"move_stack={hp['move_stack']} royal={hp.get('royal', False)}",
                    f"disguise={hp.get('disguised_as', '') or '-'}",
                ]
                for line in dbg_lines:
                    surf = info_font.render(line, True, (180, 220, 180))
                    screen.blit(surf, (debug_x, dy)); dy += 18
            dy += 8

        # Selected piece detail (relocated to debug panel to avoid overlap)
        if selected_lines:
            surf = info_font.render("Selected:", True, PANEL_TEXT)
            screen.blit(surf, (debug_x, dy)); dy += 18
            for line in selected_lines:
                surf = info_font.render(line, True, (220, 220, 180))
                screen.blit(surf, (debug_x, dy)); dy += 18
            dy += 8

        # Royal pieces list
        royals = [p for p in gs.engine.board_state() if p.get('royal')]
        surf = info_font.render(f"Royals ({len(royals)}):", True, PANEL_TEXT)
        screen.blit(surf, (debug_x, dy)); dy += 20
        for rp in royals:
            line = f"{rp['color'][0]} {rp['type']} @ {rp['file']},{rp['rank']}" + (f" disguise={rp.get('disguised_as','') or '-'}" if rp.get('disguised_as') else "")
            surf = info_font.render(line, True, (210, 210, 255))
            screen.blit(surf, (debug_x, dy)); dy += 18
        if not royals:
            surf = info_font.render("(none)", True, (140, 140, 160))
            screen.blit(surf, (debug_x, dy)); dy += 18
        dy += 10

        # All pieces quick table (color:type@pos stun/ms)
        surf = info_font.render("Pieces:", True, PANEL_TEXT)
        screen.blit(surf, (debug_x, dy)); dy += 18
        pcs = sorted(gs.engine.board_state(), key=lambda p: (p['color'], p['type'], p['file'], p['rank']))
        for p in pcs:
            line = f"{p['color'][0]} {p['type']} {p['file']},{p['rank']} s={p['stun']}/{p['move_stack']}" + (" R" if p.get('royal') else "")
            if p.get('disguised_as'):
                line += f" dg={p['disguised_as']}"
            surf = info_font.render(line, True, (180, 200, 255) if p.get('royal') else PANEL_TEXT)
            screen.blit(surf, (debug_x, dy)); dy += 16
    else:
        # Non-debug: render selected detail in main panel below existing info
        for line in selected_lines:
            surf = info_font.render(line, True, PANEL_TEXT)
            screen.blit(surf, (base_x, y))
            y += 20

    pygame.display.flip()


def handle_click(gs: EngineState, pos: Tuple[int, int]) -> None:
    debug_btn, end_btn = layout_buttons(gs)

    # Debug toggle button
    if debug_btn.collidepoint(pos):
        gs.debug = not gs.debug
        gs.status = "Debug overlay ON" if gs.debug else "Debug overlay OFF"
        return

    # Check if END TURN button was clicked
    if end_btn.collidepoint(pos):
        gs.engine.next_turn()
        gs.refresh()
        gs.selected = None
        gs.targets = []
        gs.status = "Turn passed"
        return

    sq = board_from_mouse(pos)

    if sq is None or gs.game_over:
        return
    else:
        x, y = sq

    if gs.mode == "promote":
        if gs.promoting_pos:
            # 폰/킹은 프로모션 불가
            if gs.drop_kind in ("P", "K"):
                gs.status = "Cannot promote to Pawn or King"
                return
            if gs.try_promote(gs.promoting_pos[0], gs.promoting_pos[1], gs.drop_kind):
                pass
            else:
                gs.status = "Cannot promote to this piece"
    elif gs.mode == "drop":
        if gs.pockets[gs.turn].get(gs.drop_kind, 0) <= 0:
            gs.status = "No piece in pocket"
            return
        if gs.try_drop(gs.drop_kind, x, y):
            gs.status = f"Dropped {gs.drop_kind} at {x},{y}"
        else:
            gs.status = "Cannot drop there"
    elif gs.mode == "stun":
        if gs.try_stun(x, y):
            gs.status = "Added stun and ended turn"
        else:
            gs.status = "Invalid stun target"
    elif gs.mode == "move":
        p = gs.piece_at(x, y)
        if gs.selected is None:
            if p and p["color"] == gs.turn and not p["stunned"]:
                moves = gs.legal_moves(x, y)
                if not moves:
                    gs.status = "No legal moves"
                    return
                gs.selected = (x, y)
                gs.targets = moves
                gs.status = "Select destination"
            else:
                gs.status = "Select your piece"
        else:
            if (x, y) == gs.selected:
                gs.selected = None
                gs.targets = []
                return
            if (x, y) in gs.targets:
                if gs.try_move(gs.selected, (x, y)):
                    gs.status = "Moved"
                    gs.selected = None
                    gs.targets = []
                else:
                    gs.status = "Move rejected"
            else:
                gs.status = "Not a legal target"
    elif gs.mode == "succession":
        # Succession mode: 선택할 기물(왕이 아닌) 클릭
        p = gs.piece_at(x, y)
        if p and p["color"] == gs.turn and not p["stunned"]:
            if p["type"] != "K":  # 왕이 아닌 기물만 선택 가능
                if gs.try_succession(x, y):
                    gs.status = "Succession successful"
                    gs.selected = None
                    gs.targets = []
                else:
                    gs.status = "Succession failed"
            else:
                gs.status = "Cannot use king as successor"
        else:
            gs.status = "Select your piece (not king)"
    elif gs.mode == "disguise":
        # Disguise mode: 왕(로얄 피스) 선택
        p = gs.piece_at(x, y)
        if p and p["color"] == gs.turn and p["type"] == "K":
            if gs.try_disguise(x, y, gs.drop_kind):
                gs.status = f"Disguised as {gs.drop_kind}"
                gs.selected = None
                gs.targets = []
            else:
                gs.status = "Cannot disguise"
        else:
            gs.status = "Select your king"


def main() -> None:
    pygame.init()
    gs = EngineState()
    screen = pygame.display.set_mode((BOARD_PX + layout_panel_width(gs), BOARD_PX))
    pygame.display.set_caption("변형체스 (C++ 엔진)")
    font = pygame.font.SysFont("arial", SQUARE_SIZE // 2)
    info_font = pygame.font.SysFont("arial", 18)

    clock = pygame.time.Clock()
    running = True

    while running:
        clock.tick(30)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_ESCAPE, pygame.K_q):
                    running = False
                elif event.key == pygame.K_r:
                    gs = EngineState()
                    gs.status = "Game reset"
                    gs.promoted_piece = None
                elif pygame.key.get_mods() & pygame.KMOD_LSHIFT and event.key in range(pygame.K_1, pygame.K_9 + 1):
                    # Shift + 숫자로 포지션 로드
                    pos_idx = event.key - pygame.K_1
                    positions = test_positions.list_positions()
                    if pos_idx < len(positions):
                        pos_name = positions[pos_idx]
                        gs.load_position(pos_name)
                elif event.key == pygame.K_m:
                    gs.mode = "move"
                    gs.status = "Move mode"
                elif event.key == pygame.K_d:
                    gs.mode = "drop"
                    gs.status = "Drop mode"
                elif event.key == pygame.K_s:
                    gs.mode = "stun"
                    gs.status = "Stun mode"
                elif event.key == pygame.K_u:
                    gs.mode = "succession"
                    gs.status = "Succession mode (선택할 기물 클릭)"
                elif event.key == pygame.K_v:
                    gs.mode = "disguise"
                    gs.status = "Disguise mode (변장할 킹 선택)"
                    gs.drop_kind = "Q"  # 기본 변장 기물
                # Cycle with Tab / Shift+Tab for scalable selection
                # 프로모션 모드일 때는 P/K 제외, disguise 모드일 때는 K 제외
                elif event.key == pygame.K_TAB and not pygame.key.get_mods() & pygame.KMOD_SHIFT:
                    try:
                        cur_idx = PIECE_ORDER.index(gs.drop_kind)
                    except ValueError:
                        cur_idx = 0
                    
                    if gs.mode == "promote":
                        # 프로모션용: P/K 제외된 리스트
                        promo_pieces = [p for p in PIECE_ORDER if p not in ("P", "K")]
                        try:
                            cur_idx = promo_pieces.index(gs.drop_kind)
                        except ValueError:
                            cur_idx = 0
                        gs.drop_kind = promo_pieces[(cur_idx + 1) % len(promo_pieces)]
                    elif gs.mode == "disguise":
                        # 변장용: K 제외된 리스트 (다른 기물로만 변장 가능)
                        disguise_pieces = [p for p in PIECE_ORDER if p != "K"]
                        try:
                            cur_idx = disguise_pieces.index(gs.drop_kind)
                        except ValueError:
                            cur_idx = 0
                        gs.drop_kind = disguise_pieces[(cur_idx + 1) % len(disguise_pieces)]
                    else:
                        gs.drop_kind = PIECE_ORDER[(cur_idx + 1) % len(PIECE_ORDER)]
                    gs.status = f"Selected {PIECE_NAMES.get(gs.drop_kind, gs.drop_kind)}"
                elif event.key == pygame.K_TAB and (pygame.key.get_mods() & pygame.KMOD_SHIFT):
                    try:
                        cur_idx = PIECE_ORDER.index(gs.drop_kind)
                    except ValueError:
                        cur_idx = 0
                    
                    if gs.mode == "promote":
                        # 프로모션용: P/K 제외된 리스트
                        promo_pieces = [p for p in PIECE_ORDER if p not in ("P", "K")]
                        try:
                            cur_idx = promo_pieces.index(gs.drop_kind)
                        except ValueError:
                            cur_idx = 0
                        gs.drop_kind = promo_pieces[(cur_idx - 1) % len(promo_pieces)]
                    elif gs.mode == "disguise":
                        # 변장용: K 제외된 리스트 (역방향)
                        disguise_pieces = [p for p in PIECE_ORDER if p != "K"]
                        try:
                            cur_idx = disguise_pieces.index(gs.drop_kind)
                        except ValueError:
                            cur_idx = 0
                        gs.drop_kind = disguise_pieces[(cur_idx - 1) % len(disguise_pieces)]
                    else:
                        gs.drop_kind = PIECE_ORDER[(cur_idx - 1) % len(PIECE_ORDER)]
                    gs.status = f"Selected {PIECE_NAMES.get(gs.drop_kind, gs.drop_kind)}"
                # Cycle selection for larger indexes
                elif event.key in (pygame.K_RIGHT, pygame.K_PERIOD, pygame.K_RIGHTBRACKET):
                    try:
                        cur_idx = PIECE_ORDER.index(gs.drop_kind)
                    except ValueError:
                        cur_idx = 0
                    gs.drop_kind = PIECE_ORDER[(cur_idx + 1) % len(PIECE_ORDER)]
                    gs.status = f"Selected {PIECE_NAMES.get(gs.drop_kind, gs.drop_kind)}"
                elif event.key in (pygame.K_LEFT, pygame.K_COMMA, pygame.K_LEFTBRACKET):
                    try:
                        cur_idx = PIECE_ORDER.index(gs.drop_kind)
                    except ValueError:
                        cur_idx = 0
                    gs.drop_kind = PIECE_ORDER[(cur_idx - 1) % len(PIECE_ORDER)]
                    gs.status = f"Selected {PIECE_NAMES.get(gs.drop_kind, gs.drop_kind)}"

            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                handle_click(gs, event.pos)

            elif event.type == pygame.MOUSEMOTION:
                sq = board_from_mouse(event.pos)
                gs.hovered = sq
            elif gs.game_over == True:
                gs.refresh()

        draw(gs, screen, font, info_font)

    pygame.quit()


if __name__ == "__main__":
    main()
