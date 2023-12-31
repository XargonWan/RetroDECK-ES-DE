//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  HttpReq.cpp
//
//  HTTP requests using libcurl.
//  Used by the scraper and application updater.
//

#include "HttpReq.h"

#include "Log.h"
#include "Settings.h"
#include "resources/ResourceManager.h"
#include "utils/FileSystemUtil.h"

#include <assert.h>

std::string HttpReq::urlEncode(const std::string& s)
{
    const std::string unreserved {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~"};

    std::string escaped {""};
    for (size_t i {0}; i < s.length(); ++i) {
        if (unreserved.find_first_of(s[i]) != std::string::npos) {
            escaped.push_back(s[i]);
        }
        else {
            escaped.append("%");
            char buf[3];
            snprintf(buf, 3, "%.2X", static_cast<unsigned char>(s[i]));
            escaped.append(buf);
        }
    }
    return escaped;
}

HttpReq::HttpReq(const std::string& url, bool scraperRequest)
    : mStatus(REQ_IN_PROGRESS)
    , mHandle(nullptr)
    , mTotalBytes {0}
    , mDownloadedBytes {0}
{
    // The multi-handle is cleaned up via a call from GuiScraperSearch after the scraping
    // has been completed for a game, meaning the handle is valid for all curl requests
    // performed for the current game.
    if (!sMultiHandle)
        sMultiHandle = curl_multi_init();

    mHandle = curl_easy_init();

#if defined(USE_BUNDLED_CERTIFICATES)
    // Use the bundled curl TLS/SSL certificates (which actually come from the Mozilla project).
    // This is enabled by default on Windows. Although there is a possibility to use the OS
    // provided Schannel certificates I haven't been able to get this to work, and it also seems
    // to be problematic on older Windows versions.
    // The bundled certificates are also required on Linux when building an AppImage package as
    // distributions such as Debian, Ubuntu, Linux Mint and Manjaro place the TLS certificates in
    // a different directory than for example Fedora and openSUSE. This makes curl unusable on
    // these latter operating systems unless the bundled file is used.
    curl_easy_setopt(mHandle, CURLOPT_CAINFO,
                     ResourceManager::getInstance()
                         .getResourcePath(":/certificates/curl-ca-bundle.crt")
                         .c_str());
#endif

    if (mHandle == nullptr) {
        mStatus = REQ_IO_ERROR;
        onError("curl_easy_init failed");
        return;
    }

    // Set the url.
    CURLcode err {curl_easy_setopt(mHandle, CURLOPT_URL, url.c_str())};
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    long connectionTimeout;

    if (scraperRequest) {
        connectionTimeout =
            static_cast<long>(Settings::getInstance()->getInt("ScraperConnectionTimeout"));

        if (connectionTimeout < 0 || connectionTimeout > 300)
            connectionTimeout = static_cast<long>(
                Settings::getInstance()->getDefaultInt("ScraperConnectionTimeout"));
    }
    else {
        connectionTimeout = 30;
    }

    // Set connection timeout (default is 30 seconds).
    err = curl_easy_setopt(mHandle, CURLOPT_CONNECTTIMEOUT, connectionTimeout);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    long transferTimeout;

    if (scraperRequest) {
        transferTimeout =
            static_cast<long>(Settings::getInstance()->getInt("ScraperTransferTimeout"));

        if (transferTimeout < 0 || transferTimeout > 300)
            transferTimeout =
                static_cast<long>(Settings::getInstance()->getDefaultInt("ScraperTransferTimeout"));
    }
    else {
        transferTimeout = 0;
    }

    // Set transfer timeout (default is 120 seconds for the scraper and infinite otherwise).
    err = curl_easy_setopt(mHandle, CURLOPT_TIMEOUT, transferTimeout);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Set curl to handle redirects.
    err = curl_easy_setopt(mHandle, CURLOPT_FOLLOWLOCATION, 1L);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Set curl max redirects.
    err = curl_easy_setopt(mHandle, CURLOPT_MAXREDIRS, 2L);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Set curl restrict redirect protocols.
#if defined(__APPLE__) || LIBCURL_VERSION_MAJOR < 7 ||                                             \
    (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR < 85)
    err = curl_easy_setopt(mHandle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
#else
    err = curl_easy_setopt(mHandle, CURLOPT_REDIR_PROTOCOLS_STR, "http,https");
#endif

    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Tell curl how to write the data.
    err = curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, &HttpReq::writeContent);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Pass curl a pointer to this HttpReq so we know where to write the data to in our
    // write function.
    err = curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, this);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Enable the curl progress meter.
    err = curl_easy_setopt(mHandle, CURLOPT_NOPROGRESS, 0);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Pass curl a pointer to HttpReq to provide access to the counter variables.
    err = curl_easy_setopt(mHandle, CURLOPT_XFERINFODATA, this);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Progress meter callback.
    err = curl_easy_setopt(mHandle, CURLOPT_XFERINFOFUNCTION, HttpReq::transferProgress);
    if (err != CURLE_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_easy_strerror(err));
        return;
    }

    // Add the handle to our multi.
    CURLMcode merr {curl_multi_add_handle(sMultiHandle, mHandle)};
    if (merr != CURLM_OK) {
        mStatus = REQ_IO_ERROR;
        onError(curl_multi_strerror(merr));
        return;
    }

    sRequests[mHandle] = this;
}

