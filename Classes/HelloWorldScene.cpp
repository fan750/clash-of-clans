#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "GameEntity.h" // 根据你的实际路径，可能是 "GameEntity.h"
#include "Building.h"
#include "Troop.h"
#include "GameManager.h"
#include "GameUI.h"
#include "BattleScene.h"
#include "ui/CocosGUI.h"
#include "LevelMapScene.h"
USING_NS_CC;
using namespace ui;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();
    // 1. 创建背景精灵 (确保文件名正确，且图片在 Resources 里)
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
    auto myTown = Building::create(BuildingType::TOWN_HALL);
    myTown->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(myTown);

    // 1. 初始化数据
    // 因为我们在 GameManager 里加了检查，所以这里放心调用，只有第一次会给 2000
    GameManager::getInstance()->initAccount(2000, 2000);

    m_pendingBuilding = nullptr;

    // 【新增】初始化
    m_isConfirming = false;
    m_confirmLayer = nullptr;

    // 2. 添加 UI 层 (之前的代码)
    auto uiLayer = GameUI::create();
    this->addChild(uiLayer, 100);

    // 3. 【新增】商店按钮 (放在左下角)
    auto shopBtn = Button::create("CloseNormal.png");
    shopBtn->setTitleText("SHOP");
    shopBtn->setTitleFontSize(24);
    shopBtn->setColor(Color3B::ORANGE);
    shopBtn->setPosition(Vec2(visibleSize.width - 200, 100));
    shopBtn->addClickEventListener([=](Ref*) {
        this->toggleShop(); // 点击开关商店
        });
    this->addChild(shopBtn, 10);

    // 4. 【新增】初始化商店面板 (默认隐藏)
    initShopUI();

    // 5. 进攻按钮 (之前的代码，保持不变)
    auto attackBtn = Button::create("CloseNormal.png");
    attackBtn->setTitleText("ATTACK!");
    attackBtn->setPosition(Vec2(visibleSize.width - 100, 100));
    attackBtn->addClickEventListener([=](Ref*) {
        auto scene = LevelMapScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });
    this->addChild(attackBtn);

    // ==========================================
    // 【新增】 关键步骤：恢复之前保存的建筑
    // ==========================================
    const auto& savedBuildings = GameManager::getInstance()->getHomeBuildings();
    for (const auto& data : savedBuildings) {
        // 按照保存的类型和位置，重新创建实体
        auto b = Building::create(data.type);
        b->setPosition(data.position);
        // 因为是恢复出来的，已经是实体的，所以不透明度设为255
        b->setOpacity(255);
        this->addChild(b);
    }

    // 6. 注册触摸监听 (用于放置建筑)
    auto listener = EventListenerTouchOneByOne::create();
    // setSwallowTouches(true) 表示如果我在放置建筑，别的东西就别响应触摸了
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this); // 移动跟随
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void HelloWorld::initShopUI() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建一个半透明黑色背景板
    m_shopLayer = Layout::create();
    m_shopLayer->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    m_shopLayer->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    m_shopLayer->setBackGroundColor(Color3B::BLACK);
    m_shopLayer->setBackGroundColorOpacity(200);
    // 居中显示
    m_shopLayer->setPosition(Vec2(visibleSize.width * 0.1, visibleSize.height * 0.2));
    m_shopLayer->setVisible(false); // 默认看不见
    this->addChild(m_shopLayer, 200); // 层级很高，盖住一切

    // 2. 定义商品列表
    std::vector<ShopItem> items = {
        {"Gold Mine", BuildingType::GOLD_MINE, 100, false}, // 100圣水买金矿
        {"Cannon", BuildingType::CANNON, 200, true},        // 200金币买加农炮
        {"Wall", BuildingType::WALL, 50, true},             // 50金币买墙
        {"Archer Twr", BuildingType::ARCHER_TOWER, 300, true},
        { "Elixir Pump", BuildingType::ELIXIR_COLLECTOR, 100, true }, // 用金币买收集器
        {"Elixir Tank", BuildingType::ELIXIR_STORAGE, 300, true},   // 用金币买圣水瓶
        {"Gold Storage", BuildingType::GOLD_STORAGE, 300, false}    // 用圣水买储金罐
    };

    // 3. 循环创建商品按钮
    for (int i = 0; i < items.size(); i++) {
        ShopItem item = items[i];

        auto btn = Button::create("CloseNormal.png");
        btn->setTitleText(item.name + "\n$" + std::to_string(item.price));
        btn->setTitleFontSize(16);
        // 简单排版：每行放 4 个 (这里简单点，直接横排)
        btn->setPosition(Vec2(100 + i * 150, m_shopLayer->getContentSize().height / 2));

        // 点击购买
        btn->addClickEventListener([=](Ref*) {
            this->tryBuyBuilding(item);
            });

        m_shopLayer->addChild(btn);
    }
}

