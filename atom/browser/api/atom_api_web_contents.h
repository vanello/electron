// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_API_ATOM_API_WEB_CONTENTS_H_
#define ATOM_BROWSER_API_ATOM_API_WEB_CONTENTS_H_

#include <string>
#include <vector>

#include "atom/browser/api/frame_subscriber.h"
#include "atom/browser/api/save_page_handler.h"
#include "atom/browser/api/trackable_object.h"
#include "atom/browser/common_web_contents_delegate.h"
#include "atom/common/options_switches.h"
#include "content/common/view_messages.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/favicon_url.h"
#include "native_mate/handle.h"
#include "ui/gfx/image/image.h"

namespace blink {
struct WebDeviceEmulationParams;
}

namespace brightray {
class InspectableWebContents;
}

namespace mate {

template<>
struct Converter<WindowOpenDisposition> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   WindowOpenDisposition val) {
    std::string disposition = "other";
    switch (val) {
      case CURRENT_TAB: disposition = "default"; break;
      case NEW_FOREGROUND_TAB: disposition = "foreground-tab"; break;
      case NEW_BACKGROUND_TAB: disposition = "background-tab"; break;
      case NEW_POPUP: disposition = "new-popup"; break;
      case NEW_WINDOW: disposition = "new-window"; break;
      default: disposition = "other"; break;
    }
    return mate::ConvertToV8(isolate, disposition);
  }
};

class Arguments;
class Dictionary;
}  // namespace mate

namespace atom {

struct SetSizeParams;
class AtomBrowserContext;
class WebViewGuestDelegate;

namespace api {

class WebContents : public mate::TrackableObject<WebContents>,
                    public CommonWebContentsDelegate,
                    public content::WebContentsObserver {
 public:
  // For node.js callback function type: function(error, buffer)
  using PrintToPDFCallback =
      base::Callback<void(v8::Local<v8::Value>, v8::Local<v8::Value>)>;

  // Create from an existing WebContents.
  static mate::Handle<WebContents> CreateFrom(
      v8::Isolate* isolate, content::WebContents* web_contents);

  // Create a new WebContents.
  static mate::Handle<WebContents> Create(
      v8::Isolate* isolate,
      const mate::Dictionary& options);

  // Create a new WebContents with CreateParams
  static mate::Handle<WebContents> CreateWithParams(
      v8::Isolate* isolate,
      const mate::Dictionary& options,
      const content::WebContents::CreateParams& params);

  int GetID() const;
  bool Equal(const WebContents* web_contents) const;
  void LoadURL(const GURL& url, const mate::Dictionary& options);
  void DownloadURL(const GURL& url);
  GURL GetURL() const;
  base::string16 GetTitle() const;
  bool IsLoading() const;
  bool IsWaitingForResponse() const;
  void Stop();
  void ReloadIgnoringCache();
  void GoBack();
  void GoForward();
  void GoToOffset(int offset);
  bool IsCrashed() const;
  void SetUserAgent(const std::string& user_agent);
  std::string GetUserAgent();
  void InsertCSS(const std::string& css);
  bool SavePage(const base::FilePath& full_file_path,
                const content::SavePageType& save_type,
                const SavePageHandler::SavePageCallback& callback);
  void ExecuteJavaScript(const base::string16& code,
                         bool has_user_gesture);
  void OpenDevTools(mate::Arguments* args);
  void CloseDevTools();
  bool IsDevToolsOpened();
  bool IsDevToolsFocused();
  void ToggleDevTools();
  void EnableDeviceEmulation(const blink::WebDeviceEmulationParams& params);
  void DisableDeviceEmulation();
  void InspectElement(int x, int y);
  void InspectServiceWorker();
  void HasServiceWorker(const base::Callback<void(bool)>&);
  void UnregisterServiceWorker(const base::Callback<void(bool)>&);
  void SetAudioMuted(bool muted);
  bool IsAudioMuted();
  void Print(mate::Arguments* args);

  // Print current page as PDF.
  void PrintToPDF(const base::DictionaryValue& setting,
                  const PrintToPDFCallback& callback);

  // DevTools workspace api.
  void AddWorkSpace(mate::Arguments* args, const base::FilePath& path);
  void RemoveWorkSpace(mate::Arguments* args, const base::FilePath& path);

  // Editing commands.
  void Undo();
  void Redo();
  void Cut();
  void Copy();
  void Paste();
  void PasteAndMatchStyle();
  void Delete();
  void SelectAll();
  void Unselect();
  void Replace(const base::string16& word);
  void ReplaceMisspelling(const base::string16& word);
  uint32 FindInPage(mate::Arguments* args);
  void StopFindInPage(content::StopFindAction action);

  // Focus.
  void Focus();
  void TabTraverse(bool reverse);

  // Send messages to browser.
  bool SendIPCMessage(const base::string16& channel,
                      const base::ListValue& args);

  // Send WebInputEvent to the page.
  void SendInputEvent(v8::Isolate* isolate, v8::Local<v8::Value> input_event);

  // Subscribe to the frame updates.
  void BeginFrameSubscription(
      const FrameSubscriber::FrameCaptureCallback& callback);
  void EndFrameSubscription();

  // Methods for creating <webview>.
  void SetSize(const SetSizeParams& params);
  void SetAllowTransparency(bool allow);
  bool IsGuest() const;

  // Returns the web preferences of current WebContents.
  v8::Local<v8::Value> GetWebPreferences(v8::Isolate* isolate);

  // Returns the owner window.
  v8::Local<v8::Value> GetOwnerBrowserWindow();

  // Properties.
  v8::Local<v8::Value> Session(v8::Isolate* isolate);
  v8::Local<v8::Value> DevToolsWebContents(v8::Isolate* isolate);

  // mate::TrackableObject:
  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::ObjectTemplate> prototype);

