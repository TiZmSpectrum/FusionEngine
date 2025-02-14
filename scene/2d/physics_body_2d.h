/*************************************************************************/
/*  physics_body_2d.h                                                    */
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
#ifndef PHYSICS_BODY_2D_H
#define PHYSICS_BODY_2D_H

#include "scene/2d/collision_object_2d.h"
#include "servers/physics_2d_server.h"
#include "vset.h"


class PhysicsBody2D : public CollisionObject2D {

	OBJ_TYPE(PhysicsBody2D,CollisionObject2D);

	uint32_t mask;
protected:

	void _notification(int p_what);
	PhysicsBody2D(Physics2DServer::BodyMode p_mode);

	static void _bind_methods();
public:

	void set_layer_mask(uint32_t p_mask);
	uint32_t get_layer_mask() const;

	void add_collision_exception_with(Node* p_node); //must be physicsbody
	void remove_collision_exception_with(Node* p_node);

	PhysicsBody2D();

};

class StaticBody2D : public PhysicsBody2D {

	OBJ_TYPE(StaticBody2D,PhysicsBody2D);

	Vector2 constant_linear_velocity;
	real_t constant_angular_velocity;

	real_t bounce;
	real_t friction;


protected:

	static void _bind_methods();

public:

	void set_friction(real_t p_friction);
	real_t get_friction() const;

	void set_bounce(real_t p_bounce);
	real_t get_bounce() const;


	void set_constant_linear_velocity(const Vector2& p_vel);
	void set_constant_angular_velocity(real_t p_vel);

	Vector2 get_constant_linear_velocity() const;
	real_t get_constant_angular_velocity() const;

	StaticBody2D();
	~StaticBody2D();

};

class RigidBody2D : public PhysicsBody2D {

	OBJ_TYPE(RigidBody2D,PhysicsBody2D);
public:

	enum Mode {
		MODE_RIGID,
		MODE_STATIC,
		MODE_CHARACTER,
		MODE_KINEMATIC,
	};

	enum CCDMode {
		CCD_MODE_DISABLED,
		CCD_MODE_CAST_RAY,
		CCD_MODE_CAST_SHAPE,
	};

private:

	bool can_sleep;
	Physics2DDirectBodyState *state;
	Mode mode;

	real_t bounce;
	real_t mass;
	real_t friction;

	Vector2 linear_velocity;
	real_t angular_velocity;
	bool sleeping;


	int max_contacts_reported;

	bool custom_integrator;

	CCDMode ccd_mode;


	struct ShapePair {

		int body_shape;
		int local_shape;
		bool tagged;
		bool operator<(const ShapePair& p_sp) const {
			if (body_shape==p_sp.body_shape)
				return local_shape < p_sp.local_shape;
			else
				return body_shape < p_sp.body_shape;
		}

		ShapePair() {}
		ShapePair(int p_bs, int p_ls) { body_shape=p_bs; local_shape=p_ls; }
	};
	struct RigidBody2D_RemoveAction {


		ObjectID body_id;
		ShapePair pair;

	};
	struct BodyState {

		//int rc;
		bool in_scene;
		VSet<ShapePair> shapes;
	};

	struct ContactMonitor {


		Map<ObjectID,BodyState> body_map;

	};


	ContactMonitor *contact_monitor;
	void _body_enter_tree(ObjectID p_id);
	void _body_exit_tree(ObjectID p_id);


	void _body_inout(int p_status, ObjectID p_instance, int p_body_shape,int p_local_shape);
	void _direct_state_changed(Object *p_state);


protected:

	static void _bind_methods();
public:

	void set_mode(Mode p_mode);
	Mode get_mode() const;

	void set_mass(real_t p_mass);
	real_t get_mass() const;

	void set_weight(real_t p_weight);
	real_t get_weight() const;

	void set_friction(real_t p_friction);
	real_t get_friction() const;

	void set_bounce(real_t p_bounce);
	real_t get_bounce() const;

	void set_linear_velocity(const Vector2& p_velocity);
	Vector2 get_linear_velocity() const;

	void set_axis_velocity(const Vector2& p_axis);

	void set_angular_velocity(real_t p_velocity);
	real_t get_angular_velocity() const;

	void set_use_custom_integrator(bool p_enable);
	bool is_using_custom_integrator();

	void set_sleeping(bool p_sleeping);
	bool is_sleeping() const;

	void set_can_sleep(bool p_active);
	bool is_able_to_sleep() const;

	void set_contact_monitor(bool p_enabled);
	bool is_contact_monitor_enabled() const;

	void set_max_contacts_reported(int p_amount);
	int get_max_contacts_reported() const;

	void set_continuous_collision_detection_mode(CCDMode p_mode);
	CCDMode get_continuous_collision_detection_mode() const;

	void apply_impulse(const Vector2& p_pos, const Vector2& p_impulse);

	void set_applied_force(const Vector2& p_force);
	Vector2 get_applied_force() const;

	Array get_colliding_bodies() const; //function for script

	RigidBody2D();
	~RigidBody2D();

};

VARIANT_ENUM_CAST(RigidBody2D::Mode);
VARIANT_ENUM_CAST(RigidBody2D::CCDMode);



class CharacterBody2D : public PhysicsBody2D {

	OBJ_TYPE(CharacterBody2D,PhysicsBody2D);

	float margin;
	bool collide_static;
	bool collide_rigid;
	bool collide_kinematic;
	bool collide_character;

	bool colliding;
	Vector2 collision;
	Vector2 normal;
	Vector2 collider_vel;
	ObjectID collider;
	int collider_shape;
	Variant collider_metadata;

	Variant _get_collider() const;

	_FORCE_INLINE_ bool _ignores_mode(Physics2DServer::BodyMode) const;
protected:

	static void _bind_methods();
public:

	Vector2 move(const Vector2& p_motion);
	Vector2 move_to(const Vector2& p_position);

	bool can_move_to(const Vector2& p_position,bool p_discrete=false);
	bool is_colliding() const;
	Vector2 get_collision_pos() const;
	Vector2 get_collision_normal() const;
	Vector2 get_collider_velocity() const;
	ObjectID get_collider() const;
	int get_collider_shape() const;
	Variant get_collider_metadata() const;

	void set_collide_with_static_bodies(bool p_enable);
	bool can_collide_with_static_bodies() const;

	void set_collide_with_rigid_bodies(bool p_enable);
	bool can_collide_with_rigid_bodies() const;

	void set_collide_with_kinematic_bodies(bool p_enable);
	bool can_collide_with_kinematic_bodies() const;

	void set_collide_with_character_bodies(bool p_enable);
	bool can_collide_with_character_bodies() const;

	void set_collision_margin(float p_margin);
	float get_collision_margin() const;

	CharacterBody2D();
	~CharacterBody2D();

};


#endif // PHYSICS_BODY_2D_H
