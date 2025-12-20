# 테스트 포지션 정의

# 포지션 형식:
# {
#   "turn": "white" | "black",  # 선택 차례 (생략 시 white)
#   "pieces": [{"type": "K", "color": "white", "file": 4, "rank": 0, "stun": 0, "move_stack": 2}, ...],
#   "pockets": {                  # 옵션: 포켓 재고 지정 (생략 시 기본 재고)
#       "white": {"K":1, "Q":1, "B":2, "N":2, "R":2, "P":8},
#       "black": {"K":1, "Q":1, "B":2, "N":2, "R":2, "P":8},
#   }
# }

POSITIONS = {
    # 이동 스택 테스트 (기본: 백선)
    "move_stack_test": {
        "turn": "white",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 0, "stun": 0, "move_stack": 3},
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 3},
            {"type": "Q", "color": "white", "file": 3, "rank": 0, "stun": 0, "move_stack": 5},
            {"type": "Q", "color": "black", "file": 3, "rank": 7, "stun": 0, "move_stack": 5},
        ],
    },
    
    # 스턴 테스트 (기본: 백선)
    "stun_test": {
        "turn": "white",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 0, "stun": 0, "move_stack": 1},
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 1},
            {"type": "R", "color": "white", "file": 0, "rank": 0, "stun": 3, "move_stack": 0},
            {"type": "R", "color": "black", "file": 0, "rank": 7, "stun": 3, "move_stack": 0},
            {"type": "N", "color": "white", "file": 1, "rank": 0, "stun": 1, "move_stack": 2},
            {"type": "N", "color": "black", "file": 1, "rank": 7, "stun": 1, "move_stack": 2},
        ],
    },
    
    # 프로모션 테스트 (기본: 백선)
    "promotion_test": {
        "turn": "black",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 0, "stun": 0, "move_stack": 1},
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 1},
            {"type": "P", "color": "white", "file": 0, "rank": 6, "stun": 0, "move_stack": 1},
            {"type": "P", "color": "black", "file": 7, "rank": 1, "stun": 0, "move_stack": 1},
        ],
        "pockets": {
            "white": {"K":1, "Q":1, "B":2, "N":2, "R":2, "P":0},
            "black": {"K":1, "Q":1, "B":2, "N":2, "R":2, "P":0},
        }
    },
    
    # 복잡한 상황 테스트 (기본: 백선)
    "complex_test": {
        "turn": "white",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 0, "stun": 0, "move_stack": 2},
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 2},
            {"type": "Q", "color": "white", "file": 3, "rank": 3, "stun": 1, "move_stack": 3},
            {"type": "R", "color": "white", "file": 0, "rank": 0, "stun": 0, "move_stack": 2},
            {"type": "B", "color": "white", "file": 5, "rank": 2, "stun": 2, "move_stack": 0},
            {"type": "N", "color": "white", "file": 6, "rank": 2, "stun": 0, "move_stack": 1},
            {"type": "Q", "color": "black", "file": 3, "rank": 4, "stun": 1, "move_stack": 3},
            {"type": "R", "color": "black", "file": 7, "rank": 7, "stun": 0, "move_stack": 2},
            {"type": "B", "color": "black", "file": 2, "rank": 5, "stun": 2, "move_stack": 0},
            {"type": "N", "color": "black", "file": 1, "rank": 5, "stun": 0, "move_stack": 1},
        ],
    },
    
    # 로얄 피스(킹) 체크 테스트 (기본: 백선)
    # 백 킹이 흑 룩에게 공격받는 상황 (체크 상태)
    "royal_check_test": {
        "turn": "white",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 3, "stun": 0, "move_stack": 1},
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 1},
            {"type": "R", "color": "black", "file": 4, "rank": 5, "stun": 0, "move_stack": 1},  # 백 킹 위에서 체크
            {"type": "Q", "color": "white", "file": 2, "rank": 2, "stun": 0, "move_stack": 2},
            {"type": "N", "color": "white", "file": 6, "rank": 2, "stun": 0, "move_stack": 2},
            {"type": "Q", "color": "black", "file": 3, "rank": 6, "stun": 0, "move_stack": 2},
            {"type": "B", "color": "black", "file": 5, "rank": 5, "stun": 0, "move_stack": 1},
        ],
        "pockets": {
            "white": {"K":1, "Q":0, "B":1, "N":1, "R":2, "P":8},
            "black": {"K":1, "Q":1, "B":2, "N":2, "R":1, "P":8},
        }
    },
    
    # 로얄 피스 변장 테스트 (기본: 백선)
    # 백 킹이 변장할 수 있는 상황 (다른 기물들이 주변에 있음)
    "royal_disguise_test": {
        "turn": "white",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 4, "stun": 0, "move_stack": 1},  # 로얄 피스
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 1},
            {"type": "Q", "color": "white", "file": 5, "rank": 4, "stun": 0, "move_stack": 2},
            {"type": "R", "color": "white", "file": 3, "rank": 4, "stun": 0, "move_stack": 1},
            {"type": "B", "color": "white", "file": 2, "rank": 3, "stun": 0, "move_stack": 1},
            {"type": "N", "color": "white", "file": 6, "rank": 3, "stun": 0, "move_stack": 1},
            {"type": "Q", "color": "black", "file": 4, "rank": 6, "stun": 0, "move_stack": 1},
            {"type": "R", "color": "black", "file": 6, "rank": 6, "stun": 0, "move_stack": 1},
        ],
        "pockets": {
            "white": {"K":1, "Q":0, "B":1, "N":1, "R":1, "P":8},
            "black": {"K":1, "Q":1, "B":2, "N":2, "R":1, "P":8},
        }
    },
    
    # 로얄 피스 승격 테스트 (기본: 백선)
    # 백 킹이 잡혀서 다른 기물(퀸)을 로얄 피스로 승격하는 상황
    "royal_succession_test": {
        "turn": "white",
        "pieces": [
            {"type": "K", "color": "white", "file": 4, "rank": 4, "stun": 1, "move_stack": 0},  # 체크 상태의 로얄 피스
            {"type": "K", "color": "black", "file": 4, "rank": 7, "stun": 0, "move_stack": 1},
            {"type": "Q", "color": "white", "file": 3, "rank": 3, "stun": 0, "move_stack": 2},  # 새 로얄 후보
            {"type": "R", "color": "white", "file": 5, "rank": 3, "stun": 0, "move_stack": 1},
            {"type": "N", "color": "white", "file": 6, "rank": 4, "stun": 0, "move_stack": 1},
            {"type": "R", "color": "black", "file": 4, "rank": 5, "stun": 0, "move_stack": 1},  # 흑 룩이 백 킹 공격
            {"type": "Q", "color": "black", "file": 3, "rank": 6, "stun": 0, "move_stack": 1},
        ],
        "pockets": {
            "white": {"K":1, "Q":0, "B":2, "N":2, "R":1, "P":8},
            "black": {"K":1, "Q":1, "B":2, "N":2, "R":1, "P":8},
        }
    },
}

def _copy_position(pos_dict):
    """딥카피: 내부 리스트/딕셔너리 수정 안전"""
    out = {
        "turn": pos_dict.get("turn", "white"),
        "pieces": [dict(p) for p in pos_dict.get("pieces", [])],
    }
    if "pockets" in pos_dict and pos_dict.get("pockets") is not None:
        pockets = pos_dict.get("pockets", {})
        out["pockets"] = {
            "white": dict(pockets.get("white", {})),
            "black": dict(pockets.get("black", {})),
        }
    return out

def get_position(name, turn_override=None):
    """포지션 이름과 선택 턴(옵션)으로 데이터 반환. name에 ":white" 또는 ":black" 접미사도 허용."""
    base_name, _, suffix = name.partition(":")
    pos = POSITIONS.get(base_name)
    if not pos:
        return {}
    data = _copy_position(pos)
    if turn_override:
        data["turn"] = turn_override
    elif suffix in ("white", "black"):
        data["turn"] = suffix
    return data

def list_positions():
    """사용 가능한 포지션 목록 반환 (기본 이름만). 필요하면 get_position(name, turn_override="black") 또는 name+":black"을 직접 사용."""
    return list(POSITIONS.keys())
