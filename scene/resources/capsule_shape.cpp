/*************************************************************************/
/*  capsule_shape.cpp                                                    */
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
#include "capsule_shape.h"
#include "servers/physics_server.h"



void CapsuleShape3D::_update_shape() {

	Dictionary d;
	d["radius"]=radius;
	d["height"]=height;
	PhysicsServer::get_singleton()->shape_set_data(get_shape(),d);
}

void CapsuleShape3D::set_radius(float p_radius) {

	radius=p_radius;
	_update_shape();
	notify_change_to_owners();
	_change_notify("radius");
}

float CapsuleShape3D::get_radius() const {

	return radius;
}

void CapsuleShape3D::set_height(float p_height) {

	height=p_height;
	_update_shape();
	notify_change_to_owners();
	_change_notify("height");
}

float CapsuleShape3D::get_height() const {

	return height;
}


void CapsuleShape3D::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_radius","radius"),&CapsuleShape3D::set_radius);
	ObjectTypeDB::bind_method(_MD("get_radius"),&CapsuleShape3D::get_radius);
	ObjectTypeDB::bind_method(_MD("set_height","height"),&CapsuleShape3D::set_height);
	ObjectTypeDB::bind_method(_MD("get_height"),&CapsuleShape3D::get_height);

	ADD_PROPERTY( PropertyInfo(Variant::REAL,"radius",PROPERTY_HINT_RANGE,"0.01,4096,0.01"), _SCS("set_radius"),_SCS("get_radius") );
	ADD_PROPERTY( PropertyInfo(Variant::REAL,"height",PROPERTY_HINT_RANGE,"0.01,4096,0.01"), _SCS("set_height"),_SCS("get_height") );

}

CapsuleShape3D::CapsuleShape3D() : Shape3D( PhysicsServer::get_singleton()->shape_create(PhysicsServer::SHAPE_CAPSULE)) {

	radius=1.0;
	height=1.0;
	_update_shape();
}
