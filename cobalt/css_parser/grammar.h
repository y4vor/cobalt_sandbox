// Copyright 2014 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COBALT_CSS_PARSER_GRAMMAR_H_
#define COBALT_CSS_PARSER_GRAMMAR_H_

// A wrapper around grammar_generated.h, the latter should never be included
// directly.
//
// This file confines violations of a C++ style guide caused by Bison to a
// single place. Bison does not generate header guards nor it allows to specify
// a namespace for generated code. This causes three problems:
//
//   - files generated by Bison are not complete and cannot be included in
//     the same way as headers that comply with style guide;
//
//   - files generated by Bison should be included inside our namespace
//     to avoid a polution of a global namespace;
//
//   - as a corollary to the previous problem, header files that contain
//     namespaces cannot be included from grammar (.y) as this will cause double
//     nesting of included classes.

#include "cobalt/css_parser/animation_shorthand_property_parse_structures.h"
#include "cobalt/css_parser/background_shorthand_property_parse_structures.h"
#include "cobalt/css_parser/border_shorthand_property_parse_structures.h"
#include "cobalt/css_parser/font_shorthand_property_parse_structures.h"
#include "cobalt/css_parser/position_parse_structures.h"
#include "cobalt/css_parser/shadow_property_parse_structures.h"
#include "cobalt/css_parser/transition_shorthand_property_parse_structures.h"
#include "cobalt/css_parser/trivial_string_piece.h"
#include "cobalt/css_parser/trivial_type_pairs.h"
#include "cobalt/cssom/attribute_selector.h"
#include "cobalt/cssom/color_stop.h"
#include "cobalt/cssom/combinator.h"
#include "cobalt/cssom/complex_selector.h"
#include "cobalt/cssom/compound_selector.h"
#include "cobalt/cssom/css_declared_style_data.h"
#include "cobalt/cssom/css_font_face_rule.h"
#include "cobalt/cssom/css_keyframe_rule.h"
#include "cobalt/cssom/css_keyframes_rule.h"
#include "cobalt/cssom/css_media_rule.h"
#include "cobalt/cssom/css_style_declaration.h"
#include "cobalt/cssom/css_style_rule.h"
#include "cobalt/cssom/css_style_sheet.h"
#include "cobalt/cssom/filter_function.h"
#include "cobalt/cssom/filter_function_list_value.h"
#include "cobalt/cssom/integer_value.h"
#include "cobalt/cssom/keyword_value.h"
#include "cobalt/cssom/length_value.h"
#include "cobalt/cssom/linear_gradient_value.h"
#include "cobalt/cssom/local_src_value.h"
#include "cobalt/cssom/media_feature.h"
#include "cobalt/cssom/media_feature_keyword_value.h"
#include "cobalt/cssom/media_list.h"
#include "cobalt/cssom/media_query.h"
#include "cobalt/cssom/mtm_function.h"
#include "cobalt/cssom/number_value.h"
#include "cobalt/cssom/percentage_value.h"
#include "cobalt/cssom/property_key_list_value.h"
#include "cobalt/cssom/property_list_value.h"
#include "cobalt/cssom/radial_gradient_value.h"
#include "cobalt/cssom/ratio_value.h"
#include "cobalt/cssom/selector.h"
#include "cobalt/cssom/shadow_value.h"
#include "cobalt/cssom/simple_selector.h"
#include "cobalt/cssom/time_list_value.h"
#include "cobalt/cssom/timing_function_list_value.h"
#include "cobalt/cssom/transform_function.h"
#include "cobalt/cssom/transform_function_list_value.h"
#include "cobalt/cssom/url_src_value.h"
#include "third_party/glm/glm/gtc/type_ptr.hpp"
#include "third_party/glm/glm/mat4x4.hpp"

namespace cobalt {
namespace css_parser {

struct MarginOrPaddingShorthand;
class ParserImpl;
struct PropertyDeclaration;

// Override the default source location type from Bison with our own.
struct cobalt_yyltype {
  int first_line;
  int first_column;

  // A pointer to the start of the line the action or token was found on.
  const char* line_start;
};
#define YYLTYPE cobalt::css_parser::cobalt_yyltype
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 0

#if defined(OS_STARBOARD)
#include "starboard/memory.h"
#define YYFREE SbMemoryDeallocate
#define YYMALLOC SbMemoryAllocate
#endif

// A header generated by Bison must be included inside our namespace
// to avoid global namespace pollution.
#include "cobalt/css_parser/grammar_generated.h"

}  // namespace css_parser
}  // namespace cobalt

#endif  // COBALT_CSS_PARSER_GRAMMAR_H_
