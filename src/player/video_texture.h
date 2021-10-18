//
// Created by cpasjuste on 09/12/18.
//

#ifndef PPLAY_VIDEO_TEXTURE_H
#define PPLAY_VIDEO_TEXTURE_H

#include "player.h"
#include "gradient_rectangle.h"

class Main;

class VideoTexture : public c2d::GLTextureBuffer {

public:

    explicit VideoTexture(Main *_main,const c2d::Vector2f &size, Mpv *mpv);

    void showFade();

    void hideFade();
	
	void setCpuText();

    mpv_render_context *getContext();

	

    bool fboUpdate = false;

private:

    void onDraw(c2d::Transform &transform, bool draw = true) override;

	Main *main = nullptr;
    Mpv *mpv = nullptr;
    c2d::Texture *fade = nullptr;
	c2d::Texture *overlayStatus = nullptr;
    c2d::TweenAlpha *fadeTween = nullptr;
	
	c2d::Text *cpuHzText = nullptr;
	
	c2d::Text *cpu0Text = nullptr;
	c2d::Text *cpu1Text = nullptr;
	c2d::Text *cpu2Text = nullptr;
	
	c2d::Text *gpuHzText = nullptr;
	c2d::Text *gpuLoadText = nullptr;
	
	c2d::Text *ramHzText = nullptr;
	c2d::Text *ramStatsText = nullptr;
	
	c2d::Text *socTempText = nullptr;
	c2d::Text *fanSpeedText = nullptr;
	
};

#endif //PPLAY_VIDEO_TEXTURE_H
