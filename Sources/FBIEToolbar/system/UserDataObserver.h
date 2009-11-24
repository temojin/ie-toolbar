/**
* Facebook Internet Explorer Toolbar Software License 
* Copyright (c) 2009 Facebook, Inc. 
*
* Permission is hereby granted, free of charge, to any person or organization
* obtaining a copy of the software and accompanying documentation covered by
* this license (which, together with any graphical images included with such
* software, are collectively referred to below as the "Software") to (a) use,
* reproduce, display, distribute, execute, and transmit the Software, (b)
* prepare derivative works of the Software (excluding any graphical images
* included with the Software, which may not be modified or altered), and (c)
* permit third-parties to whom the Software is furnished to do so, all
* subject to the following:
*
* The copyright notices in the Software and this entire statement, including
* the above license grant, this restriction and the following disclaimer,
* must be included in all copies of the Software, in whole or in part, and
* all derivative works of the Software, unless such copies or derivative
* works are solely in the form of machine-executable object code generated by
* a source language processor.  
*
* Facebook, Inc. retains ownership of the Software and all associated
* intellectual property rights.  All rights not expressly granted in this
* license are reserved by Facebook, Inc.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/ 
#ifndef USERDATAOBSERVER_H
#define USERDATAOBSERVER_H

#include <queue>
#include <boost/noncopyable.hpp>

#include "ClientServiceConnection.h"
#include "ClientServiceObserver.h"

#include "../../common/DataChangeEvents.h"



namespace facebook{

const int kWaitTimeout = 500;

enum ClientServiceEvents{
  EVENT_NO_EVENT = 0,
  EVENT_IS_LOGGED_IN,
  EVENT_LOG_IN,
  EVENT_LOG_OUT,
  EVENT_CAN_CHANGE_STATUS,
  EVENT_GET_POKES_COUNT,
  EVENT_GET_MESSAGES_COUNT,
  EVENT_GET_REQUESTS_COUNT,
  EVENT_GET_EVENTS_COUNT,
  EVENT_GET_GROUPS_INVS_COUNT,
  EVENT_GET_FRIENDS,
  EVENT_GET_LOGGED_IN_USER,
  EVENT_SET_STATUS,
  EVENT_DATA_UPDATE,
  EVENT_LANGUAGE_UPDATE,
  EVENT_SET_SESSION
};

/**
 * class ClientServiceMessage
 *
 * Helper class that includes description of one request to the Client Service part
 */

class ClientServiceMessage{
public:
  ClientServiceMessage(void) {
    event_ = EVENT_NO_EVENT;
    callingThreadId_ = ::GetCurrentThreadId();
  }

  ClientServiceMessage(const ClientServiceEvents& newEvent) {
    event_ = newEvent;
    callingThreadId_ = ::GetCurrentThreadId();
  }

  ClientServiceMessage(const ClientServiceEvents& newEvent, const DWORD& callingThreadId) {
    event_ = newEvent;
    callingThreadId_ = callingThreadId;
  }

  ClientServiceMessage(const ClientServiceEvents& newEvent, const String& statusMessage) {
    event_ = newEvent;
    statusMessage_ = statusMessage;
    callingThreadId_ = ::GetCurrentThreadId();
  }

  ClientServiceMessage(const ClientServiceEvents& newEvent, const DataChangeEvents& dataEvent) {
    event_ = newEvent;
    dataEvent_ = dataEvent;
    callingThreadId_ = ::GetCurrentThreadId();
  }

  ClientServiceMessage(const ClientServiceEvents& newEvent, 
                      const DataChangeEvents& dataEvent, 
                      const DWORD& callingThreadId) {
    event_ = newEvent;
    dataEvent_ = dataEvent;
    callingThreadId_ = callingThreadId;
  }

  const ClientServiceEvents& getEvent() {
    return event_;
  }

  const String& getStatusMessage() {
    return statusMessage_;
  }

  const DWORD& getCallingThreadId() {
    return callingThreadId_;
  }

