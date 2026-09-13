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

// =============================================================================
// 7. ĐỒNG XU VÀNG HOÀNG GIA (gold_coin.png) - 32x32
// =============================================================================
Image generateGoldCoin() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 70, 45, 10, 255 };
    Color goldDark  = Color{ 150, 100, 20, 255 };
    Color goldMid   = Color{ 225, 170, 35, 255 };
    Color goldLite  = Color{ 255, 225, 80, 255 };
    Color goldWhite = Color{ 255, 250, 190, 255 };
    Color sparkle   = Color{ 255, 255, 255, 255 };

    // Vẽ hình tròn đồng xu vàng đường kính ~18px, tâm (16, 16)
    float cx = 15.5f, cy = 15.5f, r = 8.5f;
    for (int y = 5; y <= 26; ++y) {
        for (int x = 5; x <= 26; ++x) {
            float dist = std::hypot((float)x - cx, (float)y - cy);
            if (dist <= r) {
                if (dist > r - 1.2f) {
                    setP(img, x, y, outline);
                } else if (dist > r - 2.4f) {
                    // Viền nổi 3D: góc trên trái sáng, góc dưới phải tối
                    if (x + y < 31) setP(img, x, y, goldLite);
                    else setP(img, x, y, goldDark);
                } else {
                    // Thân đồng xu
                    if (x < 14 && y < 14) setP(img, x, y, goldLite);
                    else if (x > 18 || y > 18) setP(img, x, y, goldMid);
                    else setP(img, x, y, goldLite);
                }
            }
        }
    }

    // Điểm phản quang bóng loáng góc trên trái
    setP(img, 11, 10, goldWhite);
    setP(img, 12, 10, goldWhite);
    setP(img, 10, 11, goldWhite);
    setP(img, 11, 11, goldWhite);

    // Hoa văn vương miện chạm khắc nổi ở tâm đồng xu
    setP(img, 13, 14, goldWhite); setP(img, 15, 13, goldWhite); setP(img, 17, 14, goldWhite);
    setP(img, 13, 15, goldDark);  setP(img, 15, 14, goldDark);  setP(img, 17, 15, goldDark);
    for (int x = 13; x <= 17; ++x) {
        setP(img, x, 16, goldDark);
        setP(img, x, 17, goldWhite);
        setP(img, x, 18, goldDark);
    }

    // Các tia sáng lấp lánh (Sparkles) 4 phương
    setP(img, 24, 7, sparkle);
    setP(img, 24, 6, Color{ 255, 240, 150, 220 }); setP(img, 24, 8, Color{ 255, 240, 150, 220 });
    setP(img, 23, 7, Color{ 255, 240, 150, 220 }); setP(img, 25, 7, Color{ 255, 240, 150, 220 });

    setP(img, 6, 21, sparkle);
    setP(img, 6, 20, Color{ 255, 240, 150, 200 }); setP(img, 6, 22, Color{ 255, 240, 150, 200 });
    setP(img, 5, 21, Color{ 255, 240, 150, 200 }); setP(img, 7, 21, Color{ 255, 240, 150, 200 });

    return img;
}

