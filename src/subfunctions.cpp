#include "subfunctions.h"
#include <exact_subtraction.hpp>
namespace ccd {

// cube
cube::cube(double eps)
{
    vr[0] = Vector3r(-eps, -eps, eps), vr[1] = Vector3r(eps, -eps, eps),
    vr[2] = Vector3r(eps, eps, eps), vr[3] = Vector3r(-eps, eps, eps),
    vr[4] = Vector3r(-eps, -eps, -eps), vr[5] = Vector3r(eps, -eps, -eps),
    vr[6] = Vector3r(eps, eps, -eps), vr[7] = Vector3r(-eps, eps, -eps);
    edgeid[0] = { { 0, 1 } };
    edgeid[1] = { { 1, 2 } };
    edgeid[2] = { { 2, 3 } };
    edgeid[3] = { { 3, 0 } };
    edgeid[4] = { { 4, 5 } };
    edgeid[5] = { { 5, 6 } };
    edgeid[6] = { { 6, 7 } };
    edgeid[7] = { { 7, 4 } };
    edgeid[8] = { { 0, 4 } };
    edgeid[9] = { { 1, 5 } };
    edgeid[10] = { { 2, 6 } };
    edgeid[11] = { { 3, 7 } };
    faceid[0] = { { 0, 1, 2, 3 } };
    faceid[1] = { { 4, 7, 6, 5 } };
    faceid[2] = { { 0, 4, 5, 1 } };
    faceid[3] = { { 1, 5, 6, 2 } };
    faceid[4] = { { 3, 2, 6, 7 } };
    faceid[5] = { { 0, 3, 7, 4 } }; // orientation out
    bmax = vr[2];
    bmin = vr[4];
    epsilon = eps;
}

// get aabb corners
void get__corners(const std::vector<Vector3d>& p, Vector3d& min, Vector3d& max)
{
    min = p[0];
    max = p[0];
    for (int i = 0; i < p.size(); i++) {
        if (min[0] > p[i][0])
            min[0] = p[i][0];
        if (min[1] > p[i][1])
            min[1] = p[i][1];
        if (min[2] > p[i][2])
            min[2] = p[i][2];

        if (max[0] < p[i][0])
            max[0] = p[i][0];
        if (max[1] < p[i][1])
            max[1] = p[i][1];
        if (max[2] < p[i][2])
            max[2] = p[i][2];
    }
}

std::array<Vector3d, 6> get_prism_vertices_double(
    const Vector3d& x0,
    const Vector3d& x1,
    const Vector3d& x2,
    const Vector3d& x3,
    const Vector3d& x0b,
    const Vector3d& x1b,
    const Vector3d& x2b,
    const Vector3d& x3b,
    double& k,
    bool& correct,
    double& maxerror)
{
    std::vector<std::pair<double, double>> sub;
    sub.reserve(18);
    std::pair<double, double> temp;
    correct = true;
    for (int i = 0; i < 3; i++) {
        temp.first = x0[i];
        temp.second = x1[i];
        sub.push_back(temp);
    }
    for (int i = 0; i < 3; i++) {
        temp.first = x0[i];
        temp.second = x3[i];
        sub.push_back(temp);
    }
    for (int i = 0; i < 3; i++) {
        temp.first = x0[i];
        temp.second = x2[i];
        sub.push_back(temp);
    }
    //
    for (int i = 0; i < 3; i++) {
        temp.first = x0b[i];
        temp.second = x1b[i];
        sub.push_back(temp);
    }
    for (int i = 0; i < 3; i++) {
        temp.first = x0b[i];
        temp.second = x3b[i];
        sub.push_back(temp);
    }
    for (int i = 0; i < 3; i++) {
        temp.first = x0b[i];
        temp.second = x2b[i];
        sub.push_back(temp);
    }
    std::vector<std::pair<double, double>> sub_record
        = sub; // record the origin data
    k = displaceSubtractions_double(sub);
    std::array<Vector3d, 6> result;
    int ct = 0;
    maxerror = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            result[i][j] = sub[ct].first - sub[ct].second;

            if (Rational(sub[ct].first) - Rational(sub[ct].second)
                != result[i][j]) {
                correct = false;
            }
            double mns = sub_record[ct].first - sub_record[ct].second;
            Rational m = Rational(result[i][j]) - Rational(mns);
            double md = m.to_double();
            if (fabs(md) > maxerror) {
                maxerror = fabs(md);
            }

            ct++;
        }
    }
    return result;
}

