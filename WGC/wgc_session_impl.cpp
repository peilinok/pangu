#include "pch.h"

#include <memory>

#define CHECK_INIT                                                             \
  if (!is_initialized_)                                                        \
  return AM_ERROR::AE_NEED_INIT

namespace am {

wgc_session_impl::wgc_session_impl() {}

wgc_session_impl::~wgc_session_impl() {}

void wgc_session_impl::release() { delete this; }

int wgc_session_impl::initialize(HWND hwnd) {
  target_.hwnd = hwnd;
  target_.is_window = true;
  return initialize();
}

int wgc_session_impl::initialize(HMONITOR hmonitor) {
  target_.hmonitor = hmonitor;
  target_.is_window = false;
  return initialize();
}

void wgc_session_impl::register_observer(const wgc_session_observer *observer) {
}

void wgc_session_impl::unregister_observer(
    const wgc_session_observer *observer) {}

int wgc_session_impl::start() {
  CHECK_INIT;
  return 0;
}

int wgc_session_impl::stop() {
  CHECK_INIT;
  return 0;
}

int wgc_session_impl::pause() {
  CHECK_INIT;
  return 0;
}

int wgc_session_impl::resume() {
  CHECK_INIT;
  return 0;
}

int wgc_session_impl::initialize() {
  if (is_initialized_)
    return AM_ERROR::AE_NO;

  if (!(device_ = create_d3d11_device()))
    return AM_ERROR::AE_D3D_CREATE_DEVICE_FAILED;

  std::unique_ptr<SimpleCapture> capturer;
  try {

    if (target_.is_window)
      capturer = std::make_unique<SimpleCapture>(
          device_, CreateCaptureItemForWindow(target_.hwnd));
    else
      capturer = std::make_unique<SimpleCapture>(
          device_, CreateCaptureItemForMonitor(target_.hmonitor));
  } catch (winrt::hresult_error) {
    return AM_ERROR::AE_WGC_CREATE_CAPTURER_FAILED;
  } catch (...) {
    return AM_ERROR::AE_WGC_CREATE_CAPTURER_FAILED;
  }

  capturer->StartCapture();

  return 0;
}

auto wgc_session_impl::create_d3d11_device() {
  try {
    auto d3d_device = CreateD3DDevice();
    auto dxgi_device = d3d_device.as<IDXGIDevice>();
    return CreateDirect3DDevice(dxgi_device.get());
  } catch (...) {
    return nullptr;
  }
}

} // namespace am