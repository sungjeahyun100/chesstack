#pragma once

enum class pieceType{
    NONE=-1,
    KING,
    QUEEN,
    BISHOP,
    KNIGHT,
    ROOK,
    PWAN,
    AMAZON
};

enum class colorType{
    NONE=-1,
    WHITE,
    BLACK
};

/*이 클래스는 각 기물이 가질 수 있는 행동반경의 생성규칙을 의미한다.
ray_infinite: 마치 레이저처럼 행동반경이 특정한 방향을 따라 보드에 끝이나 각 기물에 정해진 행동규칙에 의해 잡을 수 없는 기물이 나오기 전까진 끌없이 나아가여 생성된다. 
ray_finite: 위의 경우에 한계을 설정한 경우이다. 마찬가지로 경로에 보드의 끝이나 각 기물에 정해진 행동규칙에 의해 잡을 수 없는 기물이 나오면 행동반경은 더욱 줄어든다.
jump: 이 생성규칙 또한 방향에 따라 행동반경이 생성되며, ray_finite처럼 제한이 존재한다. 다른 점은 그 방향의 맨 끝점으로만 이동 가능하며, 경로 상의 기물이 생성에 영향을 주지 않는다.
*/
enum class moveType{
    NONE=-1,
    RAY_INFINITE,
    RAY_FINITE,
    JUMP
};


/*이 클래스는 각 기물이 행동반경 내에서 어떤 행동을 할 수 있는지를 정의한다.
catch:이 행마법은 행동반경에 존재하는 적 기물을 잡아낼 수 있으나 기물이 그 위치로 이동하지는 않는다.
take:이 행마법은 행동반경에 존재하는 적 기물이 있을 때"만" 잡아낸 다음 그 위치로 이동할 수 있다.
move:이 행마법은 행동반경 내에서 자유롭게 움직일 수 있으나, 적 기물이 행동반경에 존재할 경우 이동이 불가하다.
takemove:이 행마법은 가장 일반적인 행마법으로서, 행동반경 내에서 자유롭게 움직일 수 있고 적 기물이 행동반경 내에 존재할 경우 잡아내어 기 위치로 이동할 수 있다.
*/
enum class threatType{
    NONE=-1,
    CATCH,
    TAKE,
    MOVE,
    TAKEMOVE
};

