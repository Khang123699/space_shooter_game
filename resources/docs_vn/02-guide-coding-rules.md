<h1 align="center">Quy tắc lập trình và Hướng dẫn phong cách</h1>

Tài liệu này định nghĩa các quy ước đặt tên, phong cách mã nguồn, định dạng thông báo commit và quy ước đặt tên tệp tài liệu được sử dụng trong dự án. Mục tiêu là đảm bảo mã nguồn do các thành viên đóng góp luôn nhất quán về hình thức và dễ theo dõi thông qua các công cụ tìm kiếm, quá trình đánh giá mã nguồn (code review) và kiểm soát phiên bản (version control).

---

## Mục lục

- [I. Quy ước đặt tên](#i-quy-uoc-dat-ten)
  - [1. Thư mục](#1-thu-muc)
  - [2. Tệp nguồn và tệp tiêu đề (header)](#2-tep-nguon-va-tep-tieu-de-header)
  - [3. Header guard](#3-header-guard)
  - [4. Macro và hằng số biên dịch](#4-macro-va-hang-so-bien-dich)
  - [5. Tín hiệu (enum values)](#5-tin-hieu-enum-values)
  - [6. ID Tác vụ (Task ID)](#6-id-tac-vu-task-id)
  - [7. Kiểu dữ liệu và typedef](#7-kieu-du-lieu-va-typedef)
  - [8. Hàm](#8-ham)
  - [9. Biến](#9-bien)
- [II. Quy ước thông báo commit](#ii-quy-uoc-thong-bao-commit)
- [III. Quy ước đặt tên tệp tài liệu](#iii-quy-uoc-dat-ten-tep-tai-lieu)
- [IV. Tài liệu tham khảo](#iv-tai-lieu-tham-khao)

---

## I. Quy ước đặt tên

Các quy ước dưới đây được rút ra trực tiếp từ mã nguồn hiện tại. Hãy tuân thủ các quy ước này để phát triển mã nguồn sao cho công cụ, quá trình tìm kiếm và những người đánh giá (reviewers) đều hoạt động nhất quán.

**Các kiểu chữ (case styles) được sử dụng trong tài liệu này:**

| Kiểu chữ | Mô tả | Ví dụ trong dự án | Phạm vi áp dụng |
|---|---|---|---|
| `lower_snake_case` | Chữ thường, các từ phân tách bằng dấu gạch dưới `_` | `game_shooter_player_task`, `g_score` | Biến, hàm, typedef, tên tệp nguồn, tên thư mục |
| `UPPER_SNAKE_CASE` | Chữ in hoa, các từ phân tách bằng dấu gạch dưới `_` | `MAX_ENEMIES`, `AC_GAME_BTN_UP`, `AC_TASK_DISPLAY_ID` | Hằng số `#define`, enum tín hiệu, ID tác vụ, macro |
| `kebab-case` | Chữ thường, các từ phân tách bằng dấu gạch ngang `-` | `02-guide-coding-rules.md` | Tên tệp tài liệu trong thư mục `docs/` |

### 1. Thư mục

Sử dụng `lower_snake_case` cho tên thư mục. Tổ chức theo tính năng (feature-based), không theo loại tệp.

```
application/sources/app/
  space_shooter/          # thư mục chứa toàn bộ mã nguồn của các đối tượng game Space Shooter
  screens/                # thư mục chứa toàn bộ mã nguồn của các màn hình giao diện
  ...
```

### 2. Tệp nguồn và tệp tiêu đề (header)

Tệp nguồn và header luôn phải mang một tiền tố (prefix) module để có thể nhận diện module ngay từ tên tệp:

| Tiền tố | Ý nghĩa | Ví dụ |
|---|---|---|
| `scr_*` | Bộ xử lý của một màn hình UI | `scr_game_play.cpp`, `screens.h` |
| `game_shooter_*` | Đối tượng thuộc về game Space Shooter | `game_shooter_player_task.cpp`, `game_shooter_enemy_task.h` |

Mỗi game định nghĩa tiền tố ngắn gọn của riêng nó (ví dụ `game_shooter_*` cho Space Shooter) và áp dụng nhất quán cho mọi tệp trong thư mục của game đó.

Đuôi tệp: `.h` cho header, `.cpp` cho triển khai (dự án được biên dịch dưới dạng C++).

### 3. Header guard

Sử dụng mẫu `__<FILE_NAME>_H__`, viết hoa toàn bộ, khớp chính xác với tên tệp:

```cpp
#ifndef __GAME_SHOOTER_ENEMY_TASK_H__
#define __GAME_SHOOTER_ENEMY_TASK_H__
...
#endif //__GAME_SHOOTER_ENEMY_TASK_H__
```

### 4. Macro và hằng số biên dịch

Sử dụng `UPPER_SNAKE_CASE` cho tên. Luôn bọc các giá trị số học trong dấu ngoặc đơn để tránh lỗi khi macro được mở rộng.

**Quy tắc bắt buộc: một macro thuộc về một đối tượng BẮT BUỘC phải mang tên của đối tượng đó làm tiền tố.**

Cấu trúc: `<OBJECT>_<PROPERTY>` hoặc `<OBJECT>_<ACTION>` — tên đối tượng luôn đứng trước. Đọc tên macro sẽ cho biết ngay lập tức nó thuộc về module nào, và tìm kiếm (grep) theo tên đối tượng sẽ trả về mọi hằng số của module đó.

| Loại hằng số | Dạng chuẩn xác |
|---|---|
| Số lượng / Đếm | `MAX_ENEMIES`, `MAX_BULLETS` |
| Tọa độ | `PLAYER_START_X`, `PLAYER_START_Y` |
| Bước di chuyển | `PLAYER_MOVE_STEP`, `BULLET_SPEED` |
| Giới hạn / Điều kiện | `SCREEN_WIDTH`, `SCREEN_HEIGHT` |

Ví dụ chuẩn xác:

```cpp
// game_shooter_bullet_task.h
#define MAX_BULLETS            (20)

// game_shooter_enemy_task.h
#define MAX_ENEMIES            (35)
#define MAX_POWERUPS           (3)
```

**Ngoại lệ — các macro cấp hệ thống / toàn dự án:** macro thuộc về toàn bộ game (không gắn với đối tượng cụ thể) sử dụng tiền tố toàn cục của game, ví dụ `GAME_STATE_*`:

```cpp
#define GAME_STATE_TITLE (0)
#define GAME_STATE_PLAY  (1)
```

Gom nhóm các hằng số liên quan vào đúng file header của module (`game_shooter_enemy_task.h` giữ hằng số kẻ địch, v.v.). Tuyệt đối không để lại các "magic numbers" (số không rõ ý nghĩa) nằm rải rác trong các tệp `.cpp`.

### 5. Tín hiệu (enum values)

Các tín hiệu (Signals) là **hợp đồng giao tiếp công khai** (public contract) giữa các tác vụ. Luôn sử dụng tiền tố đầy đủ - không viết tắt, kể cả trong các chú thích, tài liệu, hoặc biểu đồ tuần tự.

| Cấu trúc (Pattern) | Phạm vi áp dụng | Ví dụ |
|---|---|---|
| `<PREFIX>_<TASK>_<ACTION>` | Tín hiệu tác vụ | `AC_DISPLAY_GAME_UI_ANIM_TICK` |

Khai báo bộ tín hiệu của mỗi tác vụ trong `app.h` dưới dạng một khối enum riêng biệt, được neo (anchored) vào `AK_USER_DEFINE_SIG` (hoặc `AK_SYS_DEFINE_SIG` nếu là tín hiệu hệ thống):

```cpp
/*****************************************************************************/
/*  DISPLAY task define
 */
/*****************************************************************************/
enum {
    AC_DISPLAY_RENDER_SCREEN = AK_SYS_DEFINE_SIG,
    AC_DISPLAY_INITIAL = AK_USER_DEFINE_SIG,
    AC_DISPLAY_BUTTON_MODE_PRESSED,
    AC_DISPLAY_BUTTON_UP_PRESSED,
    AC_DISPLAY_BUTTON_DOWN_PRESSED,
};
```

### 6. ID Tác vụ (Task ID)

Sử dụng cấu trúc `<PREFIX>_<NAME>_ID`, viết hoa toàn bộ, được đăng ký trong `task_list.h`:

```cpp
AC_TASK_DISPLAY_ID
```

Bộ xử lý tương ứng trong `task_list.cpp` thường sử dụng tiền tố `task_` hoặc hậu tố `_task` và viết thường toàn bộ:

```cpp
{AC_TASK_DISPLAY_ID, TASK_PRI_LEVEL_4, task_display},
```

### 7. Kiểu dữ liệu và typedef

Sử dụng `lower_snake_case` với hậu tố `_t`. Bản thân struct nên là vô danh (anonymous); tên public là tên typedef:

```cpp
typedef struct {
    int8_t x, y;
    int8_t hp;
    uint8_t type;
    int8_t blink_timer;
    enemy_state_e state;
    int16_t timer;
} enemy_t;
```

Các kiểu do framework cung cấp tuân theo cùng một mẫu (`ak_msg_t`, `view_screen_t`).

### 8. Hàm

Sử dụng `lower_snake_case` với tên module làm tiền tố, để khi grep theo tiền tố sẽ trả về toàn bộ các hàm điểm vào (entry point) của module đó:

```cpp
void game_logic_init();
void game_player_shoot();
void game_player_move(int8_t dir);
void game_enemy_update();
```

### 9. Biến

Sử dụng `lower_snake_case`. Không bắt đầu tên bằng dấu gạch dưới.

- **Biến toàn cục dùng chung giữa các module (Đóng gói - Encapsulation):** Che giấu biến bằng cách khai báo `static` trong tệp `.cpp`, và cung cấp các hàm `getter` trong header để các module khác đọc dữ liệu một cách an toàn. **Tuyệt đối không sử dụng biến `extern`.**

  ```cpp
  // space_shooter/game_shooter_player_task.h
  int16_t game_get_player_x();
  uint8_t game_get_lives();
  uint32_t game_get_score();
  ```

- **Biến nội bộ của module:** khai báo `static` trong tệp `.cpp`.

  ```cpp
  // game_shooter_player_task.cpp
  static uint8_t g_shoot_cooldown = 0;
  ```

- **Biến cục bộ:** ngắn gọn, mô tả chính xác vai trò. Các bộ đếm vòng lặp có thể dùng `i`, `j`, `k` khi phạm vi đã rõ ràng.

---

## II. Quy ước thông báo commit

Mỗi commit phải tuân theo định dạng `[ACTION] mô tả ngắn` để lịch sử git dễ đọc và dễ lọc.

### 1. Quy trình thực hiện

```bash
git add .                                     # đưa mọi thay đổi vào stage
git commit -m "[ACTION] mô tả ngắn"           # bắt buộc phải có tag, phần mô tả phải ngắn gọn
git push                                      # đẩy lên remote
```

Khi bạn chỉ cần stage một vài tệp cụ thể, hãy thay thế `git add .` bằng `git add <path>` để tránh commit nhầm các tệp rác.

### 2. Các thẻ hành động (Action tags)

| Thẻ (Tag) | Khi nào nên dùng |
|---|---|
| `[ADD]` | Thêm tệp, tính năng, tài sản (asset), hoặc tài liệu mới |
| `[UPDATE]` | Cập nhật mã nguồn hiện có — tái cấu trúc (refactor), đổi tên, tinh chỉnh logic, tăng phiên bản |
| `[FIX]` | Sửa một lỗi (bug) hiện có, lỗi biên dịch, hoặc lỗi định dạng |
| `[REMOVE]` | Xóa tệp, tính năng, hoặc mã nguồn không còn dùng |
| `[DOC]` | Các thay đổi chỉ liên quan đến tài liệu (`docs/`, `README.md`, các khối chú thích lớn) 

### 3. Phong cách viết mô tả

- Thẻ (Tag) phải được viết hoa toàn bộ, bọc trong `[]`, theo sau là đúng một dấu cách, rồi đến phần mô tả.
- Phần mô tả viết thường, dùng thể mệnh lệnh (`add`, `fix`, `rename`, `move`...), không dùng dấu chấm câu ở cuối.
- Giữ độ dài khoảng 70 ký tự — nếu dài hơn, hãy thu gọn nó hoặc chuyển phần chi tiết vào thân (body) của commit.
- Khi thay đổi liên quan đến một module / tín hiệu / tệp cụ thể, hãy nhắc trực tiếp tên của nó để lịch sử git dễ dàng được tìm kiếm (grep).

### 4. Ví dụ tốt

```text
[ADD] player movement mechanic
[ADD] sequence diagram for enemy logic
[UPDATE] rename variables to match standard
[UPDATE] split logic processing into multiple functions
[UPDATE] increase max enemy speed
[FIX] overlap when drawing player
[REMOVE] unused redundant state
[DOC] coding rules
```

### 5. Ví dụ cần tránh

```text
update                          # thiếu tag
[update] fix something          # tag phải viết hoa toàn bộ
[ADD] Added new file.           # không dùng thì quá khứ và không có dấu chấm cuối câu
[FIX] fix bug                   # quá mơ hồ, sửa lỗi gì?
[ADD] game_shooter_enemy.cpp + scr_game_ui_play.cpp + ... # quá dài, hãy nhóm lại theo chủ đề
```

---

## III. Quy ước đặt tên tệp tài liệu

Các tệp trong `docs/` tuân theo định dạng `<NN>-<category>-<topic>.md`:

| Thành phần | Quy ước | Ví dụ |
|---|---|---|
| `NN` | Một dãy số gồm 2 chữ số, bắt đầu từ `01`. Phản ánh thứ tự đọc — hướng dẫn trước, thiết kế sau. | `01`, `02`, `03` |
| `category` | Hạng mục tài liệu. Chỉ dùng các giá trị định nghĩa trước; không thêm hạng mục mới. | `guide`, `design` |
| `topic` | Chủ đề chính, viết theo kiểu `kebab-case` (chữ thường, cách nhau bởi `-`). | `getting-started`, `coding-rules`, `sequence-object` |

Các hạng mục hiện đang được sử dụng:

| Hạng mục | Mục đích | Nội dung điển hình |
|---|---|---|
| `guide` | Hướng dẫn quy trình, thiết lập cho người đóng góp | Bắt đầu, quy tắc lập trình |
| `design` | Mô tả kiến trúc và hành vi lúc chạy của hệ thống | Sơ đồ tuần tự |

Ví dụ về các tệp hiện có trong kho:

```
docs/
├── 01-guide-getting-started.md
├── 02-guide-coding-rules.md
├── 03-design-sequence-object.md
├── 04-design-sequence-runtime.md
└── 05-design-data-storage.md
```

Một số lưu ý quan trọng:

- Các tệp tài liệu (`.md`) sử dụng `kebab-case` (dấu gạch ngang). Các tệp nguồn và thư mục sử dụng `snake_case` (dấu gạch dưới). Sự khác biệt này là cố ý: `kebab-case` là quy ước tiêu chuẩn cho đường dẫn và slug của Markdown.
- Hình ảnh đi kèm tài liệu nằm trong thư mục `resources/images/<topic_dir>/`, trong đó `<topic_dir>` tuân theo quy ước thư mục (snake_case). Ví dụ: `resources/images/guide-coding-rules/`.
- Khi thêm một tài liệu mới, hãy nối tiếp số thứ tự từ số lớn nhất hiện tại và giữ nguyên nhóm hạng mục để bảo toàn thứ tự đọc tự nhiên.
- Đổi tên tệp tài liệu phải thực hiện bằng `git mv` để lịch sử đổi tên được theo dõi chính xác.

---

## IV. Tài liệu tham khảo

- [Quy ước đặt tên — Định danh nhiều từ (Wikipedia)](https://en.wikipedia.org/wiki/Naming_convention_(programming)#Multiple-word_identifiers) — định nghĩa về `snake_case`, `SCREAMING_SNAKE_CASE`, và `kebab-case` được dùng ở Phần I và III.
- [Pro Git — Recording Changes to the Repository](https://git-scm.com/book/en/v2/Git-Basics-Recording-Changes-to-the-Repository) — quy trình `git add` / `git commit` / `git push` được dùng ở Phần II.
