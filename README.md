# 변형체스 보드게임

python version: 3.12.3
cmake version: 3.28.3

이 프로젝트는 체스의 규칙에 바둑의 착수 개념을 더한 일종의 변형체스를 만드는 것이 목표이다. //더 자세한 규칙은 워크스페이스 파일 경로 최상단에 있는 rule.md파일을 참고하세요.

## 구현 상태

### ✅ 구현 완료된 규칙
1. ✅ **빈 보드 시작**: 8×8 빈 보드에서 게임 시작
2. ✅ **초기 포켓**: 폰 8개, 퀸/킹 각 1개, 룩/비숍/나이트 각 2개 (+ 확장 피스는 기본 0, 필요 시 포켓에 추가)
3. ⚠️ **킹 착수 강제**: UI에서 강제하지 않음 (수동으로 킹부터 착수 필요)
4. ✅ **폰 착수 제한**: 맨 끝 랭크에 착수 불가
5. ✅ **스턴 시스템**: 스턴 스택만큼 기물 이동 불가
6. ✅ **기본 스턴**: 착수된 기물은 스턴 스택 1
7. ✅ **프로모션**: 구현됨
8. ✅ **스턴 이전**: 잡힌 기물의 스턴이 잡은 기물에게 이전
9. ⏳ **자살수 방지**: 미구현 (킹 체크/체크메이트 로직 없음)
10. ✅ **재착수**: 잡은 기물을 포켓에서 꺼내 재착수 가능
11. ✅ **이동 스택**: 구현됨
12. ⏳ **이동 스택 충전**: 미구현
13. ✅ **스턴 추가 액션**: 턴을 넘기며 상대 기물(킹 제외) 스턴 +1
14. ✅ **프로모션 스택 이전**: 구현됨
15. ✅ **폰 랭크별 스턴**: 백 랭크1=0 ~ 랭크7=6, 흑 랭크8=0 ~ 랭크2=6
16. ✅ **한 턴 한 액션**: 드롭/이동/스턴 중 하나만 가능

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
- ✅ **턴별 수 카운트**: `whiteMoveCount`, `blackMoveCount`
- ✅ **액션 제한**: `performedActionThisTurn` 플래그로 중복 방지
- ✅ **커스텀 포켓 생성자**: 초기 포켓 구성 설정 가능

### Python 바인딩 (`chess_python/`)
- ✅ **pybind11 기반**: C++ 엔진과 Python 연결
- ✅ **보드 상태**: `board_state()`, `pocket()`, `turn_color()`
- ✅ **기물 액션**: `place_piece()`, `move_piece()`, `add_stun()`
- ✅ **합법 이동**: `legal_moves(file, rank)`
- ✅ **수 카운트**: `white_move_count()`, `black_move_count()`
- ✅ **커스텀 포켓**: `Board(white_pocket_dict, black_pocket_dict)` 생성자
- ✅ **특수 기물 지원**: A, G, Kr, W, D, L, F, C, Tr 모두 인식

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
- ✅ **3가지 모드**: Drop(D), Move(M), Stun(S)
- ✅ **마우스 조작**: 클릭으로 착수/이동/스턴
- ✅ **키보드 단축키**: Tab/Shift+Tab (드롭 기물 순환), Enter(턴 넘기기), R(리셋), D/M/S(모드)
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
cd chesstack
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
- **1-7**: 드롭할 기물 선택
  - 1=P (Pawn)
  - 2=N (Knight)
  - 3=B (Bishop)
  - 4=R (Rook)
  - 5=Q (Queen)
  - 6=K (King)
  - 7=A (Amazon)
- **Enter**: 턴 넘기기 (스턴 감소만 적용)
- **R**: 게임 리셋
- **Q/Esc**: 종료

### 마우스 조작
- **Drop 모드**: 클릭한 위치에 선택한 기물 착수
- **Move 모드**: 
  1. 자신의 기물 클릭 (스턴 상태가 아니어야 함)
  2. 목적지 클릭 (합법 이동만 가능)
- **Stun 모드**: 상대 기물 클릭 시 스턴 스택 +1, 턴 종료

### 화면 정보
- **좌측: 8×8 체스보드**
  - 노란색 칸: 마지막 이동
  - 녹색 칸: 선택된 기물
  - 연두색 칸: 이동 가능한 위치
  - 기물 우상단 숫자: 스턴 스택
  - 기물 우하단 `*`: 스턴 상태 (이동 불가)
  
- **우측: 정보 패널**
  - Turn: 현재 턴 (white/black)
  - White/Black moves: 각 플레이어가 둔 수
  - Mode: 현재 모드 (drop/move/stun)
  - Drop: 선택된 드롭 기물
  - Status: 마지막 액션 결과
  - Reserves: 백/흑 포켓 (나란히 표시)
  - Controls: 조작 안내

### 게임 플레이 팁
1. **첫 수는 킹 착수 권장** (규칙 3)
2. **폰 착수 시 랭크 고려**: 낮은 랭크일수록 스턴 적음
3. **스턴 관리**: 기물을 움직이기 전 스턴이 풀릴 때까지 대기
4. **포켓 활용**: 잡은 기물을 전략적 위치에 재착수
5. **스턴 추가 액션**: 상대 핵심 기물을 묶어두기

### 미구현 기능
- ❌ 킹 체크/체크메이트 (규칙 9)
- ❌ 이동 스택 시스템 (규칙 11, 12)
- ❌ 첫 수 킹 강제 (규칙 3)

### 향후 개발 계획
- [ ] 프로모션 구현 (폰 → 다른 기물)
- [ ] 킹 체크/체크메이트 로직
- [ ] 이동 스택 시스템
- [ ] AI 상대 (강화학습)
- [ ] 네트워크 멀티플레이어
- [ ] 게임 기록 저장/불러오기



