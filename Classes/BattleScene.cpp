#include "BattleScene.h"
#include "HelloWorldScene.h"
#include "BattleManager.h"
#include "Building.h"
#include "Troop.h"

USING_NS_CC;
using namespace ui; // 使用 UI 命名空间

Scene* BattleScene::createScene(int levelIndex) {
    // 1. 先创建场景对象 (内部调用 init)
    auto scene = BattleScene::create();

    // 2. 如果创建成功，加载对应关卡的数据
    // dynamic_cast 用于安全转换，确保 scene 是 BattleScene 类型
    if (auto battleScene = dynamic_cast<BattleScene*>(scene)) {
        battleScene->loadLevel(levelIndex);
    }

    return scene;
}

bool BattleScene::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("battleback1.png");

    if (background) {
        // 2. 放在屏幕正中间
        background->setPosition(visibleSize / 2);

        // 3. 【关键】自动缩放以铺满屏幕
        // 获取图片原始大小
        Size bgSize = background->getContentSize();

        // 计算需要的缩放倍数
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;

        // 取较大的那个缩放值，确保不留黑边 (Cover 模式)
        float finalScale = std::max(scaleX, scaleY);
        background->setScale(finalScale);

        // 4. 放在最底层 (ZOrder = -99)
        this->addChild(background, -99);
    }
    else {
        CCLOG("Error: Background image not found!");
    }

    // 1. 初始化数据
    BattleManager::getInstance()->clear();
    m_selectedType = TroopType::BARBARIAN; // 默认选野蛮人

    // 3. 【新增】创建提示文字
    m_infoLabel = Label::createWithSystemFont("Selected: Barbarian", "Arial", 24);
    m_infoLabel->setPosition(Vec2(visibleSize.width / 2, 50)); // 放在底部中间
    this->addChild(m_infoLabel, 10);

    // 4. 【新增】创建一排选择按钮
    // 参数：名字，颜色，类型，第几个(用于排版)
    createSelectButton("Barb", Color3B::GREEN, TroopType::BARBARIAN, 0);
    createSelectButton("Arch", Color3B::MAGENTA, TroopType::ARCHER, 1);
    createSelectButton("Giant", Color3B::ORANGE, TroopType::GIANT, 2);
    createSelectButton("Bomb", Color3B::GRAY, TroopType::BOMBERMAN, 3); // 白色按钮文字看不清，用灰色代替背景

    // 5. 撤退按钮 (保持不变)
    auto backBtn = Button::create("CloseNormal.png");
    backBtn->setTitleText("Back");
    backBtn->setPosition(Vec2(50, visibleSize.height - 50));
    backBtn->addClickEventListener([=](Ref*) {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, HelloWorld::createScene()));
        });
    this->addChild(backBtn);

    // 6. 触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// 【新增】辅助函数：快速创建按钮
void BattleScene::createSelectButton(const std::string& title, Color3B color, TroopType type, int index) {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建按钮
    auto btn = Button::create("CloseNormal.png");
    btn->setColor(color); // 染成兵种的颜色
    btn->setTitleText(title);
    btn->setTitleFontSize(20);

    // 排列在右下角
    // index * 60 意味着每个按钮间隔 60 像素
    btn->setPosition(Vec2(visibleSize.width - 250 + index * 60, 50));

    // 点击逻辑
    btn->addClickEventListener([=](Ref*) {
        // 1. 更新选中的类型
        m_selectedType = type;

        // 2. 更新文字提示
        m_infoLabel->setString("Selected: " + title);
        m_infoLabel->setColor(color);
        });

    this->addChild(btn, 10); // ZOrder设为10，防止被兵种遮挡
}

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchLoc = touch->getLocation();

    // 如果点击到了按钮区域（屏幕下方 80 像素内），就不放兵，防止误触
    if (touchLoc.y < 80) return false;

    // 【修改】使用当前选中的类型 (m_selectedType)
    auto troop = Troop::create(m_selectedType);
    troop->setPosition(touchLoc);
    this->addChild(troop);

    return true;
}

void BattleScene::onExit() {
    Scene::onExit();
    BattleManager::getInstance()->clear();
}

// 【新增】关卡配置逻辑
void BattleScene::loadLevel(int levelIndex) {
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = visibleSize / 2;

    if (levelIndex == 1) {
        // --- 第一关：新手村 ---
        // 只有 1 个大本营
        auto town = Building::create(BuildingType::TOWN_HALL);
        town->setPosition(center);
        this->addChild(town);
    }
    else if (levelIndex == 2) {
        // --- 第二关：初级挑战 ---
        // 大本营 + 1 个加农炮
        auto town = Building::create(BuildingType::TOWN_HALL);
        town->setPosition(center);
        this->addChild(town);

        auto cannon = Building::create(BuildingType::CANNON);
        cannon->setPosition(center + Vec2(150, 0)); // 右边放个炮
        this->addChild(cannon);
    }
    else if (levelIndex == 3) {
        // --- 第三关：攻坚战 ---
        // 大本营 + 2 个炮 + 围墙保护
        auto town = Building::create(BuildingType::TOWN_HALL);
        town->setPosition(center);
        this->addChild(town);

        // 两个炮
        auto c1 = Building::create(BuildingType::CANNON);
        c1->setPosition(center + Vec2(150, 50));
        this->addChild(c1);
        auto c2 = Building::create(BuildingType::CANNON);
        c2->setPosition(center + Vec2(150, -50));
        this->addChild(c2);

        // 前面放一排墙
        for (int i = 0; i < 5; i++) {
            auto wall = Building::create(BuildingType::WALL);
            wall->setPosition(center + Vec2(80, -100 + i * 50));
            this->addChild(wall);
        }
    }
}