#include "Building.h"
#include "BattleManager.h"
#include"Troop.h"
#include"GameManager.h"

USING_NS_CC;

Building::Building()
    : m_type(BuildingType::TOWN_HALL)
    , m_level(1)
    , m_timer(0.0f)
{
}

Building::~Building() {
}

Building* Building::create(BuildingType type) {
    Building* pRet = new(std::nothrow) Building();
    if (pRet) {
        pRet->m_type = type; // 先设置类型
        if (pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

bool Building::init() {
    if (!GameEntity::init()) {
        return false;
    }

    // 初始化属性
    initBuildingProperties();

    // 开启每帧更新 (用于生产资源或攻击)
    this->scheduleUpdate();

    // 告诉管理者：我出生了！
    BattleManager::getInstance()->addBuilding(this);
    return true;
}

void Building::onDeath() {
    // 告诉管理者：我挂了，别让兵再来打我了！
    BattleManager::getInstance()->removeBuilding(this);

    // 调用父类的死亡逻辑 (移除节点)
    GameEntity::onDeath();
}

void Building::initBuildingProperties() {
    // 1. 定义一个变量存文件名
    std::string filename = "CloseNormal.png"; // 默认图片(防崩)
    int hp = 500;

   
    switch (m_type) {
    case BuildingType::TOWN_HALL:
        filename = "TownHall.png";
        hp = 2000;
        break;
    case BuildingType::CANNON:
        filename = "Cannon.png";
        hp = 800;
        break;
    case BuildingType::GOLD_MINE:
        filename = "GoldMine.png";
        hp = 600;
        break;
    case BuildingType::ARCHER_TOWER:
        filename = "ArcherTower.png";
        hp = 700;
        break;
    case BuildingType::WALL:
        filename = "Wall.png";
        hp = 1000;
        break;
    default:
        break;
    }

    // 3. 【核心修改】加载图片纹理
    this->setTexture(filename);

    // 4. 设置其他属性
    this->setProperties(hp, CampType::PLAYER);

    // 5. 【可选】调整大小 (Scale)
    // 图片可能很大(比如 500x500)，我们需要把它缩放到合适的大小(比如 64x64)
    // 假设你想让所有建筑大约占 60x60 像素：
    float targetSize = 60.0f;
    Size contentSize = this->getContentSize(); // 获取图片原始大小
    if (contentSize.width > 0) {
        this->setScale(targetSize / contentSize.width);
    }
}

void Building::upgrade() {
    m_level++;
    m_maxHp += 200; // 升级加血上限
    m_currentHp += 200; // 并回血
    CCLOG("Building Upgraded to Level %d! Max HP is now %d", m_level, m_maxHp);

    // 稍微变大一点表示升级
    this->setScale(this->getScale() * 1.1f);
}

void Building::updateLogic(float dt) {
    m_timer += dt;

    // 1. 资源建筑逻辑 (之前写过的)
    if (m_type == BuildingType::GOLD_MINE) {
        if (m_timer >= 2.0f) {
            // 产出金币...
            m_timer = 0;
            GameManager::getInstance()->addGold(10);
        }
    }
    // 2. 【新增】防御塔逻辑
    else if (m_type == BuildingType::CANNON || m_type == BuildingType::ARCHER_TOWER) {

        // 只有冷却时间到了才攻击 (假设攻速是 1.0秒)
        if (m_timer >= 1.0f) {

            // 设定射程 (加农炮一般射程是 150-200 左右)
            float range = 20.0f;

            // 询问 BattleManager：我射程内有没有敌人？
            Troop* target = BattleManager::getInstance()->findClosestTroop(this->getPosition(), range);

            if (target) {
                // 找到了！开火！
                m_timer = 0; // 重置冷却

                // 造成伤害 (假设加农炮伤害 80)
                CCLOG("Cannon Fired at Troop!");
                target->takeDamage(80);

                // 【视觉反馈】做一个简单的闪烁效果，表示它开火了
                auto seq = Sequence::create(TintTo::create(0.1f, Color3B::ORANGE), TintTo::create(0.1f, this->getColor()), nullptr);
                this->runAction(seq);
            }
        }
    }
}