void HelloWorld::toggleShop() {
    // 切换显示状态
    m_shopLayer->setVisible(!m_shopLayer->isVisible());
}

void HelloWorld::tryBuyBuilding(const ShopItem& item) {
    // 1. 检查钱够不够
    int currentRes = item.isGold ? GameManager::getInstance()->getGold() : GameManager::getInstance()->getElixir();

    if (currentRes < item.price) {
        CCLOG("Not enough resources!");
        // 这里可以做一个 Tip 提示 "资金不足"
        return;
    }

    // 2. 钱够了，关闭商店，进入“放置模式”
    toggleShop();

    // 3. 创建虚影建筑
    if (m_pendingBuilding) {
        m_pendingBuilding->removeFromParent(); // 如果之前有一个没放下去，先删了
    }

    m_pendingBuilding = Building::create(item.type);
    // 设置半透明，表示还没放好
    m_pendingBuilding->setOpacity(128);
    // 放在屏幕中心跟随
    m_pendingBuilding->setPosition(Director::getInstance()->getVisibleSize() / 2);
    this->addChild(m_pendingBuilding);

    // 4. 记录价格，等放下去的时候再扣
    m_pendingCost = item.price;
    m_pendingIsGold = item.isGold;
}
// 手指移动时，建筑跟着跑
void HelloWorld::onTouchMoved(Touch* touch, Event* event) {
    if (m_pendingBuilding && !m_isConfirming) {
        m_pendingBuilding->setPosition(touch->getLocation());
    }
}

// 手指按下时
bool HelloWorld::onTouchBegan(Touch* touch, Event* event) {
    // 情况A: 正在等待确认中
    if (m_isConfirming) {
        // 如果正在显示 √/× 按钮，点击屏幕其他空白处无效（或者你可以设计成点击空白处等于取消）
        // 这里我们返回 true 吞掉触摸，强制玩家必须点按钮
        return true;
    }

    // 情况B: 正在拖拽虚影，准备定点
    if (m_pendingBuilding) {
        // 1. 定格位置
        m_pendingBuilding->setPosition(touch->getLocation());

        // 2. 【关键】进入确认模式
        m_isConfirming = true;

        // 3. 弹出确认菜单
        showConfirmationUI(touch->getLocation());

        return true;
    }
    // 如果当前正在放置建筑
    if (m_pendingBuilding) {

        // 1. 确认放置位置
        Vec2 location = touch->getLocation();
        m_pendingBuilding->setPosition(location);

        // 2. 恢复不透明 (表示实体化了)
        m_pendingBuilding->setOpacity(255);

        // 3. 真正的扣钱
        if (m_pendingIsGold) {
            GameManager::getInstance()->addGold(-m_pendingCost); // 负数就是扣钱
        }
        else {
            GameManager::getInstance()->addElixir(-m_pendingCost);
        }

        // ==========================================
        // 【新增】 关键步骤：保存到大管家的小本本上
        // ==========================================
        GameManager::getInstance()->addHomeBuilding(
            m_pendingBuilding->getBuildingType(), // 获取类型
            m_pendingBuilding->getPosition()      // 获取位置
        );

        // 4. 放置完成，清空指针
        m_pendingBuilding = nullptr;

        CCLOG("Building Placed!");
        return true; // 吞噬触摸，防止点到别的东西
    }

    // 如果商店是开着的，点击外部可以关闭商店
    if (m_shopLayer->isVisible()) {
        // 把触摸点转换到商店的坐标系，判断有没有点在商店背景上
        Vec2 nodePos = m_shopLayer->convertToNodeSpace(touch->getLocation());
        Rect box = Rect(0, 0, m_shopLayer->getContentSize().width, m_shopLayer->getContentSize().height);

        if (!box.containsPoint(nodePos)) {
            toggleShop(); // 点到外面了，关掉商店
        }
    }

    // 如果没在干别的，就检测是不是点到了建筑 (用于收集资源)
    Vec2 touchLoc = touch->getLocation();

    // 遍历所有子节点，看看点到了谁
    // 这种写法比较简单粗暴，适合演示
    for (auto node : this->getChildren()) {
        // dynamic_cast 尝试把节点转成 Building
        // 如果转换成功，说明点到的是个建筑
        auto building = dynamic_cast<Building*>(node);

        if (building) {
            // 检查触摸点是否在建筑图片的范围内
            Rect boundingBox = building->getBoundingBox();
            if (boundingBox.containsPoint(touchLoc)) {

                // 执行收集！
                int amount = building->collectResource();

                if (amount > 0) {
                    // 机制：收集动画
                    // 播放金币飞向右上角的动画
                    playCollectAnimation(amount, building->getPosition(), building->getBuildingType());
                }
                return true; // 吞噬触摸
            }
        }
    }

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}

