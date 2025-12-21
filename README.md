# 변형체스 보드게임

python version: 3.12.3
cmake version: 3.28.3

이 프로젝트는 체스의 규칙에 바둑의 착수 개념을 더한 일종의 변형체스를 만드는 것이 목표이다. //더 자세한 규칙은 워크스페이스 파일 경로 최상단에 있는 rule.md파일을 참고하라.

## 구현 상태

### ✅ 구현 완료된 규칙
1. ✅ **빈 보드 시작**: 8×8 빈 보드에서 게임 시작
2. ✅ **초기 포켓**: 폰 8개, 퀸/킹 각 1개, 룩/비숍/나이트 각 2개 (+ 확장 피스는 기본 0, 필요 시 포켓에 추가)
3. ⚠️ **킹 착수 강제**: UI에서 강제하지 않음 (수동으로 킹부터 착수 필요)
4. ✅ **폰 착수 제한**: 맨 끝 랭크에 착수 불가
5. ✅ **스턴 시스템**: 스턴 스택만큼 기물 이동 불가
6. ✅ **기물 점수 기반 스턴**: 착수 시 기물 점수만큼 스턴 부여 (킹=4, 퀸=9, 룩=5, 비숍/나이트=3, 아마존=13 등)
7. ✅ **프로모션**: 구현됨
8. ✅ **스택 전가**: 잡힌 기물의 스턴 스택과 이동 스택 모두 잡은 기물에게 이전
9. ⏳ **자살수 방지**: 미구현 (킹 체크/체크메이트 로직 없음) 주석:이 규칙은 현재 삭제 검토중에 있다.
10. ✅ **재착수**: 잡은 기물을 포켓에서 꺼내 재착수 가능
11. ✅ **이동 스택**: 구현됨
12. ✅ **이동 스택 충전**: 구현됨
13. ✅ **스턴 추가 액션**: 턴을 넘기며 상대 기물(킹 제외) 스턴 +1
14. ✅ **프로모션 스택 이전**: 구현됨
15. ✅ **폰 랭크별 스턴**: 백 랭크1=8 ~ 랭크7=2, 흑 랭크8=8 ~ 랭크2=2 (최소 2)
16. ✅ **한 턴 한 액션**: 드롭/이동/스턴 중 하나만 가능
17. ✅ **로얄 피스 계승**: 로얄 피스 없는 상태에서 일반 기물을 로얄로 승격 (U 키)
18. ✅ **로얄 피스 위장**: 로얄 피스가 다른 기물로 변장해 행마법 변경 (V 키)
19. ✅ **로얄 캡처 페널티**: 로얄 피스 캡처 시 같은 색 모든 기물에 스턴 +3
20. ✅ **로얄 소멸 패배**: 첫 턴 이후 한쪽 로얄이 전무하면 자동 패배 판정 

### 추가적으로 구현해야 하는 것
1. ⏳ **다중 이동 및 각종 추가된 특수 행마를 표기하는 PGN 시스템**: 미구현
2. ⏳ **강화학습으로 작동하는 자동 ai 봇**: 미구현. feature/auto_ai 브렌치에 따로 구현 예정


### 핵심 아키텍처
- **턴 관리**: `whiteMoveCount`와 `blackMoveCount`로 각 플레이어 수 추적
- **턴 결정**: `whiteMoveCount == blackMoveCount`이면 백 턴, 아니면 흑 턴
- **스턴 감소**: 각 플레이어가 수를 두기 **전에** 해당 플레이어의 모든 기물 스턴 -1
- **포켓 시스템**: `pocketIndex` 기반 15칸 통합 배열 (KING, QUEEN, BISHOP, KNIGHT, ROOK, PAWN, AMAZON, GRASSHOPPER, KNIGHTRIDER, ARCHBISHOP, DABBABA, ALFIL, FERZ, CENTAUR, TESTROOK)
- **특수/변형 기물**
  - Amazon(`A`): 나이트 + 퀸 레이
  - Grasshopper(`G`): MOVEJUMP (아무 기물이나 뛰어넘고 한 칸 뒤 착지, 착지 칸 적이면 캡처)
  - KnightRider(`Kr`): 나이트 방향 무한 레이
  - Archbishop(`W`): 나이트 + 비숍
  - Dabbaba(`D`): 직선 2칸 점프 (유한 레이)
  - Alfil(`L`): 대각 2칸 점프 (유한 레이)
  - Ferz(`F`): 왕의 대각 한 칸 (유한 레이)
  - Centaur(`C`): 킹 한 칸 + 나이트 유한 레이
  - TestRook(`Tr`): 룩 레이 TAKEMOVE (테스트용)