Vector3d get_prism_corner_double(
    const Vector3d& vertex_start,       // x0
    const Vector3d& face_vertex0_start, // x1
    const Vector3d& face_vertex1_start, // x2
    const Vector3d& face_vertex2_start, // x3
    const Vector3d& vertex_end,
    const Vector3d& face_vertex0_end,
    const Vector3d& face_vertex1_end,
    const Vector3d& face_vertex2_end,
    int i)
{
    Vector3d x0 = vertex_start, x1 = face_vertex0_start,
             x2 = face_vertex1_start, x3 = face_vertex2_start, x0b = vertex_end,
             x1b = face_vertex0_end, x2b = face_vertex1_end,
             x3b = face_vertex2_end;
    if (i == 0)
        return x0 - x1;
    if (i == 1)
        return x0 - x3;
    if (i == 2)
        return x0 - x2;
    if (i == 3)
        return x0b - x1b;
    if (i == 4)
        return x0b - x3b;
    if (i == 5)
        return x0b - x2b;

    else
        return Vector3d();
}
int seg_cut_plane(
    const Vector3r& seg0,
    const Vector3r& seg1,
    const Vector3r& t0,
    const Vector3r& t1,
    const Vector3r& t2)
{
    int o1, o2;
    o1 = orient3d(seg0, t0, t1, t2);
    o2 = orient3d(seg1, t0, t1, t2);
    if (o1 == 1 && o2 == 1)
        return NOT_INTERSECTED1;
    if (o1 == -1 && o2 == -1)
        return NOT_INTERSECTED2;
    if (o1 == 0 && o2 == 0)
        return COPLANAR;
    return INTERSECTED;
}

bool is_seg_intersect_cube(
    const double& eps, const Vector3r& e0, const Vector3r& e1)
{
    if (is_point_intersect_cube(eps, e0))
        return true;
    if (is_point_intersect_cube(eps, e1))
        return true;
    if (e0[0] == e1[0] && e0[1] == e1[1] && e0[2] == e1[2])
        return false; // degenerate case: the segment is degenerated as a point
    // if intersected, must be coplanar with the edge, or intersect edge or face
    if (is_seg_intersect_cube_2d(eps, e0, e1, 0)
        && is_seg_intersect_cube_2d(eps, e0, e1, 1)
        && is_seg_intersect_cube_2d(eps, e0, e1, 2))
        return true;
    return false;
}
bool is_seg_intersect_cube_2d(
    const double eps, const Vector3r& e0, const Vector3r& e1, int axis)
{
    Vector3r p0, p1, p2, p3, res;
    projected_cube_edges(eps, axis, p0, p1, p2, p3);
    const int i1 = (axis + 1) % 3;
    const int i2 = (axis + 2) % 3;
    if (e0[i1] <= eps && e0[i1] >= -eps && e0[i2] <= eps && e0[i2] >= -eps)
        return true;
    if (e1[i1] <= eps && e1[i1] >= -eps && e1[i2] <= eps && e1[i2] >= -eps)
        return true;
    if (segment_segment_inter_2(e0, e1, p0, p1, res, axis) >= 0)
        return true; // check if segments has intersection, or if cube points
                     // p0, p1 on e0-e1
    if (segment_segment_inter_2(e0, e1, p1, p2, res, axis) >= 0)
        return true;
    if (segment_segment_inter_2(e0, e1, p2, p3, res, axis) >= 0)
        return true;
    if (segment_segment_inter_2(e0, e1, p3, p0, res, axis) >= 0)
        return true;

    return false;
}
void projected_cube_edges(
    const double eps,
    const int axis,
    Vector3r& e0,
    Vector3r& e1,
    Vector3r& e2,
    Vector3r& e3)
{
    const int i1 = (axis + 1) % 3;
    const int i2 = (axis + 2) % 3;
    e0[axis] = 0;
    e1[axis] = 0;
    e2[axis] = 0;
    e3[axis] = 0;

    e0[i1] = -eps;
    e0[i2] = eps;

    e1[i1] = eps;
    e1[i2] = eps;

    e2[i1] = eps;
    e2[i2] = -eps;

    e3[i1] = -eps;
    e3[i2] = -eps;
}
bool is_point_intersect_cube(const double eps, const Vector3r& p)
{
    if (p[0] <= eps && p[0] >= -eps) {
        if (p[1] <= eps && p[1] >= -eps) {
            if (p[2] <= eps && p[2] >= -eps) {
                return true;
            }
        }
    }
    return false;
}

