#pragma once

#include "record_desktop.h"

#include <Windows.h>

namespace am {
class record_desktop_wgc : public record_desktop {
public:
  record_desktop_wgc();
  ~record_desktop_wgc();

  int init(const RECORD_DESKTOP_RECT &rect, const int fps) override;

  int start() override;
  int pause() override;
  int resume() override;
  int stop() override;

protected:
  void clean_up() override;

private:
  bool is_supported();
};
} // namespace am