//
// Created by proconsule on 12/10/21.
//

#ifndef PPLAY_INFO_BOX_H
#define PPLAY_INFO_BOX_H

#include "cross2d/skeleton/sfml/RectangleShape.hpp"

class Main;

class InfoBox : public c2d::Rectangle {

public:

    explicit InfoBox(Main *main, const c2d::Vector2f &position);

    ~InfoBox() override;

    void show(const std::string &title, const std::string &message, bool infinite = false, bool drawNow = false);

    void hide();

    void setAlpha(uint8_t alpha, bool recursive = false) override;

private:

    virtual void onDraw(c2d::Transform &transform, bool draw = true) override;

    Main *main;
    c2d::Vector2f pos;
    //c2d::Texture *icon;
    c2d::Text *titleText;
    c2d::Text *messageText;
    c2d::TweenAlpha *tween;
    c2d::Clock *clock;
    SDL_mutex *mutex;
    bool infinite = false;

    //friend class ScrapperStatusBox;
};

#endif //PPLAY_INFO_BOX_H
