#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>


// 辅助函数：整数转字符串
std::string intToString(int number)  {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

// 游戏状态枚举
enum GameState {
    MainMenu,
    Playing,
    GameOver,
    GameIntro,
    AboutUs,
    LoadGame,
    SaveGame
};

// 菜单项类
class MenuItem {
public:
    sf::Text text;
    sf::RectangleShape background;
    bool isSelected;
    
    MenuItem(const std::string& str, sf::Font& font, unsigned int size) {
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        isSelected = false;
        
        sf::FloatRect bounds = text.getLocalBounds();
        background.setSize(sf::Vector2f(bounds.width + 60, bounds.height + 32));
        background.setFillColor(sf::Color(0, 0, 0, 150));
        background.setOutlineThickness(2);
        background.setOutlineColor(sf::Color(100, 100, 100, 100));
    }
    
    void setPosition(float x, float y) {
        sf::FloatRect bounds = text.getLocalBounds();
        background.setPosition(x - 20, y - 10);
        text.setPosition(x, y);
    }
    
    sf::FloatRect getBounds() const {
        return background.getGlobalBounds();
    }
    
    void setSelected(bool selected) {
        isSelected = selected;
        if (selected) {
            background.setFillColor(sf::Color(30, 60, 120, 200));
            background.setOutlineColor(sf::Color(100, 200, 255));
            text.setFillColor(sf::Color(255, 255, 150));
            text.setStyle(sf::Text::Bold);
        } else {
            background.setFillColor(sf::Color(0, 0, 0, 150));
            background.setOutlineColor(sf::Color(100, 100, 100, 100));
            text.setFillColor(sf::Color::White);
            text.setStyle(sf::Text::Regular);
        }
    }
};

class Bullet {
public:
    sf::Sprite sprite;
    sf::Vector2f velocity;
    float size;
    int textureIndex;

    void update() {
        sprite.move(velocity);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

// 技能点类
class PowerUp {
public:
    sf::Sprite sprite;
    sf::Vector2f position;
    float rotationSpeed;
    float scale;
    bool active;
    sf::Clock lifeClock;
    
    PowerUp() : rotationSpeed(50.0f), scale(1.5f), active(false) {}
    
    void update(float deltaTime) {
        if (active) {
            sprite.rotate(rotationSpeed * deltaTime);
            
            scale = 0.4f + 0.1f * std::sin(lifeClock.getElapsedTime().asSeconds() * 2.0f);
            sprite.setScale(scale, scale);
            
            if (lifeClock.getElapsedTime().asSeconds() > 3.5f) {
                active = false;
            }
        }
    }
    
    void spawn(const sf::Vector2f& pos, sf::Texture& texture) {
        position = pos;
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
        sprite.setPosition(position);
        sprite.setScale(0.5f, 0.5f);
        active = true;
        lifeClock.restart();
    }
    
    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

class Player {
public:
    sf::Sprite sprite;
    bool isMoving;
    sf::Vector2f targetPosition;
    float moveSpeed;
    int powerUpCount;

    Player() : isMoving(false), moveSpeed(8.0f), powerUpCount(0) {}

    void update() {
        if (isMoving) {
            sf::Vector2f direction = targetPosition - sprite.getPosition();
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            
            if (distance > moveSpeed) {
                direction /= distance;
                sprite.move(direction * moveSpeed);
            } else {
                sprite.setPosition(targetPosition);
                isMoving = false;
            }
        }
    }

    void moveTo(const sf::Vector2f& position) {
        targetPosition = position;
        isMoving = true;
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

// 存档数据结构
struct GameSave {
    int score;
    float spawnInterval;
    float playerX;
    float playerY;
    int powerUpCount;
    float powerUpTimer;
    std::string saveTime;
    int bulletsCleared;
    int powerUpsCollected;
    int playTime;
};

// 获取当前时间字符串
std::string getCurrentTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    return std::string(buf);
}

// 保存游戏状态到文件 - 兼容旧版本
bool saveGame(const GameSave& saveData, int slot = 1) {
    // 构建文件名
    std::string filename = "savegame_" + intToString(slot) + ".dat";
    
    // 使用c_str()转换为C风格字符串
    std::ofstream file(filename.c_str(), std::ios::binary);
    
    if (!file.is_open()) {
        std::cout << "Failed to create save file: " << filename << std::endl;
        return false;
    }
    
    // 写入文件头标识
    const char* header = "PLANES_WARS_SAVE";
    file.write(header, 16);
    
    // 写入版本号
    int version = 2;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    
    // 写入所有数据
    file.write(reinterpret_cast<const char*>(&saveData.score), sizeof(saveData.score));
    file.write(reinterpret_cast<const char*>(&saveData.spawnInterval), sizeof(saveData.spawnInterval));
    file.write(reinterpret_cast<const char*>(&saveData.playerX), sizeof(saveData.playerX));
    file.write(reinterpret_cast<const char*>(&saveData.playerY), sizeof(saveData.playerY));
    file.write(reinterpret_cast<const char*>(&saveData.powerUpCount), sizeof(saveData.powerUpCount));
    file.write(reinterpret_cast<const char*>(&saveData.powerUpTimer), sizeof(saveData.powerUpTimer));
    file.write(reinterpret_cast<const char*>(&saveData.bulletsCleared), sizeof(saveData.bulletsCleared));
    file.write(reinterpret_cast<const char*>(&saveData.powerUpsCollected), sizeof(saveData.powerUpsCollected));
    file.write(reinterpret_cast<const char*>(&saveData.playTime), sizeof(saveData.playTime));
    
    // 写入时间字符串
    size_t timeLength = saveData.saveTime.length();
    file.write(reinterpret_cast<const char*>(&timeLength), sizeof(timeLength));
    file.write(saveData.saveTime.c_str(), timeLength);
    
    file.close();
    
    // 验证文件是否成功写入
    std::ifstream checkFile(filename.c_str(), std::ios::binary | std::ios::ate);
    if (checkFile.is_open()) {
        std::streamsize size = checkFile.tellg();
        checkFile.close();
        if (size > 0) {
            std::cout << "Game saved successfully to slot " << slot << "!" << std::endl;
            return true;
        }
    }
    
    std::cout << "Failed to save game to slot " << slot << "!" << std::endl;
    return false;
}

// 从文件加载游戏状态 - 兼容旧版本
GameSave loadGame(int slot = 1) {
    GameSave saveData;
    saveData.score = 0;
    saveData.spawnInterval = 1.0f;
    saveData.playerX = 400.0f;
    saveData.playerY = 400.0f;
    saveData.powerUpCount = 0;
    saveData.powerUpTimer = 0.0f;
    saveData.bulletsCleared = 0;
    saveData.powerUpsCollected = 0;
    saveData.playTime = 0;
    saveData.saveTime = "No save found";
    
    // 构建文件名
    std::string filename = "savegame_" + intToString(slot) + ".dat";
    
    // 使用c_str()转换为C风格字符串
    std::ifstream file(filename.c_str(), std::ios::binary);
    
    if (!file.is_open()) {
        std::cout << "No save file found for slot " << slot << "!" << std::endl;
        return saveData;
    }
    
    try {
        // 检查文件头
        char header[17] = {0};
        file.read(header, 16);
        if (std::string(header) != "PLANES_WARS_SAVE") {
            std::cout << "Invalid save file format for slot " << slot << "!" << std::endl;
            return saveData;
        }
        
        // 读取版本号
        int version = 0;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        
        if (version >= 1) {
            // 读取基本数据
            file.read(reinterpret_cast<char*>(&saveData.score), sizeof(saveData.score));
            file.read(reinterpret_cast<char*>(&saveData.spawnInterval), sizeof(saveData.spawnInterval));
            
            if (version >= 2) {
                // 版本2包含更多数据
                file.read(reinterpret_cast<char*>(&saveData.playerX), sizeof(saveData.playerX));
                file.read(reinterpret_cast<char*>(&saveData.playerY), sizeof(saveData.playerY));
                file.read(reinterpret_cast<char*>(&saveData.powerUpCount), sizeof(saveData.powerUpCount));
                file.read(reinterpret_cast<char*>(&saveData.powerUpTimer), sizeof(saveData.powerUpTimer));
                file.read(reinterpret_cast<char*>(&saveData.bulletsCleared), sizeof(saveData.bulletsCleared));
                file.read(reinterpret_cast<char*>(&saveData.powerUpsCollected), sizeof(saveData.powerUpsCollected));
                file.read(reinterpret_cast<char*>(&saveData.playTime), sizeof(saveData.playTime));
            }
            
            // 读取时间字符串
            size_t timeLength = 0;
            file.read(reinterpret_cast<char*>(&timeLength), sizeof(timeLength));
            
            if (timeLength > 0 && timeLength < 2000) {
                char* buffer = new char[timeLength + 1];
                file.read(buffer, timeLength);
                buffer[timeLength] = '\0';
                saveData.saveTime = std::string(buffer);
                delete[] buffer;
            }
        }
        
        file.close();
        std::cout << "Game loaded successfully from slot " << slot << "!" << std::endl;
        
    } catch (...) {
        std::cout << "Error loading save file for slot " << slot << "!" << std::endl;
    }
    
    return saveData;
}

// 检查存档是否存在
bool saveFileExists(int slot = 1) {
    std::string filename = "savegame_" + intToString(slot) + ".dat";
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (file.is_open()) {
        file.close();
        return true;
    }
    return false;
}

// 删除存档
bool deleteSaveFile(int slot = 1) {
    std::string filename = "savegame_" + intToString(slot) + ".dat";
    if (remove(filename.c_str()) == 0) {
        std::cout << "Save file deleted for slot " << slot << std::endl;
        return true;
    }
    return false;
}

int main() {
	
    std::srand(static_cast<unsigned int>(std::time(NULL)));
    const unsigned int windowWidth = 800;
    const unsigned int windowHeight = 800;
    
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Planes Wars! Come on, man~ ~");
    window.setFramerateLimit(60);

    // 纹理加载
    sf::Texture playerTexture;
    sf::Texture bulletTextures[3];
    sf::Texture backgroundTexture;
    sf::Texture menuBackgroundTexture;
    sf::Texture powerUpTexture;
    
    // 加载玩家飞机纹理
    if (!playerTexture.loadFromFile("player_plane.png.png")) {
        std::cout << "Failed to load player_plane.png.png, using default texture" << std::endl;
        sf::Image img;
        img.create(50, 50, sf::Color::Red);
        playerTexture.loadFromImage(img);
    }
    
    // 加载3种子弹纹理
    std::string bulletFiles[3] = {
        "zidan1.jpg",
        "zidan2.jpg",
        "zidan3.jpg"
    };
    
    for (int i = 0; i < 3; i++) {
        if (!bulletTextures[i].loadFromFile(bulletFiles[i].c_str())) {
            std::cout << "Failed to load bullet texture " << i+1 << ", using default texture" << std::endl;
            sf::Image img;
            img.create(10, 10, sf::Color(rand() % 255, rand() % 255, rand() % 255));
            bulletTextures[i].loadFromImage(img);
        }
    }
    
    // 加载游戏背景
    if (!backgroundTexture.loadFromFile("backgr2.jpg")) {
        std::cout << "Failed to load background.jpg.jpg, using default texture" << std::endl;
        sf::Image img;
        img.create(windowWidth, windowHeight, sf::Color::Blue);
        backgroundTexture.loadFromImage(img);
    }
    
    // 加载菜单背景
    if (!menuBackgroundTexture.loadFromFile("background.jpg.jpg")) {
        std::cout << "Failed to load up.png for menu, using default" << std::endl;
        sf::Image img;
        img.create(windowWidth, windowHeight, sf::Color(30, 60, 60));
        menuBackgroundTexture.loadFromImage(img);
    }
    
    // 加载技能点纹理
    if (!powerUpTexture.loadFromFile("powerup.png")) {
        std::cout << "Failed to load powerup.png, creating default" << std::endl;
        sf::Image img;
        img.create(64, 64, sf::Color::Transparent);
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                float x = i - 32.0f;
                float y = j - 32.0f;
                float dist = std::sqrt(x*x + y*y);
                if (dist < 30 && dist > 25) {
                    img.setPixel(i, j, sf::Color::Yellow);
                } else if (dist < 15) {
                    img.setPixel(i, j, sf::Color(255, 165, 0));
                }
            }
        }
        powerUpTexture.loadFromImage(img);
    }

    // 音频加载
    sf::SoundBuffer gameOverSoundBuffer;
    sf::SoundBuffer moveSoundBuffer;
    sf::SoundBuffer menuSelectSoundBuffer;
    sf::SoundBuffer powerUpSoundBuffer;
    sf::SoundBuffer saveSoundBuffer;
    
    // 加载游戏结束音乐
    if (!gameOverSoundBuffer.loadFromFile("game_over.wav")) {
        std::cout << "Failed to load game_over.wav" << std::endl;
    }
    
    // 加载移动音效
    if (!moveSoundBuffer.loadFromFile("move.wav")) {
        std::cout << "Failed to load move.wav" << std::endl;
    }
    
    if (!menuSelectSoundBuffer.loadFromFile("menu_select.wav")) {
        std::cout << "Failed to load menu_select.wav" << std::endl;
        menuSelectSoundBuffer = moveSoundBuffer;
    }
    
    // 加载技能点音效
    if (!powerUpSoundBuffer.loadFromFile("powerup.m4a")) {
        std::cout << "Failed to load powerup.m4a, using move sound" << std::endl;
        powerUpSoundBuffer = moveSoundBuffer;
    }
    
    // 加载保存音效
    if (!saveSoundBuffer.loadFromFile("save_sound.wav")) {
        saveSoundBuffer = menuSelectSoundBuffer;
        std::cout << "Using menu select sound for save sound" << std::endl;
    }

    sf::Sound gameOverSound;
    sf::Sound moveSound;
    sf::Sound menuSelectSound;
    sf::Sound powerUpSound;
    sf::Sound saveSound;
    gameOverSound.setBuffer(gameOverSoundBuffer);
    moveSound.setBuffer(moveSoundBuffer);
    menuSelectSound.setBuffer(menuSelectSoundBuffer);
    powerUpSound.setBuffer(powerUpSoundBuffer);
    saveSound.setBuffer(saveSoundBuffer);
    
    sf::Music backgroundMusic;
    // 加载背景音乐
    if (backgroundMusic.openFromFile("background_music.wav")) {
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(50);
    } else {
        std::cout << "Failed to load background_music.wav" << std::endl;
    }

    // 游戏对象
    Player player;
    player.sprite.setScale(0.10f, 0.10f);
    player.sprite.setTexture(playerTexture);
    player.sprite.setOrigin(playerTexture.getSize().x / 2.0f, playerTexture.getSize().y / 2.0f);
    player.sprite.setPosition(windowWidth / 2.0f, windowHeight / 2.0f);

    std::vector<Bullet> bullets;
    PowerUp powerUp;
    
    // 背景精灵
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    float bgScaleX = static_cast<float>(windowWidth) / backgroundTexture.getSize().x;
    float bgScaleY = static_cast<float>(windowHeight) / backgroundTexture.getSize().y;
    backgroundSprite.setScale(bgScaleX, bgScaleY);
    
    sf::Sprite menuBackgroundSprite;
    menuBackgroundSprite.setTexture(menuBackgroundTexture);
    float menuBgScaleX = static_cast<float>(windowWidth) / menuBackgroundTexture.getSize().x;
    float menuBgScaleY = static_cast<float>(windowHeight) / menuBackgroundTexture.getSize().y;
    menuBackgroundSprite.setScale(menuBgScaleX, menuBgScaleY);

    // 游戏状态
    GameState gameState = MainMenu;
    int score = 0;
    float spawnInterval = 1.0f;
    float minSpawnInterval = 0.8f;
    float difficultyIncreaseRate = 0.01f;
    int bulletsCleared = 0;
    int powerUpsCollected = 0;
    int playTimeSeconds = 0;
    sf::Clock gameTimeClock;
    
    // 技能点计时器
    sf::Clock powerUpSpawnClock;
    const float powerUpSpawnInterval = 15.0f;
    
    // 字体
    sf::Font font;
    if (!font.loadFromFile("ShanHaiJiGuSongKe-JianFan-2.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cout << "Failed to load font, using default" << std::endl;
            sf::Font defaultFont;
            font = defaultFont;
        }
    }
    
    // 主菜单项
    std::vector<MenuItem> menuItems;
    menuItems.push_back(MenuItem("Start Game", font, 48));
    menuItems.push_back(MenuItem("Load Game", font, 48));
    menuItems.push_back(MenuItem("Save Game", font, 48));
    menuItems.push_back(MenuItem("Game Introduction", font, 48));
    menuItems.push_back(MenuItem("About Us", font, 48));
    menuItems.push_back(MenuItem("Exit Game", font, 48));
    
    // 设置菜单位置
    float startY = 350;
    float spacing = 75;
    for (size_t i = 0; i < menuItems.size(); ++i) {
        sf::FloatRect bounds = menuItems[i].text.getLocalBounds();
        menuItems[i].setPosition(windowWidth / 2 - bounds.width / 2, startY + i * spacing);
    }
    
    // 标题文本
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("PLANES WARS");
    titleText.setCharacterSize(90);
    titleText.setFillColor(sf::Color(255, 165, 0));
    titleText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    
    // 添加标题阴影
    sf::Text titleShadow = titleText;
    titleShadow.setFillColor(sf::Color(0, 0, 0, 150));
    titleShadow.setPosition(163, 123);
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2, titleBounds.height / 2);
    titleText.setPosition(windowWidth / 2, 150);
    
    // 副标题
    sf::Text subtitleText;
    subtitleText.setFont(font);
    subtitleText.setString("Come on, man ! ! !");
    subtitleText.setCharacterSize(45);
    subtitleText.setFillColor(sf::Color(255, 200, 100));
    subtitleText.setStyle(sf::Text::Italic);
    sf::FloatRect subtitleBounds = subtitleText.getLocalBounds();
    subtitleText.setOrigin(subtitleBounds.width / 2, subtitleBounds.height / 2);
    subtitleText.setPosition(windowWidth / 2, 220);
    
    // 游戏介绍文本
    sf::Text introText;
    introText.setFont(font);
    introText.setString(
        "GAME INTRODUCTION\n\n"
        "Game Objective:\n"
        "Control your plane to avoid incoming bullets!\n"
        "Survive as long as possible and score points.\n"
        "Controls:\n"
        "- Left Click: Move plane to mouse position\n"
        "- ESC: Return to main menu\n"
        "- SPACE: Use power-up to clear screen\n"
        "New Features:\n"
        "- Multiple bullet types\n"
        "- Power-ups spawn every 15 seconds\n"
        "- Collect power-ups to clear all bullets\n"
        "- Advanced save/load system\n\n"
        "Press ESC to return to main menu"
    );
    introText.setCharacterSize(21);
    introText.setFillColor(sf::Color::White);
    introText.setPosition(50, 300);
    
    // 关于我们文本
    sf::Text aboutTitle;
    aboutTitle.setFont(font);
    aboutTitle.setString("ABOUT US");
    aboutTitle.setCharacterSize(60);
    aboutTitle.setFillColor(sf::Color(255, 165, 0));
    aboutTitle.setStyle(sf::Text::Bold);
    aboutTitle.setPosition(windowWidth / 2 - aboutTitle.getLocalBounds().width / 2, 100);
    
    sf::Text aboutContent;
    aboutContent.setFont(font);
    aboutContent.setString(
        "Development Team:\n"
        "- Project Manager: Liu Daihong and Luo Xiangqiang\n"
        "- Lead Programmer: Liu Daihong and Luo Xiangqiang\n"
        "- Game Designer: Liu Daihong and Luo Xiangqiang\n"
        "- Sound Designer: Liu Daihong and Luo Xiangqiang\n"
        "- Art Designer: Liu Daihong and Luo Xiangqiang\n\n"
        "New Features Added:\n"
        "- Multiple bullet textures\n"
        "- Power-up system\n"
        "- Advanced save/load system\n"
        "- 3 save slots\n"
        "- Detailed save information\n\n"
        "Press ESC to return to main menu"
    );
    aboutContent.setCharacterSize(24);
    aboutContent.setFillColor(sf::Color::White);
    aboutContent.setPosition(100, 200);
    
    // 加载游戏界面
    sf::Text loadGameTitle;
    loadGameTitle.setFont(font);
    loadGameTitle.setString("LOAD GAME");
    loadGameTitle.setCharacterSize(60);
    loadGameTitle.setFillColor(sf::Color(255, 165, 0));
    loadGameTitle.setStyle(sf::Text::Bold);
    loadGameTitle.setPosition(windowWidth / 2 - loadGameTitle.getLocalBounds().width / 2, 50);
    
    // 保存游戏界面
    sf::Text saveGameTitle;
    saveGameTitle.setFont(font);
    saveGameTitle.setString("SAVE GAME");
    saveGameTitle.setCharacterSize(60);
    saveGameTitle.setFillColor(sf::Color(255, 165, 0));
    saveGameTitle.setStyle(sf::Text::Bold);
    saveGameTitle.setPosition(windowWidth / 2 - saveGameTitle.getLocalBounds().width / 2, 50);
    
    // 存档槽相关变量
    const int SAVE_SLOTS = 3;
    sf::RectangleShape saveSlots[SAVE_SLOTS];
    sf::Text saveSlotTexts[SAVE_SLOTS];
    sf::Text saveSlotDetails[SAVE_SLOTS];
    sf::Text saveSlotEmptyText[SAVE_SLOTS];
    int selectedSaveSlot = 0;
    bool isSavingMode = false;
    
    // 初始化存档槽
    for (int i = 0; i < SAVE_SLOTS; i++) {
        // 存档槽背景
        saveSlots[i].setSize(sf::Vector2f(600, 120));
        saveSlots[i].setFillColor(sf::Color(40, 40, 60, 180));
        saveSlots[i].setOutlineThickness(3);
        saveSlots[i].setOutlineColor(sf::Color(100, 100, 150));
        saveSlots[i].setPosition(100, 150 + i * 150);
        
        // 存档槽标题
        saveSlotTexts[i].setFont(font);
        saveSlotTexts[i].setString("SAVE SLOT " + intToString(i + 1));
        saveSlotTexts[i].setCharacterSize(28);
        saveSlotTexts[i].setFillColor(sf::Color(200, 200, 255));
        saveSlotTexts[i].setPosition(120, 160 + i * 150);
        
        // 存档详细信息
        saveSlotDetails[i].setFont(font);
        saveSlotDetails[i].setCharacterSize(20);
        saveSlotDetails[i].setFillColor(sf::Color(180, 180, 220));
        saveSlotDetails[i].setPosition(120, 195 + i * 150);
        
        // 空存档文本
        saveSlotEmptyText[i].setFont(font);
        saveSlotEmptyText[i].setString("EMPTY SLOT");
        saveSlotEmptyText[i].setCharacterSize(24);
        saveSlotEmptyText[i].setFillColor(sf::Color(150, 150, 150, 180));
        saveSlotEmptyText[i].setStyle(sf::Text::Italic);
        saveSlotEmptyText[i].setPosition(120, 195 + i * 150);
    }
    
    // 操作提示文本
    sf::Text saveLoadHint;
    saveLoadHint.setFont(font);
    saveLoadHint.setCharacterSize(22);
    saveLoadHint.setFillColor(sf::Color(100, 255, 100));
    saveLoadHint.setPosition(100, 620);
    
    sf::Text deleteHint;
    deleteHint.setFont(font);
    deleteHint.setCharacterSize(20);
    deleteHint.setFillColor(sf::Color(255, 100, 100));
    deleteHint.setPosition(100, 650);
    
    // 游戏内文本
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition(15, 15);
    
    // 技能点数量显示
    sf::Text powerUpText;
    powerUpText.setFont(font);
    powerUpText.setCharacterSize(26);
    powerUpText.setFillColor(sf::Color(255, 255, 100));
    powerUpText.setStyle(sf::Text::Bold);
    powerUpText.setPosition(15, 65);
    
    // 游戏时间显示
    sf::Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(24);
    timeText.setFillColor(sf::Color(150, 255, 150));
    timeText.setStyle(sf::Text::Bold);
    timeText.setPosition(15, 105);
    
    // 添加分数背景
    sf::RectangleShape scoreBackground(sf::Vector2f(210, 50));
    scoreBackground.setFillColor(sf::Color(0, 0, 0, 150));
    scoreBackground.setPosition(10, 10);
    
    // 技能点背景
    sf::RectangleShape powerUpBackground(sf::Vector2f(380, 40));
    powerUpBackground.setFillColor(sf::Color(0, 0, 0, 150));
    powerUpBackground.setPosition(10, 70);
    
    // 时间背景
    sf::RectangleShape timeBackground(sf::Vector2f(250, 40));
    timeBackground.setFillColor(sf::Color(0, 0, 0, 150));
    timeBackground.setPosition(10, 110);
    
    // 游戏结束文本
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(80);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("GAME OVER");
    gameOverText.setStyle(sf::Text::Bold);
    sf::FloatRect textRect = gameOverText.getLocalBounds();
    gameOverText.setOrigin(textRect.width / 2, textRect.height / 2);
    gameOverText.setPosition(windowWidth / 2, windowHeight / 2 - 150);
    
    // 游戏结束背景
    sf::RectangleShape gameOverBackground(sf::Vector2f(600, 450));
    gameOverBackground.setFillColor(sf::Color(0, 0, 0, 200));
    gameOverBackground.setOutlineThickness(3);
    gameOverBackground.setOutlineColor(sf::Color::Red);
    gameOverBackground.setPosition(windowWidth / 2 - 300, windowHeight / 2 - 200);
    
    sf::Clock spawnClock;
    sf::Clock deltaClock;
    int selectedMenuItem = 0;
    
    // 设置初始菜单项状态
    menuItems[selectedMenuItem].setSelected(true);

    while (window.isOpen()) {
        float deltaTime = deltaClock.restart().asSeconds();
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
                
            // 键盘输入处理
            if (event.type == sf::Event::KeyPressed) {
                if (gameState == MainMenu) {
                    if (event.key.code == sf::Keyboard::Up) {
                        menuItems[selectedMenuItem].setSelected(false);
                        selectedMenuItem = (selectedMenuItem - 1 + menuItems.size()) % menuItems.size();
                        menuItems[selectedMenuItem].setSelected(true);
                        menuSelectSound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        menuItems[selectedMenuItem].setSelected(false);
                        selectedMenuItem = (selectedMenuItem + 1) % menuItems.size();
                        menuItems[selectedMenuItem].setSelected(true);
                        menuSelectSound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                        // 处理菜单项选择
                        switch (selectedMenuItem) {
                            case 0: // Start Game
                                gameState = Playing;
                                score = 0;
                                spawnInterval = 1.0f;
                                bullets.clear();
                                player.powerUpCount = 0;
                                bulletsCleared = 0;
                                powerUpsCollected = 0;
                                playTimeSeconds = 0;
                                player.sprite.setPosition(windowWidth / 2.0f, windowHeight / 2.0f);
                                powerUpSpawnClock.restart();
                                gameTimeClock.restart();
                                if (backgroundMusic.getStatus() != sf::Music::Playing) {
                                    backgroundMusic.play();
                                }
                                break;
                            case 1: // Load Game
                                gameState = LoadGame;
                                isSavingMode = false;
                                selectedSaveSlot = 0;
                                // 更新存档槽显示
                                for (int i = 0; i < SAVE_SLOTS; i++) {
                                    GameSave saveInfo = loadGame(i + 1);
                                    if (saveInfo.score > 0) {
                                        std::stringstream details;
                                        details << "Score: " << saveInfo.score 
                                                << "  |  Time: " << saveInfo.playTime << "s"
                                                << "\nPower-ups: " << saveInfo.powerUpCount
                                                << "  |  Saved: " << saveInfo.saveTime;
                                        saveSlotDetails[i].setString(details.str());
                                    }
                                }
                                break;
                            case 2: // Save Game
                                if (gameState == Playing) {
                                    gameState = SaveGame;
                                    isSavingMode = true;
                                    selectedSaveSlot = 0;
                                }
                                break;
                            case 3: // Game Introduction
                                gameState = GameIntro;
                                break;
                            case 4: // About Us
                                gameState = AboutUs;
                                break;
                            case 5: // Exit Game
                                window.close();
                                break;
                        }
                    }
                }
                else if (gameState == LoadGame || gameState == SaveGame) {
                    // 存档/加载界面的键盘控制
                    if (event.key.code == sf::Keyboard::Up) {
                        selectedSaveSlot = (selectedSaveSlot - 1 + SAVE_SLOTS) % SAVE_SLOTS;
                        menuSelectSound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        selectedSaveSlot = (selectedSaveSlot + 1) % SAVE_SLOTS;
                        menuSelectSound.play();
                    }
                    else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                        if (isSavingMode) {
                            // 保存游戏到选中的槽位
                            GameSave saveData;
                            saveData.score = score;
                            saveData.spawnInterval = spawnInterval;
                            saveData.playerX = player.sprite.getPosition().x;
                            saveData.playerY = player.sprite.getPosition().y;
                            saveData.powerUpCount = player.powerUpCount;
                            saveData.powerUpTimer = powerUpSpawnClock.getElapsedTime().asSeconds();
                            saveData.bulletsCleared = bulletsCleared;
                            saveData.powerUpsCollected = powerUpsCollected;
                            saveData.playTime = playTimeSeconds;
                            saveData.saveTime = getCurrentTime();
                            
                            if (saveGame(saveData, selectedSaveSlot + 1)) {
                                saveSound.play();
                                gameState = Playing;
                            }
                        } else {
                            // 加载选中的存档
                            GameSave loadedData = loadGame(selectedSaveSlot + 1);
                            if (loadedData.score > 0) {
                                score = loadedData.score;
                                spawnInterval = loadedData.spawnInterval;
                                player.sprite.setPosition(loadedData.playerX, loadedData.playerY);
                                player.powerUpCount = loadedData.powerUpCount;
                                bulletsCleared = loadedData.bulletsCleared;
                                powerUpsCollected = loadedData.powerUpsCollected;
                                                                playTimeSeconds = loadedData.playTime;
                                
                                // 恢复技能点计时器
                                powerUpSpawnClock.restart();
                                
                                gameState = Playing;
                                bullets.clear();
                                if (backgroundMusic.getStatus() != sf::Music::Playing) {
                                    backgroundMusic.play();
                                }
                                
                                // 重置游戏时间计时器
                                gameTimeClock.restart();
                            }
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Delete) {
                        // 删除选中的存档
                        if (deleteSaveFile(selectedSaveSlot + 1)) {
                            menuSelectSound.play();
                            // 更新存档槽显示
                            for (int i = 0; i < SAVE_SLOTS; i++) {
                                GameSave saveInfo = loadGame(i + 1);
                                if (saveInfo.score > 0) {
                                    std::stringstream details;
                                    details << "Score: " << saveInfo.score 
                                            << "  |  Time: " << saveInfo.playTime << "s"
                                            << "\nPower-ups: " << saveInfo.powerUpCount
                                            << "  |  Saved: " << saveInfo.saveTime;
                                    saveSlotDetails[i].setString(details.str());
                                } else {
                                    saveSlotDetails[i].setString("");
                                }
                            }
                        }
                    }
                }
                
                // ESC键处理（所有状态都有效）
                if (event.key.code == sf::Keyboard::Escape) {
                    if (gameState == Playing) {
                        // 在游戏中按ESC返回主菜单
                        gameState = MainMenu;
                        backgroundMusic.stop();
                        bullets.clear();
                        player.sprite.setPosition(windowWidth / 2.0f, windowHeight / 2.0f);
                    }
                    else if (gameState == GameIntro || gameState == AboutUs || 
                             gameState == LoadGame || gameState == SaveGame) {
                        gameState = MainMenu;
                    }
                }
                
                // 空格键使用技能点（在游戏中）
                if (gameState == Playing && event.key.code == sf::Keyboard::Space) {
                    if (player.powerUpCount > 0) {
                        player.powerUpCount--;
                        bulletsCleared += bullets.size();
                        bullets.clear();
                        powerUpSound.play();
                        score += 350;
                    }
                }
            }
                
            // 鼠标点击处理
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    
                    if (gameState == MainMenu) {
                        for (size_t i = 0; i < menuItems.size(); ++i) {
                            if (menuItems[i].getBounds().contains(mousePos)) {
                                menuItems[selectedMenuItem].setSelected(false);
                                selectedMenuItem = i;
                                menuItems[selectedMenuItem].setSelected(true);
                                
                                switch (i) {
                                    case 0: // Start Game
                                        gameState = Playing;
                                        score = 0;
                                        spawnInterval = 1.0f;
                                        bullets.clear();
                                        player.powerUpCount = 0;
                                        bulletsCleared = 0;
                                        powerUpsCollected = 0;
                                        playTimeSeconds = 0;
                                        player.sprite.setPosition(windowWidth / 2.0f, windowHeight / 2.0f);
                                        powerUpSpawnClock.restart();
                                        gameTimeClock.restart();
                                        if (backgroundMusic.getStatus() != sf::Music::Playing) {
                                            backgroundMusic.play();
                                        }
                                        break;
                                    case 1: // Load Game
                                        gameState = LoadGame;
                                        isSavingMode = false;
                                        selectedSaveSlot = 0;
                                        // 更新存档槽显示
                                        for (int i = 0; i < SAVE_SLOTS; i++) {
                                            GameSave saveInfo = loadGame(i + 1);
                                            if (saveInfo.score > 0) {
                                                std::stringstream details;
                                                details << "Score: " << saveInfo.score 
                                                        << "  |  Time: " << saveInfo.playTime << "s"
                                                        << "\nPower-ups: " << saveInfo.powerUpCount
                                                        << "  |  Saved: " << saveInfo.saveTime;
                                                saveSlotDetails[i].setString(details.str());
                                            }
                                        }
                                        break;
                                    case 2: // Save Game
                                        {
                                            gameState = SaveGame;
                                            isSavingMode = true;
                                            selectedSaveSlot = 0;
                                        }
                                        break;
                                    case 3: // Game Introduction
                                        gameState = GameIntro;
                                        break;
                                    case 4: // About Us
                                        gameState = AboutUs;
                                        break;
                                    case 5: // Exit Game
                                        window.close();
                                        break;
                                }
                                break;
                            }
                        }
                    }
                    else if (gameState == Playing) {
                        player.moveTo(mousePos);
                        moveSound.play();
                    }
                    else if (gameState == GameOver) {
                        gameState = Playing;
                        player.sprite.setPosition(windowWidth / 2.0f, windowHeight / 2.0f);
                        bullets.clear();
                        player.powerUpCount = 0;
                        bulletsCleared = 0;
                        powerUpsCollected = 0;
                        playTimeSeconds = 0;
                        score = 0;
                        spawnInterval = 1.0f;
                        powerUpSpawnClock.restart();
                        gameTimeClock.restart();
                        if (backgroundMusic.getStatus() != sf::Music::Playing) {
                            backgroundMusic.play();
                        }
                    }
                    else if (gameState == LoadGame || gameState == SaveGame) {
                        // 点击选择存档槽
                        for (int i = 0; i < SAVE_SLOTS; i++) {
                            if (saveSlots[i].getGlobalBounds().contains(mousePos)) {
                                selectedSaveSlot = i;
                                menuSelectSound.play();
                                
                                if (gameState == SaveGame) {
                                    // 保存游戏
                                    GameSave saveData;
                                    saveData.score = score;
                                    saveData.spawnInterval = spawnInterval;
                                    saveData.playerX = player.sprite.getPosition().x;
                                    saveData.playerY = player.sprite.getPosition().y;
                                    saveData.powerUpCount = player.powerUpCount;
                                    saveData.powerUpTimer = powerUpSpawnClock.getElapsedTime().asSeconds();
                                    saveData.bulletsCleared = bulletsCleared;
                                    saveData.powerUpsCollected = powerUpsCollected;
                                    saveData.playTime = playTimeSeconds;
                                    saveData.saveTime = getCurrentTime();
                                    
                                    if (saveGame(saveData, selectedSaveSlot + 1)) {
                                        saveSound.play();
                                        gameState = Playing;
                                    }
                                } else {
                                    // 加载游戏
                                    GameSave loadedData = loadGame(selectedSaveSlot + 1);
                                    if (loadedData.score > 0) {
                                        score = loadedData.score;
                                        spawnInterval = loadedData.spawnInterval;
                                        player.sprite.setPosition(loadedData.playerX, loadedData.playerY);
                                        player.powerUpCount = loadedData.powerUpCount;
                                        bulletsCleared = loadedData.bulletsCleared;
                                        powerUpsCollected = loadedData.powerUpsCollected;
                                        playTimeSeconds = loadedData.playTime;
                                        
                                        powerUpSpawnClock.restart();
                                        
                                        gameState = Playing;
                                        bullets.clear();
                                        if (backgroundMusic.getStatus() != sf::Music::Playing) {
                                            backgroundMusic.play();
                                        }
                                        
                                        gameTimeClock.restart();
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            
            // 鼠标移动处理（主菜单）
            if (event.type == sf::Event::MouseMoved && gameState == MainMenu) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                for (size_t i = 0; i < menuItems.size(); ++i) {
                    if (menuItems[i].getBounds().contains(mousePos)) {
                        if (i != selectedMenuItem) {
                            menuItems[selectedMenuItem].setSelected(false);
                            selectedMenuItem = i;
                            menuItems[selectedMenuItem].setSelected(true);
                        }
                        break;
                    }
                }
            }
            
            // 鼠标移动处理（存档界面）
            if (event.type == sf::Event::MouseMoved && 
                (gameState == LoadGame || gameState == SaveGame)) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                for (int i = 0; i < SAVE_SLOTS; i++) {
                    if (saveSlots[i].getGlobalBounds().contains(mousePos)) {
                        if (i != selectedSaveSlot) {
                            selectedSaveSlot = i;
                        }
                        break;
                    }
                }
            }
        }

        // 更新游戏逻辑
        if (gameState == Playing) {
            player.update();
            powerUp.update(deltaTime);
            
            // 更新游戏时间
            playTimeSeconds = static_cast<int>(gameTimeClock.getElapsedTime().asSeconds());
            
            // 检查技能点碰撞
            if (powerUp.active && player.getBounds().intersects(powerUp.getBounds())) {
                player.powerUpCount++;
                powerUpsCollected++;
                powerUp.active = false;
                powerUpSound.play();
                score += 100;
            }
            
            // 生成技能点（每15秒）
            if (powerUpSpawnClock.getElapsedTime().asSeconds() >= powerUpSpawnInterval && !powerUp.active) {
                float x = 100 + static_cast<float>(std::rand()) / RAND_MAX * (windowWidth - 200);
                float y = 100 + static_cast<float>(std::rand()) / RAND_MAX * (windowHeight - 200);
                powerUp.spawn(sf::Vector2f(x, y), powerUpTexture);
                powerUpSpawnClock.restart();
            }
            
            if (spawnClock.getElapsedTime().asSeconds() >= spawnInterval) {
                spawnClock.restart();
                spawnInterval = std::max(minSpawnInterval, spawnInterval - difficultyIncreaseRate * 0.002f);
                
                int bulletsToSpawn = 2;
                for (int i = 0; i < bulletsToSpawn; i++) {
                    Bullet newBullet;
                    
                    int textureIndex = std::rand() % 3;
                    newBullet.textureIndex = textureIndex;
                    
                    float playerSize = player.sprite.getGlobalBounds().width;
                    float minBulletSize = playerSize * 0.3f;
                    float maxBulletSize = playerSize * 3.0f;
                    float randomSize = minBulletSize + static_cast<float>(std::rand()) / RAND_MAX * (maxBulletSize - minBulletSize);
                    
                    float bulletTextureSize = bulletTextures[textureIndex].getSize().x;
                    float scale = randomSize / bulletTextureSize;
                    
                    newBullet.sprite.setTexture(bulletTextures[textureIndex]);
                    newBullet.sprite.setScale(scale, scale);
                    newBullet.sprite.setOrigin(bulletTextures[textureIndex].getSize().x / 2.0f, 
                                               bulletTextures[textureIndex].getSize().y / 2.0f);
                    newBullet.size = randomSize;
                    
                    int edge = std::rand() % 4;
                    float speed = 0.9f + (score * 0.0012f);

                    switch (edge) {
                        case 0:
                            newBullet.sprite.setPosition(std::rand() % windowWidth, 0);
                            newBullet.velocity = sf::Vector2f(0, speed);
                            break;
                        case 1:
                            newBullet.sprite.setPosition(windowWidth, std::rand() % windowHeight);
                            newBullet.velocity = sf::Vector2f(-speed, 0);
                            break;
                        case 2:
                            newBullet.sprite.setPosition(std::rand() % windowWidth, windowHeight);
                            newBullet.velocity = sf::Vector2f(0, -speed);
                            break;
                        case 3:
                            newBullet.sprite.setPosition(0, std::rand() % windowHeight);
                            newBullet.velocity = sf::Vector2f(speed, 0);
                            break;
                    }
                    
                    bullets.push_back(newBullet);
                } 
            }
            
            // 更新子弹
            for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it) {
                it->update();
            }
            
            // 移除屏幕外的子弹
            for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ) {
                sf::Vector2f pos = it->sprite.getPosition();
                if (pos.x < -100 || pos.x > windowWidth + 100 || 
                    pos.y < -100 || pos.y > windowHeight + 100) {
                    score += 8;
                    bulletsCleared++;
                    it = bullets.erase(it);
                } else {
                    ++it;
                }
            }
            
            // 碰撞检测
            for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it) {
                if (it->getBounds().intersects(player.getBounds())) {
                    gameState = GameOver;
                    gameOverSound.play();
                    backgroundMusic.stop();
                    break;
                }
            }
            
            // 更新UI文本
            scoreText.setString("Score: " + intToString(score));
            powerUpText.setString("Power-ups: " + intToString(player.powerUpCount) + "  >> click the blank!");
            
            // 格式化时间显示
            int hours = playTimeSeconds / 3600;
            int minutes = (playTimeSeconds % 3600) / 60;
            int seconds = playTimeSeconds % 60;
            std::stringstream timeStr;
            if (hours > 0) {
                timeStr << std::setw(2) << std::setfill('0') << hours << ":";
            }
            timeStr << std::setw(2) << std::setfill('0') << minutes << ":"
                    << std::setw(2) << std::setfill('0') << seconds;
            timeText.setString("Time: " + timeStr.str());
        }
        else if (gameState == LoadGame || gameState == SaveGame) {
            // 更新存档槽选择状态
            for (int i = 0; i < SAVE_SLOTS; i++) {
                if (i == selectedSaveSlot) {
                    saveSlots[i].setOutlineColor(sf::Color(255, 200, 100));
                    saveSlots[i].setOutlineThickness(4);
                    saveSlotTexts[i].setFillColor(sf::Color(255, 255, 150));
                } else {
                    saveSlots[i].setOutlineColor(sf::Color(100, 100, 150));
                    saveSlots[i].setOutlineThickness(3);
                    saveSlotTexts[i].setFillColor(sf::Color(200, 200, 255));
                }
            }
            
            // 更新操作提示
            if (isSavingMode) {
                saveLoadHint.setString("Press ENTER/SPACE to save to selected slot | Click slot to save");
                deleteHint.setString("Press DELETE to delete selected save file");
            } else {
                saveLoadHint.setString("Press ENTER/SPACE to load selected slot | Click slot to load");
                deleteHint.setString("Press DELETE to delete selected save file");
            }
        }

        // 渲染
        window.clear();
        
        if (gameState == MainMenu || gameState == GameIntro || 
            gameState == AboutUs || gameState == LoadGame || gameState == SaveGame) {
            // 绘制菜单背景图片
            window.draw(menuBackgroundSprite);
            
            // 绘制标题和副标题
            window.draw(titleShadow);
            window.draw(titleText);
            window.draw(subtitleText);
            
            if (gameState == MainMenu) {
                // 绘制菜单项
                for (size_t i = 0; i < menuItems.size(); ++i) {
                    window.draw(menuItems[i].background);
                    window.draw(menuItems[i].text);
                }
            }
            else if (gameState == GameIntro) {
                // 添加介绍页面背景
                sf::RectangleShape introBackground(sf::Vector2f(700, 400));
                introBackground.setFillColor(sf::Color(0, 0, 0, 180));
                introBackground.setOutlineThickness(2);
                introBackground.setOutlineColor(sf::Color(255, 165, 0, 200));
                introBackground.setPosition(50, 280);
                window.draw(introBackground);
                window.draw(introText);
            }
            else if (gameState == AboutUs) {
                // 添加关于我们页面背景
                sf::RectangleShape aboutBackground(sf::Vector2f(700, 500));
                aboutBackground.setFillColor(sf::Color(0, 0, 0, 180));
                aboutBackground.setOutlineThickness(2);
                aboutBackground.setOutlineColor(sf::Color(255, 165, 0, 200));
                aboutBackground.setPosition(50, 180);
                window.draw(aboutBackground);
                window.draw(aboutTitle);
                window.draw(aboutContent);
            }
            else if (gameState == LoadGame || gameState == SaveGame) {
                // 绘制存档/加载界面
                if (isSavingMode) {
                    window.draw(saveGameTitle);
                } else {
                    window.draw(loadGameTitle);
                }
                
                // 绘制存档槽
                for (int i = 0; i < SAVE_SLOTS; i++) {
                    window.draw(saveSlots[i]);
                    window.draw(saveSlotTexts[i]);
                    
                    // 检查存档是否存在
                    GameSave saveInfo = loadGame(i + 1);
                    if (saveInfo.score > 0) {
                        // 有存档，显示详细信息
                        window.draw(saveSlotDetails[i]);
                        
                        // 在右上角添加存档状态图标
                        sf::CircleShape statusCircle(8);
                        statusCircle.setPosition(saveSlots[i].getPosition().x + saveSlots[i].getSize().x - 25, 
                                                saveSlots[i].getPosition().y + 15);
                        statusCircle.setFillColor(sf::Color(100, 255, 100));
                        window.draw(statusCircle);
                        
                        // 添加分数标签
                        sf::Text scoreLabel;
                        scoreLabel.setFont(font);
                        scoreLabel.setString("SCORE: " + intToString(saveInfo.score));
                        scoreLabel.setCharacterSize(22);
                        scoreLabel.setFillColor(sf::Color(255, 255, 100));
                        scoreLabel.setPosition(saveSlots[i].getPosition().x + 400, 
                                              saveSlots[i].getPosition().y + 15);
                        window.draw(scoreLabel);
                    } else {
                        // 空存档槽
                        window.draw(saveSlotEmptyText[i]);
                        
                        // 在右上角添加空状态图标
                        sf::CircleShape statusCircle(8);
                        statusCircle.setPosition(saveSlots[i].getPosition().x + saveSlots[i].getSize().x - 25, 
                                                saveSlots[i].getPosition().y + 15);
                        statusCircle.setFillColor(sf::Color(150, 150, 150));
                        window.draw(statusCircle);
                    }
                }
                
                // 绘制操作提示
                window.draw(saveLoadHint);
                window.draw(deleteHint);
                
                // 绘制当前选中槽位的额外信息
                if (selectedSaveSlot >= 0 && selectedSaveSlot < SAVE_SLOTS) {
                    GameSave selectedSave = loadGame(selectedSaveSlot + 1);
                    if (selectedSave.score > 0) {
                        // 在底部显示详细统计信息
                        sf::Text detailedStats;
                        detailedStats.setFont(font);
                        detailedStats.setCharacterSize(18);
                        detailedStats.setFillColor(sf::Color(200, 230, 255));
                        
                        std::stringstream stats;
                        stats << "Selected Slot Details:\n";
                        stats << "? Score: " << selectedSave.score << "\n";
                        stats << "? Game Time: " << selectedSave.playTime << " seconds\n";
                        stats << "? Power-ups: " << selectedSave.powerUpCount << "\n";
                        stats << "? Bullets Cleared: " << selectedSave.bulletsCleared << "\n";
                        stats << "? Power-ups Collected: " << selectedSave.powerUpsCollected << "\n";
                        stats << "? Difficulty Level: " << std::fixed << std::setprecision(1) 
                              << ((1.0f - selectedSave.spawnInterval) * 100) << "%\n";
                        stats << "? Save Time: " << selectedSave.saveTime;
                        
                        detailedStats.setString(stats.str());
                        detailedStats.setPosition(100, 550);
                        
                        // 添加背景
                        sf::RectangleShape statsBg(sf::Vector2f(600, 130));
                        statsBg.setFillColor(sf::Color(0, 0, 0, 120));
                        statsBg.setOutlineThickness(1);
                        statsBg.setOutlineColor(sf::Color(100, 150, 255, 100));
                        statsBg.setPosition(95, 545);
                        window.draw(statsBg);
                        window.draw(detailedStats);
                    }
                }
                
                // 绘制返回提示
                sf::Text escHint;
                escHint.setFont(font);
                escHint.setString("Press ESC to return to Main Menu");
                escHint.setCharacterSize(20);
                escHint.setFillColor(sf::Color(255, 200, 100));
                escHint.setPosition(windowWidth / 2 - escHint.getLocalBounds().width / 2, 720);
                window.draw(escHint);
            }
        }
        else if (gameState == Playing || gameState == GameOver) {
            // 绘制游戏背景图片
            window.draw(backgroundSprite);
            window.draw(player.sprite);
            
            // 绘制子弹
            for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it) {
                window.draw(it->sprite);
            }
            
            // 绘制技能点（如果激活）
            if (powerUp.active) {
                window.draw(powerUp.sprite);
            }
            
            // 绘制UI背景
            window.draw(scoreBackground);
            window.draw(powerUpBackground);
            window.draw(timeBackground);
            
            // 绘制UI文本
            window.draw(scoreText);
            window.draw(powerUpText);
            window.draw(timeText);
            
            // 绘制技能点提示文本
            if (player.powerUpCount > 0) {
                sf::Text powerUpHint;
                powerUpHint.setFont(font);
                powerUpHint.setCharacterSize(20);
                powerUpHint.setFillColor(sf::Color(80, 255, 60));
                powerUpHint.setString("Press SPACE to clear screen!");
                powerUpHint.setPosition(windowWidth - 250, 15);
                window.draw(powerUpHint);
            }
            
            if (gameState == GameOver) {
                // 绘制游戏结束界面
                window.draw(gameOverBackground);
                window.draw(gameOverText);
                
                sf::Text restartText;
                restartText.setFont(font);
                restartText.setCharacterSize(30);
                restartText.setFillColor(sf::Color::White);
                
                std::stringstream gameOverStats;
                gameOverStats << "YOUR SCORE: " << score 
                            << "\nGame Time: " << playTimeSeconds << " seconds"
                            << "\nPower-ups collected: " << powerUpsCollected
                            << "\nBullets cleared: " << bulletsCleared
                            << "\n\nClick to restart\nESC: Main Menu";
                
                restartText.setString(gameOverStats.str());
                sf::FloatRect restartRect = restartText.getLocalBounds();
                restartText.setOrigin(restartRect.width / 2, restartRect.height / 2);
                restartText.setPosition(windowWidth / 2, windowHeight / 2 + 50);
                window.draw(restartText);
            }
        }
        
        window.display();
    }
    return 0;
}


