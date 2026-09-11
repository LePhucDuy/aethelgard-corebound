# Aethelgard: Corebound

Roguelike 2D side-scrolling viết bằng **C++17** và **raylib 5.5**. Người chơi vào vai chiến binh
xuống hầm ngục (dungeon) sinh địa hình ngẫu nhiên, chiến đấu với quái (heo rừng, ong, ốc sên),
thu thập vũ khí và bình thuốc, lưu/tải tiến trình.

Cửa sổ game mặc định **1280x720**, V-Sync bật, hỗ trợ toàn màn hình.

---

## 1. Yêu cầu hệ thống

| Thành phần | Linux | Windows |
|---|---|---|
| Trình biên dịch | g++ (hỗ trợ C++17), make | g++ (MSYS2 UCRT64, hỗ trợ C++17) |
| Thư viện đồ họa | raylib 5.5 + GLFW 3.4 → **script `setup-linux.sh` tự tải và build sẵn** | raylib (cài qua MSYS2: `pacman -S mingw-w64-ucrt-x86_64-raylib`) |
| Công cụ tải | curl | không cần |
| Thư viện hệ thống | X11 dev + OpenGL (xem mục 2) | có sẵn trong MSYS2 |

> Toàn bộ dự án build từ nguồn, **không cần cài raylib vào hệ thống** ở cả 2 nền tảng.

---

## 2. Build & chạy trên Linux (Ubuntu/Debian/Fedora)

Cài các gói phát triển hệ thống (chỉ cần 1 lần):

```bash
# Ubuntu / Debian
sudo apt install -y build-essential curl libx11-dev libgl1-mesa-dev

# Fedora
sudo dnf install -y gcc-c++ make curl mesa-libGL-devel libX11-devel
```

Build và chạy:

```bash
./setup-linux.sh   # 1 lần duy nhất: tải + build raylib 5.5 & GLFW 3.4 static vào lib/
make               # build game -> bin/aethelgard
./bin/aethelgard   # hoặc: make run
```

Script `setup-linux.sh` chỉ build lại khi `lib/libraylib.a` chưa tồn tại, hoặc chạy
`./setup-linux.sh --force` để build lại từ đầu.

## 3. Build & chạy trên Windows (MSYS2)

1. Cài [MSYS2](https://www.msys2.org/), mở **UCRT64** shell rồi cài công cụ:

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make
   ```

2. Chạy script build (nó tự copy DLL cần thiết vào `bin/` để game chạy độc lập):

   ```bat
   build.bat
   ```

3. Chạy game:

   ```bat
   .\bin\aethelgard.exe
   ```

Có thể dùng `make` trong UCRT64 shell thay cho `build.bat` — Makefile tự nhận diện
Windows và link đúng flag `-lraylib -lopengl32 -lgdi32 -lwinmm`.

---

## 4. Điều khiển

| Phím | Chức năng |
|---|---|
| `A` / `D` hoặc `←` / `→` | Di chuyển trái / phải |
| `W` / `S` hoặc `↑` / `↓` | Di chuyển lên / xuống |
| `Space` | Nhảy |
| `J` / `F` / `Z` | Tấn công |
| `1` … `9` | Chọn slot vật phẩm trong túi đồ |
| `R` | Chơi lại khi chết |
| `F5` | Lưu tiến trình → `saves/savegame.txt` |
| `F9` | Tải tiến trình đã lưu |
| `F11` hoặc `Alt` + `Enter` | Bật / tắt toàn màn hình |
| `F12` | Chụp màn hình → `screenshot.png` |

Chạy ở chế độ chụp ảnh màn hình rồi tự thoát (tiện cho CI/screenshot), kèm tuỳ chọn
`--spawn X Y` để dịch chuyển thẳng tới khu bất kỳ (debug/kiểm thử từng khu):

```bash
./bin/aethelgard --screenshot output.png            # chụp màn hình khu hiện tại
./bin/aethelgard --spawn 24 12                      # vào thẳng Khu B - Rừng Ép Khắc
./bin/aethelgard --spawn 60 6 --screenshot boss.png # chụp khu Boss Đền Thờ
```

---

## 5. Cấu trúc dự án

```
aethelgard-corebound/
├── assets/               # Sprite nhân vật, quái, tileset, background
├── include/              # Header (.h) theo module: core, engine, entities, graphics, items, map, systems
├── src/                  # Mã nguồn (.cpp) cùng cấu trúc với include/
├── lib/                  # (Linux, tự sinh) raylib + GLFW static — tạo lại bằng ./setup-linux.sh
├── bin/                  # (tự sinh) file chạy aethelgard / aethelgard.exe + DLL (Windows)
├── saves/                # (tự sinh) savegame.txt khi bấm F5
├── Makefile              # Build đa nền tảng (tự nhận diện Linux / Windows)
├── setup-linux.sh        # Chuẩn bị môi trường Linux: build raylib + GLFW static
└── build.bat             # Build Windows bằng MSYS2
```

## 6. Ghi chú

- `bin/`, `lib/`, `saves/` và screenshot gốc là sản phẩm build/chơi, đã được đưa vào `.gitignore`.
- Repo clone mới về: Linux chạy `./setup-linux.sh` trước khi `make`; Windows chỉ cần `build.bat`.
- Nếu `setup-linux.sh` báo thiếu thư viện X11/OpenGL, cài theo lệnh ở mục 2 cho đúng distro.