- **위협 타입 보강**:
  - `TAKEJUMP`: 적 기물이 행동반경에 있을 때에만 이동 가능하며, 적 기물을 뛰어넘으며 캡처하고, 한 칸 뒤로 착지하며 도착 적 기물도 캡처 (중간+착지 두 개 잡힘)
  - `MOVEJUMP`: 기물에 행동반경 내에 존재해야만 이동 가능하며, 아무 기물이나 뛰어넘고 한 칸 뒤로 이동; 도착지가 적이면 캡처, 이군이면 이동 불가, 중간 기물은 미캡처

### C++ 엔진 (`src/`)
- ✅ **보드 관리** (`bc_board`): 8×8 보드, 기물 배치, 이동, 제거
- ✅ **기물 관리** (`piece`): 스턴 스택, 색상, 타입, 위치
- ✅ **합법 이동 계산** (`legalMoveChunk`): RAY_INFINITE, RAY_FINITE, TAKEJUMP, MOVEJUMP
- ✅ **포켓 시스템**: 크기 15 통합 배열 (일반 6종 + 페어리 9종)
- ✅ **스턴 시스템**: 
  - `applyStunTickForColor()`: 특정 색상 기물 스턴 감소
  - 스턴 상태 확인 (`isStunned()`)
  - 스턴 추가 (`addStun()`)
  - `pieceScore()`: 기물별 점수 테이블 (착수 스턴 부여)
- ✅ **로얄 시스템**:
  - `hasRoyalPiece()`: 특정 색상 로얄 존재 확인
  - `succeedRoyalPiece()`: 일반 기물을 로얄로 승격
  - `disguisePiece()`: 로얄 피스 변장 (타입 변경 + 행마법 교체)
  - 로얄 캡처 시 같은 색 전체 기물 스턴 +3
- ✅ **턴별 수 카운트**: `whiteMoveCount`, `blackMoveCount`
- ✅ **액션 제한**: `performedActionThisTurn` 플래그로 중복 방지
- ✅ **커스텀 포켓 생성자**: 초기 포켓 구성 설정 가능

### Python 바인딩 (`chess_python/`)
- ✅ **pybind11 기반**: C++ 엔진과 Python 연결
- ✅ **보드 상태**: `board_state()`, `pocket()`, `turn_color()`
- ✅ **기물 액션**: `place_piece()`, `move_piece()`, `add_stun()`, `promote()`, `succeed_royal_piece()`, `disguise_piece()`
- ✅ **합법 이동**: `legal_moves(file, rank)`
- ✅ **수 카운트**: `white_move_count()`, `black_move_count()`
- ✅ **커스텀 포켓**: `Board(white_pocket_dict, black_pocket_dict)` 생성자
- ✅ **포지션 설정**: `setup_position()` - 임의 보드 상태 로드
- ✅ **특수 기물 지원**: A, G, Kr, W, D, L, F, C, Tr, Cl 모두 인식

### Pygame UI (`play.py`)
- ✅ **보드 렌더링**: 8×8 체스보드, 밝은/어두운 칸
- ✅ **기물 표시**: 백=대문자, 흑=소문자
- ✅ **스턴 시각화**:
  - 우상단 숫자: 스턴 스택 (0이면 숨김)
  - 우하단 `*`: 스턴 상태 (이동 불가)
