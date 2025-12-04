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
    // 【关键修改】如果已经初始化过，就不要再重置钱了！
    if (m_isInitialized) return;

    m_gold = gold;
    m_elixir = elixir;
    m_isInitialized = true; // 标记为已初始化
}

void GameManager::addGold(int amount) {
    m_gold += amount;
    CCLOG("Current Gold: %d", m_gold);

    // 【关键】发送通知：告诉全游戏“金币更新了”
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

void GameManager::addElixir(int amount) {
    m_elixir += amount;
    // 发送通知
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
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