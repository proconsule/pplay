//
// Created by cpasjuste on 22/10/18.
//

#include "main.h"
#include "pplay_config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(Main *main, int version)
        : Config("PPLAY", main->getIo()->getDataPath() + "pplay.cfg", version) {

    addOption({OPT_NETWORK, "http://samples.ffmpeg.org/"});
	addOption({OPT_ENIGMA2_IP, "enigma2://127.0.0.1/"});
#ifdef __SWITCH__
    addOption({OPT_UMS_DEVICE, "ums0:/"});
#endif
    addOption({OPT_HOME_PATH, main->getIo()->getDataPath()});
    addOption({OPT_LAST_PATH, main->getIo()->getDataPath()});
    addOption({OPT_CACHE_MEDIA_INFO, (int) 1});
    //addOption({OPT_BUFFER, "Low"}); // Low, Medium, High, VeryHigh
    addOption({OPT_CPU_BOOST, "Disabled"}); // Disabled, Enabled

    // load the configuration from file, overwriting default values
    load();

    if (!main->getIo()->exist(getOption(OPT_HOME_PATH)->getString())) {
        getOption(OPT_HOME_PATH)->setString(main->getIo()->getDataPath());
    }

    if (!main->getIo()->exist(getOption(OPT_LAST_PATH)->getString())) {
        getOption(OPT_LAST_PATH)->setString(main->getIo()->getDataPath());
    }

    // save configuration, in case new options needs to be added
    save();
}
