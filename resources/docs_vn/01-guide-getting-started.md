<h1 align="center">Hướng dẫn Cài đặt và Biên dịch Dự án</h1>

Tài liệu này vạch ra các thủ tục tiêu chuẩn để khởi tạo kho lưu trữ dự án Space Shooter, thiết lập môi trường phát triển và triển khai các thay đổi mã nguồn.

---

## Mục lục

- [1. Khởi tạo Kho lưu trữ (Forking)](#1-khoi-tao-kho-luu-tru-forking)
- [2. Cấu hình Môi trường Phát triển](#2-cau-hinh-moi-truong-phat-trien)
- [3. Quy trình Phát triển Tiêu chuẩn](#3-quy-trinh-phat-trien-tieu-chuan)
  - [3.1 Cấu hình Không gian làm việc](#31-cau-hinh-khong-gian-lam-viec)
  - [3.2 Sao chép Kho lưu trữ](#32-sao-chep-kho-luu-tru)
  - [3.3 Chỉnh sửa Mã nguồn](#33-chinh-sua-ma-nguon)
  - [3.4 Cấu hình Phiên bản Phần cứng (Ver 2.1 vs Ver 3.0)](#34-cau-hinh-phien-ban-phan-cung-ver-21-vs-ver-30)
  - [3.5 Các thao tác Kiểm soát Phiên bản](#35-cac-thao-tac-kiem-soat-phien-ban)

---

## 1. Khởi tạo Kho lưu trữ (Forking)

Để duy trì tính toàn vẹn của việc kiểm soát phiên bản, các nhà phát triển được yêu cầu làm việc trên một bản fork cá nhân của kho lưu trữ:

### 1.1 Truy cập Kho lưu trữ Gốc

**URL Nguồn:** [https://github.com/the-ak-foundation/ak-base-kit-stm32l151](https://github.com/the-ak-foundation/ak-base-kit-stm32l151) (Hoặc kho lưu trữ upstream tương ứng).

### 1.2 Tạo bản Fork

Thực thi thao tác **Fork** trên giao diện GitHub để sao chép kho lưu trữ về tài khoản cá nhân của bạn.

> **Lưu ý:**
> - Đặt tên kho lưu trữ (repository) theo tên trò chơi của bạn.
> - Thêm mô tả ngắn gọn về trò chơi của bạn vào trường **Description**.

---

## 2. Cấu hình Môi trường Phát triển

Việc biên dịch và nạp (flash) firmware yêu cầu một môi trường dựa trên Linux (khuyến nghị dùng Ubuntu). Các bước cấu hình chi tiết cho toolchain được tài liệu hóa tại đây:

**[AK Embedded Base Kit STM32L151 — Hướng dẫn Cấu hình](https://epcb.vn/blogs/ak-embedded-software/ak-embedded-base-kit-stm32l151-getting-started)**

### 2.1 Cấu hình Đường dẫn Makefile cho nhiều máy Linux khác nhau

Mặc định, các tệp `Makefile` trong dự án này (`application/Makefile` và `boot/Makefile`) chứa các đường dẫn cố định (hardcoded paths) có thể không khớp với môi trường của mọi lập trình viên. Để quá trình build thành công trên máy Linux cụ thể của bạn, hãy cập nhật các biến đường dẫn này.

1. Mở `application/Makefile` và `boot/Makefile`.
2. Tìm đến phần cấu hình đường dẫn (thường nằm ở đầu file).
3. Chỉnh sửa chúng cho khớp với đường dẫn cài đặt trên máy bạn, hoặc dùng toán tử `?=` để cho phép ghi đè bằng biến môi trường (Environment Variable):

```makefile
# Cho phép ghi đè bằng biến môi trường
GCC_PATH            ?= /usr
PROGRAMER_PATH      ?= /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin
OPENOCD_CFG_PATH    ?= /usr/share/openocd/scripts/board/stm32ldiscovery.cfg
```

Nếu bạn đã cài `gcc-arm-none-eabi` ở cấp toàn hệ thống qua `apt`, khuyến nghị nên xóa tiền tố `$(GCC_PATH)/bin/` khỏi các biến trình biên dịch (`CC`, `CPP`, v.v.) để hệ điều hành tự động tìm chúng qua biến `$PATH`:

```makefile
CC  = arm-none-eabi-gcc
CPP = arm-none-eabi-g++
```

---

## 3. Quy trình Phát triển Tiêu chuẩn

> **Yêu cầu:** Mọi thao tác `make` và `flash` phải được thực thi bên trong Linux Terminal để đảm bảo tính tương thích của toolchain.

### 3.1 Cấu hình Không gian làm việc

Tạo một thư mục gốc được chỉ định (ví dụ: `Workspace`) bên trong thư mục `Home` của người dùng để cô lập các phụ thuộc của dự án.

Bên trong thư mục `Workspace`, thiết lập hai thư mục con:

| Thư mục | Chức năng |
| --------- | --------- |
| `Sources` | Vị trí được chỉ định cho các kho git. |
| `Tools`   | Vị trí được chỉ định cho các tệp nhị phân của trình biên dịch và công cụ nạp. |

---

### 3.2 Sao chép Kho lưu trữ

> **Lưu ý:** Thao tác này được thực thi một lần trong quá trình thiết lập ban đầu.

Điều hướng đến thư mục `Sources` qua Terminal và thực thi lệnh clone (thay thế URL bằng kho lưu trữ fork của bạn):

```bash
git clone https://github.com/<username>/<repository-name>.git
```

---

### 3.3 Chỉnh sửa Mã nguồn

Khuyến nghị sử dụng Visual Studio Code for Linux để chỉnh sửa mã nguồn.

Logic ứng dụng cốt lõi nằm bên trong thư mục `application/sources/app`.

#### Quy trình sửa đổi Thành phần UI (Ví dụ: Màn hình Tiêu đề)

**Bước 3.3.1 —** Kiểm tra các định nghĩa header trong `screens/screens.h` để hiểu API kết xuất UI.

**Bước 3.3.2 —** Sửa đổi quá trình triển khai trong `scr_game_title.cpp`. Tệp này xử lý dữ liệu bitmap và cập nhật tọa độ thực thể cho Màn hình Tiêu đề.

**Bước 3.3.3 —** Nếu các tệp nguồn mới được đưa vào (ví dụ: `scr_game_new.cpp`), chúng phải được nối vào danh sách `Makefile.mk` bên trong thư mục `screens/` để được đưa vào mục tiêu biên dịch.

**Bước 3.3.4 —** Thực thi tập lệnh build và nạp tệp nhị phân thu được vào MCU.

---

### 3.4 Cấu hình Phiên bản Phần cứng (Ver 2.1 vs Ver 3.0)

Bộ Kit AK Base có các phiên bản phần cứng khác nhau đi kèm với trình điều khiển màn hình OLED khác biệt:
- **Ver 2.1**: Sử dụng màn hình OLED 1.3" (Driver SH1106).
- **Ver 3.0**: Sử dụng màn hình OLED 1.54" (Driver SSD1309).

Để chuyển đổi giữa các phiên bản này, hãy chỉnh sửa tệp `application/Makefile` ở phần `Hardware option`:

**Đối với Ver 2.1 (Mặc định):**
```makefile
HARDWARE_OPTION += \
		-USSD1309_DRIVER_EN	\
		-DSH1106_DRIVER_EN
```

**Đối với Ver 3.0:**
```makefile
HARDWARE_OPTION += \
		-DSSD1309_DRIVER_EN	\
		-USH1106_DRIVER_EN
```

> **Quan trọng:** Sau khi sửa đổi Makefile, bạn bắt buộc phải chạy lệnh `make clean` trước khi build lại (make) để áp dụng cấu hình mới.

#### Sửa lỗi lệch khung hình (Offset Deviation) trên OLED SH1106
Nếu màn hình trên Ver 2.1 (SH1106) bị lệch khung hình sang một bên hoặc bị cắt lề (thường là lệch khoảng 2 pixel), bạn cần điều chỉnh lại biến `OLED_COL_OFFSET`.
1. Mở tệp `application/sources/driver/Adafruit_oled_drv/Adafruit_oled_drv.h`.
2. Tìm khối định nghĩa `SH1106_DRIVER_EN` (khoảng dòng 8) và đổi giá trị của `OLED_COL_OFFSET` từ `0` thành `2`:
```cpp
#if defined (SH1106_DRIVER_EN)
#define OLED_COL_OFFSET 2
```
3. Chạy `make clean` và `make` lại để nạp firmware với cấu hình offset mới.

---

### 3.5 Các thao tác Kiểm soát Phiên bản

Khi hoàn thành triển khai một tính năng, commit các thay đổi lên kho lưu trữ từ xa. Thực thi các lệnh sau từ thư mục gốc của kho lưu trữ:

```bash
git add .
git commit -m "[UPDATE] Modified Title Screen rendering logic"
git push origin main
```

---

## Tài liệu Tham khảo

- Tài liệu Hệ thống Nhúng & Phần cứng: [AK Embedded Software](https://epcb.vn/blogs/ak-embedded-software)
