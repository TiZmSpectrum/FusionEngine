/*************************************************************************/
/*  camera.cpp                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "camera.h"

#include "camera_matrix.h"
#include "scene/resources/material.h"
#include "scene/resources/surface_tool.h"


void Camera3D::_update_audio_listener_state() {


}

void Camera3D::_request_camera_update() {

	_update_camera();
}

void Camera3D::_update_camera_mode() {


	force_change=true;
	switch(mode) {
		case PROJECTION_PERSPECTIVE: {


			set_perspective(fov,near,far);

		} break;
		case PROJECTION_ORTHOGONAL: {
			set_orthogonal(size,near,far);
		} break;	
	
	}

}

bool Camera3D::_set(const StringName& p_name, const Variant& p_value) {

	bool changed_all=false;
	if (p_name=="projection") {

		int proj = p_value;
		if (proj==PROJECTION_PERSPECTIVE)
			mode=PROJECTION_PERSPECTIVE;
		if (proj==PROJECTION_ORTHOGONAL)
			mode=PROJECTION_ORTHOGONAL;

		changed_all=true;
	} else if (p_name=="fov" || p_name=="fovy" || p_name=="fovx")
		fov=p_value;
	else if (p_name=="size" || p_name=="sizex" || p_name=="sizey")
		size=p_value;
	else if (p_name=="near")
		near=p_value;
	else if (p_name=="far")
		far=p_value;														
	else if (p_name=="keep_aspect")
		set_keep_aspect_mode(KeepAspect(int(p_value)));
	else if (p_name=="vaspect")
		set_keep_aspect_mode(p_value?KEEP_WIDTH:KEEP_HEIGHT);
	else if (p_name=="current") {
		if (p_value.operator bool()) {
			make_current();
		} else {
			clear_current();
		}
	} else if (p_name=="visible_layers") {
		set_visible_layers(p_value);
	} else if (p_name=="environment") {
		set_environment(p_value);
	} else
		return false;
		
	_update_camera_mode();
	if (changed_all)
		_change_notify();
	return true;

}
bool Camera3D::_get(const StringName& p_name,Variant &r_ret) const {

	if (p_name=="projection") {
		r_ret= mode;
	} else if (p_name=="fov" || p_name=="fovy" || p_name=="fovx")
		r_ret= fov;
	else if (p_name=="size" || p_name=="sizex" || p_name=="sizey")
		r_ret= size;
	else if (p_name=="near")
		r_ret= near;
	else if (p_name=="far")
		r_ret= far;
	else if (p_name=="keep_aspect")
		r_ret= int(keep_aspect);
	else if (p_name=="current") {

		if (is_inside_tree() && get_tree()->is_editor_hint()) {
			r_ret=current;
		} else {
			r_ret=is_current();
		}
	} else if (p_name=="visible_layers") {
		r_ret=get_visible_layers();
	} else if (p_name=="environment") {
		r_ret=get_environment();
	} else
		return false;

	return true;
}

void Camera3D::_get_property_list( List<PropertyInfo> *p_list) const {

	p_list->push_back( PropertyInfo( Variant::INT, "projection", PROPERTY_HINT_ENUM, "Perspective,Orthogonal") );
	
	switch(mode) {
	
		case PROJECTION_PERSPECTIVE: {
		
			p_list->push_back( PropertyInfo( Variant::REAL, "fov" , PROPERTY_HINT_RANGE, "1,89,0.1",PROPERTY_USAGE_NOEDITOR) );
			if (keep_aspect==KEEP_WIDTH)
				p_list->push_back( PropertyInfo( Variant::REAL, "fovx" , PROPERTY_HINT_RANGE, "1,89,0.1",PROPERTY_USAGE_EDITOR) );
			else
				p_list->push_back( PropertyInfo( Variant::REAL, "fovy" , PROPERTY_HINT_RANGE, "1,89,0.1",PROPERTY_USAGE_EDITOR) );

			
		} break;
		case PROJECTION_ORTHOGONAL: {
		
			p_list->push_back( PropertyInfo( Variant::REAL, "size" , PROPERTY_HINT_RANGE, "1,16384,0.01",PROPERTY_USAGE_NOEDITOR ) );
			if (keep_aspect==KEEP_WIDTH)
				p_list->push_back( PropertyInfo( Variant::REAL, "sizex" , PROPERTY_HINT_RANGE, "0.1,16384,0.01",PROPERTY_USAGE_EDITOR) );
			else
				p_list->push_back( PropertyInfo( Variant::REAL, "sizey" , PROPERTY_HINT_RANGE, "0.1,16384,0.01",PROPERTY_USAGE_EDITOR) );

		} break;
	
	}
	
	p_list->push_back( PropertyInfo( Variant::REAL, "near" , PROPERTY_HINT_EXP_RANGE, "0.01,4096.0,0.01") );
	p_list->push_back( PropertyInfo( Variant::REAL, "far" , PROPERTY_HINT_EXP_RANGE, "0.01,4096.0,0.01") );
	p_list->push_back( PropertyInfo( Variant::INT, "keep_aspect",PROPERTY_HINT_ENUM,"Keep Width,Keep Height") );
	p_list->push_back( PropertyInfo( Variant::BOOL, "current" ) );
	p_list->push_back( PropertyInfo( Variant::INT, "visible_layers",PROPERTY_HINT_ALL_FLAGS ) );
	p_list->push_back( PropertyInfo( Variant::OBJECT, "environment",PROPERTY_HINT_RESOURCE_TYPE,"Environment" ) );

}

void Camera3D::_update_camera() {

	Transform3D tr = get_camera_transform();
	VisualServer::get_singleton()->camera_set_transform( camera, tr );

// here goes listener stuff
//	if (viewport_ptr && is_inside_scene() && is_current())
//		viewport_ptr->_camera_transform_changed_notify();

	if (is_inside_tree() && is_current()) {
		if (viewport_ptr) {
			viewport_ptr->_camera_transform_changed_notify();
		}
	}

	if (is_current() && get_world().is_valid()) {
		get_world()->_update_camera(this);
	}


}

void Camera3D::_notification(int p_what) {

	switch(p_what) {
	
		case NOTIFICATION_ENTER_WORLD: {

			viewport_ptr=NULL;

			{ //find viewport stuff
				Node *parent=get_parent();

				while(parent) {

					Viewport* viewport = parent->cast_to<Viewport>();

					if (viewport) {
						viewport_ptr=viewport;
						break;
					}
					parent=parent->get_parent();
				}

			}

			camera_group = "_vp_cameras"+itos(get_viewport()->get_instance_ID());
			add_to_group(camera_group);
			if (viewport_ptr)
				viewport_ptr->cameras.insert(this);
			if (current)
				make_current();


		} break;			
		case NOTIFICATION_TRANSFORM_CHANGED: {
		
			_request_camera_update();
		} break;
		case NOTIFICATION_EXIT_WORLD: {
		
			if (is_current()) {
				clear_current();
				current=true; //keep it true

			} else {
				current=false;
			}
			if (viewport_ptr)
				viewport_ptr->cameras.erase(this);
			viewport_ptr=NULL;
			remove_from_group(camera_group);


		} break;
		case NOTIFICATION_BECAME_CURRENT: {
			if (get_world().is_valid()) {
				get_world()->_register_camera(this);
			}
		} break;
		case NOTIFICATION_LOST_CURRENT: {
			if (get_world().is_valid()) {
				get_world()->_remove_camera(this);
			}
		} break;

	
	}

}


Transform3D Camera3D::get_camera_transform() const {

	return get_global_transform().orthonormalized();
}

void Camera3D::set_perspective(float p_fovy_degrees, float p_z_near, float p_z_far) {

	if (!force_change && fov==p_fovy_degrees && p_z_near==near && p_z_far==far && mode==PROJECTION_PERSPECTIVE)
		return;

	fov=p_fovy_degrees;
	near=p_z_near;
	far=p_z_far;
	mode=PROJECTION_PERSPECTIVE;
	
	VisualServer::get_singleton()->camera_set_perspective(camera,fov,near,far);
	update_gizmo();
	force_change=false;
}
void Camera3D::set_orthogonal(float p_size, float p_z_near, float p_z_far) {

	if (!force_change && size==p_size && p_z_near==near && p_z_far==far && mode==PROJECTION_ORTHOGONAL)
		return;

	size = p_size;
	
	near=p_z_near;
	far=p_z_far;
	mode=PROJECTION_ORTHOGONAL;
	force_change=false;
	
	VisualServer::get_singleton()->camera_set_orthogonal(camera,size,near,far);
	update_gizmo();
}

RID Camera3D::get_camera() const {

	return camera;
};

void Camera3D::make_current() {

	current=true;

	if (!is_inside_tree())
		return;

	if (viewport_ptr) {
		viewport_ptr->_set_camera(this);
	}

	//get_scene()->call_group(SceneMainLoop::GROUP_CALL_REALTIME,camera_group,"_camera_make_current",this);
}


void Camera3D::_camera_make_next_current(Node *p_exclude) {

	if (this==p_exclude)
		return;
	if (!is_inside_tree())
		return;
	if (get_viewport()->get_camera()!=NULL)
		return;

	make_current();
}


void Camera3D::clear_current() {

	current=false;
	if (!is_inside_tree())
		return;

	if (viewport_ptr) {
		if (viewport_ptr->get_camera()==this) {
			viewport_ptr->_set_camera(NULL);
			//a group is used beause this needs to be in order to be deterministic
			get_tree()->call_group(SceneTree::GROUP_CALL_REALTIME,camera_group,"_camera_make_next_current",this);

		}
	}

}

bool Camera3D::is_current() const {

	if (is_inside_tree()) {
		if (viewport_ptr)
			return viewport_ptr->get_camera()==this;
	} else
		return current;

	return false;
}


bool Camera3D::_can_gizmo_scale() const {

	return false;
}


RES Camera3D::_get_gizmo_geometry() const {


	Ref<SurfaceTool> surface_tool( memnew( SurfaceTool ));

	Ref<FixedMaterial> mat( memnew( FixedMaterial ));

	mat->set_parameter( FixedMaterial::PARAM_DIFFUSE,Color(1.0,0.5,1.0,0.5) );
	mat->set_line_width(4);
	mat->set_flag(Material::FLAG_DOUBLE_SIDED,true);
	mat->set_flag(Material::FLAG_UNSHADED,true);
	//mat->set_hint(Material::HINT_NO_DEPTH_DRAW,true);

	surface_tool->begin(Mesh::PRIMITIVE_LINES);
	surface_tool->set_material(mat);

	switch(mode) {

		case PROJECTION_PERSPECTIVE: {



			Vector3 side=Vector3( Math::sin(Math::deg2rad(fov)), 0, -Math::cos(Math::deg2rad(fov)) );
			Vector3 nside=side;
			nside.x=-nside.x;
			Vector3 up=Vector3(0,side.x,0);


#define ADD_TRIANGLE( m_a, m_b, m_c)\
{\
	surface_tool->add_vertex(m_a);\
	surface_tool->add_vertex(m_b);\
	surface_tool->add_vertex(m_b);\
	surface_tool->add_vertex(m_c);\
	surface_tool->add_vertex(m_c);\
	surface_tool->add_vertex(m_a);\
}

			ADD_TRIANGLE( Vector3(), side+up, side-up );
			ADD_TRIANGLE( Vector3(), nside+up, nside-up );
			ADD_TRIANGLE( Vector3(), side+up, nside+up );
			ADD_TRIANGLE( Vector3(), side-up, nside-up );

			side.x*=0.25;
			nside.x*=0.25;
			Vector3 tup( 0, up.y*3/2,side.z);
			ADD_TRIANGLE( tup, side+up, nside+up );

		} break;
		case PROJECTION_ORTHOGONAL: {

#define ADD_QUAD( m_a, m_b, m_c, m_d)\
{\
	surface_tool->add_vertex(m_a);\
	surface_tool->add_vertex(m_b);\
	surface_tool->add_vertex(m_b);\
	surface_tool->add_vertex(m_c);\
	surface_tool->add_vertex(m_c);\
	surface_tool->add_vertex(m_d);\
	surface_tool->add_vertex(m_d);\
	surface_tool->add_vertex(m_a);\
}

			float hsize=size*0.5;
			Vector3 right(hsize,0,0);
			Vector3 up(0,hsize,0);
			Vector3 back(0,0,-1.0);
			Vector3 front(0,0,0);

			ADD_QUAD( -up-right,-up+right,up+right,up-right);
			ADD_QUAD( -up-right+back,-up+right+back,up+right+back,up-right+back);
			ADD_QUAD( up+right,up+right+back,up-right+back,up-right);
			ADD_QUAD( -up+right,-up+right+back,-up-right+back,-up-right);

			right.x*=0.25;
			Vector3 tup( 0, up.y*3/2,back.z );
			ADD_TRIANGLE( tup, right+up+back, -right+up+back );

		} break;

	}

	return surface_tool->commit();

}

Vector3 Camera3D::project_ray_normal(const Point2& p_pos) const {

	Vector3 ray = project_local_ray_normal(p_pos);
	return get_camera_transform().basis.xform(ray).normalized();
};

Vector3 Camera3D::project_local_ray_normal(const Point2& p_pos) const {

	if (!is_inside_tree()) {
		ERR_EXPLAIN("Camera3D is not inside scene.");
		ERR_FAIL_COND_V(!is_inside_tree(),Vector3());
	}


#if 0
	Size2 viewport_size = viewport_ptr->get_visible_rect().size;
	Vector2 cpos = p_pos;
#else

	Size2 viewport_size = viewport_ptr->get_camera_rect_size();
	Vector2 cpos = viewport_ptr->get_camera_coords(p_pos);
#endif

	Vector3 ray;

	if (mode==PROJECTION_ORTHOGONAL) {

		ray=Vector3(0,0,-1);
	} else {
		CameraMatrix cm;
		cm.set_perspective(fov,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);
		float screen_w,screen_h;
		cm.get_viewport_size(screen_w,screen_h);
		ray=Vector3( ((cpos.x/viewport_size.width)*2.0-1.0)*screen_w, ((1.0-(cpos.y/viewport_size.height))*2.0-1.0)*screen_h,-near).normalized();
	}

	return ray;
};


Vector3 Camera3D::project_ray_origin(const Point2& p_pos) const {

	if (!is_inside_tree()) {
		ERR_EXPLAIN("Camera3D is not inside scene.");
		ERR_FAIL_COND_V(!is_inside_tree(),Vector3());
	}

#if 0
	Size2 viewport_size = viewport_ptr->get_visible_rect().size;
	Vector2 cpos = p_pos;
#else

	Size2 viewport_size = viewport_ptr->get_camera_rect_size();
	Vector2 cpos = viewport_ptr->get_camera_coords(p_pos);
#endif

	ERR_FAIL_COND_V( viewport_size.y == 0, Vector3() );
//	float aspect = viewport_size.x / viewport_size.y;

	if (mode == PROJECTION_PERSPECTIVE) {

		return get_camera_transform().origin;
	} else {

		Vector2 pos = cpos / viewport_size;
		float vsize,hsize;
		if (keep_aspect==KEEP_WIDTH) {
			vsize = size/viewport_size.get_aspect();
			hsize = size;
		} else {
			hsize = size*viewport_size.get_aspect();
			vsize = size;

		}



		Vector3 ray;
		ray.x = pos.x * (hsize) - hsize/2;
		ray.y = (1.0 - pos.y) * (vsize) - vsize/2;
		ray.z = -near;
		ray = get_camera_transform().xform(ray);
		return ray;
	};
};

Point2 Camera3D::unproject_position(const Vector3& p_pos) const {

	if (!is_inside_tree()) {
		ERR_EXPLAIN("Camera3D is not inside scene.");
		ERR_FAIL_COND_V(!is_inside_tree(),Vector2());
	}

	Size2 viewport_size = viewport_ptr->get_visible_rect().size;

	CameraMatrix cm;


	if (mode==PROJECTION_ORTHOGONAL)
		cm.set_orthogonal(size,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);
	else
		cm.set_perspective(fov,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);

	Plane p(get_camera_transform().xform_inv(p_pos),1.0);

	p=cm.xform4(p);
	p.normal/=p.d;


	Point2 res;
	res.x = (p.normal.x * 0.5 + 0.5)  * viewport_size.x;
	res.y = (-p.normal.y * 0.5 + 0.5)  * viewport_size.y;

	return res;

}

Vector3 Camera3D::project_position(const Point2& p_point) const {

	if (!is_inside_tree()) {
		ERR_EXPLAIN("Camera3D is not inside scene.");
		ERR_FAIL_COND_V(!is_inside_tree(),Vector3());
	}

	Size2 viewport_size = viewport_ptr->get_visible_rect().size;

	CameraMatrix cm;

	if (mode==PROJECTION_ORTHOGONAL)
		cm.set_orthogonal(size,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);
	else
		cm.set_perspective(fov,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);

	Size2 vp_size;
	cm.get_viewport_size(vp_size.x,vp_size.y);

	Vector2 point;
	point.x = (p_point.x/viewport_size.x) * 2.0 - 1.0;
	point.y = (p_point.y/viewport_size.y) * 2.0 - 1.0;
	point*=vp_size;

	Vector3 p(point.x,point.y,-near);


	return get_camera_transform().xform(p);
}

/*
void Camera3D::_camera_make_current(Node *p_camera) {


	if (p_camera==this) {
		VisualServer::get_singleton()->viewport_attach_camera(viewport_id,camera);
		active=true;
	} else {
		if (active && p_camera==NULL) {
			//detech camera because no one else will claim it
			VisualServer::get_singleton()->viewport_attach_camera(viewport_id,RID());
		}
		active=false;
	}
}
*/