// =============================================================================
// 8. ĐỐNG TIỀN VÀNG KHO BÁU (gold_pile.png) - 32x32
// =============================================================================
Image generateGoldPile() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 60, 38, 8, 255 };
    Color goldDark  = Color{ 140, 90, 15, 255 };
    Color goldMid   = Color{ 215, 160, 30, 255 };
    Color goldLite  = Color{ 255, 225, 75, 255 };
    Color goldWhite = Color{ 255, 255, 180, 255 };
    Color sparkle   = Color{ 255, 255, 255, 255 };

    // Tầng đáy: đống tiền vàng trải rộng từ x=4..27, y=20..28
    auto drawSmallCoin = [&](int cx, int cy, int rx, int ry) {
        for (int y = cy - ry; y <= cy + ry; ++y) {
            for (int x = cx - rx; x <= cx + rx; ++x) {
                float dx = (float)(x - cx) / rx;
                float dy = (float)(y - cy) / ry;
                if (dx * dx + dy * dy <= 1.0f) {
                    if (dx * dx + dy * dy >= 0.75f) setP(img, x, y, outline);
                    else if (y < cy) setP(img, x, y, goldLite);
                    else setP(img, x, y, goldMid);
                }
            }
        }
        setP(img, cx - 1, cy - 1, goldWhite);
    };

    // Đống tiền vàng nhiều tầng
    drawSmallCoin(8,  25, 4, 3);
    drawSmallCoin(15, 26, 5, 3);
    drawSmallCoin(22, 25, 4, 3);
    drawSmallCoin(11, 22, 4, 3);
    drawSmallCoin(19, 22, 5, 3);
    drawSmallCoin(15, 18, 4, 3);
    drawSmallCoin(12, 15, 3, 2);
    drawSmallCoin(17, 14, 4, 3);
    drawSmallCoin(15, 11, 3, 2);

    // Tia sáng hào quang chớp nháy
    setP(img, 15, 6, sparkle);
    setP(img, 15, 5, goldWhite); setP(img, 15, 7, goldWhite);
    setP(img, 14, 6, goldWhite); setP(img, 16, 6, goldWhite);

    setP(img, 7, 16, sparkle);
    setP(img, 24, 18, sparkle);

    return img;
}

// =============================================================================
// 9. RƯƠNG KHO BÁU ĐÓNG KHÓA VÀNG (chest_gold_closed.png) - 32x32
// =============================================================================
Image generateChestClosed() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 25, 18, 12, 255 };
    Color woodDark  = Color{ 95, 55, 22, 255 };
    Color woodMid   = Color{ 145, 85, 38, 255 };
    Color woodLite  = Color{ 185, 115, 55, 255 };
    Color goldDark  = Color{ 135, 90, 15, 255 };
    Color goldMid   = Color{ 220, 165, 30, 255 };
    Color goldLite  = Color{ 255, 230, 85, 255 };
    Color lockHole  = Color{ 15, 10, 5, 255 };

    // Thân rương: x=5..26, y=10..27
    // Viền ngoài
    for (int x = 5; x <= 26; ++x) {
        setP(img, x, 10, outline);
        setP(img, x, 27, outline);
    }
    for (int y = 10; y <= 27; ++y) {
        setP(img, 5, y, outline);
        setP(img, 26, y, outline);
    }

    // Ruột gỗ
    for (int y = 11; y <= 26; ++y) {
        for (int x = 6; x <= 25; ++x) {
            if (y < 16) setP(img, x, y, (x % 4 == 0) ? woodDark : woodLite);
            else if (y == 16) setP(img, x, y, outline); // Rãnh nắp rương
            else setP(img, x, y, (x % 4 == 0) ? woodDark : woodMid);
        }
    }

    // 3 đai vàng kim gia cố rương (Trái x=7..9, Giữa x=14..17, Phải x=22..24)
    int straps[] = { 7, 8, 14, 15, 16, 17, 23, 24 };
    for (int sx : straps) {
        for (int y = 10; y <= 27; ++y) {
            if (y == 10 || y == 27) setP(img, sx, y, outline);
            else if (y < 16) setP(img, sx, y, goldLite);
            else setP(img, sx, y, goldMid);
        }
    }

    // Ổ khóa vàng hoàng gia ở chính giữa (x=14..17, y=15..19)
    for (int y = 15; y <= 20; ++y) {
        for (int x = 14; x <= 17; ++x) {
            setP(img, x, y, goldLite);
        }
    }
    setP(img, 13, 15, outline); setP(img, 18, 15, outline);
    setP(img, 13, 20, outline); setP(img, 18, 20, outline);
    // Lỗ khóa
    setP(img, 15, 17, lockHole);
    setP(img, 16, 17, lockHole);
    setP(img, 15, 18, lockHole);

    // Chân đế rương
    setP(img, 6, 28, outline); setP(img, 7, 28, outline);
    setP(img, 24, 28, outline); setP(img, 25, 28, outline);

    return img;
}

