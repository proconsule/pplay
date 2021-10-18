//
// Created by cpasjuste on 09/12/18.
//

#include "cross2d/c2d.h"
#include "video_texture.h"
#include "main.h"

using namespace c2d;

static void on_mpv_render_update(void *ctx) {
    ((VideoTexture *) ctx)->fboUpdate = true;
}

VideoTexture::VideoTexture(Main *_main,const c2d::Vector2f &size, Mpv *m) : GLTextureBuffer(size, Format::RGBA8) {

	main = _main;
    mpv = m;

    mpv_render_context_set_update_callback(mpv->getContext(), on_mpv_render_update, this);

    // fade
    fade = new C2DTexture(c2d_renderer->getIo()->getRomFsPath() + "skin/overlay.png");
    fade->setScale(size.x / fade->getTextureRect().width, size.y / fade->getTextureRect().height);
    fade->setFillColor(Color::Black);
    fade->setAlpha(0);
    //fadeTween = new TweenAlpha(0, 255, 0.5f);
    //fade->add(fadeTween);
	
	cpuHzText = new Text("CPU Hz",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	cpuHzText->setOrigin(Origin::Left);
	cpuHzText->setPosition(16, 10);
	
	cpu0Text = new Text("CPU0",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	cpu0Text->setOrigin(Origin::Left);
	cpu0Text->setPosition(16, 20);
	cpu1Text = new Text("CPU1",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	cpu1Text->setOrigin(Origin::Left);
	cpu1Text->setPosition(16, 30);
	cpu2Text = new Text("CPU2",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	cpu2Text->setOrigin(Origin::Left);
	cpu2Text->setPosition(16, 40);
	
	gpuHzText = new Text("GPU Hz",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	gpuHzText->setOrigin(Origin::Left);
	gpuHzText->setPosition(16, 60);
	gpuLoadText = new Text("GPU Load",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	gpuLoadText->setOrigin(Origin::Left);
	gpuLoadText->setPosition(16, 70);
	
	ramHzText = new Text("RAM Hz",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	ramHzText->setOrigin(Origin::Left);
	ramHzText->setPosition(16, 90);
	ramStatsText = new Text("RAM Stats",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	ramStatsText->setOrigin(Origin::Left);
	ramStatsText->setPosition(16, 100);
	
	socTempText = new Text("SOC Temp",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	socTempText->setOrigin(Origin::Left);
	socTempText->setPosition(16, 120);
	
	fanSpeedText = new Text("FAN Speed",main->getFontSize(Main::FontSize::VerySmall), main->getFont());
	fanSpeedText->setOrigin(Origin::Left);
	fanSpeedText->setPosition(16, 130);
	
	fade->add(cpuHzText);
	fade->add(cpu0Text);
	fade->add(cpu1Text);
	fade->add(cpu2Text);
	
	fade->add(gpuHzText);
	fade->add(gpuLoadText);
	
	fade->add(ramHzText);
	fade->add(ramStatsText);
	
	fade->add(socTempText);
	fade->add(fanSpeedText);
	
	GLTextureBuffer::add(fade);
	fade->setVisibility(Visibility::Hidden);
}

void VideoTexture::setCpuText(){
	main->getCpuMeter()->GetCores_Perc();
	
	cpuHzText->setString(main->getCpuMeter()->CPU_Hz_c);
	cpu0Text->setString(main->getCpuMeter()->CPU_Usage0);
	cpu1Text->setString(main->getCpuMeter()->CPU_Usage1);
	cpu2Text->setString(main->getCpuMeter()->CPU_Usage2);
	
	gpuHzText->setString(main->getCpuMeter()->GPU_Hz_c);
	gpuLoadText->setString(main->getCpuMeter()->GPU_Load_c);
	
	ramHzText->setString(main->getCpuMeter()->RAM_Hz_c);
	ramStatsText->setString(main->getCpuMeter()->RAM_all_c);
	
	socTempText->setString(main->getCpuMeter()->SoCPCB_temperature_c);
	fanSpeedText->setString(main->getCpuMeter()->Rotation_SpeedLevel_c);
	
	
	if(main->getPlayer()->isStatusOverlay() && main->getPlayer()->isFullscreen()){
		
		
		fade->setVisibility(Visibility::Visible);
		
		
	}else{
		fade->setVisibility(Visibility::Hidden);
		
	}
	
}

void VideoTexture::hideFade() {
    //fadeTween->play(TweenDirection::Backward);
	
}

void VideoTexture::showFade() {
    //fadeTween->play(TweenDirection::Forward);
}

mpv_render_context *VideoTexture::getContext() {
    return mpv->getContext();
}

void VideoTexture::onDraw(c2d::Transform &transform, bool draw) {

    if (draw && fboUpdate) {
        int flip_y{0};
        mpv_opengl_fbo mpv_fbo{
                .fbo = (int) fbo,
                .w = (int) getSize().x, .h = (int) getSize().y,
                .internal_format = GL_RGB};
        mpv_render_param r_params[] = {
                {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
                {MPV_RENDER_PARAM_FLIP_Y,     &flip_y},
                {MPV_RENDER_PARAM_INVALID,    nullptr}
        };

        mpv_render_context_render(mpv->getContext(), r_params);
        fboUpdate = false;
		setCpuText();
    }
	

    GLTextureBuffer::onDraw(transform, draw);
}
