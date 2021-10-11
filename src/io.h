//
// Created by cpasjuste on 31/03/19.
//

#ifndef PPLAY_IO_H
#define PPLAY_IO_H

#include "cross2d/c2d.h"

class Browser;
class Enigma2;

namespace pplay {

    class Io : c2d::C2DIo {

    public:

        Io();

        ~Io() override;

        enum class DeviceType {
            Sdmc, Http, Ftp, Enigma2
        };

        std::vector<Io::File> getDirList(const DeviceType &type, const std::vector<std::string> &extensions,
                                         const std::string &path, bool sort = false, bool showHidden = false);

        DeviceType getType(const std::string &path) const;

    private:

        Browser *browser;
		Enigma2 *enigma2;

    };
}

#endif //PPLAY_IO_H