- ✅ **정보 패널**:
  - 현재 턴, 백/흑 수 카운트
  - 모드 표시, 드롭 기물 선택
  - 포켓 나란히 표시 (White | Black)
- ✅ **3가지 모드**: Drop(D), Move(M), Stun(S), Succession(U), Disguise(V)
- ✅ **마우스 조작**: 클릭으로 착수/이동/스턴/계승/변장
- ✅ **키보드 단축키**: Tab/Shift+Tab (드롭 기물 순환), END TURN 버튼(턴 넘기기), R(리셋), D/M/S/U/V(모드)
- ✅ **디버그 패널**: DEBUG OVERLAY 버튼으로 토글, 호버 정보/로얄 목록/전체 기물 상태 표시
- ✅ **게임 종료 판정**: 로얄 소멸 시 자동 승패 표시
- ✅ **커스텀 포켓**: 표준 피스 + 페어리 피스 모두 표시/드롭 가능 (포켓에 있으면 표시)

### 파일 구조
```
project_bc/
├── src/                    # C++ 엔진
│   ├── chess.hpp          # 기물 패턴 설정 (아마존: 나이트+퀸)
│   ├── enum.hpp           # pieceType, colorType, pocketIndex
│   ├── gameboard.hpp/cpp  # 보드 관리, 포켓 시스템
│   ├── piece.hpp/cpp      # 기물 클래스, 스턴 관리
│   ├── moves.hpp          # 이동 패턴 정의
│   └── move.cpp           # 합법 이동 계산
├── chess_python/          # Python 바인딩
│   └── chess_python.cpp   # pybind11 래퍼
├── play.py                # Pygame UI
├── test/                  # C++ 테스트
├── playground/            # 터미널 플레이
└── build/                 # 빌드 출력
```

## 사용법 (Pygame UI)

### 빌드 및 실행
```bash
sudo apt update && sudo apt-get install python3.12-dev
cd chesstack
python3 -m venv .venv && source .venv/bin/activate
pip3 install -r requirements.txt
mkdir -p build && cd build
cmake .. -DBUILD_PYTHON_BINDINGS=ON
make -j4
cd ..
python3 play.py
```

### Windows (Visual Studio + pip pybind11)
- PowerShell에서:
```powershell
cd chesstack\scripts
./build_windows.ps1 -Build
```
- 또는 CMD에서:
```bat
cd chesstack\scripts
build_windows.bat
```
위 스크립트는 현재 Python 환경에서 pybind11의 CMake 경로를 자동으로 탐색해 `-Dpybind11_DIR=...`로 전달합니다. 반드시 동일한 Python/가상환경에서 `pip install pybind11`이 되어 있어야 합니다.

#### 전제 조건

 - 동일한 Python/가상환경에서 pip install pybind11이 설치되어 있어야 합니다.
 - Visual Studio C++ 툴체인이 설치되어 있어야 합니다. 기본 Generator는 “Visual Studio 17 2022”, 아키텍처는 x64로 설정되어 있습니다.

### 키보드 컨트롤
- **D**: Drop 모드 (기물 착수)
- **M**: Move 모드 (기물 이동)
- **S**: Stun 모드 (상대 기물 스턴 추가)
- **U**: Succession 모드 (로얄 피스 계승)
- **V**: Disguise 모드 (로얄 피스 위장)
- **Tab/Shift+Tab**: 드롭/변장 기물 순환 선택
- **←/→ [,]**: 기물 선택 좌우 이동
- **END TURN 버튼**: 턴 넘기기 (스턴 감소만 적용)
- **DEBUG OVERLAY 버튼**: 디버그 패널 토글
- **R**: 게임 리셋
- **Shift+1~9**: 테스트 포지션 로드
- **Q/Esc**: 종료

### 테스트 포지션
`test_positions.py`에 정의된 포지션을 불러올 수 있습니다:
- **Shift+1**: move_stack_test - 이동 스택 테스트
- **Shift+2**: stun_test - 스턴 테스트
- **Shift+3**: promotion_test - 프로모션 테스트
- **Shift+4**: complex_test - 복잡한 상황 테스트

