#include "pch.h"

#include <memory>

#define CHECK_INIT                                                             \
  if (!is_initialized_)                                                        \
  return AM_ERROR::AE_NEED_INIT

#define CHECK_CLOSED                                                           \
  if (cleaned_.load() == true) {                                               \
    throw winrt::hresult_error(RO_E_CLOSED);                                   \
  }

extern "C" {
HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(
    ::IDXGIDevice *dxgiDevice, ::IInspectable **graphicsDevice);
}

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
  CHECK_CLOSED;
  capture_session_.StartCapture();
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

auto wgc_session_impl::create_d3d11_device() {
  auto create_d3d_device = [](D3D_DRIVER_TYPE const type,
                              winrt::com_ptr<ID3D11Device> &device) {
    WINRT_ASSERT(!device);

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    //#ifdef _DEBUG
    //	flags |= D3D11_CREATE_DEVICE_DEBUG;
    //#endif

    return ::D3D11CreateDevice(nullptr, type, nullptr, flags, nullptr, 0,
                               D3D11_SDK_VERSION, device.put(), nullptr,
                               nullptr);
  };
  auto create_d3d_device_wrapper = [&create_d3d_device]() {
    winrt::com_ptr<ID3D11Device> device;
    HRESULT hr = create_d3d_device(D3D_DRIVER_TYPE_HARDWARE, device);

    if (DXGI_ERROR_UNSUPPORTED == hr) {
      hr = create_d3d_device(D3D_DRIVER_TYPE_WARP, device);
    }

    winrt::check_hresult(hr);
    return device;
  };

  auto d3d_device = create_d3d_device_wrapper();
  auto dxgi_device = d3d_device.as<IDXGIDevice>();

  winrt::com_ptr<::IInspectable> d3d11_device;
  winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(
      dxgi_device.get(), d3d11_device.put()));
  return d3d11_device
      .as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}

auto wgc_session_impl::create_capture_item(HWND hwnd) {
  auto activation_factory = winrt::get_activation_factory<
      winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
  auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
  winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = {nullptr};
  interop_factory->CreateForWindow(
      hwnd,
      winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
      reinterpret_cast<void **>(winrt::put_abi(item)));
  return item;
}

auto wgc_session_impl::create_capture_item(HMONITOR hmonitor) {
  auto activation_factory = winrt::get_activation_factory<
      winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
  auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
  winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = {nullptr};
  interop_factory->CreateForMonitor(
      hmonitor,
      winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
      reinterpret_cast<void **>(winrt::put_abi(item)));
  return item;
}

HRESULT wgc_session_impl::create_mapped_texture(
    winrt::com_ptr<ID3D11Texture2D> src_texture, unsigned int width,
    unsigned int height) {

  D3D11_TEXTURE2D_DESC src_desc;
  src_texture->GetDesc(&src_desc);
  D3D11_TEXTURE2D_DESC map_desc;
  map_desc.Width = width == 0 ? src_desc.Width : width;
  map_desc.Height = height == 0 ? src_desc.Height : height;
  map_desc.MipLevels = src_desc.MipLevels;
  map_desc.ArraySize = src_desc.ArraySize;
  map_desc.Format = src_desc.Format;
  map_desc.SampleDesc = src_desc.SampleDesc;
  map_desc.Usage = D3D11_USAGE_STAGING;
  map_desc.BindFlags = 0;
  map_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  map_desc.MiscFlags = 0;

  auto d3dDevice = get_dxgi_interface<ID3D11Device>(d3d11_direct_device_);

  return d3dDevice->CreateTexture2D(&map_desc, nullptr,
                                    d3d11_texture_mapped_.put());
}

