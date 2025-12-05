#include "GameUI.h"
#include "GameManager.h" // 需要读取数据

USING_NS_CC;

bool GameUI::init() {
    if (!Layer::init()) {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建金币标签 (使用系统字体 Arial，字号 24)
    m_goldLabel = Label::createWithSystemFont("Gold: 0", "Arial", 24);
    m_goldLabel->setColor(Color3B::YELLOW);
    // 放在右上角
    m_goldLabel->setPosition(Vec2(visibleSize.width - 100, visibleSize.height - 30));
    // 锚点设为右对齐
    m_goldLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_goldLabel);

    // 2. 创建圣水标签
    m_elixirLabel = Label::createWithSystemFont("Elixir: 0", "Arial", 24);
    m_elixirLabel->setColor(Color3B::MAGENTA);
    m_elixirLabel->setPosition(Vec2(visibleSize.width - 100, visibleSize.height - 60));
    m_elixirLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_elixirLabel);

    // 3. 【核心】监听事件
    // 当 GameManager 喊“钱变了”，我就调用 updateLabels
    auto listener = EventListenerCustom::create("EVENT_UPDATE_GOLD", [this](EventCustom* event) {
        this->updateLabels();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 初始化显示一次
    updateLabels();

    return true;
}

void GameUI::updateLabels() {
    auto gm = GameManager::getInstance();

    // 显示格式： Gold: 100 / 1000
    std::string goldStr = "Gold: " + std::to_string(gm->getGold()) +
        "/" + std::to_string(gm->getMaxGold());

    std::string elixirStr = "Elixir: " + std::to_string(gm->getElixir()) +
        "/" + std::to_string(gm->getMaxElixir());

    m_goldLabel->setString(goldStr);
    m_elixirLabel->setString(elixirStr);
}