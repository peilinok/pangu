#include "record_desktop_wgc.h"

namespace am {
record_desktop_wgc::record_desktop_wgc() {}

record_desktop_wgc::~record_desktop_wgc() {}

int record_desktop_wgc::init(const RECORD_DESKTOP_RECT &rect, const int fps) {
  return 0;
}

int record_desktop_wgc::start() { return 0; }

int record_desktop_wgc::pause() { return 0; }

int record_desktop_wgc::resume() { return 0; }

int record_desktop_wgc::stop() { return 0; }

void record_desktop_wgc::clean_up() {}

bool record_desktop_wgc::is_supported() { return false; }

} // namespace am