  const DataChangeEvents& getDataEvent() {
    return dataEvent_;
  }

private:
   ClientServiceEvents event_;
   DWORD callingThreadId_;
   String statusMessage_;
   DataChangeEvents dataEvent_;
};

/**
 * class UserDataObserver
 *
 * Wrapper for low level COM Client Service Connection. Includes all the functionality for 
 * asynchronious calls
 */

class UserDataObserver : 
  public ClientServiceObserver,
  private boost::noncopyable{
public:
  UserDataObserver(const DWORD&  threadId);
  virtual ~UserDataObserver(void);

  /**
   * returns userdataObserver for the 
   * current thread
   */
  static UserDataObserver& getInstance();
  /**
   * removes all the userdataObserver instances from the memory
   */
  static void clearInstances();
  /**
   * stops all the routines in the current thread userdataObserver
   */
  static void releaseInstance();

private:

  typedef std::map<DWORD, UserDataObserver*> ConnectionFasadeItemsMap;
  static ConnectionFasadeItemsMap connectionFasades_;

  /**
   * Create new async request to the client - service part
   *
   * @param requestData - type of the request
   */
  void postCustomRequest(const ClientServiceMessage& requestData);

  /**
   * Process all the posted requests
   *
   */
  void processCustomRequests();

  /**
   * Process posted request
   *
   * @param requestData - request description
   */
  void processCustomRequest(ClientServiceMessage requestData);

  /**
   * Async thread handling procedure
   *
   */
  static DWORD WINAPI threadHandlingProc(LPVOID lpParameter);

private:

  CComCriticalSection customRequestsCriticalSection;
  ClientServiceConnection serviceConnection;
  std::queue<ClientServiceMessage> requestsQueue_;
  DWORD  threadId_;
  DWORD  processingThreadId_;
  HANDLE thread_;   

public:

  // function wrappers
  bool isLoggedIn(bool needUpdate = true);

  void login(bool needUpdate = true);

  void logout(bool needUpdate = true);

  bool canChangeStatus(bool needUpdate = true);

  size_t getPokesCount(bool needUpdate = true);

  size_t getMessagesCount(bool needUpdate = true);

  size_t getRequestsCount(bool needUpdate = true);

  size_t getEventsCount(bool needUpdate = true);

  size_t getGroupsInvsCount(bool needUpdate = true);

  FriendsList getFriends(bool needUpdate = true);

  UserData getLoggedInUser(bool needUpdate = true);

  void setStatus(const String& statusMessage);

  void setSession(const String& session);

  void updateView(int changeId);

  void dataUpdated(unsigned long dataId);

private:

  // event handling
  bool handleIsLoggedIn();
  void handleLogin();
  void handleLogout();
  void handleCanChangeStatus();
  void handleGetPokesCount();
  void handleGetMessagesCount();
  void handleGetRequestsCount();
  void handleGetEventsCount();
  void handleGetGroupsInvsCount();
  void handleGetFriends();
  void handleGetLoggedInUser();
  void handleSetStatus(const String& statusMessage);
  void handleSetSession(const String& session);
  void handleLanguageUpdate();
  void handleDataUpdate(int dataId);
  void handleError(HRESULT result);

  void subscribeServiceObserver(ClientServiceObserver* observer);

  void unsubscribeServiceObserver(ClientServiceObserver* observer);

  /**
   * Set the cookies for current process
   *
   * @param cookies - cookies to set
   */
  void setCookies(String cookies);

private: 
  bool isInitialUpdate_;
  //sometimes user data is empty so try to get one more time,
  //but only once!
  int getUserRequestsCount_; 
  //cached data
  bool isLoggedIn_;
  bool canChangeStatus_;
  size_t pokesCount_;
  size_t messagesCount_;
  size_t requestsCount_;
  size_t eventsCount_;
  size_t groupsInvsCount_;
  FriendsList friendList_;
  UserData loggedInUser_;
};

} // !namespace facebook

#endif // USERDATAOBSERVER_H
