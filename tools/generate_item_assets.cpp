#include <raylib.h>
#include <iostream>
#include <vector>
#include <cmath>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

// Helper function để đặt pixel an toàn
void setP(Image& img, int x, int y, Color c) {
    if (x >= 0 && x < img.width && y >= 0 && y < img.height) {
        ImageDrawPixel(&img, x, y, c);
    }
}

// Helper vẽ đường pixel
void drawPLine(Image& img, int x0, int y0, int x1, int y1, Color c) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        setP(img, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// =============================================================================
// 1. THANH KIẾM THÉP (sword_steel.png) - 32x32
// =============================================================================
Image generateSwordSteel() {
    Image img = GenImageColor(32, 32, BLANK);

    Color outline   = Color{ 18, 18, 24, 255 };
    Color bladeDark = Color{ 105, 120, 140, 255 };
    Color bladeMid  = Color{ 175, 190, 210, 255 };
    Color bladeLite = Color{ 235, 245, 255, 255 };
    Color shine     = Color{ 255, 255, 255, 255 };
    Color goldDark  = Color{ 140, 95, 20, 255 };
    Color goldMid   = Color{ 220, 170, 35, 255 };
    Color goldLite  = Color{ 255, 235, 110, 255 };
    Color gripDark  = Color{ 65, 35, 15, 255 };
    Color gripMid   = Color{ 115, 65, 30, 255 };

    // 1. Chuôi kiếm & Đốc kiếm (Pommel & Grip): từ (5, 27) đến (11, 21)
    // Pommel vàng tròn ở góc dưới trái (5, 27)
    setP(img, 4, 28, outline); setP(img, 5, 28, outline); setP(img, 6, 28, outline);
    setP(img, 3, 27, outline); setP(img, 4, 27, goldLite); setP(img, 5, 27, goldMid); setP(img, 6, 27, outline);
    setP(img, 4, 26, outline); setP(img, 5, 26, goldDark); setP(img, 6, 26, outline);

    // Cán bọc da (Grip): từ (6, 26) lên (10, 22)
    for (int i = 0; i < 4; ++i) {
        int gx = 7 + i;
        int gy = 25 - i;
        // Viền
        setP(img, gx - 1, gy + 1, outline);
        setP(img, gx + 1, gy - 1, outline);
        // Da bọc & chỉ quấn
        setP(img, gx, gy, (i % 2 == 1) ? goldLite : gripMid);
        setP(img, gx, gy + 1, gripDark);
    }

    // 2. Thanh chắn tay (Crossguard) mạ vàng vuốt nhọn vuông góc với lưỡi
    // Trục kiếm tại (11, 21), vuông góc theo vector (-1, -1) và (+1, +1)
    int cx = 11, cy = 21;
    for (int k = -4; k <= 4; ++k) {
        int gx = cx + k;
        int gy = cy + k;
        // Viền ngoài
        setP(img, gx - 1, gy + 1, outline);
        setP(img, gx + 1, gy - 1, outline);
        
        // Thân chắn tay
        if (k == 0) {
            setP(img, gx, gy, goldLite);
            setP(img, gx - 1, gy, goldMid);
            setP(img, gx + 1, gy, goldDark);
        } else if (std::abs(k) <= 3) {
            setP(img, gx, gy, (k < 0) ? goldLite : goldMid);
            setP(img, gx, gy + 1, goldDark);
        } else {
            // Mũi nhọn 2 đầu cánh guard
            setP(img, gx, gy, goldMid);
            setP(img, gx, gy - 1, outline);
            setP(img, gx, gy + 1, outline);
        }
    }

    // 3. Lưỡi kiếm (Blade): từ (12, 20) lên (26, 6)
    for (int t = 1; t <= 14; ++t) {
        int bx = 11 + t;
        int by = 21 - t;

        // Cạnh trên / sống kiếm
        setP(img, bx - 1, by, outline);
        setP(img, bx, by - 1, outline);

        // Mặt trên sáng (phản chiếu ánh sáng bầu trời)
        setP(img, bx, by, bladeLite);
        
        // Rãnh giữa (Fuller)
        if (t <= 10) {
            setP(img, bx + 1, by, bladeDark);
        } else {
            setP(img, bx + 1, by, bladeMid);
        }

        // Mặt dưới bóng
        setP(img, bx + 1, by + 1, bladeMid);
        setP(img, bx + 2, by + 1, bladeDark);

        // Viền dưới
        setP(img, bx + 2, by + 2, outline);
    }

    // Mũi kiếm nhọn (Tip) tại (26, 6) -> (28, 4)
    setP(img, 25, 6, bladeLite);
    setP(img, 26, 5, shine);     setP(img, 27, 5, outline);
    setP(img, 27, 4, shine);     setP(img, 28, 4, outline);
    setP(img, 28, 3, shine);     setP(img, 29, 3, outline);
    setP(img, 27, 3, outline);   setP(img, 28, 2, outline);

    // Điểm sáng lấp lánh (Sparkle glints)
    setP(img, 20, 10, shine);
    setP(img, 21, 9, shine);
    setP(img, 22, 10, shine);
    setP(img, 21, 11, shine);

    return img;
}

// =============================================================================
// 2. ĐẠI KIẾM HUYỀN BÍ (sword_mystic.png) - 32x32
// =============================================================================
Image generateSwordMystic() {
    Image img = GenImageColor(32, 32, BLANK);

    Color outline   = Color{ 12, 10, 28, 255 };
    Color glowAura  = Color{ 50, 180, 255, 80 };
    Color glowBright= Color{ 90, 230, 255, 200 };
    Color bladeDark = Color{ 30, 45, 95, 255 };
    Color bladeMid  = Color{ 55, 110, 195, 255 };
    Color bladeLite = Color{ 120, 210, 255, 255 };
    Color runeCyan  = Color{ 210, 255, 255, 255 };
    Color goldDark  = Color{ 120, 75, 20, 255 };
    Color goldMid   = Color{ 210, 150, 30, 255 };
    Color goldLite  = Color{ 255, 215, 70, 255 };
    Color gemPurple = Color{ 225, 75, 255, 255 };
    Color gemPink   = Color{ 255, 160, 255, 255 };
    Color whiteStar = Color{ 255, 255, 255, 255 };

    // Hào quang ma thuật lân tinh (Magic Aura)
    for (int t = 1; t <= 15; ++t) {
        int bx = 11 + t;
        int by = 21 - t;
        for (int k = -3; k <= 3; ++k) {
            setP(img, bx + k - 2, by + k - 1, glowAura);
            setP(img, bx + k + 2, by + k + 1, glowAura);
        }
    }

    // 1. Pommel nạc ngọc tím ma thuật: (4, 28)
    setP(img, 3, 28, outline); setP(img, 4, 28, goldLite); setP(img, 5, 28, goldDark); setP(img, 6, 28, outline);
    setP(img, 3, 27, outline); setP(img, 4, 27, gemPink);  setP(img, 5, 27, gemPurple); setP(img, 6, 27, outline);
    setP(img, 4, 26, outline); setP(img, 5, 26, goldDark); setP(img, 6, 26, outline);

    // 2. Cán cầm hắc diện thạch bọc chỉ vàng: từ (6, 26) lên (10, 22)
    for (int i = 0; i < 4; ++i) {
        int gx = 6 + i;
        int gy = 26 - i;
        setP(img, gx - 1, gy + 1, outline);
        setP(img, gx + 1, gy - 1, outline);
        setP(img, gx, gy, (i % 2 == 1) ? goldLite : Color{ 35, 30, 55, 255 });
        setP(img, gx, gy + 1, Color{ 20, 18, 35, 255 });
    }

    // 3. Đại hộ thủ hoàng gia (Winged Crossguard) bản lớn dát vàng nạm ngọc
    int cx = 11, cy = 21;
    for (int k = -5; k <= 5; ++k) {
        int gx = cx + k;
        int gy = cy + k;
        setP(img, gx - 1, gy + 1, outline);
        setP(img, gx + 1, gy - 1, outline);
        if (k == 0) {
            // Ngọc tím ma thuật ở chính giữa hộ thủ
            setP(img, gx, gy, gemPink);
            setP(img, gx - 1, gy, gemPurple);
            setP(img, gx + 1, gy, goldMid);
        } else if (std::abs(k) <= 4) {
            setP(img, gx, gy, (k < 0) ? goldLite : goldMid);
            setP(img, gx, gy + 1, goldDark);
            setP(img, gx - 1, gy, goldLite);
        } else {
            setP(img, gx, gy, goldLite);
            setP(img, gx - 1, gy, outline);
            setP(img, gx + 1, gy, outline);
        }
    }

    // 4. Lưỡi đại kiếm bản rộng (Broad Blade) với rune ma thuật
    for (int t = 1; t <= 14; ++t) {
        int bx = 11 + t;
        int by = 21 - t;

        // Cạnh trên
        setP(img, bx - 2, by - 1, outline);
        setP(img, bx - 1, by - 1, bladeLite);
        setP(img, bx,     by - 1, bladeLite);

        // Thân kiếm trên
        setP(img, bx - 1, by, bladeMid);
        setP(img, bx,     by, bladeMid);

        // Trục giữa: Cổ ngữ Rune phát sáng rực rỡ
        bool isRune = (t % 3 == 0 || t == 7 || t == 11);
        setP(img, bx + 1, by, isRune ? runeCyan : glowBright);
        setP(img, bx,     by + 1, isRune ? runeCyan : bladeMid);

        // Thân kiếm dưới
        setP(img, bx + 1, by + 1, bladeDark);
        setP(img, bx + 2, by + 1, bladeDark);

        // Cạnh dưới
        setP(img, bx + 1, by + 2, bladeDark);
        setP(img, bx + 2, by + 2, outline);
        setP(img, bx + 3, by + 2, outline);
    }

    // Mũi kiếm đại kiếm vát nhọn hùng vĩ
    setP(img, 26, 6, bladeLite);
    setP(img, 27, 5, runeCyan);  setP(img, 28, 5, outline);
    setP(img, 28, 4, whiteStar); setP(img, 29, 4, outline);
    setP(img, 29, 3, whiteStar); setP(img, 30, 3, outline);
    setP(img, 28, 3, outline);   setP(img, 29, 2, outline);

    // Tinh cầu năng lượng tỏa ra xung quanh kiếm
    setP(img, 23, 5, runeCyan);
    setP(img, 24, 4, whiteStar);
    setP(img, 25, 5, runeCyan);
    setP(img, 24, 6, runeCyan);

    setP(img, 17, 13, glowBright);
    setP(img, 12, 12, glowBright);
    setP(img, 20, 18, glowBright);

    return img;
}

// =============================================================================
// 3. BÌNH THUỐC KHỞI ĐẦU (potion_starter.png) - 32x32
// =============================================================================
Image generatePotionStarter() {
    Image img = GenImageColor(32, 32, BLANK);

    Color outline   = Color{ 35, 15, 20, 255 };
    Color corkDark  = Color{ 110, 60, 30, 255 };
    Color corkLite  = Color{ 170, 110, 60, 255 };
    Color glassRim  = Color{ 160, 190, 210, 220 };
    Color glassShine= Color{ 255, 255, 255, 230 };
    Color liqDark   = Color{ 140, 20, 30, 255 };
    Color liqMid    = Color{ 220, 35, 45, 255 };
    Color liqLite   = Color{ 255, 80, 80, 255 };
    Color bubble    = Color{ 255, 170, 170, 255 };

    // Nút bần (Cork): x=14..17, y=6..8
    for (int y = 6; y <= 8; ++y) {
        setP(img, 13, y, outline);
        setP(img, 14, y, corkLite);
        setP(img, 15, y, corkLite);
        setP(img, 16, y, corkDark);
        setP(img, 17, y, corkDark);
        setP(img, 18, y, outline);
    }
    setP(img, 14, 5, outline); setP(img, 15, 5, outline);
    setP(img, 16, 5, outline); setP(img, 17, 5, outline);

    // Cổ chai thủy tinh (Neck): x=13..18, y=9..12
    for (int y = 9; y <= 12; ++y) {
        setP(img, 12, y, outline);
        setP(img, 13, y, glassRim);
        setP(img, 14, y, glassShine);
        setP(img, 15, y, (y >= 11) ? liqLite : glassRim);
        setP(img, 16, y, (y >= 11) ? liqMid  : glassRim);
        setP(img, 17, y, (y >= 11) ? liqDark : glassRim);
        setP(img, 18, y, outline);
    }

    // Thân chai mở rộng hình oval: x=8..23, y=13..26
    for (int y = 13; y <= 26; ++y) {
        int w = 0;
        if (y == 13) w = 4;
        else if (y == 14) w = 6;
        else if (y >= 15 && y <= 22) w = 7;
        else if (y == 23) w = 6;
        else if (y == 24) w = 5;
        else if (y >= 25) w = 4;

        int leftX = 15 - w;
        int rightX = 16 + w;

        setP(img, leftX, y, outline);
        setP(img, rightX, y, outline);

        for (int x = leftX + 1; x < rightX; ++x) {
            if (y <= 14) {
                setP(img, x, y, (x <= 15) ? liqLite : liqMid);
            } else {
                if (x == leftX + 1) setP(img, x, y, glassRim);
                else if (x == leftX + 2) setP(img, x, y, glassShine);
                else if (x <= 14) setP(img, x, y, liqLite);
                else if (x <= 18) setP(img, x, y, liqMid);
                else setP(img, x, y, liqDark);
            }
        }
    }

    // Đáy chai
    for (int x = 12; x <= 19; ++x) {
        setP(img, x, 27, outline);
    }

    // Vệt bóng phản quang thủy tinh hình cung
    setP(img, 10, 16, glassShine); setP(img, 10, 17, glassShine);
    setP(img, 11, 15, glassShine);
    setP(img, 10, 21, glassShine); setP(img, 10, 22, glassShine);

    // Bọt khí nổi
    setP(img, 14, 22, bubble);
    setP(img, 17, 19, bubble);
    setP(img, 13, 17, bubble);

    return img;
}

// =============================================================================
// 4. BÌNH MÁU NHỎ (potion_health.png) - 32x32 (Dạng bình cầu / Round Flask)
// =============================================================================
Image generatePotionHealth() {
    Image img = GenImageColor(32, 32, BLANK);

    Color outline   = Color{ 30, 10, 15, 255 };
    Color corkMid   = Color{ 140, 85, 45, 255 };
    Color corkLite  = Color{ 185, 125, 75, 255 };
    Color goldBand  = Color{ 225, 180, 50, 255 };
    Color glassShine= Color{ 255, 255, 255, 240 };
    Color glassSoft = Color{ 180, 210, 230, 180 };
    Color liqDark   = Color{ 120, 10, 25, 255 };
    Color liqMid    = Color{ 200, 25, 40, 255 };
    Color liqLite   = Color{ 255, 60, 70, 255 };
    Color liqGlow   = Color{ 255, 120, 120, 255 };

    // Nút bần bo tròn trên đỉnh: x=14..17, y=5..7
    for (int y = 5; y <= 7; ++y) {
        setP(img, 13, y, outline);
        setP(img, 14, y, corkLite);
        setP(img, 15, y, corkLite);
        setP(img, 16, y, corkMid);
        setP(img, 17, y, corkMid);
        setP(img, 18, y, outline);
    }
    setP(img, 14, 4, outline); setP(img, 15, 4, outline);
    setP(img, 16, 4, outline); setP(img, 17, 4, outline);

    // Đai vàng quanh cổ chai
    setP(img, 12, 8, outline);
    for (int x = 13; x <= 17; ++x) setP(img, x, 8, goldBand);
    setP(img, 18, 8, outline);

    // Cổ chai: y=9..11
    for (int y = 9; y <= 11; ++y) {
        setP(img, 12, y, outline);
        setP(img, 13, y, glassSoft);
        setP(img, 14, y, glassShine);
        setP(img, 15, y, liqLite);
        setP(img, 16, y, liqMid);
        setP(img, 17, y, liqDark);
        setP(img, 18, y, outline);
    }

    // Thân bình tròn vo (Spherical Ball): tâm (15.5, 20), bán kính ~7.5
    int cx = 15, cy = 20;
    int r = 8;
    for (int y = cy - r; y <= cy + r; ++y) {
        for (int x = cx - r; x <= cx + r + 1; ++x) {
            float dist = std::sqrt((x - 15.5f)*(x - 15.5f) + (y - 20.0f)*(y - 20.0f));
            if (dist <= 8.2f && dist >= 7.0f) {
                setP(img, x, y, outline);
            } else if (dist < 7.0f) {
                if (y < 14) {
                    setP(img, x, y, glassSoft);
                } else {
                    if (x < 13 && y < 20) setP(img, x, y, liqGlow);
                    else if (x <= 15) setP(img, x, y, liqLite);
                    else if (x <= 19) setP(img, x, y, liqMid);
                    else setP(img, x, y, liqDark);
                }
            }
        }
    }

    // Ánh sáng cong tròn trên quả cầu thủy tinh
    setP(img, 11, 16, glassShine); setP(img, 11, 17, glassShine);
    setP(img, 12, 15, glassShine); setP(img, 13, 14, glassShine);
    setP(img, 12, 16, glassShine);
    setP(img, 10, 20, glassSoft);  setP(img, 10, 21, glassSoft);

    // Hạt tinh chất hồi phục
    setP(img, 16, 21, liqGlow);
    setP(img, 16, 22, liqGlow);
    setP(img, 15, 22, liqGlow);
    setP(img, 17, 22, liqGlow);
    setP(img, 16, 23, liqGlow);

    return img;
}

// =============================================================================
// 5. BÌNH THUỐC CƯỜNG HÓA (potion_strength.png) - 32x32 (Viên kim cương tím)
// =============================================================================
Image generatePotionStrength() {
    Image img = GenImageColor(32, 32, BLANK);

    Color outline   = Color{ 25, 10, 35, 255 };
    Color silver    = Color{ 210, 220, 235, 255 };
    Color silverDark= Color{ 140, 150, 170, 255 };
    Color purpDark  = Color{ 80, 20, 120, 255 };
    Color purpMid   = Color{ 150, 45, 200, 255 };
    Color purpLite  = Color{ 215, 90, 255, 255 };
    Color purpGlow  = Color{ 245, 180, 255, 255 };
    Color shine     = Color{ 255, 255, 255, 255 };

    // Hào quang tím ma thuật nhẹ ngoài viền
    for (int y = 14; y <= 24; ++y) {
        setP(img, 6, y, Color{ 180, 50, 230, 40 });
        setP(img, 25, y, Color{ 180, 50, 230, 40 });
    }

    // Nút bạc tinh xảo: x=14..17, y=5..7
    for (int y = 5; y <= 7; ++y) {
        setP(img, 13, y, outline);
        setP(img, 14, y, shine);
        setP(img, 15, y, silver);
        setP(img, 16, y, silverDark);
        setP(img, 17, y, silverDark);
        setP(img, 18, y, outline);
    }
    setP(img, 14, 4, outline); setP(img, 15, 4, outline);
    setP(img, 16, 4, outline); setP(img, 17, 4, outline);

    // Cổ bình bạc nạm gờ: y=8..10
    for (int y = 8; y <= 10; ++y) {
        setP(img, 12, y, outline);
        setP(img, 13, y, silver);
        setP(img, 14, y, shine);
        setP(img, 15, y, purpLite);
        setP(img, 16, y, purpMid);
        setP(img, 17, y, silverDark);
        setP(img, 18, y, outline);
    }

    // Thân bình dạng góc cạnh hình lục giác / kim cương pha lê
    for (int y = 11; y <= 26; ++y) {
        int w = 0;
        if (y <= 18) {
            w = 3 + (y - 11) * 6 / 7;
        } else {
            w = 9 - (y - 18) * 6 / 8;
        }

        int lx = 15 - w;
        int rx = 16 + w;

        setP(img, lx, y, outline);
        setP(img, rx, y, outline);

        for (int x = lx + 1; x < rx; ++x) {
            if (y == 11 || y == 12) {
                setP(img, x, y, (x <= 15) ? purpLite : purpMid);
            } else {
                if (x == lx + 1) setP(img, x, y, purpGlow);
                else if (x == lx + 2) setP(img, x, y, shine);
                else if (x <= 14) setP(img, x, y, purpLite);
                else if (x <= 18) setP(img, x, y, purpMid);
                else setP(img, x, y, purpDark);
            }
        }
    }

    // Đáy chai
    for (int x = 12; x <= 19; ++x) setP(img, x, 27, outline);

    // Tinh thể năng lượng xoáy sáng ở giữa bình
    setP(img, 15, 17, purpGlow); setP(img, 16, 17, shine);
    setP(img, 14, 18, purpGlow); setP(img, 15, 18, shine); setP(img, 16, 18, purpGlow);
    setP(img, 15, 19, purpGlow); setP(img, 16, 19, purpLite);

    // Tia lấp lánh bên ngoài
    setP(img, 9, 13, shine);
    setP(img, 23, 21, purpGlow);

    return img;
}

// =============================================================================
// 6. THẦN DƯỢC AETHELGARD (potion_elixir.png) - 32x32 (Bình vàng hoàng gia thánh khiết)
// =============================================================================
Image generatePotionElixir() {
    Image img = GenImageColor(32, 32, BLANK);

    Color outline   = Color{ 40, 25, 5, 255 };
    Color goldDark  = Color{ 140, 90, 15, 255 };
    Color goldMid   = Color{ 215, 160, 30, 255 };
    Color goldLite  = Color{ 255, 220, 75, 255 };
    Color holyWhite = Color{ 255, 255, 235, 255 };
    Color holyGold  = Color{ 255, 240, 140, 255 };
    Color auraLight = Color{ 255, 215, 0, 70 };

    // Hào quang thiên thần tỏa sáng (Holy Aura)
    for (int y = 8; y <= 25; ++y) {
        for (int x = 5; x <= 26; ++x) {
            float d = std::sqrt((x - 15.5f)*(x - 15.5f) + (y - 17.0f)*(y - 17.0f));
            if (d >= 8.5f && d <= 12.0f) {
                setP(img, x, y, auraLight);
            }
        }
    }

    // Vương miện nhỏ trên nút chai: y=3..6
    setP(img, 13, 3, goldLite); setP(img, 15, 3, holyWhite); setP(img, 18, 3, goldMid);
    for (int x = 13; x <= 18; ++x) {
        setP(img, x, 4, goldLite);
        setP(img, x, 5, goldMid);
    }
    setP(img, 12, 4, outline); setP(img, 19, 4, outline);
    setP(img, 12, 5, outline); setP(img, 19, 5, outline);

    // Quai bình hoàng gia dát vàng 2 bên (Royal Wings/Handles)
    // Cánh trái
    setP(img, 9, 11, outline); setP(img, 10, 11, goldLite);
    setP(img, 8, 12, outline); setP(img, 9, 12, goldMid); setP(img, 8, 13, goldDark);
    setP(img, 8, 14, goldMid); setP(img, 9, 15, goldDark); setP(img, 10, 16, outline);
    // Cánh phải
    setP(img, 22, 11, goldLite); setP(img, 23, 11, outline);
    setP(img, 22, 12, goldMid);  setP(img, 23, 12, outline); setP(img, 23, 13, goldDark);
    setP(img, 23, 14, goldMid);  setP(img, 22, 15, goldDark); setP(img, 21, 16, outline);

    // Cổ bình nạm ngọc: y=6..9
    for (int y = 6; y <= 9; ++y) {
        setP(img, 12, y, outline);
        setP(img, 13, y, goldLite);
        setP(img, 14, y, holyWhite);
        setP(img, 15, y, holyGold);
        setP(img, 16, y, goldMid);
        setP(img, 17, y, goldDark);
        setP(img, 18, y, outline);
    }

    // Thân bình thánh thót hình giọt sương / bình amphora hoàng gia: y=10..25
    for (int y = 10; y <= 25; ++y) {
        int w = 0;
        if (y <= 16) {
            w = 4 + (y - 10) * 4 / 6;
        } else {
            w = 8 - (y - 16) * 4 / 9;
        }

        int lx = 15 - w;
        int rx = 16 + w;

        setP(img, lx, y, outline);
        setP(img, rx, y, outline);

        for (int x = lx + 1; x < rx; ++x) {
            if (x == lx + 1) setP(img, x, y, goldLite);
            else if (x == lx + 2) setP(img, x, y, holyWhite);
            else if (x <= 14) setP(img, x, y, holyGold);
            else if (x <= 18) setP(img, x, y, goldMid);
            else setP(img, x, y, goldDark);
        }
    }

    // Chân đế hoàng gia (Pedestal base): y=26..28
    for (int x = 11; x <= 20; ++x) {
        setP(img, x, 26, goldMid);
        setP(img, x, 27, (x == 12 || x == 13) ? holyWhite : goldLite);
        setP(img, x, 28, outline);
    }
    setP(img, 10, 27, outline); setP(img, 21, 27, outline);

    // Ngôi sao thánh tích lấp lánh giữa bình
    setP(img, 15, 14, holyWhite);
    setP(img, 14, 15, holyWhite); setP(img, 15, 15, Color{ 255, 255, 255, 255 }); setP(img, 16, 15, holyWhite);
    setP(img, 15, 16, holyWhite);

    // Tia sáng thần thánh xung quanh (Holy Sparkles)
    setP(img, 7, 8, holyWhite);
    setP(img, 24, 7, holyWhite);
    setP(img, 25, 23, holyWhite);
    setP(img, 6, 21, holyWhite);

    return img;
}

int main() {
    std::cout << "[Item Asset Generator] Dang khoi tao thu muc assets/items..." << std::endl;
    MKDIR("assets");
    MKDIR("assets/items");

    std::vector<std::pair<std::string, Image>> items = {
        { "assets/items/sword_steel.png",     generateSwordSteel() },
        { "assets/items/sword_mystic.png",    generateSwordMystic() },
        { "assets/items/potion_starter.png",  generatePotionStarter() },
        { "assets/items/potion_health.png",   generatePotionHealth() },
        { "assets/items/potion_strength.png", generatePotionStrength() },
        { "assets/items/potion_elixir.png",   generatePotionElixir() }
    };

    for (auto& pair : items) {
        bool ok = ExportImage(pair.second, pair.first.c_str());
        if (ok) {
            std::cout << " -> Da tao thanh cong: " << pair.first << " (32x32 RGBA)" << std::endl;
        } else {
            std::cerr << " -> [LOI] Khong the xuat file: " << pair.first << std::endl;
        }
        UnloadImage(pair.second);
    }

    std::cout << "[Item Asset Generator] Hoan tat tao toan bo item assets!" << std::endl;
    return 0;
}
