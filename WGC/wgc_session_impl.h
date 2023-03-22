#pragma once

namespace am {
class wgc_session_impl : public wgc_session {
  struct {
    union {
      HWND hwnd;
      HMONITOR hmonitor;
    };
    bool is_window;
  } target_{0};

public:
  wgc_session_impl();
  ~wgc_session_impl();

public:
  void release() override;

  int initialize(HWND hwnd) override;
  int initialize(HMONITOR hmonitor) override;

  void register_observer(const wgc_session_observer *observer) override;
  void unregister_observer(const wgc_session_observer *observer) override;

  int start() override;
  int stop() override;

  int pause() override;
  int resume() override;

private:
  int initialize();
  auto create_d3d11_device();

private:
  bool is_initialized_ = false;
  bool is_running_ = false;
  bool is_paused_ = false;

  wgc_session_observer *observer_ = nullptr;

  // wgc
  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device_{
      nullptr};
};

} // namespace am