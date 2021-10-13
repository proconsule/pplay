//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"
#include "utility.h"

#define ITEM_HEIGHT 50

using namespace c2d;

Filer::Filer(Main *m, const std::string &path, const c2d::FloatRect &rect) : Rectangle(rect) {

    main = m;

    Vector2f size = {rect.width - 670, rect.height - 64};
    // highlight
    highlight = new Highlight({size.x, ITEM_HEIGHT * main->getScaling()}, Highlight::CursorPosition::Left);
    add(highlight);

    // items
    item_height = ITEM_HEIGHT * main->getScaling();
    item_max = (int) (size.y / item_height);
    if ((item_max * item_height) < size.y) {
        item_height = size.y / (float) item_max;
    }

    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        FloatRect r = {0, (item_height * i) + 1, size.x - 2, item_height - 2};
        items.emplace_back(new FilerItem(main, r));
        add(items[i]);
    }

    // tween
    add(new TweenAlpha(0, 255, 0.5f));
}

void Filer::setMediaInfo(const MediaFile &target, const MediaInfo &mediaInfo) {

    for (size_t i = 0; i < files.size(); i++) {
        if (files[i].path == target.path) {
            files[i].mediaInfo = mediaInfo;
            dirty = true;
            break;
        }
    }
}

void Filer::setSelection(int index) {

    item_index = index;
    int page = item_index / item_max;
    unsigned int index_start = (unsigned int) page * item_max;

    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        if (index_start + i >= files.size()) {
            items[i]->setVisibility(Visibility::Hidden);
        } else {
            // load media info, set file
            MediaFile file(files[index_start + i]);
            items[i]->setFile(file);
            items[i]->setVisibility(Visibility::Visible);
            // set highlight position
            if (index_start + i == (unsigned int) item_index) {
                highlight->tweenTo(items[i]->getPosition());
                
            }
        }
    }

    if (files.empty()) {
        highlight->setVisibility(Visibility::Hidden);
    } else {
        highlight->setVisibility(Visibility::Visible);
    }
}

MediaFile Filer::getSelection() const {

    if (!files.empty() && files.size() > (unsigned int) item_index) {
        return files[item_index];
    }

    return MediaFile();
}

bool Filer::onInput(c2d::Input::Player *players) {

    if (main->getMenuMain()->isMenuVisible()
        || main->getPlayer()->isFullscreen()) {
        return false;
    }

    unsigned int keys = players[0].keys;

    if (keys & c2d::Input::Start || keys & c2d::Input::Select) {
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Up) {
        item_index--;
        if (item_index < 0)
            item_index = (int) (files.size() - 1);
        setSelection(item_index);
    } else if (keys & Input::Key::Down) {
        item_index++;
        if (item_index >= (int) files.size()) {
            item_index = 0;
        }
        setSelection(item_index);
    } else if (keys & Input::Key::Left) {
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Right) {
        if (!main->getPlayer()->getMpv()->isStopped()
            && !main->getPlayer()->isFullscreen()) {
            main->getPlayer()->setFullscreen(true);
        }
    } else if (keys & Input::Key::Fire1) {
        if (getSelection().type == Io::Type::Directory) {
            enter(item_index);
        } else if (pplay::Utility::isMedia(getSelection())) {
            main->getPlayer()->load(files[item_index]);
        }else if (getSelection().type == Io::Type::File && c2d::Utility::startWith(getSelection().path, "enigma2://")) {
			MediaFile myfile = files[item_index];
			myfile.path.erase(0,10);
			main->getPlayer()->load(myfile);
		}
		
    } else if (keys & Input::Key::Fire2) {
        exit();
    } else if (keys & Input::Key::Fire3) {
		
    }

    return true;
}

void Filer::onUpdate() {

    if (dirty) {
        setSelection(item_index);
        dirty = false;
    }

    C2DObject::onUpdate();
}

static bool compare(const MediaFile &a, const MediaFile &b) {

    if (a.type == Io::Type::Directory && b.type != Io::Type::Directory) {
        return true;
    }
    if (a.type != Io::Type::Directory && b.type == Io::Type::Directory) {
        return false;
    }

    return Utility::toLower(a.name) < Utility::toLower(b.name);
}

bool Filer::getDir(const std::string &p) {
#ifdef NDEBUG
    printf("getDir(%s)\n", p.c_str());
#endif
    files.clear();
    path = p;
    if (path.size() > 1 && Utility::endsWith(path, "/")) {
        path = Utility::removeLastSlash(path);
    }

    std::vector<std::string> ext = pplay::Utility::getMediaExtensions();
    pplay::Io::DeviceType type = ((pplay::Io *) main->getIo())->getType(p);
    std::vector<MediaFile> _files =
            ((pplay::Io *) main->getIo())->getDirList(type, ext, path, false);

	
    for (auto &file : _files) {
		/*
        MediaFile mf(file, MediaInfo(file));
        if (file.type == Io::Type::File) {
            
        }
        files.emplace_back(mf);
		*/
		files.push_back(file);
    }
	
    // sort after title have been scrapped
	if(type!=pplay::Io::DeviceType::Enigma2){
		std::sort(files.begin(), files.end(), compare);
	}
    if (files.empty() || files.at(0).name != "..") {
        Io::File file("..", "..", Io::Type::Directory, 0);
        files.insert(files.begin(), MediaFile{file, MediaInfo(file)});
    }

    setSelection(0);

    return true;
}

void Filer::enter(int index) {

    MediaFile file = getSelection();
    bool success;

    if (file.name == "..") {
        exit();
        return;
    }
	
	if(c2d::Utility::endsWith(file.path, "bouquet")){
		success = getDir(file.path);
	}
    else if (path == "/") {
        success = getDir(path + file.name);
    } else {
        success = getDir(path + "/" + file.name);
    }
    if (success) {
        item_index_prev.push_back(index);
        setSelection(item_index);
    }
}

void Filer::exit() {

    std::string p = path;
	
	pplay::Io::DeviceType type = ((pplay::Io *) main->getIo())->getType(p);
	if (type == pplay::Io::DeviceType::Enigma2) {
		p=main->getConfig()->getOption(OPT_ENIGMA2_IP)->getString();
	}
	if (p == "/" || p.find('/') == std::string::npos) {
        return;
    }

    if (type != pplay::Io::DeviceType::Sdmc) {
        std::string s = p;
        if (!Utility::endsWith(s, "/")) {
            s += "/";
        }
        if (s == main->getConfig()->getOption(OPT_NETWORK)->getString()) {
            return;
        }
    }

    while (p.back() != '/') {
        p.erase(p.size() - 1);
    }

    if (p.size() > 1 && Utility::endsWith(p, "/")) {
        p.erase(p.size() - 1);
    }

    if (getDir(p)) {
        if (!item_index_prev.empty()) {
            int last = (int) item_index_prev.size() - 1;
            if (item_index_prev[last] < (int) files.size()) {
                item_index = item_index_prev[last];
            }
            item_index_prev.erase(item_index_prev.end() - 1);
        }
        setSelection(item_index);
    }
}

void Filer::clearHistory() {
    item_index_prev.clear();
}

std::string Filer::getPath() {
    return path;
}
