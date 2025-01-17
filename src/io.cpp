//
// Created by cpasjuste on 31/03/19.
//

#include <regex>
#include "io.h"
#include "media_info.h"
#include "ftplib.h"
#include "Browser/Browser.hpp"
#include "Enigma2/Enigma2.h"

using namespace pplay;

static size_t find_Nth(const std::string &str, unsigned n, const std::string &find) {

    size_t pos = std::string::npos, from = 0;
    unsigned i = 0;

    if (0 == n) {
        return std::string::npos;
    }

    while (i < n) {
        pos = str.find(find, from);
        if (std::string::npos == pos) { break; }
        from = pos + 1;
        ++i;
    }
    return pos;
}

Io::Io() : c2d::C2DIo() {
#ifndef NDEBUG
	socketInitializeDefault();
#endif
    // http io
    browser = new Browser();
    browser->set_handle_gzip(true);
    browser->set_handle_redirect(true);
    browser->set_handle_ssl(false);
    browser->fetch_forms(false);

    // ftp io
    FtpInit();
	
	//Enigma2 io
	enigma2 = new Enigma2();
}

std::vector<MediaFile> Io::getDirList(const pplay::Io::DeviceType &type, const std::vector<std::string> &extensions,
                                          const std::string &path, bool sort, bool showHidden) {

    std::vector<MediaFile> files;

    printf("Io::getDir(%s)\n", path.c_str());

    if (type == DeviceType::Sdmc) {
		std::vector<c2d::Io::File> sdmcfiles = c2d::C2DIo::getDirList(path, sort, showHidden);
		 for (auto &file : sdmcfiles) {
			MediaFile tmpfile(file,MediaInfo(file));
			files.push_back(tmpfile); 
		}
		
	} else if (type == DeviceType::Enigma2) {
		std::vector<c2d::Io::File> enigmafiles;
		if (c2d::Utility::startWith(path, "enigma2://")) {
			string teststr = path;
			teststr.erase(0,10);
            enigma2->enigma2ip = teststr;
			enigma2->getServices();
			enigmafiles.emplace_back("..", "..", Io::Type::Directory, 0);
			for(unsigned int i=0;i<enigma2->e2services.size();i++){
				enigmafiles.emplace_back(enigma2->e2services[i].name, enigma2->e2services[i].bouquetref, Io::Type::Directory, 0);
#ifdef NDEBUG
				printf("%s\n",enigma2->e2services[i].bouquetref.c_str());
#endif
			}
			for (auto &file : enigmafiles) {
				MediaFile tmpfile(file,MediaInfo(file));
				files.push_back(tmpfile); 
			}
			
        } if (c2d::Utility::endsWith(path, "bouquet")) {
			vector<ChannelStruct> channels = enigma2->m3uParser((char *)path.c_str());
			enigmafiles.emplace_back("..", path, Io::Type::Directory, 0);
			for(unsigned int i=0;i<channels.size();i++){
#ifdef NDEBUG
				printf("%s\n",channels[i].name.c_str());
#endif
				c2d::Io::File e2tmpfile(channels[i].name, channels[i].url, Io::Type::File, 0);
				MediaFile tmpfile(e2tmpfile,MediaInfo(e2tmpfile));
				tmpfile.isEnigma2 = true;
				tmpfile.epgtitle = channels[i].epgtitle;
				files.push_back(tmpfile);
			}
			
		}
		
    } else if (type == DeviceType::Http) {
		std::vector<c2d::Io::File> httpfiles;
        std::string http_path = path;
        if (!c2d::Utility::endsWith(http_path, "/")) {
            http_path += "/";
        }
        // extract home from path
        size_t pos = find_Nth(http_path, 3, "/");
        std::string home = http_path.substr(0, pos + 1);
        std::string dir = browser->escape(http_path.substr(pos + 1, http_path.length() - 1));
        dir = std::regex_replace(dir, std::regex("%2F"), "/");
        //printf("home: %s | dir: %s\n", home.c_str(), dir.c_str());
        browser->open(home + dir, 3);
        if (browser->error() || browser->links.size() < 1) {
            return files;
        }

        // add up/back ("..")
        httpfiles.emplace_back("..", "..", Io::Type::Directory, 0);

        for (int i = 0; i < browser->links.size(); i++) {
            // skip apache2 stuff
            if (browser->links[i].name() == ".."
                || browser->links[i].name() == "../"
                || browser->links[i].name() == "Name"
                || browser->links[i].name() == "Last modified"
                || browser->links[i].name() == "Size"
                || browser->links[i].name() == "Description"
                || browser->links[i].name() == "Parent Directory") {
                continue;
            }

            Io::Type t = c2d::Utility::endsWith(browser->links[i].name(), "/") ?
                         Io::Type::Directory : Io::Type::File;
            std::string name = browser->unescape(browser->links[i].name());
            if (c2d::Utility::endsWith(name, "/")) {
                name = c2d::Utility::removeLastSlash(name);
            }
            httpfiles.emplace_back(name, http_path + name, t);
        }
        if (sort) {
            std::sort(files.begin(), files.end(), compare);
        }
		for (auto &file : httpfiles) {
			MediaFile tmpfile(file,MediaInfo(file));
			files.push_back(tmpfile); 
		}
		
    } else if (type == DeviceType::Ftp) {
		std::string ftp_path = path;
        if (!c2d::Utility::endsWith(ftp_path, "/")) {
            ftp_path += "/";
        }
        // split user/pwd/host/port/path
        // TODO: check for nullptr etc..
        size_t colon_2 = find_Nth(ftp_path, 2, ":");
        size_t colon_3 = ftp_path.find_last_of(':');
        size_t at = ftp_path.find_last_of('@');
        size_t last_slash = find_Nth(ftp_path, 3, "/");
        std::string user = ftp_path.substr(6, colon_2 - 6);
        std::string pwd = ftp_path.substr(colon_2 + 1, at - colon_2 - 1);
        std::string host = ftp_path.substr(at + 1, colon_3 - at - 1);
        std::string port = ftp_path.substr(colon_3 + 1, ftp_path.find('/', colon_3) - (colon_3 + 1));
        std::string host_port = host + ":" + port;
        std::string new_path = ftp_path.substr(last_slash, ftp_path.length() - last_slash);
        if (c2d::Utility::startWith(new_path, "/")) {
            new_path.erase(0, 1);
        }

        printf("user: %s, pwd: %s, host: %s, port: %s, path: %s\n",
               user.c_str(), pwd.c_str(), host.c_str(), port.c_str(), new_path.c_str());

        netbuf *ftp_con = nullptr;
        if (!FtpConnect(host_port.c_str(), &ftp_con)) {
            printf("could not connect to ftp server");
            return files;
        }

        if (!FtpLogin(user.c_str(), pwd.c_str(), ftp_con)) {
            printf("could not connect to ftp server");
            FtpQuit(ftp_con);
            return files;
        }
        
        std::vector<Io::File> _files = FtpDirList(new_path.c_str(), ftp_con);
        _files.insert(_files.begin(), Io::File("..", "..", Io::Type::Directory, 0));
        for (auto &file : _files) {
            if (file.path != "..") {
                file.path = ftp_path + file.name;
            }
			MediaFile tmpfile(file,MediaInfo(file));
            files.push_back(tmpfile);
        }

        FtpQuit(ftp_con);
    }

    // remove items by extensions, if provided
    if (!extensions.empty() && type != DeviceType::Enigma2) {
        files.erase(
                std::remove_if(files.begin(), files.end(), [extensions](const Io::File &file) {
                    for (auto &ext : extensions) {
                        if (c2d::Utility::endsWith(file.name, ext, false)) {
                            return false;
                        }
                    }
                    return file.type == c2d::Io::Type::File;
                }), files.end());
    }

    return files;
}

Io::DeviceType Io::getType(const std::string &path) const {

    Io::DeviceType type = Io::DeviceType::Sdmc;

    if (c2d::Utility::startWith(path, "http://")) {
        type = pplay::Io::DeviceType::Http;
    } else if (c2d::Utility::startWith(path, "ftp://")) {
        type = pplay::Io::DeviceType::Ftp;
    } else if (c2d::Utility::startWith(path, "enigma2://")) {
        type = pplay::Io::DeviceType::Enigma2;
    } else if (c2d::Utility::endsWith(path, "bouquet")) {
        type = pplay::Io::DeviceType::Enigma2;
    }
	

    return type;
}

Io::~Io() {
	//printf("Deinit Browser\n");
    //delete (browser);
	printf("Deinit Enigma2\n");
	delete (enigma2);
}
