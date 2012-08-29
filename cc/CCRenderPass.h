// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCRenderPass_h
#define CCRenderPass_h

#include "CCDrawQuad.h"
#include "CCOcclusionTracker.h"
#include "CCSharedQuadState.h"
#include "SkColor.h"
#include <public/WebFilterOperations.h>
#include <public/WebTransformationMatrix.h>
#include <wtf/HashMap.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

class CCLayerImpl;
class CCRenderSurface;

struct CCAppendQuadsData;

// A list of CCDrawQuad objects, sorted internally in front-to-back order.
class CCQuadList : public Vector<OwnPtr<CCDrawQuad> > {
public:
    typedef reverse_iterator backToFrontIterator;
    typedef const_reverse_iterator constBackToFrontIterator;

    inline backToFrontIterator backToFrontBegin() { return rbegin(); }
    inline backToFrontIterator backToFrontEnd() { return rend(); }
    inline constBackToFrontIterator backToFrontBegin() const { return rbegin(); }
    inline constBackToFrontIterator backToFrontEnd() const { return rend(); }
};

typedef Vector<OwnPtr<CCSharedQuadState> > CCSharedQuadStateList;

class CCRenderPass {
    WTF_MAKE_NONCOPYABLE(CCRenderPass);
public:
    static PassOwnPtr<CCRenderPass> create(int id, IntRect outputRect, const WebKit::WebTransformationMatrix& transformToRootTarget);

    void appendQuadsForLayer(CCLayerImpl*, CCOcclusionTrackerImpl*, CCAppendQuadsData&);
    void appendQuadsForRenderSurfaceLayer(CCLayerImpl*, const CCRenderPass* contributingRenderPass, CCOcclusionTrackerImpl*, CCAppendQuadsData&);
    void appendQuadsToFillScreen(CCLayerImpl* rootLayer, SkColor screenBackgroundColor, const CCOcclusionTrackerImpl&);

    const CCQuadList& quadList() const { return m_quadList; }

    int id() const { return m_id; }

    // FIXME: Modify this transform when merging the RenderPass into a parent compositor.
    // Transforms from quad's original content space to the root target's content space.
    const WebKit::WebTransformationMatrix& transformToRootTarget() const { return m_transformToRootTarget; }

    // This denotes the bounds in physical pixels of the output generated by this RenderPass.
    const IntRect& outputRect() const { return m_outputRect; }

    FloatRect damageRect() const { return m_damageRect; }
    void setDamageRect(FloatRect rect) { m_damageRect = rect; }

    const WebKit::WebFilterOperations& filters() const { return m_filters; }
    void setFilters(const WebKit::WebFilterOperations& filters) { m_filters = filters; }

    const WebKit::WebFilterOperations& backgroundFilters() const { return m_backgroundFilters; }
    void setBackgroundFilters(const WebKit::WebFilterOperations& filters) { m_backgroundFilters = filters; }

    bool hasTransparentBackground() const { return m_hasTransparentBackground; }
    void setHasTransparentBackground(bool transparent) { m_hasTransparentBackground = transparent; }

    bool hasOcclusionFromOutsideTargetSurface() const { return m_hasOcclusionFromOutsideTargetSurface; }
    void setHasOcclusionFromOutsideTargetSurface(bool hasOcclusionFromOutsideTargetSurface) { m_hasOcclusionFromOutsideTargetSurface = hasOcclusionFromOutsideTargetSurface; }
protected:
    CCRenderPass(int id, IntRect outputRect, const WebKit::WebTransformationMatrix& transformToRootTarget);

    int m_id;
    CCQuadList m_quadList;
    CCSharedQuadStateList m_sharedQuadStateList;
    WebKit::WebTransformationMatrix m_transformToRootTarget;
    IntRect m_outputRect;
    FloatRect m_damageRect;
    bool m_hasTransparentBackground;
    bool m_hasOcclusionFromOutsideTargetSurface;
    WebKit::WebFilterOperations m_filters;
    WebKit::WebFilterOperations m_backgroundFilters;
};

typedef Vector<CCRenderPass*> CCRenderPassList;
typedef HashMap<int, OwnPtr<CCRenderPass> > CCRenderPassIdHashMap;

}

#endif
