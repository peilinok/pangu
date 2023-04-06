#include "record_desktop_wgc.h"

#include "utils_string.h"

#include "system_error.h"
#include "error_define.h"
#include "log_helper.h"

BOOL WINAPI EnumMonitorProc(HMONITOR hmonitor, HDC hdc, LPRECT lprc,
                            LPARAM data) {

  MONITORINFOEX info_ex;
  info_ex.cbSize = sizeof(MONITORINFOEX);

  GetMonitorInfo(hmonitor, &info_ex);

  if (info_ex.dwFlags == DISPLAY_DEVICE_MIRRORING_DRIVER)
    return true;

  if (info_ex.dwFlags & MONITORINFOF_PRIMARY) {
    *(HMONITOR *)data = hmonitor;
  }

  return true;
}

HMONITOR GetPrimaryMonitor() {
  HMONITOR hmonitor = nullptr;

  ::EnumDisplayMonitors(NULL, NULL, EnumMonitorProc, (LPARAM)&hmonitor);

  return hmonitor;
}

namespace am {


record_desktop_wgc::record_desktop_wgc() {}

record_desktop_wgc::~record_desktop_wgc() {
  stop();
  clean_up();
}

int record_desktop_wgc::init(const RECORD_DESKTOP_RECT &rect, const int fps) {
  int error = AE_NO;
  if (_inited == true)
    return error;

  _fps = fps;
  _rect = rect;
  _start_time = av_gettime_relative();
  _time_base = {1, AV_TIME_BASE};
  _pixel_fmt = AV_PIX_FMT_BGRA;

  do {
    if (!module_.is_supported()) {
      error = AE_UNSUPPORT;
      break;
    }

    session_ = module_.create_session();
    if (!session_) {
      error = AE_WGC_CREATE_CAPTURER_FAILED;
      break;
    }

    session_->register_observer(this);

    error = session_->initialize(GetPrimaryMonitor());

    _inited = true;
  } while (0);

  if (error != AE_NO) {
    al_debug("%s,last error:%s", err2str(error),
             system_error::error2str(GetLastError()).c_str());
  }

  return error;
}

int record_desktop_wgc::start() {
  if (_running == true) {
    al_warn("record desktop duplication is already running");
    return AE_NO;
  }

  if (_inited == false) {
    return AE_NEED_INIT;
  }

  _running = true;
  session_->start();

  return AE_NO;
}

int record_desktop_wgc::pause() {
  _paused = true;
  if (session_)
    session_->pause();
  return AE_NO;
}

int record_desktop_wgc::resume() {
  _paused = false;
  if (session_)
    session_->resume();
  return AE_NO;
}

int record_desktop_wgc::stop() {
  _running = false;

  if (session_)
    session_->stop();

  return AE_NO;
}

void record_desktop_wgc::on_frame(const wgc_session::wgc_session_frame &frame) {
  al_debug("wgc on frame");
}

void record_desktop_wgc::clean_up() {
  _inited = false;

  if (session_)
    session_->release();

  session_ = nullptr;
}

} // namespace am