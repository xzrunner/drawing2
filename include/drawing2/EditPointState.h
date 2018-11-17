#pragma once

#include "drawing2/ShapeCapture.h"

#include <ee0/EditOpState.h>
#include <ee0/typedef.h>

#include <functional>

namespace gs { class Shape; }

namespace dw2
{

class EditPointState : public ee0::EditOpState
{
public:
	EditPointState(const std::shared_ptr<pt0::Camera>& camera, const ee0::SubjectMgrPtr& sub_mgr,
		std::function<ShapeCapture::NodeRef()> get_selected);

	virtual bool OnMousePress(int x, int y) override;
	virtual bool OnMouseRelease(int x, int y) override;
	virtual bool OnMouseDrag(int x, int y) override;

	virtual bool Clear() override;

private:
	ee0::SubjectMgrPtr m_sub_mgr;

	std::function<ShapeCapture::NodeRef()> m_get_selected;

	sm::vec2 m_first_pos;

}; // EditPointState

}