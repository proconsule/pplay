//
// Created by cpasjuste on 09/01/19.
//

#include <sstream>
#include <iomanip>
#include <time.h>

#include "cross2d/c2d.h"
#include "status_bar.h"
#include "main.h"

#ifdef __SWITCH__

#include <switch.h>

#endif

using namespace c2d;

class Battery : public RectangleShape {

public:

	unsigned int percent = 100;
    explicit Battery(const FloatRect &rect) : RectangleShape(rect) {

        setFillColor(Color::Transparent);
        setOutlineColor(COLOR_FONT);
        setOutlineThickness(1);

        RectangleShape *rightRect = new RectangleShape({4, rect.height / 3});
        rightRect->setFillColor(COLOR_FONT);
        rightRect->setOrigin(Origin::Left);
        rightRect->setPosition(rect.width, rect.height / 2);
        add(rightRect);

        Vector2f size = {rect.width - 4, rect.height - 4};
        percentRect = new RectangleShape(size);
        percentRect->setFillColor(COLOR_FONT);
        percentRect->setOrigin(Origin::Left);
        percentRect->setPosition(2, rect.height / 2);
        add(percentRect);
    }

    void onUpdate() override {

        if (!isVisible()) {
            return;
        }
        
#ifdef __SWITCH__
        psmGetBatteryChargePercentage(&percent);
#endif
        float width = ((float) percent / 100) * (getSize().x - 2);
        percentRect->setSize(std::min(width, getSize().x - 4), percentRect->getSize().y);
        percentRect->setFillColor(percent > 15 ? COLOR_FONT : Color::Red);

        RectangleShape::onUpdate();
    }

    RectangleShape *percentRect = nullptr;
};

StatusBar::StatusBar(Main *m) : GradientRectangle({0, 0, m->getSize().x, 32 * m->getScaling()}) {

#ifdef __SWITCH__
    psmInitialize();
#endif

	main = m;

    float height = getLocalBounds().height;

    setColor(COLOR_BG, Color::Transparent);
    setPosition(0, -height);
    add(new TweenPosition(getPosition(), {0, 0}, 0.5f));

    battery = new Battery({main->getSize().x - 16, height / 2 + 1, (height - 16) * 2, height / 2});
    battery->setOrigin(Origin::Right);
    add(battery);
	
	batteryPercText = new Text("100%", main->getFontSize(Main::FontSize::Medium), main->getFont());
	batteryPercText->setOrigin(Origin::Right);
    batteryPercText->setPosition(battery->getPosition().x-batteryPercText->getLocalBounds().width+22, height / 2);
    batteryPercText->setFillColor(COLOR_FONT);
    add(batteryPercText);

    // time
    timeText = new Text("12:00", main->getFontSize(Main::FontSize::Medium), main->getFont());
    timeText->setOrigin(Origin::Right);
    timeText->setPosition(batteryPercText->getPosition().x - timeText->getLocalBounds().width - 22, height / 2);
    timeText->setFillColor(COLOR_FONT);
    add(timeText);
	setVisibility(Visibility::Visible, true);
}

void StatusBar::onUpdate() {

    if (!isVisible()) {
        return;
    }

    time_t time_raw;
    struct tm *time_struct;

    time(&time_raw);
    time_struct = localtime(&time_raw);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << time_struct->tm_hour << ":";
    oss << std::setfill('0') << std::setw(2) << time_struct->tm_min;
    timeText->setString(oss.str());
	GradientRectangle::onUpdate();
	char bperctext[32];
	snprintf(bperctext, sizeof bperctext, "%3d%s",battery->percent, "%");
	batteryPercText->setString(bperctext);
	
}

StatusBar::~StatusBar() {
#ifdef __SWITCH__
    psmExit();
#endif
}
