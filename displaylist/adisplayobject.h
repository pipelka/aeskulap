#ifndef AESKULAP_DISPLAYOBJECT_H
#define AESKULAP_DISPLAYOBJECT_H

#include "plane3d.h"

namespace Aeskulap {

class Line2D;
class Line3D;

class DisplayObject {
public:

	virtual bool screen_to_point(int x, int y, RPoint2D p) = 0;

	virtual bool point_to_screen(const Point2D& p, int& x, int& y) = 0;

	virtual bool point_to_screen(const RPoint2D& p, int& x, int& y) = 0;

	virtual void draw_point(int x, int y) = 0;

	virtual void draw_cross(int x, int y) = 0;

	virtual void draw_line(int x0, int y0, int x1, int y1) = 0;

	inline const RPlane3D& get_plane() {
		return m_projection_plane;
	}

protected:

	RPlane3D m_projection_plane;
};

} // namespace Aeskulap

#endif // AESKULAP_DISPLAYOBJECT_H