// =============================================================================
// 10. RƯƠNG KHO BÁU MỞ NẮP TỎA HÀO QUANG (chest_gold_open.png) - 32x32
// =============================================================================
Image generateChestOpen() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 25, 18, 12, 255 };
    Color woodDark  = Color{ 95, 55, 22, 255 };
    Color woodMid   = Color{ 145, 85, 38, 255 };
    Color goldMid   = Color{ 220, 165, 30, 255 };
    Color goldLite  = Color{ 255, 235, 90, 255 };
    Color glow      = Color{ 255, 250, 180, 220 };
    Color white     = Color{ 255, 255, 255, 255 };

    // Nắp rương mở ngửa lên trên: y=5..13
    for (int x = 5; x <= 26; ++x) {
        setP(img, x, 5, outline);
        setP(img, x, 12, outline);
    }
    for (int y = 6; y <= 11; ++y) {
        for (int x = 6; x <= 25; ++x) {
            setP(img, x, y, woodDark);
        }
    }

    // Ánh hào quang vàng kim tỏa ra từ lòng rương: y=10..16
    for (int y = 9; y <= 15; ++y) {
        for (int x = 8; x <= 23; ++x) {
            if ((x + y) % 2 == 0) setP(img, x, y, glow);
            else setP(img, x, y, goldLite);
        }
    }
    // Hạt châu báu lấp lánh
    setP(img, 11, 12, white); setP(img, 15, 10, white); setP(img, 20, 11, white);
    setP(img, 13, 8, white);  setP(img, 18, 7, white);

    // Thân rương dưới: y=16..27
    for (int y = 16; y <= 27; ++y) {
        for (int x = 5; x <= 26; ++x) {
            if (x == 5 || x == 26 || y == 27) setP(img, x, y, outline);
            else setP(img, x, y, woodMid);
        }
    }
    // Đai vàng thân rương
    int straps[] = { 7, 8, 14, 15, 16, 17, 23, 24 };
    for (int sx : straps) {
        for (int y = 16; y <= 26; ++y) {
            setP(img, sx, y, goldMid);
        }
    }

    return img;
}

// =============================================================================
// 11. BIỂU TƯỢNG CỬA HÀNG / TÚI TIỀN THƯƠNG NHÂN (icon_shop.png) - 32x32
// =============================================================================
Image generateIconShop() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline  = Color{ 35, 15, 15, 255 };
    Color pouchRed = Color{ 195, 35, 45, 255 };
    Color pouchLite= Color{ 235, 65, 75, 255 };
    Color pouchDark= Color{ 130, 20, 30, 255 };
    Color goldLite = Color{ 255, 230, 85, 255 };
    Color goldMid  = Color{ 215, 160, 25, 255 };
    Color white    = Color{ 255, 255, 255, 255 };

    // Thân túi tròn: tâm (16, 19), bán kính rx=9, ry=8
    for (int y = 11; y <= 27; ++y) {
        for (int x = 6; x <= 26; ++x) {
            float dx = (float)(x - 16) / 9.5f;
            float dy = (float)(y - 19) / 8.5f;
            if (dx * dx + dy * dy <= 1.0f) {
                if (dx * dx + dy * dy >= 0.85f) setP(img, x, y, outline);
                else if (x < 14 && y < 18) setP(img, x, y, pouchLite);
                else if (x > 18 || y > 21) setP(img, x, y, pouchDark);
                else setP(img, x, y, pouchRed);
            }
        }
    }

    // Cổ túi thắt nơ vàng kim: y=10..13, x=12..20
    for (int x = 12; x <= 20; ++x) {
        setP(img, x, 12, goldLite);
        setP(img, x, 13, goldMid);
    }
    // Nơ vàng 2 cánh
    setP(img, 10, 11, goldLite); setP(img, 11, 12, goldLite);
    setP(img, 21, 12, goldLite); setP(img, 22, 11, goldLite);

    // Miệng túi xòe ra ở trên: y=6..10
    for (int y = 6; y <= 10; ++y) {
        for (int x = 11; x <= 21; ++x) {
            if (x == 11 || x == 21 || y == 6) setP(img, x, y, outline);
            else setP(img, x, y, (x % 2 == 0) ? pouchLite : pouchRed);
        }
    }

    // Biểu tượng đồng tiền vàng đính giữa thân túi
    for (int y = 17; y <= 21; ++y) {
        for (int x = 14; x <= 18; ++x) {
            setP(img, x, y, goldLite);
        }
    }
    setP(img, 16, 18, white);
    setP(img, 16, 20, goldMid);

    return img;
}

