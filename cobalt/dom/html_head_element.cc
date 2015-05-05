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

#include "cobalt/dom/html_head_element.h"

#include "cobalt/dom/document.h"

namespace cobalt {
namespace dom {

// static
const char* HTMLHeadElement::kTagName = "head";

HTMLHeadElement::HTMLHeadElement(HTMLElementFactory* html_element_factory,
                                 cssom::CSSParser* css_parser)
    : HTMLElement(html_element_factory, css_parser) {}

const std::string& HTMLHeadElement::tag_name() const {
  static const std::string kHeadTagString(kTagName);
  return kHeadTagString;
}

HTMLHeadElement::~HTMLHeadElement() {}

void HTMLHeadElement::AttachToDocument(Document* document) {
  HTMLElement::AttachToDocument(document);
  owner_document()->SetHeadInternal(this);
}

void HTMLHeadElement::DetachFromDocument() {
  owner_document()->SetHeadInternal(NULL);
  Node::DetachFromDocument();
}

}  // namespace dom
}  // namespace cobalt
