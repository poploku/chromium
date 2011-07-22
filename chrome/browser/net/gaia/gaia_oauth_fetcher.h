// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_GAIA_GAIA_OAUTH_FETCHER_H_
#define CHROME_BROWSER_NET_GAIA_GAIA_OAUTH_FETCHER_H_
#pragma once

#include <string>

#include "base/memory/scoped_ptr.h"
#include "chrome/browser/net/chrome_cookie_notification_details.h"
#include "chrome/browser/net/gaia/gaia_oauth_consumer.h"
#include "content/common/notification_observer.h"
#include "content/common/notification_registrar.h"
#include "content/common/url_fetcher.h"
#include "googleurl/src/gurl.h"

struct ChromeCookieDetails;

class Browser;
class Profile;

// Authenticate a user using Gaia's OAuth1 and OAuth2 support.
//
// Users of this class typically desire an OAuth2 Access token scoped for a
// specific service.  This will typically start with either an interactive
// login, using StartGetOAuthToken, or with a long-lived OAuth1 all-scope
// token obtained through a previous login or other means, using
// StartOAuthGetAccessToken.  In fact, one can start with any of these
// routines:
//   StartGetOAuthToken()
//   StartOAuthGetAccessToken()
//   StartOAuthWrapBridge()
//   StartUserInfo()
// with the expectation that each of these calls the next Start* routine in
// the sequence, except for StartUserInfo as it's the last one.
//
// This class can handle one request at a time, and all calls through an
// instance should be serialized.
class GaiaOAuthFetcher : public URLFetcher::Delegate,
                         public NotificationObserver {
 public:
  GaiaOAuthFetcher(GaiaOAuthConsumer* consumer,
                   net::URLRequestContextGetter* getter,
                   Profile* profile,
                   const std::string& service_scope);

  virtual ~GaiaOAuthFetcher();

  // Obtains an OAuth 1 request token
  //
  // Pops up a window aimed at the Gaia OAuth URL for GetOAuthToken and then
  // listens for COOKIE_CHANGED notifications
  virtual void StartGetOAuthToken();

  // Obtains an OAuth1 access token and secret
  //
  // oauth1_request_token is from GetOAuthToken's result.
  virtual void StartOAuthGetAccessToken(
      const std::string& oauth1_request_token);

  // Obtains an OAuth2 access token using Gaia's OAuth1-to-OAuth2 bridge.
  //
  // oauth1_access_token and oauth1_access_token_secret are from
  // OAuthGetAccessToken's result.
  //
  // wrap_token_duration is typically one hour,
  // which is also the max -- you can only decrease it.
  //
  // oauth2_scope should be specific to a service.  For example, Chromium Sync
  // uses https://www.googleapis.com/auth/chromesync as it's OAuth2 scope.
  virtual void StartOAuthWrapBridge(
      const std::string& oauth1_access_token,
      const std::string& oauth1_access_token_secret,
      const std::string& wrap_token_duration,
      const std::string& oauth2_scope);

  // Obtains user information related to an OAuth2 access token
  //
  // oauth2_access_token is from OAuthWrapBridge's result.
  virtual void StartUserInfo(const std::string& oauth2_access_token);

  // NotificationObserver implementation.
  virtual void Observe(int type,
                       const NotificationSource& source,
                       const NotificationDetails& details) OVERRIDE;

  // Called when a cookie, e. g. oauth_token, changes
  virtual void OnCookieChanged(Profile* profile,
                               ChromeCookieDetails* cookie_details);

  // Called when a cookie, e. g. oauth_token, changes
  virtual void OnBrowserClosing(Browser* profile,
                                bool detail);

  // Implementation of URLFetcher::Delegate
  virtual void OnURLFetchComplete(const URLFetcher* source,
                                  const GURL& url,
                                  const net::URLRequestStatus& status,
                                  int response_code,
                                  const net::ResponseCookies& cookies,
                                  const std::string& data) OVERRIDE;

  // StartGetOAuthToken (or other Start* routine) been called, but results
  // are not back yet.
  virtual bool HasPendingFetch();

  // Stop any URL fetches in progress.
  virtual void CancelRequest();

 private:
  // Process the results of a GetOAuthToken fetch.
  virtual void OnGetOAuthTokenFetched(const std::string& token);

  // Process the results of a OAuthGetAccessToken fetch.
  virtual void OnOAuthGetAccessTokenFetched(const std::string& data,
                                            const net::URLRequestStatus& status,
                                            int response_code);

  // Process the results of a OAuthWrapBridge fetch.
  virtual void OnOAuthWrapBridgeFetched(const std::string& data,
                                        const net::URLRequestStatus& status,
                                        int response_code);

  // Process the results of a userinfo fetch.
  virtual void OnUserInfoFetched(const std::string& data,
                                 const net::URLRequestStatus& status,
                                 int response_code);

  // Tokenize the results of a OAuthGetAccessToken fetch.
  static void ParseOAuthGetAccessTokenResponse(const std::string& data,
                                               std::string* token,
                                               std::string* secret);

  // Tokenize the results of a OAuthWrapBridge fetch.
  static void ParseOAuthWrapBridgeResponse(const std::string& data,
                                           std::string* token,
                                           std::string* expires_in);

  // Tokenize the results of a userinfo fetch.
  static void ParseUserInfoResponse(const std::string& data,
                                    std::string* email);

  // From a URLFetcher result, generate an appropriate error.
  static GoogleServiceAuthError GenerateAuthError(
      const std::string& data,
      const net::URLRequestStatus& status);

  // Given parameters, create a OAuthGetAccessToken request body.
  static std::string MakeOAuthGetAccessTokenBody(
      const std::string& oauth1_request_token);

  // Given parameters, create a OAuthWrapBridge request body.
  static std::string MakeOAuthWrapBridgeBody(
      const std::string& oauth1_access_token,
      const std::string& oauth1_access_token_secret,
      const std::string& wrap_token_duration,
      const std::string& oauth2_service_scope);

  // Given parameters, create a userinfo request body.
  static std::string MakeUserInfoBody(const std::string& oauth2_access_token);

  // Create a fetcher useable for making any Gaia OAuth request.
  static URLFetcher* CreateGaiaFetcher(net::URLRequestContextGetter* getter,
                                       const GURL& gaia_gurl_,
                                       const std::string& body,
                                       const std::string& headers,
                                       URLFetcher::Delegate* delegate);

  // These fields are common to GaiaOAuthFetcher, same every request
  GaiaOAuthConsumer* const consumer_;
  net::URLRequestContextGetter* const getter_;
  Profile* profile_;
  std::string service_scope_;
  Browser* popup_;
  NotificationRegistrar registrar_;

  // While a fetch is going on:
  scoped_ptr<URLFetcher> fetcher_;
  std::string request_body_;
  std::string request_headers_;
  bool fetch_pending_;

  DISALLOW_COPY_AND_ASSIGN(GaiaOAuthFetcher);
};

#endif  // CHROME_BROWSER_NET_GAIA_GAIA_OAUTH_FETCHER_H_