// 显示 √ 和 × 按钮
void HelloWorld::showConfirmationUI(Vec2 pos) {
    // 创建一个节点容器，把两个按钮放进去，方便管理
    m_confirmLayer = Node::create();
    this->addChild(m_confirmLayer, 200); // 层级很高，保证在最上面

    // --- 1. 确定按钮 (绿色) ---
    auto btnYes = Button::create("CloseNormal.png");
    btnYes->setTitleText("YES");
    btnYes->setTitleFontSize(20);
    btnYes->setColor(Color3B::GREEN);
    // 放在建筑右边
    btnYes->setPosition(pos + Vec2(60, 0));
    btnYes->addClickEventListener([=](Ref*) {
        this->onConfirmPlacement();
        });
    m_confirmLayer->addChild(btnYes);

    // --- 2. 取消按钮 (红色) ---
    auto btnNo = Button::create("CloseNormal.png");
    btnNo->setTitleText("NO");
    btnNo->setTitleFontSize(20);
    btnNo->setColor(Color3B::RED);
    // 放在建筑左边
    btnNo->setPosition(pos + Vec2(-60, 0));
    btnNo->addClickEventListener([=](Ref*) {
        this->onCancelPlacement();
        });
    m_confirmLayer->addChild(btnNo);
}

// 玩家点了 YES
void HelloWorld::onConfirmPlacement() {
    if (!m_pendingBuilding) return;

    // 1. 恢复实体不透明度
    m_pendingBuilding->setOpacity(255);

    // 2. 扣钱
    if (m_pendingIsGold) {
        GameManager::getInstance()->addGold(-m_pendingCost);
    }
    else {
        GameManager::getInstance()->addElixir(-m_pendingCost);
    }

    // 3. 存档 (保存到 GameManager)
    GameManager::getInstance()->addHomeBuilding(
        m_pendingBuilding->getBuildingType(),
        m_pendingBuilding->getPosition()
    );

    // 4. 清理现场
    m_pendingBuilding = nullptr; // 指针置空，表示放置结束

    // 移除确认按钮
    if (m_confirmLayer) {
        m_confirmLayer->removeFromParent();
        m_confirmLayer = nullptr;
    }

    // 退出确认状态
    m_isConfirming = false;

    CCLOG("Placed Successfully!");
}

// 玩家点了 NO
void HelloWorld::onCancelPlacement() {
    // 1. 移除那个虚影建筑
    if (m_pendingBuilding) {
        m_pendingBuilding->removeFromParent();
        m_pendingBuilding = nullptr;
    }

    // 2. 移除确认按钮
    if (m_confirmLayer) {
        m_confirmLayer->removeFromParent();
        m_confirmLayer = nullptr;
    }

    // 3. 退出确认状态
    m_isConfirming = false;

    CCLOG("Placement Cancelled!");
}

void HelloWorld::playCollectAnimation(int amount, Vec2 startPos, BuildingType type) {
    // 1. 决定飞什么图标 (金币还是圣水)
    std::string iconName = (type == BuildingType::GOLD_MINE) ? "coin_icon.png" : "elixir_icon.png";
    // 假设右上角 UI 的位置是 (VisibleWidth - 50, VisibleHeight - 30)
    // 最好从 GameUI::getInstance() 获取，这里先硬编码近似位置
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 targetPos = Vec2(visibleSize.width - 100, visibleSize.height - 30);

    auto icon = Sprite::create(iconName);
    icon->setScale(0.03);

    icon->setPosition(startPos);
    this->addChild(icon, 200); // 层级要在最上面

    // 3. 飘字 (显示 +100)
    auto label = Label::createWithSystemFont("+" + std::to_string(amount), "Arial", 24);
    label->setColor(Color3B::WHITE);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(startPos + Vec2(0, 40)); // 在建筑头顶
    this->addChild(label, 201);

    // 4. 动作序列
    // 图标动作：先放大弹出 -> 飞向右上角 -> 缩小消失
    auto jump = JumpBy::create(0.3f, Vec2(0, 0), 30, 1); // 原地跳一下
    auto move = MoveTo::create(0.5f, targetPos); // 飞过去
    auto scale = ScaleTo::create(0.1f, 0.0f); // 变没
    auto remove = RemoveSelf::create();

    icon->runAction(Sequence::create(jump, move, scale, remove, nullptr));

    // 文字动作：向上飘 -> 淡出
    auto labelMove = MoveBy::create(0.8f, Vec2(0, 50));
    auto fade = FadeOut::create(0.8f);
    auto labelSeq = Sequence::create(Spawn::create(labelMove, fade, nullptr), RemoveSelf::create(), nullptr);
    label->runAction(labelSeq);

    // 5. 播放音效 (如果之前加了的话)
    // SimpleAudioEngine::getInstance()->playEffect("collect.wav");
}