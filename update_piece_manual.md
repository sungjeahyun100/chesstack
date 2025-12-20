# 새 기물 추가 시 업데이트 체크리스트

아래 순서를 따라 새 기물을 추가하세요.

1) 엔진 타입 정의
- `src/enum.hpp`
  - `pieceType`에 새 항목 추가
  - `pocketIndex`에 대응 인덱스 추가 (마지막에 붙이고 주석 포함)
  - 포켓 크기 변경 시 `POCKET_SIZE`와 `DEFAULT_POCKET_STOCK` 갱신 (기본 보유량 설정)

2) 포켓 매핑
- `src/gameboard.cpp`
  - `pieceTypeToPocketIndex`에 새 타입 → 포켓 인덱스 매핑 추가

3) 이동 패턴
- `src/gameboard.hpp`
  - `setupPiecePatterns`에 새 기물의 이동/위협 패턴 추가
  - 필요하면 방향 상수(`*_DIRECTIONS`)를 새로 정의하거나 기존 상수 재사용
  - 위협 타입(`TAKEJUMP`, `MOVEJUMP`, `TAKEMOVE` 등) 선택 시 의도한 규칙과 일치하는지 확인

4) 이동/캡처 로직 주의점 (점프형일 때)
- `src/move.cpp`
  - `TAKEJUMP`: 중간 적 기물 필수, 중간+착지 적 모두 캡처
  - `MOVEJUMP`: 중간은 아군/적 무관히 뛰어넘고, 착지 적만 캡처
  - 특수 점프 규칙을 바꾸면 PGN 추가 필드나 처리 로직이 필요한지 검토

5) Python 바인딩
- `chess_python/chess_python.cpp`
  - `piece_type_from_str`에 문자열 코드 추가
  - `piece_to_str`에 코드 문자열 추가 (UI/포켓 표시용)
  - 포켓 배열 크기 변경 시 `pocket_to_dict` / `dict_to_pocket` 배열 길이와 키 목록 모두 갱신 (새 코드 키 포함)

6) UI(Pygame)
- `play.py`
  - `PIECE_ORDER`에 새 코드 추가 (Tab/Shift+Tab 순환용)
  - `PIECE_NAMES`, `PIECE_GLYPH`에 이름/표시 문자 추가 (ASCII 유지)
  - 필요 시 초기 포켓 `white_pocket` / `black_pocket`에 기본 수량 추가

7) 테스트/플레이그라운드
- `test/test_play.cpp`, `playground/chess_mutated.cpp`
  - 포켓 출력 배열 크기와 라벨에 새 항목 추가 (POCKET_SIZE 변경 시 필수)
  - 새 기물 드롭/이동 시나리오가 필요하면 간단한 케이스 추가

8) 문서
- `README.md`
  - 페어리 피스 목록과 이동 규칙, 포켓 크기 설명에 새 기물 반영

9) 빌드/검증
- `build/`에서 `make -j4`로 재빌드 (파이썬 바인딩 포함 시 `-DBUILD_PYTHON_BINDINGS=ON`으로 CMake 구성되어 있어야 함)
- 필요 시 `python3 play.py`로 UI 확인

참고: 포켓 크기를 늘릴 때는 포켓 배열을 사용하거나 출력하는 모든 코드가 새 길이를 맞추도록 반드시 함께 수정하세요.
