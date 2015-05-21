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

#ifndef CSS_PARSER_TRANSITION_SHORTHAND_PROPERTY_PARSE_STRUCTURES_H_
#define CSS_PARSER_TRANSITION_SHORTHAND_PROPERTY_PARSE_STRUCTURES_H_

#include "base/optional.h"
#include "cobalt/cssom/const_string_list_value.h"
#include "cobalt/cssom/initial_style.h"
#include "cobalt/cssom/time_list_value.h"
#include "cobalt/cssom/timing_function_list_value.h"

namespace cobalt {
namespace css_parser {

// This file contains a collection of structures used as synthetic values for
// different parser reductions related to the 'transition' shorthand property.

// The SingleTransition structure is the type used for the result of the
// single_transition parser reduction.
struct SingleTransitionShorthand {
  void ReplaceNullWithInitialValues();

  base::optional<const char*> property;
  base::optional<base::TimeDelta> duration;
  scoped_refptr<cssom::TimingFunction> timing_function;
  base::optional<base::TimeDelta> delay;
};

// As we are parsing a transition, maintain builders for all of its components.
struct TransitionShorthandBuilder {
  TransitionShorthandBuilder()
      : property_list_builder(new cssom::ConstStringListValue::Builder()),
        duration_list_builder(new cssom::TimeListValue::Builder()),
        timing_function_list_builder(
            new cssom::TimingFunctionListValue::Builder()),
        delay_list_builder(new cssom::TimeListValue::Builder()) {}

  scoped_ptr<cssom::ConstStringListValue::Builder> property_list_builder;
  scoped_ptr<cssom::TimeListValue::Builder> duration_list_builder;
  scoped_ptr<cssom::TimingFunctionListValue::Builder>
      timing_function_list_builder;
  scoped_ptr<cssom::TimeListValue::Builder> delay_list_builder;
};

struct TransitionShorthand {
  scoped_refptr<cssom::PropertyValue> property_list;
  scoped_refptr<cssom::PropertyValue> duration_list;
  scoped_refptr<cssom::PropertyValue> timing_function_list;
  scoped_refptr<cssom::PropertyValue> delay_list;
};

}  // namespace css_parser
}  // namespace cobalt

#endif  // CSS_PARSER_TRANSITION_SHORTHAND_PROPERTY_PARSE_STRUCTURES_H_
