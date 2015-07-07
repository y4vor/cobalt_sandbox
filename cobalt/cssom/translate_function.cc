/*
 * Copyright 2014 Google Inc. All Rights Reserved.
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

#include "cobalt/cssom/translate_function.h"

#include "cobalt/cssom/transform_function_visitor.h"
#include "cobalt/base/polymorphic_downcast.h"

namespace cobalt {
namespace cssom {


TranslateFunction::OffsetType TranslateFunction::offset_type() const {
  if (offset_->GetTypeId() == base::GetTypeId<LengthValue>()) {
    return kLength;
  } else if (offset_->GetTypeId() == base::GetTypeId<PercentageValue>()) {
    return kPercentage;
  } else {
    NOTREACHED();
    return kLength;
  }
}

scoped_refptr<LengthValue> TranslateFunction::offset_as_length() const {
  DCHECK_EQ(kLength, offset_type());
  return scoped_refptr<LengthValue>(
      base::polymorphic_downcast<LengthValue*>(offset_.get()));
}
scoped_refptr<PercentageValue> TranslateFunction::offset_as_percentage() const {
  DCHECK_EQ(kPercentage, offset_type());
  return scoped_refptr<PercentageValue>(
      base::polymorphic_downcast<PercentageValue*>(offset_.get()));
}

void TranslateFunction::Accept(TransformFunctionVisitor* visitor) const {
  visitor->VisitTranslate(this);
}

}  // namespace cssom
}  // namespace cobalt