bool is_cube_edges_intersect_triangle(
    ccd::cube& cb, const Vector3r& t0, const Vector3r& t1, const Vector3r& t2)
{
    // the vertices of triangle are checked before going here, the edges are
    // also checked so, only need to check if cube edge has intersection with
    // the open triangle.
    Vector3r normal = cross(t0 - t1, t0 - t2);
    if (normal[0] == 0 && normal[1] == 0 && normal[2] == 0) {
        return false; // if triangle degenerated as a segment or point, then no
                      // intersection, because before here we already check that
    }
    int axis = 0; // maybe axis calculation can move to the function
                  // is_seg_intersect_triangle? but it is cheap, so whatever
    if (normal[0] * normal[0].get_sign() >= normal[1] * normal[1].get_sign())
        if (normal[0] * normal[0].get_sign()
            >= normal[2] * normal[2].get_sign())
            axis = 0;
    if (normal[1] * normal[1].get_sign() >= normal[0] * normal[0].get_sign())
        if (normal[1] * normal[1].get_sign()
            >= normal[2] * normal[2].get_sign())
            axis = 1;
    if (normal[2] * normal[2].get_sign() >= normal[1] * normal[1].get_sign())
        if (normal[2] * normal[2].get_sign()
            >= normal[0] * normal[0].get_sign())
            axis = 2;
    Vector3r s0, s1;
    for (int i = 0; i < 12; i++) {
        s0 = cb.vr[cb.edgeid[i][0]];
        s1 = cb.vr[cb.edgeid[i][1]];
        if (is_seg_intersect_triangle(s0, s1, t0, t1, t2, axis))
            return true;
    }
    return false;
}

//  triangle not degenerated
bool is_seg_intersect_triangle(
    const Vector3r& s0,
    const Vector3r& s1,
    const Vector3r& t0,
    const Vector3r& t1,
    const Vector3r& t2,
    const int& axis)
{
    int o1 = orient3d(s0, t0, t1, t2);
    bool degeneration = false;
    if (s0[0] == s1[0] && s0[1] == s1[1] && s0[2] == s1[2]) {
        degeneration = true;
        if (o1 != 0)
            return false; // degenerated as a point but not on the plane
    }
    int o2 = orient3d(s1, t0, t1, t2);
    if (o1 * o2 > 0)
        return false; // segment on the same side of the triangle

    if (o1 == 0 && o2 == 0) { // if coplanar

        if (is_coplanar_seg_intersect_triangle(s0, s1, t0, t1, t2, axis)) {
            return true;
        } else {
            return false;
        }
    }
    // not degenerated segment, not coplanar with triangle
    if (segment_triangle_inter(s0, s1, t0, t1, t2) == 1)
        return true;
    else {
        return false;
    }
}
bool is_coplanar_seg_intersect_triangle(
    const Vector3r& s0,
    const Vector3r& s1,
    const Vector3r& t0,
    const Vector3r& t1,
    const Vector3r& t2,
    const int axis)
{
    int o1, o2, o3;
    o1 = orient2d(s0, t0, t1, axis);
    o2 = orient2d(s0, t1, t2, axis);
    o3 = orient2d(s0, t2, t0, axis);
    if (o1 * o2 >= 0 && o1 * o3 >= 0 && o2 * o3 >= 0) {
        return true; // if same orientation, then point is inside
    }
    o1 = orient2d(s1, t0, t1, axis);
    o2 = orient2d(s1, t1, t2, axis);
    o3 = orient2d(s1, t2, t0, axis);
    if (o1 * o2 >= 0 && o1 * o3 >= 0 && o2 * o3 >= 0) {
        return true; // if same orientation, then point is inside
    }
    // since we already check triangle edge-box intersection, so no need to
    // check here
    return false;
}