void wgc_session_impl::on_frame(
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const &sender,
    winrt::Windows::Foundation::IInspectable const &args) {
  auto is_new_size = false;

  {
    auto frame = sender.TryGetNextFrame();
    auto frame_size = frame.ContentSize();

    if (frame_size.Width != capture_frame_size_.Width ||
        frame_size.Height != capture_frame_size_.Height) {
      // The thing we have been capturing has changed size.
      // We need to resize our swap chain first, then blit the pixels.
      // After we do that, retire the frame and then recreate our frame pool.
      is_new_size = true;
      capture_frame_size_ = frame_size;
    }

    // copy to mapped texture
    {
      auto frame_captured =
          get_dxgi_interface<ID3D11Texture2D>(frame.Surface());

      if (!d3d11_texture_mapped_ || is_new_size)
        create_mapped_texture(frame_captured);

      d3d11_device_context_->CopyResource(d3d11_texture_mapped_.get(),
                                          frame_captured.get());

      D3D11_MAPPED_SUBRESOURCE map_result;
      d3d11_device_context_->Map(d3d11_texture_mapped_.get(), 0, D3D11_MAP_READ,
                                 D3D11_MAP_FLAG_DO_NOT_WAIT, &map_result);

      // copy data from mapInfo.pData
#if 1
      if (map_result.pData) {
        static unsigned char *buffer = nullptr;
        if (buffer && is_new_size)
          delete[] buffer;

        if (!buffer)
          buffer = new unsigned char[frame_size.Width * frame_size.Height * 4];

        int dstRowPitch = frame_size.Width * 4;
        for (int h = 0; h < frame_size.Height; h++) {
          memcpy_s(buffer + h * dstRowPitch, dstRowPitch,
                   (BYTE *)map_result.pData + h * map_result.RowPitch,
                   min(map_result.RowPitch, dstRowPitch));
        }

        BITMAPINFOHEADER bi;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = frame_size.Width;
        bi.biHeight = frame_size.Height * (-1);
        bi.biPlanes = 1;
        bi.biBitCount = 32; // should get from system color bits
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        BITMAPFILEHEADER bf;
        bf.bfType = 0x4d42;
        bf.bfReserved1 = 0;
        bf.bfReserved2 = 0;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bf.bfSize = bf.bfOffBits + frame_size.Width * frame_size.Height * 4;

        FILE *fp = nullptr;

        fopen_s(&fp, ".\\save.bmp", "wb+");

        fwrite(&bf, 1, sizeof(bf), fp);
        fwrite(&bi, 1, sizeof(bi), fp);
        fwrite(buffer, 1, frame_size.Width * frame_size.Height * 4, fp);

        fflush(fp);
        fclose(fp);
      }
#endif

      d3d11_device_context_->Unmap(d3d11_texture_mapped_.get(), 0);
    }
  }

  if (is_new_size) {
    capture_framepool_.Recreate(d3d11_direct_device_,
                                winrt::Windows::Graphics::DirectX::
                                    DirectXPixelFormat::B8G8R8A8UIntNormalized,
                                2, capture_frame_size_);
  }
}

int wgc_session_impl::initialize() {
  if (is_initialized_)
    return AM_ERROR::AE_NO;

  if (!(d3d11_direct_device_ = create_d3d11_device()))
    return AM_ERROR::AE_D3D_CREATE_DEVICE_FAILED;

  try {

    if (target_.is_window)
      capture_item_ = create_capture_item(target_.hwnd);
    else
      capture_item_ = create_capture_item(target_.hmonitor);

    // Set up
    auto d3d11_device = get_dxgi_interface<ID3D11Device>(d3d11_direct_device_);
    d3d11_device->GetImmediateContext(d3d11_device_context_.put());

    auto size = capture_item_.Size();
    capture_framepool_ =
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
            d3d11_direct_device_,
            winrt::Windows::Graphics::DirectX::DirectXPixelFormat::
                B8G8R8A8UIntNormalized,
            2, size);
    capture_session_ = capture_framepool_.CreateCaptureSession(capture_item_);
    capture_frame_size_ = size;
    capture_framepool_trigger_ = capture_framepool_.FrameArrived(
        winrt::auto_revoke, {this, &wgc_session_impl::on_frame});
  } catch (winrt::hresult_error) {
    return AM_ERROR::AE_WGC_CREATE_CAPTURER_FAILED;
  } catch (...) {
    return AM_ERROR::AE_WGC_CREATE_CAPTURER_FAILED;
  }

  return 0;
}

void wgc_session_impl::cleanup() {
  auto expected = false;
  if (cleaned_.compare_exchange_strong(expected, true)) {
    capture_framepool_trigger_.revoke();
    capture_framepool_.Close();
    capture_session_.Close();

    capture_framepool_ = nullptr;
    capture_session_ = nullptr;
    capture_item_ = nullptr;
  }
}

} // namespace am