void Camera3D::set_environment(const Ref<Environment>& p_environment) {

	environment=p_environment;
	if (environment.is_valid())
		VS::get_singleton()->camera_set_environment(camera,environment->get_rid());
	else
		VS::get_singleton()->camera_set_environment(camera,RID());
}

Ref<Environment> Camera3D::get_environment() const {

	return environment;
}


void Camera3D::set_keep_aspect_mode(KeepAspect p_aspect) {

	keep_aspect=p_aspect;
	VisualServer::get_singleton()->camera_set_use_vertical_aspect(camera,p_aspect==KEEP_WIDTH);

	_change_notify();
}

Camera3D::KeepAspect Camera3D::get_keep_aspect_mode() const{

	return keep_aspect;
}



void Camera3D::_bind_methods() {

	ObjectTypeDB::bind_method( _MD("project_ray_normal","screen_point"), &Camera3D::project_ray_normal);
	ObjectTypeDB::bind_method( _MD("project_local_ray_normal","screen_point"), &Camera3D::project_local_ray_normal);
	ObjectTypeDB::bind_method( _MD("project_ray_origin","screen_point"), &Camera3D::project_ray_origin);
	ObjectTypeDB::bind_method( _MD("unproject_position","world_point"), &Camera3D::unproject_position);
	ObjectTypeDB::bind_method( _MD("project_position","screen_point"), &Camera3D::project_position);
	ObjectTypeDB::bind_method( _MD("set_perspective","fov","z_near","z_far"),&Camera3D::set_perspective );
	ObjectTypeDB::bind_method( _MD("set_orthogonal","size","z_near","z_far"),&Camera3D::set_orthogonal );
	ObjectTypeDB::bind_method( _MD("make_current"),&Camera3D::make_current );
	ObjectTypeDB::bind_method( _MD("clear_current"),&Camera3D::clear_current );
	ObjectTypeDB::bind_method( _MD("is_current"),&Camera3D::is_current );
	ObjectTypeDB::bind_method( _MD("get_camera_transform"),&Camera3D::get_camera_transform );
	ObjectTypeDB::bind_method( _MD("get_fov"),&Camera3D::get_fov );
	ObjectTypeDB::bind_method( _MD("get_size"),&Camera3D::get_size );
	ObjectTypeDB::bind_method( _MD("get_zfar"),&Camera3D::get_zfar );
	ObjectTypeDB::bind_method( _MD("get_znear"),&Camera3D::get_znear );
	ObjectTypeDB::bind_method( _MD("get_projection"),&Camera3D::get_projection );
	ObjectTypeDB::bind_method( _MD("set_visible_layers","mask"),&Camera3D::set_visible_layers );
	ObjectTypeDB::bind_method( _MD("get_visible_layers"),&Camera3D::get_visible_layers );
	ObjectTypeDB::bind_method( _MD("look_at","target","up"),&Camera3D::look_at );
	ObjectTypeDB::bind_method( _MD("look_at_from_pos","pos","target","up"),&Camera3D::look_at_from_pos );
	ObjectTypeDB::bind_method(_MD("set_environment","env:Environment"),&Camera3D::set_environment);
	ObjectTypeDB::bind_method(_MD("get_environment:Environment"),&Camera3D::get_environment);
	ObjectTypeDB::bind_method(_MD("set_keep_aspect_mode","mode"),&Camera3D::set_keep_aspect_mode);
	ObjectTypeDB::bind_method(_MD("get_keep_aspect_mode"),&Camera3D::get_keep_aspect_mode);
	ObjectTypeDB::bind_method(_MD("_camera_make_next_current"),&Camera3D::_camera_make_next_current);
	//ObjectTypeDB::bind_method( _MD("_camera_make_current"),&Camera3D::_camera_make_current );

	BIND_CONSTANT( PROJECTION_PERSPECTIVE );
	BIND_CONSTANT( PROJECTION_ORTHOGONAL );

	BIND_CONSTANT( KEEP_WIDTH );
	BIND_CONSTANT( KEEP_HEIGHT );

}