prism::prism(
    const Vector3d& vs,
    const Vector3d& fs0,
    const Vector3d& fs1,
    const Vector3d& fs2,
    const Vector3d& ve,
    const Vector3d& fe0,
    const Vector3d& fe1,
    const Vector3d& fe2)
{
    for (int i = 0; i < 3; i++) {
        vsr[i] = vs[i];
        ver[i] = ve[i];
        fs0r[i] = fs0[i];
        fs1r[i] = fs1[i];
        fs2r[i] = fs2[i];
        fe0r[i] = fe0[i];
        fe1r[i] = fe1[i];
        fe2r[i] = fe2[i];
    }
    p_vertices[0] = get_prism_corner(0, 0, 0);
    p_vertices[1] = get_prism_corner(0, 1, 0);
    p_vertices[2] = get_prism_corner(1, 0, 0);
    p_vertices[3] = get_prism_corner(0, 0, 1);
    p_vertices[4] = get_prism_corner(0, 1, 1);
    p_vertices[5] = get_prism_corner(1, 0, 1); 
	// these are the 6 vertices of the prism,right hand law
    std::array<int, 2> eid;

    eid[0] = 0;
    eid[1] = 1;
    prism_edge_id[0] = eid;
    eid[0] = 1;
    eid[1] = 2;
    prism_edge_id[1] = eid;
    eid[0] = 2;
    eid[1] = 0;
    prism_edge_id[2] = eid;

    eid[0] = 3;
    eid[1] = 4;
    prism_edge_id[3] = eid;
    eid[0] = 4;
    eid[1] = 5;
    prism_edge_id[4] = eid;
    eid[0] = 5;
    eid[1] = 3;
    prism_edge_id[5] = eid;

    eid[0] = 0;
    eid[1] = 3;
    prism_edge_id[6] = eid;
    eid[0] = 1;
    eid[1] = 4;
    prism_edge_id[7] = eid;
    eid[0] = 2;
    eid[1] = 5;
    prism_edge_id[8] = eid;
}
Vector3r prism::get_prism_corner(int u, int v, int t)
{
    const Rational ur(u);
    const Rational vr(v);
    const Rational tr(t);
    return (1 - tr) * vsr + tr * ver
        - ((1 - tr) * fs0r + t * fe0r) * (1 - ur - vr)
        - ((1 - tr) * fs1r + t * fe1r) * ur - ((1 - tr) * fs2r + t * fe2r) * vr;
}

bilinear::bilinear(
    const Vector3r& v0,
    const Vector3r& v1,
    const Vector3r& v2,
    const Vector3r& v3)
{
    int ori = orient3d(v0, v1, v2, v3);
    if (ori == 0) {
        is_degenerated = true;
    } else {
        is_degenerated = false;
    }
    if (ori == 1) {
        facets.resize(4);
        facets[0] = { { 0, 1, 2 } }; // 0,1 are one pair
        facets[1] = { { 0, 2, 3 } };

        facets[2] = { { 3, 1, 0 } }; // 2,3 are one pair
        facets[3] = { { 3, 2, 1 } };
    }
    if (ori == -1) {
        facets.resize(4);
        facets[0] = { { 0, 2, 1 } }; // 0,1 are one pair
        facets[1] = { { 0, 3, 2 } };

        facets[2] = { { 3, 0, 1 } }; // 2,3 are one pair
        facets[3] = { { 3, 1, 2 } };
    }
}
// the facets of the tet are all oriented to outside. check if p is inside of
// OPEN tet
bool is_point_inside_tet(const bilinear& bl, const Vector3r& p)
{

    for (int i = 0; i < 4; i++) { // facets.size()==4
        if (orient3d(
                p, bl.v[bl.facets[i][0]], bl.v[bl.facets[i][1]],
                bl.v[bl.facets[i][2]])
            >= 0) {
            return false;
        }
    }
    return true; // all the orientations are -1, then point inside
}
// vin is true, this vertex has intersection with open tet
bool is_cube_intersect_tet_opposite_faces(
    const bilinear& bl, const cube& cube, std::array<bool, 8>& vin)
{
    for (int i = 0; i < 8; i++) {
        if (is_point_inside_tet(bl, cube.vr[i])) {
            vin[i] = true;
        } else {
            vin[i] = false;
        }
    }

    bool side1 = false;
    bool side2 = false;

    for (int i = 0; i < 12; i++) {
        if (vin[cube.edgeid[i][0]]
            && vin[cube.edgeid[i][1]]) { // two points of edge are all
                                         // inside of open tet
            continue;
        }
        for (int j = 0; j < 4; j++) {
            if (segment_triangle_inter(
                    cube.vr[cube.edgeid[i][0]], cube.vr[cube.edgeid[i][1]],
                    bl.v[bl.facets[j][0]], bl.v[bl.facets[j][1]],
                    bl.v[bl.facets[j][2]])) {
                if (j == 0 || j == 1)
                    side1 = true;
                if (j == 2 || j == 3)
                    side2 = true;
                if (side1 && side2)
                    return true;
            }
        }
    }
    return false;
}

} // namespace ccd