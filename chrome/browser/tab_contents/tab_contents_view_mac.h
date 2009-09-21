// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TAB_CONTENTS_TAB_CONTENTS_VIEW_MAC_H_
#define CHROME_BROWSER_TAB_CONTENTS_TAB_CONTENTS_VIEW_MAC_H_

#import <Cocoa/Cocoa.h>

#include <string>

#include "base/gfx/size.h"
#include "base/scoped_ptr.h"
#include "base/scoped_nsobject.h"
#include "chrome/browser/cocoa/base_view.h"
#include "chrome/browser/tab_contents/tab_contents_view.h"
#include "chrome/common/notification_registrar.h"

class FilePath;
class FindBarMac;
@class FocusTracker;
@class SadTabView;
class TabContentsViewMac;
@class WebDragSource;
@class WebDropTarget;

@interface TabContentsViewCocoa : BaseView {
 @private
  TabContentsViewMac* tabContentsView_;  // WEAK; owns us
  scoped_nsobject<WebDragSource> dragSource_;
  scoped_nsobject<WebDropTarget> dropTarget_;
}

// Expose this, since sometimes one needs both the NSView and the TabContents.
- (TabContents*)tabContents;
@end

// Mac-specific implementation of the TabContentsView. It owns an NSView that
// contains all of the contents of the tab and associated child views.
class TabContentsViewMac : public TabContentsView,
                           public NotificationObserver {
 public:
  // The corresponding TabContents is passed in the constructor, and manages our
  // lifetime. This doesn't need to be the case, but is this way currently
  // because that's what was easiest when they were split.
  explicit TabContentsViewMac(TabContents* web_contents);

  // TabContentsView implementation --------------------------------------------

  virtual void CreateView();
  virtual RenderWidgetHostView* CreateViewForWidget(
      RenderWidgetHost* render_widget_host);
  virtual gfx::NativeView GetNativeView() const;
  virtual gfx::NativeView GetContentNativeView() const;
  virtual gfx::NativeWindow GetTopLevelNativeWindow() const;
  virtual void GetContainerBounds(gfx::Rect* out) const;
  virtual void OnContentsDestroy();
  virtual void RenderViewCreated(RenderViewHost* host);
  virtual void SetPageTitle(const std::wstring& title);
  virtual void OnTabCrashed();
  virtual void SizeContents(const gfx::Size& size);
  virtual void Focus();
  virtual void SetInitialFocus();
  virtual void StoreFocus();
  virtual void RestoreFocus();
  virtual RenderWidgetHostView* CreateNewWidgetInternal(int route_id,
                                                        bool activatable);
  virtual void ShowCreatedWidgetInternal(RenderWidgetHostView* widget_host_view,
                                         const gfx::Rect& initial_pos);

  // Backend implementation of RenderViewHostDelegate::View.
  virtual void ShowContextMenu(const ContextMenuParams& params);
  virtual void StartDragging(const WebDropData& drop_data,
                             WebKit::WebDragOperationsMask allowed_operations);
  virtual void UpdateDragCursor(WebKit::WebDragOperation operation);
  virtual void GotFocus();
  virtual void TakeFocus(bool reverse);
  virtual void HandleKeyboardEvent(const NativeWebKeyboardEvent& event);

  // NotificationObserver implementation ---------------------------------------

  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

 private:
  // The Cocoa NSView that lives in the view hierarchy.
  scoped_nsobject<TabContentsViewCocoa> cocoa_view_;

  // Keeps track of which NSView has focus so we can restore the focus when
  // focus returns.
  scoped_nsobject<FocusTracker> focus_tracker_;

  // Used to get notifications about renderers coming and going.
  NotificationRegistrar registrar_;

  // Used to render the sad tab. This will be non-NULL only when the sad tab is
  // visible.
  scoped_nsobject<SadTabView> sad_tab_;

  // The page content's intrinsic width.
  int preferred_width_;

  DISALLOW_COPY_AND_ASSIGN(TabContentsViewMac);
};

#endif  // CHROME_BROWSER_TAB_CONTENTS_TAB_CONTENTS_VIEW_MAC_H_
