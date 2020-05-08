/// Our exact CCD method
#include <CCD/ccd.hpp>
namespace ccd {

bool vertexFaceCCD(
    const Vector3d& vertex_start,
    const Vector3d& face_vertex0_start,
    const Vector3d& face_vertex1_start,
    const Vector3d& face_vertex2_start,
    const Vector3d& vertex_end,
    const Vector3d& face_vertex0_end,
    const Vector3d& face_vertex1_end,
    const Vector3d& face_vertex2_end,
    const double minimum_distance)
{

    prism vfprism(
        vertex_start, face_vertex0_start, face_vertex1_start,
        face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
        face_vertex2_end);

    // step 1. bounding box checking
    Vector3r bmin(-minimum_distance, -minimum_distance, -minimum_distance),
        bmax(minimum_distance, minimum_distance, minimum_distance);
    bool intersection = vfprism.is_prism_bbox_cut_bbox(bmin, bmax);
    if (!intersection)
        return false; // if bounding box not intersected, then not intersected

    // step 2. prism edges & prism bottom triangles check
    // prism edges test, segment degenerate cases already handled
    for (int i = 0; i < 9; i++) {
        /* if (is_seg_intersect_cube(
                 minimum_distance,
           vfprism.p_vertices[vfprism.prism_edge_id[i][0]],
                 vfprism.p_vertices[vfprism.prism_edge_id[i][1]]))
             return true;*/
        if (seg_intersect_cube(
                minimum_distance,
                vfprism.p_vertices[vfprism.prism_edge_id[i][0]],
                vfprism.p_vertices[vfprism.prism_edge_id[i][1]])) {
            /*std::cout << "here wrong in seg_cube intersection rational" <<
             * std::endl;*/
            return true;
        }
    }

    // prism top/bottom triangle test
    cube cb(minimum_distance);
    if (is_cube_edges_intersect_triangle(
            cb, vfprism.p_vertices[0], vfprism.p_vertices[1],
            vfprism.p_vertices[2]))
        return true;
    if (is_cube_edges_intersect_triangle(
            cb, vfprism.p_vertices[3], vfprism.p_vertices[4],
            vfprism.p_vertices[5]))
        return true;

    // step 3 tet facets- cube edges
    std::array<std::array<bool, 8>, 3>
        v_tet; // cube vertices - tets positions
               // TODO one solution for v_tet is to  make a boolean which can
               // show if pt is on the border
    bilinear bl0(
        vfprism.p_vertices[0], vfprism.p_vertices[1], vfprism.p_vertices[4],
        vfprism.p_vertices[3]);
    bilinear bl1(
        vfprism.p_vertices[1], vfprism.p_vertices[2], vfprism.p_vertices[5],
        vfprism.p_vertices[4]);
    bilinear bl2(
        vfprism.p_vertices[0], vfprism.p_vertices[2], vfprism.p_vertices[5],
        vfprism.p_vertices[3]);
    std::array<bilinear, 3> bls = { { bl0, bl1, bl2 } };
    bool cube_inter_tet[3];
    if (is_cube_intersect_tet_opposite_faces(
            bl0, cb, v_tet[0], cube_inter_tet[0]))
        return true;
    if (is_cube_intersect_tet_opposite_faces(
            bl1, cb, v_tet[1], cube_inter_tet[1]))
        return true;
    if (is_cube_intersect_tet_opposite_faces(
            bl2, cb, v_tet[2], cube_inter_tet[2]))
        return true;

    // if cube intersect any tet, need check if intersect bilinear;
    // if cube not intersect any tet, shoot a ray
    // TODO we can also have some information about the edge-face intersection
    // above
    if (cube_inter_tet[0]) {
        if (is_cube_edge_intersect_bilinear(bl0, cb, v_tet[0]))
            return true;
    }
    if (cube_inter_tet[1]) {
        if (is_cube_edge_intersect_bilinear(bl1, cb, v_tet[1]))
            return true;
    }
    if (cube_inter_tet[2]) {
        if (is_cube_edge_intersect_bilinear(bl2, cb, v_tet[2]))
            return true;
    }
    // down here is the last part of the algorithm

    int min_v = 3;
    int curr_v;
    int target = 0;
    for (int i = 0; i < 8; i++) {
        curr_v = v_tet[0][i] + v_tet[1][i] + v_tet[2][i];
        if (curr_v < min_v) {
            min_v = curr_v;
            target = i;
        }
    }
    // std::cout << "shoot a ray in rational" << std::endl;
    std::vector<bool> p_tet;
    p_tet.resize(3);
    p_tet[0] = v_tet[0][target];
    p_tet[1] = v_tet[1][target];
    p_tet[2] = v_tet[2][target];

    return retrial_ccd(vfprism, bls, cb.vr[target], p_tet);
    return 0;
}

// Detect collisions between two edges as they move.
bool edgeEdgeCCD(
    const Vector3d& edge0_vertex0_start,
    const Vector3d& edge0_vertex1_start,
    const Vector3d& edge1_vertex0_start,
    const Vector3d& edge1_vertex1_start,
    const Vector3d& edge0_vertex0_end,
    const Vector3d& edge0_vertex1_end,
    const Vector3d& edge1_vertex0_end,
    const Vector3d& edge1_vertex1_end,
    const double minimum_distance)
{
	//timer1.start();
	hex hx(
		edge0_vertex0_start, edge0_vertex1_start, edge1_vertex0_start,
		edge1_vertex1_start, edge0_vertex0_end, edge0_vertex1_end,
		edge1_vertex0_end, edge1_vertex1_end);
	//time3 += timer1.getElapsedTimeInSec();
	// step 1. bounding box checking
	
	Vector3d bmin(-minimum_distance, -minimum_distance, -minimum_distance),
		bmax(minimum_distance, minimum_distance, minimum_distance);
	bool intersection = hx.is_hex_bbox_cut_bbox(bmin, bmax);
	
	if (!intersection)
		return false; // if bounding box not intersected, then not intersected

	// step 2. prism edges & prism bottom triangles check
	// prism edges test, segment degenerate cases already handled
	// std::cout << "before seg- intersect cube in double" << std::endl;
	bool rt = false;
	
	for (int i = 0; i < 12; i++) {
		if (is_seg_intersect_cube(
			minimum_distance, hx.h_vertices[hx.hex_edge_id[i][0]],
			hx.h_vertices[hx.hex_edge_id[i][1]])) {
			// std::cout << "which seg intersect cube "<<i << std::endl;
			rt = true;
			break;
		}
	}
	
	if (rt) return true;
	cube cb(minimum_distance);

	// step 3 tet facets- cube edges
	std::array<std::array<bool, 8>, 6> v_tet; // cube vertices - tets positions
	
	bilinear bl0(
		hx.h_vertices[0], hx.h_vertices[1], hx.h_vertices[2], hx.h_vertices[3]);
	bilinear bl1(
		hx.h_vertices[4], hx.h_vertices[5], hx.h_vertices[6], hx.h_vertices[7]);
	bilinear bl2(
		hx.h_vertices[0], hx.h_vertices[1], hx.h_vertices[5], hx.h_vertices[4]);
	bilinear bl3(
		hx.h_vertices[1], hx.h_vertices[2], hx.h_vertices[6], hx.h_vertices[5]);
	bilinear bl4(
		hx.h_vertices[2], hx.h_vertices[3], hx.h_vertices[7], hx.h_vertices[6]);
	bilinear bl5(
		hx.h_vertices[0], hx.h_vertices[3], hx.h_vertices[7], hx.h_vertices[4]);
	std::array<bilinear, 6> bls = { { bl0, bl1, bl2, bl3, bl4, bl5 } };
	
	bool cube_inter_tet[6];
	// std::cout << "before cube - opposite faces in double" << std::endl;
	timer1.start();
	int discrete = 5;
	for (int i = 0; i < 6; i++) {
		if (is_cube_intersect_tet_opposite_faces(
			bls[i], cb, v_tet[i], cube_inter_tet[i])) {

			// bool rr=cube_discrete_bilinear_intersection(cb,bls[i],5);
			// if(!rr){
			//     std::cout<<"result do not match in opposite check, ori vs discrete "<<1<<" "<<rr<<std::endl;
			// }
			rt = true;
			break;
		}

	}
	
	if (rt) return true;

	for (int i = 0; i < 6; i++) {
		if (cube_inter_tet[i]) {
			timer1.start();
			bool cit0 = is_cube_edge_intersect_bilinear(bls[i], cb, v_tet[i]);
			// bool rr=cube_discrete_bilinear_intersection(cb,bls[i],5);
			// if(cit0!=rr){
			//     std::cout<<"result do not match, ori vs discrete "<<cit0<<" "<<rr<<std::endl;
			// }
			
			if (cit0)
				return true;
		}
	}

	timer1.start();
	int min_v = 6;
	int curr_v;
	int target = 0;
	for (int i = 0; i < 8; i++) {
		curr_v = v_tet[0][i] + v_tet[1][i] + v_tet[2][i] + v_tet[3][i]
			+ v_tet[4][i] + v_tet[5][i];
		if (curr_v < min_v) {
			min_v = curr_v;
			target = i;
		}
	}
	// std::cout << "shoot a ray in double" << std::endl;
	std::vector<bool> p_tet;
	p_tet.resize(6);
	p_tet[0] = v_tet[0][target];
	p_tet[1] = v_tet[1][target];
	p_tet[2] = v_tet[2][target];
	p_tet[3] = v_tet[3][target];
	p_tet[4] = v_tet[4][target];
	p_tet[5] = v_tet[5][target];

	bool rtccd = retrial_ccd_hex(bls, cb.vr[target], p_tet);
	
	return rtccd;
	return false;
}

} // namespace ccd