 protected:
  explicit WebContents(content::WebContents* web_contents);
  WebContents(v8::Isolate* isolate,
              const mate::Dictionary& options,
              const content::WebContents::CreateParams* create_params = NULL);
  ~WebContents();

  // content::WebContentsDelegate:
  bool AddMessageToConsole(content::WebContents* source,
                           int32 level,
                           const base::string16& message,
                           int32 line_no,
                           const base::string16& source_id) override;
  bool ShouldCreateWebContents(
      content::WebContents* web_contents,
      int route_id,
      int main_frame_route_id,
      WindowContainerType window_container_type,
      const std::string& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace) override;
  void WebContentsCreated(content::WebContents* source_contents,
                          int opener_render_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) override;
  void AddNewContents(content::WebContents* source,
                      content::WebContents* new_contents,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_rect,
                      bool user_gesture,
                      bool* was_blocked) override;
  bool ShouldResumeRequestsForCreatedWindow() override;
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  void BeforeUnloadFired(content::WebContents* tab,
                         bool proceed,
                         bool* proceed_to_fire_unload) override;
  void MoveContents(content::WebContents* source,
                    const gfx::Rect& pos) override;
  void CloseContents(content::WebContents* source) override;
  void ActivateContents(content::WebContents* contents) override;
  bool IsPopupOrPanel(const content::WebContents* source) const override;
  void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) override;
  void EnterFullscreenModeForTab(content::WebContents* source,
                                 const GURL& origin) override;
  void ExitFullscreenModeForTab(content::WebContents* source) override;
  void RendererUnresponsive(content::WebContents* source) override;
  void RendererResponsive(content::WebContents* source) override;
  bool HandleContextMenu(const content::ContextMenuParams& params) override;
  bool OnGoToEntryOffset(int offset) override;
  void FindReply(content::WebContents* web_contents,
                 int request_id,
                 int number_of_matches,
                 const gfx::Rect& selection_rect,
                 int active_match_ordinal,
                 bool final_update) override;

  // content::WebContentsObserver:
  void BeforeUnloadFired(const base::TimeTicks& proceed_time) override;
  void RenderViewDeleted(content::RenderViewHost*) override;
  void RenderProcessGone(base::TerminationStatus status) override;
  void DocumentLoadedInFrame(
      content::RenderFrameHost* render_frame_host) override;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  void DidFailLoad(content::RenderFrameHost* render_frame_host,
                   const GURL& validated_url,
                   int error_code,
                   const base::string16& error_description,
                   bool was_ignored_by_handler) override;
  void DidFailProvisionalLoad(content::RenderFrameHost* render_frame_host,
                              const GURL& validated_url,
                              int error_code,
                              const base::string16& error_description,
                              bool was_ignored_by_handler) override;
  void DidStartLoading() override;
  void DidStopLoading() override;
  void DidGetResourceResponseStart(
      const content::ResourceRequestDetails& details) override;
  void DidGetRedirectForResourceRequest(
      content::RenderFrameHost* render_frame_host,
      const content::ResourceRedirectDetails& details) override;
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;
  bool OnMessageReceived(const IPC::Message& message) override;
  void WebContentsDestroyed() override;
  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override;
  void TitleWasSet(content::NavigationEntry* entry, bool explicit_set) override;
  void DidUpdateFaviconURL(
      const std::vector<content::FaviconURL>& urls) override;
  void PluginCrashed(const base::FilePath& plugin_path,
                     base::ProcessId plugin_pid) override;
  void MediaStartedPlaying() override;
  void MediaPaused() override;
  void DidChangeThemeColor(SkColor theme_color) override;

  // brightray::InspectableWebContentsViewDelegate:
  void DevToolsFocused() override;
  void DevToolsOpened() override;
  void DevToolsClosed() override;

 private:
  enum Type {
    BROWSER_WINDOW,  // Used by BrowserWindow.
    WEB_VIEW,  // Used by <webview>.
    REMOTE,  // Thin wrap around an existing WebContents.
  };

  AtomBrowserContext* GetBrowserContext() const;

  uint32 GetNextRequestId() {
    return ++request_id_;
  }

  // Called when received a message from renderer.
  void OnRendererMessage(const base::string16& channel,
                         const base::ListValue& args);

  // Called when received a synchronous message from renderer.
  void OnRendererMessageSync(const base::string16& channel,
                             const base::ListValue& args,
                             IPC::Message* message);

  // Called when guests need to be notified of
  // embedders' zoom level change.
  void OnZoomLevelChanged(double level);

  v8::Global<v8::Value> session_;
  v8::Global<v8::Value> devtools_web_contents_;

  // When a new tab is created asynchronously, stores the LoadURLParams
  // needed to continue loading the page once the tab is ready.
  scoped_ptr<content::NavigationController::LoadURLParams>
    delayed_load_url_params_;
  bool delayed_load_url_;

  scoped_ptr<WebViewGuestDelegate> guest_delegate_;

  // The type of current WebContents.
  Type type_;

  // Request id used for findInPage request.
  uint32 request_id_;

  DISALLOW_COPY_AND_ASSIGN(WebContents);
};

}  // namespace api

}  // namespace atom

#endif  // ATOM_BROWSER_API_ATOM_API_WEB_CONTENTS_H_
