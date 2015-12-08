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

#include "cobalt/network/net_poster.h"

#include <algorithm>

#include "cobalt/network/network_module.h"
#include "net/base/net_errors.h"

namespace cobalt {
namespace network {

NetPoster::NetPoster(NetworkModule* network_module)
    : network_module_(network_module) {}

NetPoster::~NetPoster() { DCHECK(thread_checker_.CalledOnValidThread()); }

void NetPoster::Send(const GURL& url, const std::string& content_type,
                     const std::string& data) {
  DCHECK(thread_checker_.CalledOnValidThread());
  net::URLFetcher* url_fetcher =
      net::URLFetcher::Create(url, net::URLFetcher::POST, this);
  url_fetcher->SetRequestContext(network_module_->url_request_context_getter());

  if (data.size()) {
    url_fetcher->SetUploadData(content_type, data);
  }
  fetchers_.push_back(url_fetcher);
  url_fetcher->Start();
}

void NetPoster::OnURLFetchComplete(const net::URLFetcher* source) {
  DCHECK(thread_checker_.CalledOnValidThread());
  net::URLRequestStatus status = source->GetStatus();
  if (!status.is_success()) {
    DLOG(WARNING) << "NetPoster failed to POST to " << source->GetURL()
                  << " with error " << net::ErrorToString(status.error());
  }
  ScopedVector<net::URLFetcher>::iterator it =
      std::find(fetchers_.begin(), fetchers_.end(), source);
  if (it != fetchers_.end()) {
    fetchers_.erase(it);
  } else {
    NOTREACHED() << "Expected to find URLFetcher";
  }
}

}  // namespace network
}  // namespace cobalt
