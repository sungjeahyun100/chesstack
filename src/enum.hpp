#pragma once

// 기물 인덱스 (일반 기물 + 특수 기물 통합)
enum class pieceType{
    NONE=-1,
    KING,
    QUEEN,
    BISHOP,
    KNIGHT,
    ROOK,
    PWAN,
    AMAZON,
    GRASSHOPPER,
    KNIGHTRIDER,
    ARCHBISHOP,
    DABBABA,
    ALFIL,
    FERZ,
    CENTAUR,
    TESTROOK,
    CAMEL
};

// 포켓 인덱스 (일반 기물 + 특수 기물 통합)
enum class pocketIndex{
    NONE = -1,
    KING = 0,
    QUEEN = 1,
    BISHOP = 2,
    KNIGHT = 3,
    ROOK = 4,
    PAWN = 5,
    AMAZON = 6,
    GRASSHOPPER = 7,
    KNIGHTRIDER = 8,
    ARCHBISHOP = 9,
    DABBABA = 10,
    ALFIL = 11,
    FERZ = 12,
    CENTAUR =13,
    TESTROOK =14,
    CAMEL = 15
    // 나중에 추가: CHANCELLOR = 7, ARCHBISHOP = 8 등
};

enum class colorType{
    NONE=-1,
    WHITE,
    BLACK
};

/*이 클래스는 각 기물이 가질 수 있는 행동반경의 생성규칙을 의미한다.
ray_infinite: 마치 레이저처럼 행동반경이 특정한 방향을 따라 보드에 끝이나 각 기물에 정해진 행동규칙에 의해 잡을 수 없는 기물이 나오기 전까진 끌없이 나아가여 생성된다. 
ray_finite: 위의 경우에 한계을 설정한 경우이다. 마찬가지로 경로에 보드의 끝이나 각 기물에 정해진 행동규칙에 의해 잡을 수 없는 기물이 나오면 행동반경은 더욱 줄어든다.
*/
enum class moveType{
    NONE=-1,
    RAY_INFINITE,
    RAY_FINITE
};


/*이 클래스는 각 기물이 행동반경 내에서 어떤 행동을 할 수 있는지를 정의한다.
catch:이 행마법은 행동반경에 존재하는 적 기물을 잡아낼 수 있으나 기물이 그 위치로 이동하지는 않는다.
take:이 행마법은 행동반경에 존재하는 적 기물이 있을 때"만" 잡아낸 다음 그 위치로 이동할 수 있다.
move:이 행마법은 행동반경 내에서 자유롭게 움직일 수 있으나, 적 기물이 행동반경에 존재할 경우 이동이 불가하다.
takemove:이 행마법은 가장 일반적인 행마법으로서, 행동반경 내에서 자유롭게 움직일 수 있고 적 기물이 행동반경 내에 존재할 경우 잡아내어 기 위치로 이동할 수 있다.
takejump:이 행마법은 행동반경에 적 기물이 존재할 때만 움직일 수 있으며, 그 기물을 잡아낸 다음 자신이 이동한 방향으로 1만큼 더 전진한 칸으로 이동한다.
movejump:이 행마법은 행동반경에 적 기물이 존재할 때만 움직일 수 있으며, 그 기물을 뛰어넘고 자신이 이동한 방향으로 1만큼 더 전진한 칸으로 이동한다. 도착 칸에 적 기물이 있으면 잡아낼 수 있다. ex)그래스호퍼
*/
enum class threatType{
    NONE=-1,
    CATCH,
    TAKE,
    MOVE,
    TAKEMOVE,
    TAKEJUMP,
    MOVEJUMP
};

