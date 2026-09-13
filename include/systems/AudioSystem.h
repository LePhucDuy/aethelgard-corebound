#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <string>
#include <unordered_map>
#include <raylib.h>

/**
 * @brief Lớp AudioSystem quản lý toàn bộ hiệu ứng âm thanh (SFX) trong game.
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Mẫu thiết kế Singleton Pattern (GoF Creational Pattern):
 *    - Đảm bảo chỉ có một thể hiện duy nhất quản lý thiết bị âm thanh phần cứng (Raylib Audio Device).
 *    - Hàm khởi tạo được đặt ở phạm vi private, ngăn chặn việc khởi tạo tùy tiện từ bên ngoài.
 *    - Phương thức tĩnh `getInstance()` trả về tham chiếu toàn cục an toàn luồng (Meyers' Singleton).
 * 
 * 2. Đóng gói & Quản lý Tài nguyên (RAII - Resource Acquisition Is Initialization):
 *    - Tự động sinh hoặc nạp các tệp âm thanh chuẩn WAV 16-bit PCM vào `assets/audio/`.
 *    - Tự động dọn dẹp và giải phóng tài nguyên âm thanh khi kết thúc trò chơi.
 */
class AudioSystem {
private:
    std::unordered_map<std::string, Sound> soundMap;
    bool initialized;
    bool deviceReady;

    // Singleton: Constructor & Destructor private
    AudioSystem();
    ~AudioSystem();

    // Ngăn chặn sao chép và gán (Chương 3)
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // Phương thức nội bộ sinh tệp âm thanh WAV tổng hợp nếu chưa tồn tại
    void ensureAudioAssetsExist();
    void generateWavFile(const std::string& filepath, int soundType);

public:
    // Trả về thể hiện duy nhất (Meyers' Singleton)
    static AudioSystem& getInstance();

    // Khởi tạo Audio Device và nạp toàn bộ danh mục âm thanh
    void init();

    // Phát âm thanh theo tên ("sword_slash", "hit_impact", "parry_clash", "shield_block", "poison_tick", "coin_pickup")
    void play(const std::string& name, float volume = 1.0f, float pitch = 1.0f);

    // Dọn dẹp và giải phóng thiết bị âm thanh
    void shutdown();

    bool isReady() const { return deviceReady; }
};

#endif // AUDIO_SYSTEM_H

