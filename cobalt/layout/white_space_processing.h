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
 * limitations under the License.`
 */

#ifndef LAYOUT_WHITE_SPACE_PROCESSING_H_
#define LAYOUT_WHITE_SPACE_PROCESSING_H_

#include <string>

namespace cobalt {
namespace layout {

// Performs white space collapsing and transformation that correspond to
// the phase I of the white space processing.
//   http://www.w3.org/TR/css3-text/#white-space-phase-1
void CollapseWhiteSpace(std::string* text);

// Trims the white space in a preparation for the phase II of the white space
// processing which happens during a layout.
//   http://www.w3.org/TR/css3-text/#white-space-phase-2
void TrimWhiteSpace(std::string* text, bool* has_leading_white_space,
                    bool* has_trailing_white_space);

}  // namespace layout
}  // namespace cobalt

#endif  // LAYOUT_WHITE_SPACE_PROCESSING_H_