float Camera3D::get_fov() const {

	return fov;
}

float Camera3D::get_size() const {

	return size;
}

float Camera3D::get_znear() const {

	return near;
}

float Camera3D::get_zfar() const {

	return far;
}


Camera3D::Projection Camera3D::get_projection() const {

	return mode;
}

void Camera3D::set_visible_layers(uint32_t p_layers) {

	layers=p_layers;
	VisualServer::get_singleton()->camera_set_visible_layers(camera,layers);
}

uint32_t Camera3D::get_visible_layers() const{

	return layers;
}


Vector<Plane> Camera3D::get_frustum() const {

	ERR_FAIL_COND_V(!is_inside_world(),Vector<Plane>());

	Size2 viewport_size = viewport_ptr->get_visible_rect().size;
	CameraMatrix cm;
	if (mode==PROJECTION_PERSPECTIVE)
		cm.set_perspective(fov,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);
	else
		cm.set_orthogonal(size,viewport_size.get_aspect(),near,far,keep_aspect==KEEP_WIDTH);

	return cm.get_projection_planes(get_camera_transform());

}



void Camera3D::look_at(const Vector3& p_target, const Vector3& p_up_normal) {

	Transform3D lookat;
	lookat.origin=get_camera_transform().origin;
	lookat=lookat.looking_at(p_target,p_up_normal);
	set_global_transform(lookat);
}

void Camera3D::look_at_from_pos(const Vector3& p_pos,const Vector3& p_target, const Vector3& p_up_normal) {

	Transform3D lookat;
	lookat.origin=p_pos;
	lookat=lookat.looking_at(p_target,p_up_normal);
	set_global_transform(lookat);

}


Camera3D::Camera3D() {

	camera = VisualServer::get_singleton()->camera_create();
	size=1;
	fov=0;
	near=0;
	far=0;
	current=false;
	viewport_ptr=NULL;
	force_change=false;
	mode=PROJECTION_PERSPECTIVE;
	set_perspective(60.0,0.1,100.0);
	keep_aspect=KEEP_HEIGHT;
	layers=0xfffff;
	VisualServer::get_singleton()->camera_set_visible_layers(camera,layers);
	//active=false;
}


Camera3D::~Camera3D() {

	VisualServer::get_singleton()->free(camera);

}


