#include "drawing2/ShapeCapture.h"

#include <ee0/SceneNodeContainer.h>

#include <node0/SceneNode.h>
#include <node2/CompShape.h>
#include <SM_Calc.h>

#include <geoshape/Point2D.h>
#include <geoshape/Rect.h>
#include <geoshape/Circle.h>
#include <geoshape/Polyline.h>
#include <geoshape/Polygon.h>

namespace dw2
{

ShapeCapture::NodeRef
ShapeCapture::Capture(const ee0::SceneNodeContainer& nodes, float threshold, const sm::vec2& pos)
{
	ShapeCapture::NodeRef ret;

	nodes.Traverse([&](const ee0::GameObj& obj)->bool
	{
		if (!obj->HasUniqueComp<n2::CompShape>()) {
			return true;
		}

		auto& cshape = obj->GetUniqueComp<n2::CompShape>();
		auto& shape = cshape.GetShape();
		auto type = shape->get_type();

		ShapeCapture::NodeRef cap;
		if (type == rttr::type::get<gs::Point2D>()) {
			cap = CapturePoint(shape, threshold, pos);
		} else if (type == rttr::type::get<gs::Rect>()) {
			cap = CaptureRect(shape, threshold, pos);
		} else if (type == rttr::type::get<gs::Circle>()) {
			cap = CaptureCircle(shape, threshold, pos);
		} else if (type == rttr::type::get<gs::Polyline>()) {
			cap = CapturePolyline(shape, threshold, pos);
		}

		if (!cap.shape)
		{
			auto shape2d = std::dynamic_pointer_cast<gs::Shape2D>(shape);
			if (shape2d->IsIntersect(sm::rect(pos, threshold, threshold))) {
				cap.type = NodeRef::Type::SHAPE;
				cap.obj = obj;
				cap.shape = shape;
				cap.pos.MakeInvalid();
			}
		}

		if (cap.shape) {
			ret = cap;
			ret.obj = obj;
			return false;
		} else {
			return true;
		}
	});

	return ret;
}

ShapeCapture::NodeRef
ShapeCapture::CapturePoint(const std::shared_ptr<gs::Shape>& shape, float threshold, const sm::vec2& pos)
{
	ShapeCapture::NodeRef ret;

	auto& point = std::static_pointer_cast<gs::Point2D>(shape)->GetPos();
	if (sm::dis_pos_to_pos(point, pos) < threshold)
	{
		ret.type = NodeRef::Type::SHAPE;
		ret.shape = shape;
		ret.pos = point;
	}

	return ret;
}

ShapeCapture::NodeRef
ShapeCapture::CaptureRect(const std::shared_ptr<gs::Shape>& shape, float threshold, const sm::vec2& pos)
{
	ShapeCapture::NodeRef ret;

	auto& rect = std::static_pointer_cast<gs::Rect>(shape)->GetRect();
	if (sm::dis_pos_to_pos(pos, rect.Center()) < threshold) {
		ret.type = NodeRef::Type::SHAPE;
		ret.shape = shape;
		ret.pos = rect.Center();
	} else if (sm::dis_pos_to_pos(pos, { rect.xmin, rect.ymin }) < threshold) {
		ret.type = NodeRef::Type::CTRL_NODE;
		ret.shape = shape;
		ret.pos = { rect.xmin, rect.ymin };
	} else if (sm::dis_pos_to_pos(pos, { rect.xmax, rect.ymin }) < threshold) {
		ret.type = NodeRef::Type::CTRL_NODE;
		ret.shape = shape;
		ret.pos = { rect.xmax, rect.ymin };
	} else if (sm::dis_pos_to_pos(pos, { rect.xmax, rect.ymax }) < threshold) {
		ret.type = NodeRef::Type::CTRL_NODE;
		ret.shape = shape;
		ret.pos = { rect.xmax, rect.ymax };
	} else if (sm::dis_pos_to_pos(pos, { rect.xmin, rect.ymax }) < threshold) {
		ret.type = NodeRef::Type::CTRL_NODE;
		ret.shape = shape;
		ret.pos = { rect.xmin, rect.ymax };
	}

	return ret;
}

ShapeCapture::NodeRef
ShapeCapture::CaptureCircle(const std::shared_ptr<gs::Shape>& shape, float threshold, const sm::vec2& pos)
{
	NodeRef ret;

	auto& circle = std::static_pointer_cast<gs::Circle>(shape);
	const float dis = sm::dis_pos_to_pos(circle->GetCenter(), pos);
	if (dis < threshold)
	{
		ret.type = NodeRef::Type::SHAPE;
		ret.shape = shape;
		ret.pos = circle->GetCenter();
	}
	else if (dis < circle->GetRadius() + threshold &&
		     dis > circle->GetRadius() - threshold)
	{
		ret.type = NodeRef::Type::CTRL_NODE;
		ret.shape = circle;
		ret.pos = (pos - circle->GetCenter()).Normalized() * circle->GetRadius();
	}

	return ret;
}

ShapeCapture::NodeRef
ShapeCapture::CapturePolyline(const std::shared_ptr<gs::Shape>& shape, float threshold, const sm::vec2& pos)
{
	ShapeCapture::NodeRef ret;

	auto& vs = std::static_pointer_cast<gs::Polyline>(shape)->GetVertices();
	if (vs.empty()) {
		return ret;
	}

	sm::vec2 center;
	for (auto& v : vs)
	{
		if (sm::dis_pos_to_pos(v, pos) < threshold)
		{
			ret.type = NodeRef::Type::CTRL_NODE;
			ret.shape = shape;
			ret.pos = v;
			return ret;
		}
		center += v;
	}

	center /= static_cast<float>(vs.size());
	if (sm::dis_pos_to_pos(center, pos) < threshold)
	{
		ret.type = NodeRef::Type::SHAPE;
		ret.shape = shape;
		ret.pos = center;
	}

	return ret;
}

}