새로운 포지션을 추가하려면 `test_positions.py`의 `POSITIONS` 딕셔너리에 추가하세요.

### 마우스 조작
- **Drop 모드**: 클릭한 위치에 선택한 기물 착수
- **Move 모드**: 
  1. 자신의 기물 클릭 (스턴 상태가 아니어야 함)
  2. 목적지 클릭 (합법 이동만 가능)
- **Stun 모드**: 상대 기물 클릭 시 스턴 스택 +1, 턴 종료
- **Succession 모드**: 자신의 일반 기물 클릭 시 로얄 피스로 승격, 턴 종료
- **Disguise 모드**: 자신의 로얄 피스 클릭 시 선택한 타입으로 변장, 턴 종료

### 화면 정보
- **좌측: 8×8 체스보드**
  - 노란색 칸: 마지막 이동
  - 녹색 칸: 선택된 기물
  - 연두색 칸: 이동 가능한 위치
  - 기물 좌상단 파란 숫자: 이동 스택
  - 기물 우상단 빨간 숫자: 스턴 스택
  - 기물 우하단 `*`: 스턴 상태 (이동 불가)
  
- **우측: 정보 패널**
  - Turn: 현재 턴 (white/black)
  - White/Black moves: 각 플레이어가 둔 수
  - Mode: 현재 모드 (drop/move/stun/succession/disguise)
  - Drop: 선택된 드롭 기물
  - Status: 마지막 액션 결과 또는 게임 종료 메시지
  - Debug: 디버그 오버레이 상태 (ON/OFF)
  - Reserves: 백/흑 포켓 (나란히 표시)
  - Controls: 조작 안내

- **우측 확장: 디버그 패널** (DEBUG OVERLAY ON 시)
  - Hover: 현재 마우스 호버 칸의 기물 상세 정보
  - Selected: 선택된 기물의 정보 및 이동 가능 위치
  - Royals: 현재 보드의 로얄 피스 목록 (색/타입/위치/변장)
  - Pieces: 전체 기물 목록 (색/타입/위치/스턴/이동스택/로얄 여부)

### 게임 플레이 팁
1. **첫 수는 킹 착수 권장** (규칙 3)
2. **폰 착수 시 랭크 고려**: 높은 랭크일수록 스턴 적음 (백: 랭크1=8스턴, 랭크7=2스턴)
3. **스턴 관리**: 기물을 움직이기 전 스턴이 풀릴 때까지 대기
4. **포켓 활용**: 잡은 기물을 전략적 위치에 재착수
5. **스턴 추가 액션**: 상대 핵심 기물을 묶어두기
6. **로얄 계승**: 로얄 소멸 후 즉시 다른 기물을 로얄로 승격해 패배 방지
7. **로얄 위장**: 로얄을 강한 기물로 변장시켜 공격력 확보 (퀸/아마존 추천)
8. **스택 전가 활용**: 이동 스택 많은 기물로 캡처해 연속 이동 확보
9. **디버그 패널**: 로얄 위치 확인 및 전체 기물 상태 모니터링

### 테스트 포지션
추가된 테스트 포지션:
- **Shift+5**: royal_check_test - 로얄 피스 체크 상황
- **Shift+6**: royal_disguise_test - 로얄 피스 위장 테스트
- **Shift+7**: royal_succession_test - 로얄 피스 계승 테스트

### 미구현 기능
- ❌ 킹 체크/체크메이트 판정 (체크 상태 표시 없음, 계승은 수동)
- ❌ 완전한 PGN 기록 시스템 (다중 이동/계승/변장 표기)
- ❌ AI 봇 (feature/auto_ai 브랜치 예정)
- ❌ 첫 수 킹 강제 (규칙 3)

### 향후 개발 계획
- [ ] 킹 체크/체크메이트 로직
- [ ] 이동 스택 시스템
- [ ] AI 상대 (강화학습)
- [ ] 네트워크 멀티플레이어
- [ ] 게임 기록 저장/불러오기



