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

#include "cobalt/cssom/radial_gradient_value.h"

#include "base/stringprintf.h"
#include "cobalt/cssom/keyword_names.h"
#include "cobalt/cssom/property_value_visitor.h"

namespace cobalt {
namespace cssom {

void RadialGradientValue::Accept(PropertyValueVisitor* visitor) {
  visitor->VisitRadialGradient(this);
}

std::string RadialGradientValue::ToString() const {
  std::string result;
  switch (shape_) {
    case kCircle:
      result.append(kCircleKeywordName);
      break;
    case kEllipse:
      result.append(kEllipseKeywordName);
      break;
    default:
      NOTREACHED();
  }

  if (size_keyword_) {
    result.push_back(' ');
    switch (size_keyword_.value()) {
      case kClosestSide:
        result.append(kClosestSideKeywordName);
        break;
      case kFarthestSide:
        result.append(kFarthestSideKeywordName);
        break;
      case kClosestCorner:
        result.append(kClosestCornerKeywordName);
        break;
      case kFarthestCorner:
        result.append(kFarthestCornerKeywordName);
        break;
      default:
        NOTREACHED();
    }
  } else if (size_value_) {
    result.push_back(' ');
    result.append(size_value_->ToString());
  }

  if (position_) {
    result.push_back(' ');
    result.append(kAtKeywordName);
    for (size_t i = 0; i < position_->value().size(); ++i) {
      result.push_back(' ');
      result.append(position_->value()[i]->ToString());
    }
  }

  for (size_t i = 0; i < color_stop_list_.size(); ++i) {
    if (!result.empty()) result.append(", ");
    result.append(color_stop_list_[i]->ToString());
  }
  return result;
}

bool RadialGradientValue::operator==(const RadialGradientValue& other) const {
  // shape must the same.
  if (shape_ != other.shape_) {
    return false;
  }

  // The optionals must match.
  if (!(size_keyword_ == other.size_keyword_) ||
      ((!size_value_ && other.size_value_) ||
       (size_value_ && !other.size_value_) ||
       (size_value_ && !(size_value_->Equals(*other.size_value_))))) {
    return false;
  }
  // Exactly one of the optionals is engaged.
  if ((size_keyword_ && other.size_value_) ||
      (size_value_ && other.size_keyword_)) {
    return false;
  }

  if ((!position_ && other.position_) || (position_ && !other.position_) ||
      (position_ && !(position_->Equals(*other.position_)))) {
    return false;
  }

  // The stop lists have to be empty or have the same size.
  size_t stop_list_size = color_stop_list_.size();
  size_t other_stop_list_size = other.color_stop_list_.size();
  if (stop_list_size != other_stop_list_size) {
    return false;
  }
  // Each color stop has to be the same.
  for (size_t i = 0; i < stop_list_size; ++i) {
    if (!(*color_stop_list_[i] == *other.color_stop_list_[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace cssom
}  // namespace cobalt
