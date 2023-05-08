#include "record_desktop_mag.h"

#include "error_define.h"
#include "log_helper.h"

namespace am {

namespace {
// kMagnifierWindowClass has to be "Magnifier" according to the Magnification
// API. The other strings can be anything.
static wchar_t kMagnifierHostClass[] = L"ScreenCapturerWinMagnifierHost";
static wchar_t kHostWindowName[] = L"MagnifierHost";
static wchar_t kMagnifierWindowClass[] = L"Magnifier";
static wchar_t kMagnifierWindowName[] = L"MagnifierWindow";

DWORD GetTlsIndex() {
  static const DWORD tls_index = TlsAlloc();
  return tls_index;
}
} // namespace

BOOL __stdcall record_desktop_mag::on_mag_scaling_callback(
    HWND hwnd, void *srcdata, MAGIMAGEHEADER srcheader, void *destdata,
    MAGIMAGEHEADER destheader, RECT unclipped, RECT clipped, HRGN dirty) {
  record_desktop_mag *owner =
      reinterpret_cast<record_desktop_mag *>(TlsGetValue(GetTlsIndex()));
  TlsSetValue(GetTlsIndex(), nullptr);
  owner->on_mag_data(srcdata, srcheader);
  return TRUE;
}

record_desktop_mag::record_desktop_mag() {}

record_desktop_mag::~record_desktop_mag() {}

int record_desktop_mag::init(const RECORD_DESKTOP_RECT &rect, const int fps) {
  if (_inited == true) {
    return AE_NO;
  }

  _fps = fps;
  _rect = rect;
  _width = rect.right - rect.left;
  _height = rect.bottom - rect.top;

  _start_time = av_gettime_relative();
  _time_base = {1, AV_TIME_BASE};
  _pixel_fmt = AV_PIX_FMT_BGRA;

  _inited = true;

  return AE_NO;
}

int record_desktop_mag::start() {
  if (_running == true) {
    al_warn("record desktop gdi is already running");
    return AE_NO;
  }

  if (_inited == false) {
    return AE_NEED_INIT;
  }

  _running = true;
  _thread = std::thread(std::bind(&record_desktop_mag::record_func, this));

  return AE_NO;
}

int record_desktop_mag::pause() {
  _paused = true;
  return AE_NO;
}

int record_desktop_mag::resume() {
  _paused = false;
  return AE_NO;
}

int record_desktop_mag::stop() {
  _running = false;
  if (_thread.joinable())
    _thread.join();

  return AE_NO;
}

void record_desktop_mag::clean_up() {
  // DestroyWindow must be called before MagUninitialize. magnifier_window_ is
  // destroyed automatically when host_window_ is destroyed.
  if (host_window_)
    DestroyWindow(host_window_);
  if (magnifier_initialized_)
    mag_uninitialize_func_();
  if (mag_lib_handle_)
    FreeLibrary(mag_lib_handle_);
  if (desktop_dc_)
    ReleaseDC(NULL, desktop_dc_);

  _inited = false;
}

void record_desktop_mag::record_func() {
  int64_t pre_pts = 0;
  int64_t dur = AV_TIME_BASE / _fps;

  int ret = AE_NO;

  // must call this in a new thread, otherwise SetWindowPos will stuck before
  // capture
  if (!do_mag_initialize()) {
    al_info("Failed to initialize ScreenCapturerWinMagnifier.");
    if (_on_error)
      _on_error(AE_NEED_INIT);

    return;
  }

  while (_running) {
    ret = do_mag_record();
    if (ret != AE_NO) {
      if (_on_error)
        _on_error(ret);
      break;
    }

    do_sleep(dur, pre_pts, _current_pts);

    pre_pts = _current_pts;
  }
}

void record_desktop_mag::do_sleep(int64_t dur, int64_t pre, int64_t now) {
  int64_t delay = now - pre;
  dur = delay > dur ? max(0, dur - (delay - dur)) : (dur + dur - delay);

  // al_debug("%lld", delay);

  if (dur)
    av_usleep(dur);
}

bool record_desktop_mag::do_mag_initialize() {
#if 0 // we can handle crash
      if (GetSystemMetrics(SM_CMONITORS) != 1) {
    // Do not try to use the magnifier in multi-screen setup (where the API
    // crashes sometimes).
    al_info("Magnifier capturer cannot work on multi-screen system.");
    return false;
  }
#endif
  desktop_dc_ = GetDC(nullptr);
  mag_lib_handle_ = LoadLibraryW(L"Magnification.dll");
  if (!mag_lib_handle_)
    return false;
  // Initialize Magnification API function pointers.
  mag_initialize_func_ = reinterpret_cast<MagInitializeFunc>(
      GetProcAddress(mag_lib_handle_, "MagInitialize"));
  mag_uninitialize_func_ = reinterpret_cast<MagUninitializeFunc>(
      GetProcAddress(mag_lib_handle_, "MagUninitialize"));
  mag_set_window_source_func_ = reinterpret_cast<MagSetWindowSourceFunc>(
      GetProcAddress(mag_lib_handle_, "MagSetWindowSource"));
  mag_set_window_filter_list_func_ =
      reinterpret_cast<MagSetWindowFilterListFunc>(
          GetProcAddress(mag_lib_handle_, "MagSetWindowFilterList"));
  mag_set_image_scaling_callback_func_ =
      reinterpret_cast<MagSetImageScalingCallbackFunc>(
          GetProcAddress(mag_lib_handle_, "MagSetImageScalingCallback"));
  if (!mag_initialize_func_ || !mag_uninitialize_func_ ||
      !mag_set_window_source_func_ || !mag_set_window_filter_list_func_ ||
      !mag_set_image_scaling_callback_func_) {
    al_info(
        "Failed to initialize ScreenCapturerWinMagnifier: library functions "
        "missing.");
    return false;
  }
  BOOL result = mag_initialize_func_();
  if (!result) {
    al_info("Failed to initialize ScreenCapturerWinMagnifier: error from "
            "MagInitialize %ld",
            GetLastError());
    return false;
  }
  HMODULE hInstance = nullptr;
  result =
      GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         reinterpret_cast<char *>(&DefWindowProc), &hInstance);
  if (!result) {
    mag_uninitialize_func_();
    al_info("Failed to initialize ScreenCapturerWinMagnifier: "
            "error from GetModulehandleExA %ld",
            GetLastError());
    return false;
  }
  // Register the host window class. See the MSDN documentation of the
  // Magnification API for more infomation.
  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.lpfnWndProc = &DefWindowProc;
  wcex.hInstance = hInstance;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.lpszClassName = kMagnifierHostClass;
  // Ignore the error which may happen when the class is already registered.
  RegisterClassExW(&wcex);
  // Create the host window.
  host_window_ =
      CreateWindowExW(WS_EX_LAYERED, kMagnifierHostClass, kHostWindowName, 0, 0,
                      0, 0, 0, nullptr, nullptr, hInstance, nullptr);
  if (!host_window_) {
    mag_uninitialize_func_();
    al_info("Failed to initialize ScreenCapturerWinMagnifier: "
            "error from creating host window %ld",
            GetLastError());
    return false;
  }
  // Create the magnifier control.
  magnifier_window_ = CreateWindowW(kMagnifierWindowClass, kMagnifierWindowName,
                                    WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                                    host_window_, nullptr, hInstance, nullptr);
  if (!magnifier_window_) {
    mag_uninitialize_func_();
    al_info("Failed to initialize ScreenCapturerWinMagnifier: "
            "error from creating magnifier window %ld",
            GetLastError());
    return false;
  }
  // Hide the host window.
  ShowWindow(host_window_, SW_HIDE);
  // Set the scaling callback to receive captured image.
  result = mag_set_image_scaling_callback_func_(
      magnifier_window_, &record_desktop_mag::on_mag_scaling_callback);
  if (!result) {
    mag_uninitialize_func_();
    al_info("Failed to initialize ScreenCapturerWinMagnifier: "
            "error from MagSetImageScalingCallback %ld",
            GetLastError());
    return false;
  }
  if (excluded_window_) {
    result = mag_set_window_filter_list_func_(
        magnifier_window_, MW_FILTERMODE_EXCLUDE, 1, &excluded_window_);
    if (!result) {
      mag_uninitialize_func_();
      al_warn("Failed to initialize ScreenCapturerWinMagnifier: "
              "error from MagSetWindowFilterList %ld",
              GetLastError());
      return false;
    }
  }
  magnifier_initialized_ = true;
  return true;
}

