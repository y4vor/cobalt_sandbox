/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/renderer/rasterizer/skia/cobalt_skia_type_conversions.h"

namespace cobalt {
namespace renderer {
namespace rasterizer {
namespace skia {

GrPixelConfig CobaltSurfaceFormatToGrSkia(
    cobalt::renderer::backend::SurfaceInfo::Format cobalt_format) {
  switch (cobalt_format) {
    case cobalt::renderer::backend::SurfaceInfo::kFormatARGB8: {
      return kBGRA_8888_GrPixelConfig;
    }
    case cobalt::renderer::backend::SurfaceInfo::kFormatBGRA8: {
      return kBGRA_8888_GrPixelConfig;
    }
    case cobalt::renderer::backend::SurfaceInfo::kFormatRGBA8: {
      return kRGBA_8888_GrPixelConfig;
    }
    case cobalt::renderer::backend::SurfaceInfo::kFormatA8: {
      return kAlpha_8_GrPixelConfig;
    }
    default: {
      DLOG(FATAL) << "Unexpected pixel pixel format.";
      return kUnknown_GrPixelConfig;
    }
  }
}

backend::SurfaceInfo::Format SkiaSurfaceFormatToCobalt(
    SkColorType skia_format) {
  switch (skia_format) {
    case kRGBA_8888_SkColorType:
      return backend::SurfaceInfo::kFormatRGBA8;
    case kBGRA_8888_SkColorType:
      return backend::SurfaceInfo::kFormatBGRA8;
    default:
      DLOG(FATAL) << "Unsupported Skia image format!";
      return backend::SurfaceInfo::kFormatRGBA8;
  }
}

SkColorType RenderTreeSurfaceFormatToSkia(
    render_tree::PixelFormat render_tree_format) {
  switch (render_tree_format) {
    case render_tree::kPixelFormatRGBA8:
      return kRGBA_8888_SkColorType;
    case render_tree::kPixelFormatY8:
      return kAlpha_8_SkColorType;
    case render_tree::kPixelFormatU8:
      return kAlpha_8_SkColorType;
    case render_tree::kPixelFormatV8:
      return kAlpha_8_SkColorType;
    default:
      DLOG(FATAL) << "Unknown render tree pixel format!";
      return kUnknown_SkColorType;
  }
}

SkAlphaType RenderTreeAlphaFormatToSkia(
    render_tree::AlphaFormat render_tree_format) {
  switch (render_tree_format) {
    case render_tree::kAlphaFormatPremultiplied:
      return kPremul_SkAlphaType;
    case render_tree::kAlphaFormatUnpremultiplied:
      return kUnpremul_SkAlphaType;
    default: DLOG(FATAL) << "Unknown render tree alpha format!";
  }
  return kUnpremul_SkAlphaType;
}

SkFontStyle CobaltFontStyleToSkFontStyle(render_tree::FontStyle style) {
  SkFontStyle::Slant slant = style.slant == render_tree::FontStyle::kItalicSlant
                                 ? SkFontStyle::kItalic_Slant
                                 : SkFontStyle::kUpright_Slant;

  return SkFontStyle(style.weight, SkFontStyle::kNormal_Width, slant);
}

SkRect CobaltRectFToSkiaRect(const math::RectF& rect) {
  return SkRect::MakeXYWH(rect.x(), rect.y(), rect.width(), rect.height());
}

}  // namespace skia
}  // namespace rasterizer
}  // namespace renderer
}  // namespace cobalt
