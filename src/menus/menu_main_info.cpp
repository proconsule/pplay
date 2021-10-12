//
// Created by proconsule on 12/10/21.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_main_info.h"

using namespace c2d;

MenuMainInfo::MenuMainInfo(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items)
        : Menu(main, rect, "INFO", items, true) {
}

void MenuMainInfo::onOptionSelection(MenuItem *item) {
	if(item->name == "MPV"){
		main->getInfoBox()->show("MPV Version",main->getPlayer()->getMpv()->mpv_version.c_str(), true, false);
	}
	if(item->name == "FFMPEG"){
		main->getInfoBox()->show("FFMPEG Version",main->getPlayer()->getMpv()->ffmpeg_version.c_str(), true, false);
	}
	
    //setVisibility(Visibility::Hidden);
    //main->getMenuMain()->getMenuMainOptionsSubmenu(item->name)->setVisibility(Visibility::Visible);
}

bool MenuMainInfo::onInput(c2d::Input::Player *players) {

    if (players[0].keys & Input::Right || players[0].keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
		main->getInfoBox()->hide();
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
        return true;
    }

    if (players[0].keys & Input::Left) {
        onOptionSelection(getSelection());
        return true;
    }

    return Menu::onInput(players);
}
