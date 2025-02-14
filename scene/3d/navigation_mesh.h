#ifndef NAVIGATION_MESH_H
#define NAVIGATION_MESH_H

#include "scene/3d/spatial.h"
#include "scene/resources/mesh.h"


class NavigationMesh : public Resource  {

	OBJ_TYPE( NavigationMesh, Resource );

	DVector<Vector3> vertices;
	struct Polygon {
		Vector<int> indices;
	};
	Vector<Polygon> polygons;

protected:

	static void _bind_methods();

	void _set_polygons(const Array& p_array);
	Array _get_polygons() const;
public:

	void create_from_mesh(const Ref<Mesh>& p_mesh);

	void set_vertices(const DVector<Vector3>& p_vertices);
	DVector<Vector3> get_vertices() const;

	void add_polygon(const Vector<int>& p_polygon);
	int get_polygon_count() const;
	Vector<int> get_polygon(int p_idx);
	void clear_polygons();

	NavigationMesh();
};


class Navigation;

class NavigationMeshInstance : public Node3D {

	OBJ_TYPE(NavigationMeshInstance,Node3D);

	bool enabled;
	int nav_id;
	Navigation *navigation;
	Ref<NavigationMesh> navmesh;
protected:

	void _notification(int p_what);
	static void _bind_methods();
public:



	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	void set_navigation_mesh(const Ref<NavigationMesh>& p_navmesh);
	Ref<NavigationMesh> get_navigation_mesh() const;

	NavigationMeshInstance();
};


#endif // NAVIGATION_MESH_H
