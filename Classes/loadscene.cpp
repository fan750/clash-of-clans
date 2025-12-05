/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "loadscene.h"
#include "SimpleAudioEngine.h"
#include"HelloWorldScene.h"

USING_NS_CC;

Scene* Load::createScene()
{
    return Load::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool Load::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto layer = Layer::create();
    auto sprite = Sprite::create("load.jpg");
    if (sprite == nullptr)
    {
        problemLoading("load.jpg");
    }
    else
    {
        // position the sprite on the center of the screen
        Size texSize = sprite->getContentSize();
        float scaleX = visibleSize.width / texSize.width;
        float scaleY = visibleSize.height / texSize.height;
        float scale = std::max(scaleX, scaleY); // 等比缩放，完整显示
        sprite->setScale(scale);
        sprite->setPosition(visibleSize / 2);
        layer->addChild(sprite);
    }
    auto beginitem = MenuItemImage::create("play.png", "play.png", CC_CALLBACK_1(Load::onAssaultMenuCallback, this));
    beginitem->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.2f));
    beginitem->setScale(1.0f);
    auto menu = Menu::create(beginitem, NULL);
    menu->setPosition(Vec2::ZERO);
    layer->addChild(menu);
    this->addChild(layer);
    return true;
}


void Load::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);



}

void Load::onAssaultMenuCallback(Ref* pSender)
{
    Director::getInstance()->replaceScene(HelloWorld::createScene());
}
