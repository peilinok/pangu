#pragma once

// documents
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/magapi/magapi-intro
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/magapi/magapi-whats-new
// https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Magnification/cpp/Windowed/MagnifierSample.cpp

#include <magnification.h>

#include "record_desktop.h"

namespace am {
class record_desktop_mag : public record_desktop {
  typedef BOOL(WINAPI *MagImageScalingCallback)(
      HWND hwnd, void *srcdata, MAGIMAGEHEADER srcheader, void *destdata,
      MAGIMAGEHEADER destheader, RECT unclipped, RECT clipped, HRGN dirty);
  typedef BOOL(WINAPI *MagInitializeFunc)(void);
  typedef BOOL(WINAPI *MagUninitializeFunc)(void);
  typedef BOOL(WINAPI *MagSetWindowSourceFunc)(HWND hwnd, RECT rect);
  typedef BOOL(WINAPI *MagSetWindowFilterListFunc)(HWND hwnd,
                                                   DWORD dwFilterMode,
                                                   int count, HWND *pHWND);
  typedef BOOL(WINAPI *MagSetImageScalingCallbackFunc)(
      HWND hwnd, MagImageScalingCallback callback);

  static BOOL WINAPI on_mag_scaling_callback(
      HWND hwnd, void *srcdata, MAGIMAGEHEADER srcheader, void *destdata,
      MAGIMAGEHEADER destheader, RECT unclipped, RECT clipped, HRGN dirty);

public:
  record_desktop_mag();
  ~record_desktop_mag() override;

  int init(const RECORD_DESKTOP_RECT &rect, const int fps) override;

  int start() override;
  int pause() override;
  int resume() override;
  int stop() override;

  void set_exclude(HWND excluded_window);

protected:
  void clean_up() override;

private:
  void record_func();

  void do_sleep(int64_t dur, int64_t pre, int64_t now);

  bool do_mag_initialize();

  int do_mag_record();

  void on_mag_data(void *data, const MAGIMAGEHEADER &header);

  bool seh_mag_set_window_source(HWND hwnd, RECT rect, DWORD &exception);

private:
  uint32_t _width = 0;
  uint32_t _height = 0;
  int64_t _current_pts = -1;

  // Used to exclude window with specified window id.
  HWND _excluded_window = NULL;

  // Used for getting the screen dpi.
  HDC _desktop_dc = NULL;

  // Module handler
  HMODULE _mag_lib_handle = NULL;

  // Mag functions
  MagInitializeFunc _mag_initialize_func = nullptr;
  MagUninitializeFunc _mag_uninitialize_func = nullptr;
  MagSetWindowSourceFunc _mag_set_window_source_func = nullptr;
  MagSetWindowFilterListFunc _mag_set_window_filter_list_func = nullptr;
  MagSetImageScalingCallbackFunc _mag_set_image_scaling_callback_func = nullptr;

  // The hidden window hosting the magnifier control.
  HWND _host_window = NULL;

  // The magnifier control that captures the screen.
  HWND _magnifier_window = NULL;

  // True if the magnifier control has been successfully initialized.
  bool _magnifier_initialized = false;

  // True if the last OnMagImageScalingCallback was called and handled
  // successfully. Reset at the beginning of each CaptureImage call.
  bool _magnifier_capture_succeeded = true;
};
} // namespace am