int record_desktop_mag::do_mag_record() {
  if (!magnifier_initialized_) {
    al_error("Magnifier initialization failed.");
    return AE_NEED_INIT;
  }

  auto capture_image = [&](const RECORD_DESKTOP_RECT &rect) {
    // Set the magnifier control to cover the captured rect. The content of the
    // magnifier control will be the captured image.

    BOOL result =
        SetWindowPos(magnifier_window_, NULL, rect.left, rect.top,
                     rect.right - rect.left, rect.bottom - rect.top, 0);
    if (!result) {
      al_error("Failed to call SetWindowPos: %d. Rect = {%d, %d, %d, %d}",
               GetLastError(), rect.left, rect.top, rect.right, rect.bottom);
      return false;
    }

    magnifier_capture_succeeded_ = false;
    RECT native_rect = {rect.left, rect.top, rect.right, rect.bottom};
    TlsSetValue(GetTlsIndex(), this);

    // on_mag_data will be called via on_mag_scaling_callback and fill in the
    // frame before mag_set_window_source_func_ returns.
    result = mag_set_window_source_func_(magnifier_window_, native_rect);
    if (!result) {
      al_error("Failed to call MagSetWindowSource: %d. Rect = {%d, %d, %d, %d}",
               GetLastError(), rect.left, rect.top, rect.right, rect.bottom);
      return false;
    }
    return magnifier_capture_succeeded_;
  };

#if 0
  // Switch to the desktop receiving user input if different from the current
  // one.
  std::unique_ptr<Desktop> input_desktop(Desktop::GetInputDesktop());
  if (input_desktop.get() != NULL && !desktop_.IsSame(*input_desktop)) {
    // Release GDI resources otherwise SetThreadDesktop will fail.
    if (desktop_dc_) {
      ReleaseDC(NULL, desktop_dc_);
      desktop_dc_ = NULL;
    }
    // If SetThreadDesktop() fails, the thread is still assigned a desktop.
    // So we can continue capture screen bits, just from the wrong desktop.
    desktop_.SetThreadDesktop(input_desktop.release());
  }

  DesktopRect rect = GetScreenRect(current_screen_id_, current_device_key_);
#endif

  // capture_image may fail in some situations, e.g. windows8 metro mode. So
  // defer to the fallback capturer if magnifier capturer did not work.
  if (!capture_image(_rect)) {
    al_error("Magnifier capturer failed to capture a frame.");
    return AE_ERROR;
  }

  return AE_NO;
}

