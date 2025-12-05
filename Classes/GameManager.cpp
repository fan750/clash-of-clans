#include "GameManager.h"
#include"Building.h"

USING_NS_CC;

GameManager* GameManager::s_instance = nullptr;

// 构造函数里初始化标记为 false
GameManager::GameManager()
    : m_gold(0), m_elixir(0), m_isInitialized(false)
{
}

GameManager* GameManager::getInstance() {
    if (!s_instance) {
        s_instance = new GameManager();
    }
    return s_instance;
}

void GameManager::initAccount(int gold, int elixir) {
    if (m_isInitialized) return;

    m_gold = gold;
    m_elixir = elixir;

    // 初始自带一点容量 (比如大本营自带的容量)
    m_maxGold = 1000;
    m_maxElixir = 1000;

    m_isInitialized = true;
}

void GameManager::addGold(int amount) {
    m_gold += amount;
    // 核心机制：安全存储-容量限制
    // 钱不能超过上限
    if (m_gold > m_maxGold) m_gold = m_maxGold;
    if (m_gold < 0) m_gold = 0; // 消费时不能扣成负数

    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

void GameManager::addElixir(int amount) {
    m_elixir += amount;
    // 核心机制：安全存储-容量限制
    if (m_elixir > m_maxElixir) m_elixir = m_maxElixir;
    if (m_elixir < 0) m_elixir = 0;

    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

// 当建造了圣水瓶，上限增加
void GameManager::modifyMaxElixir(int amount) {
    m_maxElixir += amount;
    // 也可以发个事件更新UI，显示容量变化
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

void GameManager::modifyMaxGold(int amount) {
    m_maxGold += amount;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

// 【新增实现】保存建筑
void GameManager::addHomeBuilding(BuildingType type, Vec2 pos) {
    BuildingData data;
    data.type = type;
    data.position = pos;
    m_homeBuildings.push_back(data);
}

// 【新增实现】获取列表
const std::vector<BuildingData>& GameManager::getHomeBuildings() {
    return m_homeBuildings;
}