// Copyright 2015 Google Inc. All Rights Reserved.
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

#ifndef COBALT_CSSOM_NEXT_SIBLING_COMBINATOR_H_
#define COBALT_CSSOM_NEXT_SIBLING_COMBINATOR_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "cobalt/cssom/combinator.h"

namespace cobalt {
namespace cssom {

class CombinatorVisitor;

// Next-sibling combinator describes that the elements represented by the two
// compound selectors share the same parent in the document tree and the element
// represented by the first compound selector immediately precedes the element
// represented by the second one.
//   https://www.w3.org/TR/selectors4/#adjacent-sibling-combinators
class NextSiblingCombinator : public Combinator {
 public:
  NextSiblingCombinator() {}
  ~NextSiblingCombinator() OVERRIDE {}

  // From Combinator.
  void Accept(CombinatorVisitor* visitor) OVERRIDE;
  CombinatorType GetCombinatorType() OVERRIDE { return kNextSiblingCombinator; }

 private:
  DISALLOW_COPY_AND_ASSIGN(NextSiblingCombinator);
};

}  // namespace cssom
}  // namespace cobalt

#endif  // COBALT_CSSOM_NEXT_SIBLING_COMBINATOR_H_
