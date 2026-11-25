#include "ChafaRenderer.hpp"
#include <algorithm>
#include <chafa.h>
#include <cstdlib>
#include <cstring>

extern char **environ;

namespace {

void clearFramebuffer(std::vector<uint8_t> &framebuffer) {
  std::fill(framebuffer.begin(), framebuffer.end(), uint8_t{0});
}

void rasterizeFilledRect(std::vector<uint8_t> &framebuffer,
                         const gw::RendererRect &rect,
                         const gw::DrawCommand &cmd) {
  const int32_t x0 = std::max<int32_t>(0, static_cast<int32_t>(cmd.x));
  const int32_t y0 = std::max<int32_t>(0, static_cast<int32_t>(cmd.y));
  const int32_t x1 = std::min<int32_t>(static_cast<int32_t>(rect.width),
                                       static_cast<int32_t>(cmd.x + cmd.w));
  const int32_t y1 = std::min<int32_t>(static_cast<int32_t>(rect.height),
                                       static_cast<int32_t>(cmd.y + cmd.h));

  for (int32_t y = y0; y < y1; ++y) {
    for (int32_t x = x0; x < x1; ++x) {
      const size_t offset =
          (static_cast<size_t>(y) * rect.width + static_cast<size_t>(x)) * 4;
      framebuffer[offset + 0] = cmd.r;
      framebuffer[offset + 1] = cmd.g;
      framebuffer[offset + 2] = cmd.b;
      framebuffer[offset + 3] = cmd.a;
    }
  }
}

} // namespace

bool ChafaRenderer::init(gw::RendererRect rect) {
  resizeFramebuffer(rect);
  return true;
}

void ChafaRenderer::shutdown() { _framebuffer.clear(); }

void ChafaRenderer::setRect(gw::RendererRect rect) { resizeFramebuffer(rect); }

void ChafaRenderer::resizeFramebuffer(gw::RendererRect rect) {
  _rect = rect;
  _framebuffer.assign(static_cast<size_t>(rect.width) * rect.height * 4,
                      uint8_t{0});
}

void ChafaRenderer::beginFrame() { clearFramebuffer(_framebuffer); }

void ChafaRenderer::drawBatch(const gw::DrawCommand *commands, size_t count) {
  if (_rect.width == 0 || _rect.height == 0)
    return;
  for (size_t i = 0; i < count; ++i) {
    if (commands[i].kind == gw::DrawCommandKind::FilledRect) {
      rasterizeFilledRect(_framebuffer, _rect, commands[i]);
    }
  }
}

void ChafaRenderer::endFrame() {
  _frameText.clear();
  if (_rect.width == 0 || _rect.height == 0)
    return;

  ChafaTermInfo *termInfo =
      chafa_term_db_detect(chafa_term_db_get_default(), environ);
  const ChafaCanvasMode canvasMode =
      chafa_term_info_get_best_canvas_mode(termInfo);
  const bool inMultiplexer =
      std::getenv("TMUX") != nullptr || std::getenv("STY") != nullptr;
  const ChafaPixelMode pixelMode =
      inMultiplexer ? CHAFA_PIXEL_MODE_SYMBOLS
                    : chafa_term_info_get_best_pixel_mode(termInfo);

  gint destWidth = static_cast<gint>(_rect.width);
  gint destHeight = static_cast<gint>(_rect.height);
  chafa_calc_canvas_geometry(static_cast<gint>(_rect.width),
                             static_cast<gint>(_rect.height), &destWidth,
                             &destHeight, 0.5f, FALSE, FALSE);

  ChafaCanvasConfig *config = chafa_canvas_config_new();
  chafa_canvas_config_set_canvas_mode(config, canvasMode);
  chafa_canvas_config_set_pixel_mode(config, pixelMode);
  chafa_canvas_config_set_geometry(config, destWidth, destHeight);

  ChafaCanvas *canvas = chafa_canvas_new(config);
  chafa_canvas_draw_all_pixels(
      canvas, CHAFA_PIXEL_RGBA8_UNASSOCIATED, _framebuffer.data(),
      static_cast<gint>(_rect.width), static_cast<gint>(_rect.height),
      static_cast<gint>(_rect.width) * 4);

  GString *output = chafa_canvas_print(canvas, termInfo);
  _frameText.assign(output->str, output->len);
  g_string_free(output, TRUE);

  chafa_term_info_unref(termInfo);
  chafa_canvas_unref(canvas);
  chafa_canvas_config_unref(config);
}

const char *ChafaRenderer::frameText() const {
  return _frameText.empty() ? nullptr : _frameText.c_str();
}

std::vector<uint8_t> ChafaRenderer::serializeState() {
  std::vector<uint8_t> bytes(sizeof(gw::RendererRect));
  std::memcpy(bytes.data(), &_rect, sizeof(gw::RendererRect));
  return bytes;
}

bool ChafaRenderer::deserializeState(const uint8_t *data, size_t size) {
  if (size != sizeof(gw::RendererRect))
    return false;
  gw::RendererRect rect;
  std::memcpy(&rect, data, sizeof(gw::RendererRect));
  resizeFramebuffer(rect);
  return true;
}
