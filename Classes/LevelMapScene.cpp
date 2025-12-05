#include "LevelMapScene.h"
#include "HelloWorldScene.h" // 为了能返回主菜单
#include "BattleScene.h"     // 为了进入战斗

USING_NS_CC;
using namespace ui;

Scene* LevelMapScene::createScene() {
    return LevelMapScene::create();
}

bool LevelMapScene::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 背景 (可以用你的 Background.jpg)
    auto bg = Sprite::create("Background.jpg");
    if (bg) {
        bg->setPosition(visibleSize / 2);
        // 简单适配铺满
        float scaleX = visibleSize.width / bg->getContentSize().width;
        float scaleY = visibleSize.height / bg->getContentSize().height;
        bg->setScale(std::max(scaleX, scaleY));
        this->addChild(bg, -1);
    }

    // 2. 标题
    auto title = Label::createWithSystemFont("Select Level", "Arial", 40);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    title->setColor(Color3B::YELLOW);
    this->addChild(title);

    // 3. 创建关卡按钮
    // 关卡 1
    createLevelButton(1, Vec2(visibleSize.width * 0.3, visibleSize.height / 2));
    // 关卡 2
    createLevelButton(2, Vec2(visibleSize.width * 0.5, visibleSize.height / 2));
    // 关卡 3
    createLevelButton(3, Vec2(visibleSize.width * 0.7, visibleSize.height / 2));

    // 4. 返回主菜单按钮
    auto backBtn = Button::create("CloseNormal.png");
    backBtn->setTitleText("Home");
    backBtn->setPosition(Vec2(50, visibleSize.height - 50));
    backBtn->addClickEventListener([=](Ref*) {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, HelloWorld::createScene()));
        });
    this->addChild(backBtn);

    return true;
}

void LevelMapScene::createLevelButton(int levelIndex, Vec2 pos) {
    auto btn = Button::create("CloseNormal.png");
    btn->setTitleText("Level " + std::to_string(levelIndex));
    btn->setTitleFontSize(24);
    btn->setColor(Color3B::GREEN); // 关卡按钮设为绿色
    btn->setPosition(pos);

    // 点击逻辑
    btn->addClickEventListener([=](Ref*) {
        // 【关键】跳转到战斗场景，并告诉它我要打第几关
        auto scene = BattleScene::createScene(levelIndex);
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    this->addChild(btn);
}