// =============================================================================
// 12. BIỂU TƯỢNG ĐE RÈN CƯỜNG HÓA VŨ KHÍ (icon_forge.png) - 32x32
// =============================================================================
Image generateIconForge() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 20, 20, 25, 255 };
    Color ironDark  = Color{ 65, 70, 85, 255 };
    Color ironMid   = Color{ 110, 120, 140, 255 };
    Color ironLite  = Color{ 175, 185, 205, 255 };
    Color fireRed   = Color{ 245, 60, 20, 255 };
    Color fireGold  = Color{ 255, 200, 30, 255 };
    Color gold      = Color{ 255, 225, 75, 255 };
    Color spark     = Color{ 255, 255, 255, 255 };

    // Thân đe sắt (Anvil): y=14..26
    // Mặt đe phẳng trên cùng: y=14..16, x=5..27
    for (int x = 5; x <= 27; ++x) {
        setP(img, x, 14, outline);
        setP(img, x, 15, ironLite);
        setP(img, x, 16, ironMid);
    }
    // Mũi nhọn bên trái: (4, 15), (5, 15)
    setP(img, 4, 15, outline);
    // Eo đe thu hẹp: x=11..21, y=17..21
    for (int y = 17; y <= 21; ++y) {
        for (int x = 11; x <= 21; ++x) {
            if (x == 11 || x == 21) setP(img, x, y, outline);
            else setP(img, x, y, ironDark);
        }
    }
    // Đế đe loe rộng: x=7..25, y=22..26
    for (int y = 22; y <= 26; ++y) {
        for (int x = 7; x <= 25; ++x) {
            if (x == 7 || x == 25 || y == 26) setP(img, x, y, outline);
            else setP(img, x, y, ironMid);
        }
    }

    // Búa thợ rèn gõ trên đe: cán từ (26, 4) xuống (17, 13)
    for (int i = 0; i < 9; ++i) {
        setP(img, 25 - i, 5 + i, Color{ 120, 70, 30, 255 });
    }
    // Đầu búa vàng kim đặt tại (15, 12):
    for (int y = 10; y <= 13; ++y) {
        for (int x = 14; x <= 18; ++x) {
            setP(img, x, y, gold);
        }
    }

    // Tia lửa rèn tóe sáng (Sparks)
    setP(img, 13, 11, spark);
    setP(img, 19, 10, spark);
    setP(img, 12, 13, fireGold);
    setP(img, 20, 12, fireRed);

    return img;
}

// =============================================================================
// 13. KHIÊN HỘ MỆNH THÉP VIỀN VÀNG (armor_shield.png) - 32x32
// =============================================================================
Image generateArmorShield() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 20, 20, 25, 255 };
    Color goldMid   = Color{ 220, 165, 35, 255 };
    Color goldLite  = Color{ 255, 230, 85, 255 };
    Color steelDark = Color{ 85, 100, 125, 255 };
    Color steelMid  = Color{ 140, 160, 190, 255 };
    Color steelLite = Color{ 205, 225, 250, 255 };
    Color gemGreen  = Color{ 40, 225, 120, 255 };

    // Dáng khiên hình khiên hiệp sĩ cổ điển: x=6..25, y=5..27
    for (int y = 5; y <= 27; ++y) {
        int halfW = (y <= 16) ? 9 : (9 - (y - 16) * 9 / 11);
        int lx = 15 - halfW;
        int rx = 16 + halfW;
        for (int x = lx; x <= rx; ++x) {
            if (x == lx || x == rx || y == 5 || y == 27) {
                setP(img, x, y, outline);
            } else if (x == lx + 1 || x == rx - 1 || y == 6) {
                setP(img, x, y, goldLite);
            } else if (x == lx + 2 || x == rx - 2) {
                setP(img, x, y, goldMid);
            } else {
                if (x < 16) setP(img, x, y, steelLite);
                else setP(img, x, y, steelDark);
            }
        }
    }

    // Viên ngọc bích hộ mệnh ở tâm khiên (15, 14)
    for (int y = 12; y <= 16; ++y) {
        for (int x = 14; x <= 17; ++x) {
            setP(img, x, y, gemGreen);
        }
    }
    setP(img, 15, 13, Color{ 255, 255, 255, 255 });

    return img;
}

