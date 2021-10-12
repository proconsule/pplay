//
// Created by proconsule on 12/10/21.
//

#ifndef PPLAY_MENU_MAIN_INFO_H
#define PPLAY_MENU_MAIN_INFO_H

#include "menu.h"

class MenuMainInfo : public Menu {

public:

    MenuMainInfo(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items);

private:

    bool onInput(c2d::Input::Player *players) override;

    void onOptionSelection(MenuItem *item) override;
};


#endif //PPLAY_MENU_MAIN_INFO_H