HttpReq::~HttpReq()
{
    if (mHandle) {
        sRequests.erase(mHandle);

        CURLMcode merr {curl_multi_remove_handle(sMultiHandle, mHandle)};

        if (merr != CURLM_OK) {
            LOG(LogError) << "Error removing curl_easy handle from curl_multi: "
                          << curl_multi_strerror(merr);
        }

        curl_easy_cleanup(mHandle);
    }
}

HttpReq::Status HttpReq::status()
{
    if (mStatus == REQ_IN_PROGRESS) {
        int handleCount {0};
        CURLMcode merr {curl_multi_perform(sMultiHandle, &handleCount)};
        if (merr != CURLM_OK && merr != CURLM_CALL_MULTI_PERFORM) {
            mStatus = REQ_IO_ERROR;
            onError(curl_multi_strerror(merr));
            return mStatus;
        }

        int msgsLeft;
        CURLMsg* msg;
        while ((msg = curl_multi_info_read(sMultiHandle, &msgsLeft)) != nullptr) {
            if (msg->msg == CURLMSG_DONE) {
                HttpReq* req {sRequests[msg->easy_handle]};

                if (req == nullptr) {
                    LOG(LogError) << "Cannot find easy handle!";
                    continue;
                }

                if (msg->data.result == CURLE_OK) {
                    req->mStatus = REQ_SUCCESS;
                }
                else if (msg->data.result == CURLE_PEER_FAILED_VERIFICATION) {
                    req->mStatus = REQ_FAILED_VERIFICATION;
                    req->onError(curl_easy_strerror(msg->data.result));
                }
                else {
                    req->mStatus = REQ_IO_ERROR;
                    req->onError(curl_easy_strerror(msg->data.result));
                }
            }
        }
    }

    return mStatus;
}

std::string HttpReq::getContent() const
{
    assert(mStatus == REQ_SUCCESS);
    return mContent.str();
}

int HttpReq::transferProgress(
    void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    // Note that it's not guaranteed that the server will actually provide the total size.
    if (dltotal > 0)
        static_cast<HttpReq*>(clientp)->mTotalBytes = static_cast<long>(dltotal);
    if (dlnow > 0)
        static_cast<HttpReq*>(clientp)->mDownloadedBytes = static_cast<long>(dlnow);

    return CURLE_OK;
}

size_t HttpReq::writeContent(void* buff, size_t size, size_t nmemb, void* req_ptr)
{
    // size = size of an element, nmemb = number of elements.
    std::stringstream& ss {static_cast<HttpReq*>(req_ptr)->mContent};
    ss.write(static_cast<char*>(buff), size * nmemb);

    // Return value is number of elements successfully read.
    return nmemb;
}