// =============================================================================
// 14. NHẪN MA THUẬT CỔ NGỮ (ring_power.png) - 32x32
// =============================================================================
Image generateRingPower() {
    Image img = GenImageColor(32, 32, BLANK);
    Color outline   = Color{ 30, 20, 10, 255 };
    Color goldDark  = Color{ 145, 95, 20, 255 };
    Color goldMid   = Color{ 220, 165, 30, 255 };
    Color goldLite  = Color{ 255, 230, 80, 255 };
    Color rubyDark  = Color{ 150, 15, 35, 255 };
    Color rubyMid   = Color{ 230, 35, 65, 255 };
    Color rubyLite  = Color{ 255, 110, 140, 255 };
    Color rubyWhite = Color{ 255, 240, 245, 255 };

    // Thân vòng nhẫn vàng kim elip: tâm (16, 18), rx=8, ry=6
    for (int y = 12; y <= 25; ++y) {
        for (int x = 7; x <= 25; ++x) {
            float dx = (float)(x - 16) / 8.5f;
            float dy = (float)(y - 18) / 6.5f;
            float d = dx * dx + dy * dy;
            if (d <= 1.0f && d >= 0.50f) {
                if (d > 0.88f) setP(img, x, y, outline);
                else if (y < 18) setP(img, x, y, goldLite);
                else setP(img, x, y, goldMid);
            }
        }
    }

    // Viên đá quý Ruby đỏ hình thoi giác cắt kim cương ở đỉnh nhẫn (x=16, y=10)
    int gemCoords[7][7] = {
        {0, 0, 0, 1, 0, 0, 0},
        {0, 0, 1, 2, 1, 0, 0},
        {0, 1, 2, 3, 2, 1, 0},
        {1, 2, 3, 4, 3, 2, 1},
        {0, 1, 2, 3, 2, 1, 0},
        {0, 0, 1, 2, 1, 0, 0},
        {0, 0, 0, 1, 0, 0, 0}
    };
    for (int gy = 0; gy < 7; ++gy) {
        for (int gx = 0; gx < 7; ++gx) {
            int val = gemCoords[gy][gx];
            int px = 13 + gx;
            int py = 7 + gy;
            if (val == 1) setP(img, px, py, outline);
            else if (val == 2) setP(img, px, py, rubyDark);
            else if (val == 3) setP(img, px, py, rubyMid);
            else if (val == 4) setP(img, px, py, rubyWhite);
        }
    }

    return img;
}

int main() {
    std::cout << "[Item Asset Generator] Dang khoi tao thu muc assets/items..." << std::endl;
    MKDIR("assets");
    MKDIR("assets/items");

    std::vector<std::pair<std::string, Image>> items = {
        { "assets/items/sword_steel.png",        generateSwordSteel() },
        { "assets/items/sword_mystic.png",       generateSwordMystic() },
        { "assets/items/potion_starter.png",     generatePotionStarter() },
        { "assets/items/potion_health.png",      generatePotionHealth() },
        { "assets/items/potion_strength.png",    generatePotionStrength() },
        { "assets/items/potion_elixir.png",      generatePotionElixir() },
        { "assets/items/gold_coin.png",          generateGoldCoin() },
        { "assets/items/gold_pile.png",          generateGoldPile() },
        { "assets/items/chest_gold_closed.png",  generateChestClosed() },
        { "assets/items/chest_gold_open.png",    generateChestOpen() },
        { "assets/items/icon_shop.png",          generateIconShop() },
        { "assets/items/icon_forge.png",         generateIconForge() },
        { "assets/items/armor_shield.png",       generateArmorShield() },
        { "assets/items/ring_power.png",         generateRingPower() }
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
