/*************************************************************************/
/*  area_sw.h                                                            */
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
#ifndef AREA_SW_H
#define AREA_SW_H

#include "servers/physics_server.h"
#include "collision_object_sw.h"
#include "self_list.h"
//#include "servers/physics/query_sw.h"

class SpaceSW;
class BodySW;
class ConstraintSW;

class AreaSW : public CollisionObjectSW{


	PhysicsServer::AreaSpaceOverrideMode space_override_mode;
	float gravity;
	Vector3 gravity_vector;
	bool gravity_is_point;
	float point_attenuation;
	float density;
	int priority;
	bool monitorable;
	
	ObjectID monitor_callback_id;
	StringName monitor_callback_method;

	ObjectID area_monitor_callback_id;
	StringName area_monitor_callback_method;
	
	SelfList<AreaSW> monitor_query_list;
	SelfList<AreaSW> moved_list;

	struct BodyKey {

		RID rid;
		ObjectID instance_id;
		uint32_t body_shape;
		uint32_t area_shape;

		_FORCE_INLINE_ bool operator<( const BodyKey& p_key) const {

			if (rid==p_key.rid) {

				if (body_shape==p_key.body_shape) {

					return area_shape < p_key.area_shape;
				} else
					return body_shape < p_key.body_shape;
			} else
				return rid < p_key.rid;

		}

		_FORCE_INLINE_ BodyKey() {}
		BodyKey(BodySW *p_body, uint32_t p_body_shape,uint32_t p_area_shape);
		BodyKey(AreaSW *p_body, uint32_t p_body_shape,uint32_t p_area_shape);

	};

	struct BodyState {

		int state;
		_FORCE_INLINE_ void inc() { state++; }
		_FORCE_INLINE_ void dec() { state--; }
		_FORCE_INLINE_ BodyState() { state=0; }
	};

	Map<BodyKey,BodyState> monitored_bodies;
	Map<BodyKey,BodyState> monitored_areas;
	//virtual void shape_changed_notify(ShapeSW *p_shape);
	//virtual void shape_deleted_notify(ShapeSW *p_shape);

	Set<ConstraintSW*> constraints;


	virtual void _shapes_changed();
	void _queue_monitor_update();

public:

	//_FORCE_INLINE_ const Transform3D& get_inverse_transform() const { return inverse_transform; }
	//_FORCE_INLINE_ SpaceSW* get_owner() { return owner; }

	void set_monitor_callback(ObjectID p_id, const StringName& p_method);
	_FORCE_INLINE_ bool has_monitor_callback() const { return monitor_callback_id; }
	
	void set_area_monitor_callback(ObjectID p_id, const StringName& p_method);
	_FORCE_INLINE_ bool has_area_monitor_callback() const { return area_monitor_callback_id; }
	
	_FORCE_INLINE_ void add_body_to_query(BodySW *p_body, uint32_t p_body_shape,uint32_t p_area_shape);
	_FORCE_INLINE_ void remove_body_from_query(BodySW *p_body, uint32_t p_body_shape,uint32_t p_area_shape);

	_FORCE_INLINE_ void add_area_to_query(AreaSW *p_area, uint32_t p_area_shape,uint32_t p_self_shape);
	_FORCE_INLINE_ void remove_area_from_query(AreaSW *p_area, uint32_t p_area_shape,uint32_t p_self_shape);
	
	void set_param(PhysicsServer::AreaParameter p_param, const Variant& p_value);
	Variant get_param(PhysicsServer::AreaParameter p_param) const;

	void set_space_override_mode(PhysicsServer::AreaSpaceOverrideMode p_mode);
	PhysicsServer::AreaSpaceOverrideMode get_space_override_mode() const { return space_override_mode; }

	_FORCE_INLINE_ void set_gravity(float p_gravity) { gravity=p_gravity; }
	_FORCE_INLINE_ float get_gravity() const { return gravity; }

	_FORCE_INLINE_ void set_gravity_vector(const Vector3& p_gravity) { gravity_vector=p_gravity; }
	_FORCE_INLINE_ Vector3 get_gravity_vector() const { return gravity_vector; }

	_FORCE_INLINE_ void set_gravity_as_point(bool p_enable) { gravity_is_point=p_enable; }
	_FORCE_INLINE_ bool is_gravity_point() const { return gravity_is_point; }

	_FORCE_INLINE_ void set_point_attenuation(float p_point_attenuation) { point_attenuation=p_point_attenuation; }
	_FORCE_INLINE_ float get_point_attenuation() const { return point_attenuation; }

	_FORCE_INLINE_ void set_density(float p_density) { density=p_density; }
	_FORCE_INLINE_ float get_density() const { return density; }

	_FORCE_INLINE_ void set_priority(int p_priority) { priority=p_priority; }
	_FORCE_INLINE_ int get_priority() const { return priority; }

	_FORCE_INLINE_ void add_constraint( ConstraintSW* p_constraint) { constraints.insert(p_constraint); }
	_FORCE_INLINE_ void remove_constraint( ConstraintSW* p_constraint) { constraints.erase(p_constraint); }
	_FORCE_INLINE_ const Set<ConstraintSW*>& get_constraints() const { return constraints; }


	void set_monitorable(bool p_monitorable);
	_FORCE_INLINE_ bool is_monitorable() const { return monitorable; }

	void set_transform(const Transform3D& p_transform);

	void set_space(SpaceSW *p_space);


	void call_queries();

	AreaSW();
	~AreaSW();
};

void AreaSW::add_body_to_query(BodySW *p_body, uint32_t p_body_shape,uint32_t p_area_shape) {

	BodyKey bk(p_body,p_body_shape,p_area_shape);
	monitored_bodies[bk].inc();
	if (!monitor_query_list.in_list())
		_queue_monitor_update();
}
void AreaSW::remove_body_from_query(BodySW *p_body, uint32_t p_body_shape,uint32_t p_area_shape) {

	BodyKey bk(p_body,p_body_shape,p_area_shape);
	monitored_bodies[bk].dec();
	if (!monitor_query_list.in_list())
		_queue_monitor_update();
}



void AreaSW::add_area_to_query(AreaSW *p_area, uint32_t p_area_shape,uint32_t p_self_shape) {


	BodyKey bk(p_area,p_area_shape,p_self_shape);
	monitored_areas[bk].inc();
	if (!monitor_query_list.in_list())
		_queue_monitor_update();


}
void AreaSW::remove_area_from_query(AreaSW *p_area, uint32_t p_area_shape,uint32_t p_self_shape) {


	BodyKey bk(p_area,p_area_shape,p_self_shape);
	monitored_areas[bk].dec();
	if (!monitor_query_list.in_list())
		_queue_monitor_update();
}



#endif // AREA__SW_H