void record_desktop_mag::set_exclude(HWND excluded_window) {
  excluded_window_ = excluded_window;
  if (excluded_window_ && magnifier_initialized_) {
    mag_set_window_filter_list_func_(magnifier_window_, MW_FILTERMODE_EXCLUDE,
                                     1, &excluded_window_);
  }
}

void record_desktop_mag::on_mag_data(void *data, const MAGIMAGEHEADER &header) {
  const int kBytesPerPixel = 4;

  int captured_bytes_per_pixel = header.cbSize / header.width / header.height;
  if (header.format != GUID_WICPixelFormat32bppRGBA ||
      header.width != static_cast<UINT>(_width) ||
      header.height != static_cast<UINT>(_height) ||
      header.stride != static_cast<UINT>(kBytesPerPixel*_width) ||
      captured_bytes_per_pixel != kBytesPerPixel) {
    al_warn("Output format does not match the captured format: width = %d, "
            "height = %d, stride= %d, bpp = %d, pixel format RGBA ? %d.",
            header.width, header.height, header.stride,
            captured_bytes_per_pixel,
            (header.format == GUID_WICPixelFormat32bppRGBA));
    return;
  }

  _current_pts = av_gettime_relative();

  AVFrame *frame = av_frame_alloc();
  frame->pts = _current_pts;
  frame->pkt_dts = frame->pts;

  frame->width = _width;
  frame->height = _height;
  frame->format = AV_PIX_FMT_BGRA;
  frame->pict_type = AV_PICTURE_TYPE_I;
  frame->pkt_size = _width * _height * 4;

  av_image_fill_arrays(frame->data, frame->linesize,
                       reinterpret_cast<uint8_t *>(data), AV_PIX_FMT_BGRA,
                       _width, _height, 1);

  if (_on_data)
    _on_data(frame);

  av_frame_free(&frame);

  magnifier_capture_succeeded_ = true;
}

} // namespace am