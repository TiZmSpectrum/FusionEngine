/*************************************************************************/
/*  variant_call.cpp                                                     */
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
#include "variant.h"
#include "object.h"
#include "os/os.h"
#include "core_string_names.h"
#include "script_language.h"

typedef void (*VariantFunc)(Variant& r_ret,Variant& p_self,const Variant** p_args);
typedef void (*VariantConstructFunc)(Variant& r_ret,const Variant** p_args);

VARIANT_ENUM_CAST(Image::CompressMode);
//VARIANT_ENUM_CAST(Image::Format);


struct _VariantCall {




	static void Vector3_dot(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		r_ret=reinterpret_cast<Vector3*>(p_self._data._mem)->dot(*reinterpret_cast<const Vector3*>(p_args[0]->_data._mem));
	}

	struct FuncData {

		int arg_count;
		Vector<Variant> default_args;
		Vector<Variant::Type> arg_types;

#ifdef DEBUG_ENABLED
		Vector<StringName> arg_names;
		Variant::Type return_type;
		bool returns;
#endif
		VariantFunc func;

		_FORCE_INLINE_ bool verify_arguments(const Variant **p_args,Variant::CallError &r_error) {

			if (arg_count==0)
				return true;

			Variant::Type *tptr = &arg_types[0];

			for(int i=0;i<arg_count;i++) {

				if (!tptr[i] || tptr[i]==p_args[i]->type)
					continue; // all good
				if (!Variant::can_convert(p_args[i]->type,tptr[i])) {
					r_error.error=Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
					r_error.argument=i;
					r_error.expected=tptr[i];
					return false;

				}
			}
			return true;
		}

		_FORCE_INLINE_ void call(Variant& r_ret,Variant& p_self,const Variant** p_args,int p_argcount,Variant::CallError &r_error) {
#ifdef DEBUG_ENABLED
			if(p_argcount>arg_count) {
				r_error.error=Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS;
				r_error.argument=arg_count;
				return;
			} else
#endif
			if (p_argcount<arg_count) {
				int def_argcount = default_args.size();
#ifdef DEBUG_ENABLED
				if (p_argcount<(arg_count-def_argcount)) {
					r_error.error=Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
					r_error.argument=arg_count-def_argcount;
					return;
				}

#endif
				ERR_FAIL_COND(p_argcount>VARIANT_ARG_MAX);
				const Variant *newargs[VARIANT_ARG_MAX];
				for(int i=0;i<p_argcount;i++)
					newargs[i]=p_args[i];
				int defargcount=def_argcount;
				for(int i=p_argcount;i<arg_count;i++)
					newargs[i]=&default_args[defargcount-(i-p_argcount)-1]; //default arguments
#ifdef DEBUG_ENABLED
				if (!verify_arguments(newargs,r_error))
					return;
#endif
				func(r_ret,p_self,newargs);
			} else {
#ifdef DEBUG_ENABLED
				if (!verify_arguments(p_args,r_error))
					return;
#endif
				func(r_ret,p_self,p_args);
			}

		}

	};


	struct TypeFunc {

		Map<StringName,FuncData> functions;
	};

	static TypeFunc* type_funcs;





	struct Arg {
		StringName name;
		Variant::Type type;
		Arg() { type=Variant::NIL;}
		Arg(Variant::Type p_type,const StringName &p_name) { name=p_name; type=p_type; }
	};

//	void addfunc(Variant::Type p_type, const StringName& p_name,VariantFunc p_func);
	static void addfunc(Variant::Type p_type, Variant::Type p_return,const StringName& p_name,VariantFunc p_func, const Vector<Variant>& p_defaultarg,const Arg& p_argtype1=Arg(),const Arg& p_argtype2=Arg(),const Arg& p_argtype3=Arg(),const Arg& p_argtype4=Arg(),const Arg& p_argtype5=Arg()) {

		FuncData funcdata;
		funcdata.func=p_func;
		funcdata.default_args=p_defaultarg;
#ifdef DEBUG_ENABLED
		funcdata.return_type=p_return;
		funcdata.returns=p_return!=Variant::NIL;
#endif

		if (p_argtype1.name) {
			funcdata.arg_types.push_back(p_argtype1.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype1.name);
#endif

		} else
			goto end;

		if (p_argtype2.name) {
			funcdata.arg_types.push_back(p_argtype2.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype2.name);
#endif

		} else
			goto end;

		if (p_argtype3.name) {
			funcdata.arg_types.push_back(p_argtype3.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype3.name);
#endif

		} else
			goto end;

		if (p_argtype4.name) {
			funcdata.arg_types.push_back(p_argtype4.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype4.name);
#endif
		} else
			goto end;

		if (p_argtype5.name) {
			funcdata.arg_types.push_back(p_argtype5.type);
#ifdef DEBUG_ENABLED
			funcdata.arg_names.push_back(p_argtype5.name);
#endif
		} else
			goto end;

		end:

		funcdata.arg_count=funcdata.arg_types.size();
		type_funcs[p_type].functions[p_name]=funcdata;

	}

#define VCALL_LOCALMEM0(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._mem)->m_method(); }
#define VCALL_LOCALMEM0R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._mem)->m_method(); }
#define VCALL_LOCALMEM1(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0]); }
#define VCALL_LOCALMEM1R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0]); }
#define VCALL_LOCALMEM2(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1]); }
#define VCALL_LOCALMEM2R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1]); }
#define VCALL_LOCALMEM3(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1],*p_args[2]); }
#define VCALL_LOCALMEM3R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1],*p_args[2]); }
#define VCALL_LOCALMEM4(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3]); }
#define VCALL_LOCALMEM4R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3]); }
#define VCALL_LOCALMEM5(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3],*p_args[4]); }
#define VCALL_LOCALMEM5R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._mem)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3],*p_args[4]); }


	// built-in functions of localmem based types

	VCALL_LOCALMEM1R(String,casecmp_to);
	VCALL_LOCALMEM1R(String,nocasecmp_to);
	VCALL_LOCALMEM0R(String,length);
	VCALL_LOCALMEM2R(String,substr);
	VCALL_LOCALMEM2R(String,find);
	VCALL_LOCALMEM1R(String,find_last);
	VCALL_LOCALMEM2R(String,findn);
	VCALL_LOCALMEM2R(String,rfind);
	VCALL_LOCALMEM2R(String,rfindn);
	VCALL_LOCALMEM1R(String,match);
	VCALL_LOCALMEM1R(String,matchn);
	VCALL_LOCALMEM1R(String,begins_with);
	VCALL_LOCALMEM2R(String,replace);
	VCALL_LOCALMEM2R(String,replacen);
	VCALL_LOCALMEM2R(String,insert);
	VCALL_LOCALMEM0R(String,capitalize);
	VCALL_LOCALMEM2R(String,split);
	VCALL_LOCALMEM2R(String,split_floats);
	VCALL_LOCALMEM0R(String,to_upper);
	VCALL_LOCALMEM0R(String,to_lower);
	VCALL_LOCALMEM1R(String,left);
	VCALL_LOCALMEM1R(String,right);
	VCALL_LOCALMEM0R(String,strip_edges);
	VCALL_LOCALMEM0R(String,extension);
	VCALL_LOCALMEM0R(String,basename);
	VCALL_LOCALMEM1R(String,plus_file);
	VCALL_LOCALMEM1R(String,ord_at);
	//VCALL_LOCALMEM2R(String,erase);
	VCALL_LOCALMEM0R(String,hash);
	VCALL_LOCALMEM0R(String,md5_text);
	VCALL_LOCALMEM0R(String,md5_buffer);
	VCALL_LOCALMEM0R(String,empty);
	VCALL_LOCALMEM0R(String,is_abs_path);
	VCALL_LOCALMEM0R(String,is_rel_path);
	VCALL_LOCALMEM0R(String,get_base_dir);
	VCALL_LOCALMEM0R(String,get_file);
	VCALL_LOCALMEM0R(String,xml_escape);
	VCALL_LOCALMEM0R(String,xml_unescape);
	VCALL_LOCALMEM0R(String,percent_encode);
	VCALL_LOCALMEM0R(String,percent_decode);
	VCALL_LOCALMEM0R(String,is_valid_identifier);
	VCALL_LOCALMEM0R(String,is_valid_integer);
	VCALL_LOCALMEM0R(String,is_valid_float);
	VCALL_LOCALMEM0R(String,is_valid_html_color);
	VCALL_LOCALMEM0R(String,is_valid_ip_address);
	VCALL_LOCALMEM0R(String,to_int);
	VCALL_LOCALMEM0R(String,to_float);
	VCALL_LOCALMEM0R(String,hex_to_int);
	VCALL_LOCALMEM1R(String,pad_decimals);
	VCALL_LOCALMEM1R(String,pad_zeros);


	VCALL_LOCALMEM0R(Vector2,normalized);
	VCALL_LOCALMEM0R(Vector2,length);
	VCALL_LOCALMEM0R(Vector2,length_squared);
	VCALL_LOCALMEM1R(Vector2,distance_to);
	VCALL_LOCALMEM1R(Vector2,distance_squared_to);
	VCALL_LOCALMEM1R(Vector2,angle_to);
	VCALL_LOCALMEM1R(Vector2,angle_to_point);
	VCALL_LOCALMEM2R(Vector2,linear_interpolate);
	VCALL_LOCALMEM4R(Vector2,cubic_interpolate);
	VCALL_LOCALMEM1R(Vector2,rotated);
	VCALL_LOCALMEM0R(Vector2,tangent);
	VCALL_LOCALMEM0R(Vector2,floor);
	VCALL_LOCALMEM1R(Vector2,snapped);
	VCALL_LOCALMEM0R(Vector2,get_aspect);
	VCALL_LOCALMEM1R(Vector2,dot);
	VCALL_LOCALMEM1R(Vector2,slide);
	VCALL_LOCALMEM1R(Vector2,reflect);
	VCALL_LOCALMEM0R(Vector2,atan2);
//	VCALL_LOCALMEM1R(Vector2,cross);

	VCALL_LOCALMEM0R(Rect2,get_area);
	VCALL_LOCALMEM1R(Rect2,intersects);
	VCALL_LOCALMEM1R(Rect2,encloses);
	VCALL_LOCALMEM0R(Rect2,has_no_area);
	VCALL_LOCALMEM1R(Rect2,clip);
	VCALL_LOCALMEM1R(Rect2,merge);
	VCALL_LOCALMEM1R(Rect2,has_point);
	VCALL_LOCALMEM1R(Rect2,grow);
	VCALL_LOCALMEM1R(Rect2,expand);

	VCALL_LOCALMEM0R(Vector3, min_axis);
	VCALL_LOCALMEM0R(Vector3, max_axis);
	VCALL_LOCALMEM0R(Vector3, length);
	VCALL_LOCALMEM0R(Vector3, length_squared);
	VCALL_LOCALMEM0R(Vector3, normalized);
	VCALL_LOCALMEM0R(Vector3, inverse);
	VCALL_LOCALMEM1R(Vector3, snapped);
	VCALL_LOCALMEM2R(Vector3, rotated);
	VCALL_LOCALMEM2R(Vector3, linear_interpolate);
	VCALL_LOCALMEM4R(Vector3, cubic_interpolate);
	VCALL_LOCALMEM1R(Vector3, dot);
	VCALL_LOCALMEM1R(Vector3, cross);
	VCALL_LOCALMEM0R(Vector3, abs);
	VCALL_LOCALMEM1R(Vector3, distance_to);
	VCALL_LOCALMEM1R(Vector3, distance_squared_to);
	VCALL_LOCALMEM1R(Vector3, slide);
	VCALL_LOCALMEM1R(Vector3, reflect);


	VCALL_LOCALMEM0R(Plane,normalized);
	VCALL_LOCALMEM0R(Plane,center);
	VCALL_LOCALMEM0R(Plane,get_any_point);
	VCALL_LOCALMEM1R(Plane,is_point_over);
	VCALL_LOCALMEM1R(Plane,distance_to);
	VCALL_LOCALMEM2R(Plane,has_point);
	VCALL_LOCALMEM1R(Plane,project);

	//return vector3 if intersected, nil if not
	static void _call_Plane_intersect_3(Variant& r_ret,Variant& p_self,const Variant** p_args) {
		Vector3 result;
		if (reinterpret_cast<Plane*>(p_self._data._mem)->intersect_3(*p_args[0],*p_args[1],&result))
			r_ret=result;
		else
			r_ret=Variant();
	}

	static void _call_Plane_intersects_ray(Variant& r_ret,Variant& p_self,const Variant** p_args) {
		Vector3 result;
		if (reinterpret_cast<Plane*>(p_self._data._mem)->intersects_ray(*p_args[0],*p_args[1],&result))
			r_ret=result;
		else
			r_ret=Variant();
	}

	static void _call_Plane_intersects_segment(Variant& r_ret,Variant& p_self,const Variant** p_args) {
		Vector3 result;
		if (reinterpret_cast<Plane*>(p_self._data._mem)->intersects_segment(*p_args[0],*p_args[1],&result))
			r_ret=result;
		else
			r_ret=Variant();
	}

	static void _call_Vector2_floorf(Variant& r_ret,Variant& p_self,const Variant** p_args) {
		r_ret = reinterpret_cast<Vector2*>(p_self._data._mem)->floor();
	};

	VCALL_LOCALMEM0R(Quat,length);
	VCALL_LOCALMEM0R(Quat,length_squared);
	VCALL_LOCALMEM0R(Quat,normalized);
	VCALL_LOCALMEM0R(Quat,inverse);
	VCALL_LOCALMEM1R(Quat,dot);
	VCALL_LOCALMEM2R(Quat,slerp);
	VCALL_LOCALMEM2R(Quat,slerpni);
	VCALL_LOCALMEM4R(Quat,cubic_slerp);

	VCALL_LOCALMEM0R(Color,to_32);
	VCALL_LOCALMEM0R(Color,to_ARGB32);
	VCALL_LOCALMEM0R(Color,gray);
	VCALL_LOCALMEM0R(Color,inverted);
	VCALL_LOCALMEM0R(Color,contrasted);
	VCALL_LOCALMEM2R(Color,linear_interpolate);
	VCALL_LOCALMEM1R(Color,blend);
	VCALL_LOCALMEM1R(Color,to_html);

	VCALL_LOCALMEM0R(RID,get_id);

	VCALL_LOCALMEM0R(NodePath,is_absolute);
	VCALL_LOCALMEM0R(NodePath,get_name_count);
	VCALL_LOCALMEM1R(NodePath,get_name);
	VCALL_LOCALMEM0R(NodePath,get_subname_count);
	VCALL_LOCALMEM1R(NodePath,get_subname);
	VCALL_LOCALMEM0R(NodePath,get_property);
	VCALL_LOCALMEM0R(NodePath,is_empty);

	VCALL_LOCALMEM0R(Dictionary,size);
	VCALL_LOCALMEM0R(Dictionary,empty);
	VCALL_LOCALMEM0(Dictionary,clear);
	VCALL_LOCALMEM1R(Dictionary,has);
	VCALL_LOCALMEM1(Dictionary,erase);
	VCALL_LOCALMEM0R(Dictionary,hash);
	VCALL_LOCALMEM0R(Dictionary,keys);
	VCALL_LOCALMEM1R(Dictionary,parse_json);
	VCALL_LOCALMEM0R(Dictionary,to_json);

	VCALL_LOCALMEM2(Array,set);
	VCALL_LOCALMEM1R(Array,get);
	VCALL_LOCALMEM0R(Array,size);
	VCALL_LOCALMEM0R(Array,empty);
	VCALL_LOCALMEM0(Array,clear);
	VCALL_LOCALMEM0R(Array,hash);
	VCALL_LOCALMEM1(Array,push_back);
	VCALL_LOCALMEM1(Array,append);
	VCALL_LOCALMEM1(Array,resize);
	VCALL_LOCALMEM2(Array,insert);
	VCALL_LOCALMEM1(Array,remove);
	VCALL_LOCALMEM1R(Array,find);
	VCALL_LOCALMEM1(Array,erase);
	VCALL_LOCALMEM0(Array,sort);
	VCALL_LOCALMEM2(Array,sort_custom);
	VCALL_LOCALMEM0(Array,invert);

	static void _call_ByteArray_get_string_from_ascii(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		ByteArray* ba = reinterpret_cast<ByteArray*>(p_self._data._mem);
		String s;
		if (ba->size()>=0) {
			ByteArray::Read r = ba->read();
			CharString cs;
			cs.resize(ba->size()+1);
			copymem(cs.ptr(),r.ptr(),ba->size());
			cs[ba->size()]=0;

			s = cs.get_data();
		}
		r_ret=s;
	}

	static void _call_ByteArray_get_string_from_utf8(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		ByteArray* ba = reinterpret_cast<ByteArray*>(p_self._data._mem);
		String s;
		if (ba->size()>=0) {
			ByteArray::Read r = ba->read();
			s.parse_utf8((const char*)r.ptr(),ba->size());
		}
		r_ret=s;
	}

	VCALL_LOCALMEM0R(ByteArray,size);
	VCALL_LOCALMEM2(ByteArray,set);
	VCALL_LOCALMEM1R(ByteArray,get);
	VCALL_LOCALMEM1(ByteArray,push_back);
	VCALL_LOCALMEM1(ByteArray,resize);
	VCALL_LOCALMEM1(ByteArray,append);
	VCALL_LOCALMEM1(ByteArray,append_array);

	VCALL_LOCALMEM0R(PackedIntArray,size);
	VCALL_LOCALMEM2(PackedIntArray,set);
	VCALL_LOCALMEM1R(PackedIntArray,get);
	VCALL_LOCALMEM1(PackedIntArray,push_back);
	VCALL_LOCALMEM1(PackedIntArray,resize);
	VCALL_LOCALMEM1(PackedIntArray,append);
	VCALL_LOCALMEM1(PackedIntArray,append_array);

	VCALL_LOCALMEM0R(PackedFloatArray,size);
	VCALL_LOCALMEM2(PackedFloatArray,set);
	VCALL_LOCALMEM1R(PackedFloatArray,get);
	VCALL_LOCALMEM1(PackedFloatArray,push_back);
	VCALL_LOCALMEM1(PackedFloatArray,resize);
	VCALL_LOCALMEM1(PackedFloatArray,append);
	VCALL_LOCALMEM1(PackedFloatArray,append_array);

	VCALL_LOCALMEM0R(PackedStringArray,size);
	VCALL_LOCALMEM2(PackedStringArray,set);
	VCALL_LOCALMEM1R(PackedStringArray,get);
	VCALL_LOCALMEM1(PackedStringArray,push_back);
	VCALL_LOCALMEM1(PackedStringArray,resize);
	VCALL_LOCALMEM1(PackedStringArray,append);
	VCALL_LOCALMEM1(PackedStringArray,append_array);

	VCALL_LOCALMEM0R(PackedVector2Array,size);
	VCALL_LOCALMEM2(PackedVector2Array,set);
	VCALL_LOCALMEM1R(PackedVector2Array,get);
	VCALL_LOCALMEM1(PackedVector2Array,push_back);
	VCALL_LOCALMEM1(PackedVector2Array,resize);
	VCALL_LOCALMEM1(PackedVector2Array,append);
	VCALL_LOCALMEM1(PackedVector2Array,append_array);

	VCALL_LOCALMEM0R(PackedVector3Array,size);
	VCALL_LOCALMEM2(PackedVector3Array,set);
	VCALL_LOCALMEM1R(PackedVector3Array,get);
	VCALL_LOCALMEM1(PackedVector3Array,push_back);
	VCALL_LOCALMEM1(PackedVector3Array,resize);
	VCALL_LOCALMEM1(PackedVector3Array,append);
	VCALL_LOCALMEM1(PackedVector3Array,append_array);

	VCALL_LOCALMEM0R(PackedColorArray,size);
	VCALL_LOCALMEM2(PackedColorArray,set);
	VCALL_LOCALMEM1R(PackedColorArray,get);
	VCALL_LOCALMEM1(PackedColorArray,push_back);
	VCALL_LOCALMEM1(PackedColorArray,resize);
	VCALL_LOCALMEM1(PackedColorArray,append);
	VCALL_LOCALMEM1(PackedColorArray,append_array);

#define VCALL_PTR0(m_type,m_method)\
static void _call_##m_type##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(); }
#define VCALL_PTR0R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(); }
#define VCALL_PTR1(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0]); }
#define VCALL_PTR1R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0]); }
#define VCALL_PTR2(m_type,m_method)\
static void _call_##m_type##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1]); }
#define VCALL_PTR2R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1]); }
#define VCALL_PTR3(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1],*p_args[2]); }
#define VCALL_PTR3R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1],*p_args[2]); }
#define VCALL_PTR4(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3]); }
#define VCALL_PTR4R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3]); }
#define VCALL_PTR5(m_type,m_method)\
static void _call_##m_type##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3],*p_args[4]); }
#define VCALL_PTR5R(m_type,m_method)\
static void _call_##m_type##_##m_method(Variant& r_ret,Variant& p_self,const Variant** p_args) { r_ret=reinterpret_cast<m_type*>(p_self._data._ptr)->m_method(*p_args[0],*p_args[1],*p_args[2],*p_args[3],*p_args[4]); }

	VCALL_PTR0R(Image,get_format);
	VCALL_PTR0R(Image,get_width);
	VCALL_PTR0R(Image,get_height);
	VCALL_PTR0R(Image,empty);
	VCALL_PTR3R(Image,get_pixel);
	VCALL_PTR4(Image, put_pixel);
	VCALL_PTR0R(Image,get_used_rect);
	VCALL_PTR3R(Image,brushed);
	VCALL_PTR1R(Image,load);
	VCALL_PTR1R(Image,save_png);
	VCALL_PTR3(Image,brush_transfer);
	VCALL_PTR1R(Image,get_rect);
	VCALL_PTR1R(Image,compressed);
	VCALL_PTR0R(Image,decompressed);
	VCALL_PTR3R(Image, resized);
	VCALL_PTR0R(Image, get_data);
	VCALL_PTR3(Image, blit_rect);
	VCALL_PTR1R(Image, converted);

	VCALL_PTR0R( AABB, get_area );
	VCALL_PTR0R( AABB, has_no_area );
	VCALL_PTR0R( AABB, has_no_surface );
	VCALL_PTR1R( AABB, intersects );
	VCALL_PTR1R( AABB, encloses );
	VCALL_PTR1R( AABB, merge );
	VCALL_PTR1R( AABB, intersection );
	VCALL_PTR1R( AABB, intersects_plane );
	VCALL_PTR2R( AABB, intersects_segment );
	VCALL_PTR1R( AABB, has_point );
	VCALL_PTR1R( AABB, get_support );
	VCALL_PTR0R( AABB, get_longest_axis );
	VCALL_PTR0R( AABB, get_longest_axis_index );
	VCALL_PTR0R( AABB, get_longest_axis_size );
	VCALL_PTR0R( AABB, get_shortest_axis );
	VCALL_PTR0R( AABB, get_shortest_axis_index );
	VCALL_PTR0R( AABB, get_shortest_axis_size );
	VCALL_PTR1R( AABB, expand );
	VCALL_PTR1R( AABB, grow );
	VCALL_PTR1R( AABB, get_endpoint );

	VCALL_PTR0R( Transform2D, inverse );
	VCALL_PTR0R( Transform2D, affine_inverse );
	VCALL_PTR0R( Transform2D, get_rotation );
	VCALL_PTR0R( Transform2D, get_origin );
	VCALL_PTR0R( Transform2D, get_scale );
	VCALL_PTR0R( Transform2D, orthonormalized );
	VCALL_PTR1R( Transform2D, rotated );
	VCALL_PTR1R( Transform2D, scaled );
	VCALL_PTR1R( Transform2D, translated );
	VCALL_PTR2R( Transform2D, interpolate_with );

	static void _call_Transform2D_xform(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		switch(p_args[0]->type) {

			case Variant::VECTOR2: r_ret=reinterpret_cast<Transform2D*>(p_self._data._ptr)->xform( p_args[0]->operator Vector2()); return;
			case Variant::RECT2: r_ret=reinterpret_cast<Transform2D*>(p_self._data._ptr)->xform( p_args[0]->operator Rect2()); return;
			default: r_ret=Variant();
		}

	}

	static void _call_Transform2D_xform_inv(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		switch(p_args[0]->type) {

			case Variant::VECTOR2: r_ret=reinterpret_cast<Transform2D*>(p_self._data._ptr)->xform_inv( p_args[0]->operator Vector2()); return;
			case Variant::RECT2: r_ret=reinterpret_cast<Transform2D*>(p_self._data._ptr)->xform_inv( p_args[0]->operator Rect2()); return;
			default: r_ret=Variant();
		}
	}

	static void _call_Transform2D_basis_xform(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		switch(p_args[0]->type) {

			case Variant::VECTOR2: r_ret=reinterpret_cast<Transform2D*>(p_self._data._ptr)->basis_xform( p_args[0]->operator Vector2()); return;
			default: r_ret=Variant();
		}

	}

	static void _call_Transform2D_basis_xform_inv(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		switch(p_args[0]->type) {

			case Variant::VECTOR2: r_ret=reinterpret_cast<Transform2D*>(p_self._data._ptr)->basis_xform_inv( p_args[0]->operator Vector2()); return;
			default: r_ret=Variant();
		}
	}


	VCALL_PTR0R( Basis, inverse );
	VCALL_PTR0R( Basis, transposed );
	VCALL_PTR0R( Basis, determinant );
	VCALL_PTR2R( Basis, rotated );
	VCALL_PTR1R( Basis, scaled );
	VCALL_PTR0R( Basis, get_scale );
	VCALL_PTR0R( Basis, get_euler );
	VCALL_PTR1R( Basis, tdotx );
	VCALL_PTR1R( Basis, tdoty );
	VCALL_PTR1R( Basis, tdotz );
	VCALL_PTR1R( Basis, xform );
	VCALL_PTR1R( Basis, xform_inv );
	VCALL_PTR0R( Basis, get_orthogonal_index );
	VCALL_PTR0R( Basis, orthonormalized );


	VCALL_PTR0R( Transform3D, inverse );
	VCALL_PTR0R( Transform3D, affine_inverse );
	VCALL_PTR2R( Transform3D, rotated );
	VCALL_PTR1R( Transform3D, scaled );
	VCALL_PTR1R( Transform3D, translated );
	VCALL_PTR0R( Transform3D, orthonormalized );
	VCALL_PTR2R( Transform3D, looking_at );

	static void _call_Transform3D_xform(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		switch(p_args[0]->type) {

			case Variant::VECTOR3: r_ret=reinterpret_cast<Transform3D*>(p_self._data._ptr)->xform( p_args[0]->operator Vector3()); return;
			case Variant::PLANE: r_ret=reinterpret_cast<Transform3D*>(p_self._data._ptr)->xform( p_args[0]->operator Plane()); return;
			case Variant::_AABB: r_ret=reinterpret_cast<Transform3D*>(p_self._data._ptr)->xform( p_args[0]->operator AABB()); return;
			default: r_ret=Variant();
		}

	}

	static void _call_Transform3D_xform_inv(Variant& r_ret,Variant& p_self,const Variant** p_args) {

		switch(p_args[0]->type) {

			case Variant::VECTOR3: r_ret=reinterpret_cast<Transform3D*>(p_self._data._ptr)->xform_inv( p_args[0]->operator Vector3()); return;
			case Variant::PLANE: r_ret=reinterpret_cast<Transform3D*>(p_self._data._ptr)->xform_inv( p_args[0]->operator Plane()); return;
			case Variant::_AABB: r_ret=reinterpret_cast<Transform3D*>(p_self._data._ptr)->xform_inv( p_args[0]->operator AABB()); return;
			default: r_ret=Variant();
		}
	}

/*
	VCALL_PTR0( Transform3D, invert );
	VCALL_PTR0( Transform3D, affine_invert );
	VCALL_PTR2( Transform3D, rotate );
	VCALL_PTR1( Transform3D, scale );
	VCALL_PTR1( Transform3D, translate );
	VCALL_PTR0( Transform3D, orthonormalize ); */

	VCALL_PTR0R( InputEvent, is_pressed );
	VCALL_PTR1R( InputEvent, is_action );
	VCALL_PTR0R( InputEvent, is_echo );
	//VCALL_PTR2( InputEvent, set_as_action );

	struct ConstructData {


		int arg_count;
		Vector<Variant::Type> arg_types;
		Vector<String> arg_names;
		VariantConstructFunc func;

	};

	struct ConstructFunc {

		List<ConstructData> constructors;
	};

	static ConstructFunc* construct_funcs;

	static void Vector2_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=Vector2(*p_args[0],*p_args[1]);
	}

	static void Rect2_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=Rect2(*p_args[0],*p_args[1]);
	}

	static void Rect2_init2(Variant& r_ret,const Variant** p_args) {

		r_ret=Rect2(*p_args[0],*p_args[1],*p_args[2],*p_args[3]);
	}

	static void Matrix32_init3(Variant& r_ret,const Variant** p_args) {

		Transform2D m;
		m[0]=*p_args[0];
		m[1]=*p_args[1];
		m[2]=*p_args[2];
		r_ret=m;
	}

	static void Vector3_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=Vector3(*p_args[0],*p_args[1],*p_args[2]);
	}

	static void Plane_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=Plane(*p_args[0],*p_args[1],*p_args[2],*p_args[3]);
	}

	static void Plane_init2(Variant& r_ret,const Variant** p_args) {

		r_ret=Plane(*p_args[0],*p_args[1],*p_args[2]);
	}

	static void Plane_init3(Variant& r_ret,const Variant** p_args) {

		r_ret=Plane(p_args[0]->operator Vector3(),p_args[1]->operator real_t());
	}
	static void Plane_init4(Variant& r_ret,const Variant** p_args) {

		r_ret=Plane(p_args[0]->operator Vector3(),p_args[1]->operator Vector3());
	}

	static void Quat_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=Quat(*p_args[0],*p_args[1],*p_args[2],*p_args[3]);
	}

	static void Color_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=Color(*p_args[0],*p_args[1],*p_args[2],*p_args[3]);
	}

	static void Color_init2(Variant& r_ret,const Variant** p_args) {

		r_ret=Color(*p_args[0],*p_args[1],*p_args[2]);
	}

	static void Color_init3(Variant& r_ret,const Variant** p_args) {

		r_ret=Color::html(*p_args[0]);
	}

	static void Color_init4(Variant& r_ret,const Variant** p_args) {

		r_ret=Color::hex(*p_args[0]);
	}

	static void AABB_init1(Variant& r_ret,const Variant** p_args) {

		r_ret=AABB(*p_args[0],*p_args[1]);
	}

	static void Matrix3_init1(Variant& r_ret,const Variant** p_args) {

		Basis m;
		m.set_axis(0,*p_args[0]);
		m.set_axis(1,*p_args[1]);
		m.set_axis(2,*p_args[2]);
		r_ret=m;
	}

	static void Matrix3_init2(Variant& r_ret,const Variant** p_args) {

		r_ret=Basis(p_args[0]->operator Vector3(),p_args[1]->operator real_t());
	}

	static void Transform_init1(Variant& r_ret,const Variant** p_args) {

		Transform3D t;
		t.basis.set_axis(0,*p_args[0]);
		t.basis.set_axis(1,*p_args[1]);
		t.basis.set_axis(2,*p_args[2]);
		t.origin=*p_args[3];
		r_ret=t;
	}

	static void Transform_init2(Variant& r_ret,const Variant** p_args) {

		r_ret=Transform3D(p_args[0]->operator Basis(),p_args[1]->operator Vector3());
	}

	static void add_constructor(VariantConstructFunc p_func,const Variant::Type p_type,
			const String& p_name1="", const Variant::Type p_type1=Variant::NIL,
			const String& p_name2="", const Variant::Type p_type2=Variant::NIL,
			const String& p_name3="", const Variant::Type p_type3=Variant::NIL,
			const String& p_name4="", const Variant::Type p_type4=Variant::NIL ) {

		ConstructData cd;
		cd.func=p_func;
		cd.arg_count=0;

		if (p_name1=="")
			goto end;
		cd.arg_count++;
		cd.arg_names.push_back(p_name1);
		cd.arg_types.push_back(p_type1);

		if (p_name2=="")
			goto end;
		cd.arg_count++;
		cd.arg_names.push_back(p_name2);
		cd.arg_types.push_back(p_type2);

		if (p_name3=="")
			goto end;
		cd.arg_count++;
		cd.arg_names.push_back(p_name3);
		cd.arg_types.push_back(p_type3);

		if (p_name4=="")
			goto end;
		cd.arg_count++;
		cd.arg_names.push_back(p_name4);
		cd.arg_types.push_back(p_type4);

		end:

		construct_funcs[p_type].constructors.push_back(cd);
	}


	struct ConstantData {

		Map<StringName,int> value;
	};

	static ConstantData* constant_data;

};

_VariantCall::TypeFunc* _VariantCall::type_funcs=NULL;
_VariantCall::ConstructFunc* _VariantCall::construct_funcs=NULL;
_VariantCall::ConstantData* _VariantCall::constant_data=NULL;


Variant Variant::call(const StringName& p_method,const Variant** p_args,int p_argcount,CallError &r_error) {

	Variant ret;

	if (type==Variant::OBJECT) {
		//call object
		Object *obj = _get_obj().obj;
		if (!obj) {
			r_error.error=CallError::CALL_ERROR_INSTANCE_IS_NULL;
			return ret;
		}
#ifdef DEBUG_ENABLED
		if (ScriptDebugger::get_singleton() && _get_obj().ref.is_null()) {
			//only if debugging!
			if (!ObjectDB::instance_validate(obj)) {
				r_error.error=CallError::CALL_ERROR_INSTANCE_IS_NULL;
				return ret;
			}
		}


#endif
		return _get_obj().obj->call(p_method,p_args,p_argcount,r_error);

	//else if (type==Variant::METHOD) {

	} else {

		r_error.error=Variant::CallError::CALL_OK;

		Map<StringName,_VariantCall::FuncData>::Element *E=_VariantCall::type_funcs[type].functions.find(p_method);
#ifdef DEBUG_ENABLED
		if (!E) {
			r_error.error=Variant::CallError::CALL_ERROR_INVALID_METHOD;
			return Variant();
		}
#endif
		_VariantCall::FuncData& funcdata = E->get();
		funcdata.call(ret,*this,p_args,p_argcount,r_error);
	}

	return ret;
}

#define VCALL(m_type,m_method) _VariantCall::_call_##m_type##_##m_method


Variant Variant::construct(const Variant::Type p_type,const Variant** p_args,int p_argcount,CallError &r_error) {

	r_error.error=Variant::CallError::CALL_ERROR_INVALID_METHOD;
	ERR_FAIL_INDEX_V(p_type,VARIANT_MAX,Variant());

	r_error.error=Variant::CallError::CALL_OK;
	if (p_argcount==0) { //generic construct

		switch(p_type) {
			case NIL: return Variant();

			// atomic types
			case BOOL: return Variant( false );
			case INT: return 0;
			case REAL: return 0.0f;
			case STRING: return String();

			// math types

			case VECTOR2: return Vector2();		// 5
			case RECT2: return Rect2();
			case VECTOR3: return Vector3();
			case MATRIX32: return Transform2D();
			case PLANE: return Plane();
			case QUAT: return Quat();
			case _AABB: return AABB(); //sorry naming convention fail :( not like it's used often // 10
			case MATRIX3: return Basis();
			case TRANSFORM: return Transform3D();

			// misc types
			case COLOR: return Color();
			case IMAGE: return Image();;
			case NODE_PATH: return NodePath();;		// 15
			case _RID: return RID();;
			case OBJECT: return (Object*)NULL;
			case INPUT_EVENT: return InputEvent();;
			case DICTIONARY: return Dictionary();;
			case ARRAY: return Array();;			// 20
			case RAW_ARRAY: return ByteArray();;
			case INT_ARRAY: return PackedIntArray();;
			case REAL_ARRAY: return PackedFloatArray();;
			case STRING_ARRAY: return PackedStringArray();;
			case VECTOR2_ARRAY: return PackedVector2Array();; 	// 25
			case VECTOR3_ARRAY: return PackedVector3Array();; 	// 25
			case COLOR_ARRAY: return PackedColorArray();;
			default: return Variant();
		}

	} else if (p_argcount>1) {

		_VariantCall::ConstructFunc & c = _VariantCall::construct_funcs[p_type];

		for(List<_VariantCall::ConstructData>::Element *E=c.constructors.front();E;E=E->next()) {
			const _VariantCall::ConstructData &cd = E->get();

			if (cd.arg_count!=p_argcount)
				continue;

			//validate parameters
			for(int i=0;i<cd.arg_count;i++) {
				if (!Variant::can_convert(p_args[i]->type,cd.arg_types[i])) {
					r_error.error=Variant::CallError::CALL_ERROR_INVALID_ARGUMENT; //no such constructor
					r_error.argument=i;
					r_error.expected=cd.arg_types[i];
					return Variant();
				}
			}

			Variant v;
			cd.func(v,p_args);
			return v;
		}


	} else if (p_argcount==1 && p_args[0]->type==p_type) {
		return *p_args[0]; //copy construct
	} else if (p_argcount==1 && Variant::can_convert(p_args[0]->type,p_type)) {
		//near match construct

		switch(p_type) {
			case NIL: {

				return Variant();
			} break;
			case BOOL: { return Variant(bool(*p_args[0])); }
			case INT: { return (int(*p_args[0])); }
			case REAL: { return real_t(*p_args[0]); }
			case STRING: { return String(*p_args[0]); }
			case VECTOR2: { return Vector2(*p_args[0]); }
			case RECT2: return (Rect2(*p_args[0]));
			case VECTOR3: return (Vector3(*p_args[0]));
			case PLANE: return (Plane(*p_args[0]));
			case QUAT: return (Quat(*p_args[0]));
			case _AABB: return (AABB(*p_args[0])); //sorry naming convention fail :( not like it's used often // 10
			case MATRIX3: return (Basis(p_args[0]->operator Basis()));
			case TRANSFORM: return (Transform3D(p_args[0]->operator Transform3D()));

			// misc types
			case COLOR: return p_args[0]->type == Variant::STRING ? Color::html(*p_args[0]) : Color::hex(*p_args[0]);
			case IMAGE: return (Image(*p_args[0]));
			case NODE_PATH: return (NodePath(p_args[0]->operator NodePath()));		// 15
			case _RID: return (RID(*p_args[0]));
			case OBJECT: return ((Object*)(p_args[0]->operator Object *()));
			case INPUT_EVENT: return (InputEvent(*p_args[0]));
			case DICTIONARY: return p_args[0]->operator Dictionary();
			case ARRAY: return p_args[0]->operator Array();

			// arrays
			case RAW_ARRAY: return (ByteArray(*p_args[0]));
			case INT_ARRAY: return (PackedIntArray(*p_args[0]));
			case REAL_ARRAY: return (PackedFloatArray(*p_args[0]));
			case STRING_ARRAY: return (PackedStringArray(*p_args[0]));
			case VECTOR2_ARRAY: return (PackedVector2Array(*p_args[0])); 	// 25
			case VECTOR3_ARRAY: return (PackedVector3Array(*p_args[0])); 	// 25
			case COLOR_ARRAY: return (Color(*p_args[0]));
			default: return Variant();
		}
	}
	r_error.error=Variant::CallError::CALL_ERROR_INVALID_METHOD; //no such constructor
	return Variant();
}


bool Variant::has_method(const StringName& p_method) const {


	if (type==OBJECT) {
		Object *obj = operator Object*();
		if (!obj)
			return false;
#ifdef DEBUG_ENABLED
		if (ScriptDebugger::get_singleton()) {
			if (ObjectDB::instance_validate(obj)) {
#endif
				return obj->has_method(p_method);
#ifdef DEBUG_ENABLED

			}
		}
#endif
	}


	const _VariantCall::TypeFunc &fd = _VariantCall::type_funcs[type];
	return fd.functions.has(p_method);

}

void Variant::get_method_list(List<MethodInfo> *p_list) const {


	const _VariantCall::TypeFunc &fd = _VariantCall::type_funcs[type];

	for (const Map<StringName,_VariantCall::FuncData>::Element *E=fd.functions.front();E;E=E->next()) {

		const _VariantCall::FuncData &fd = E->get();

		MethodInfo mi;
		mi.name=E->key();

		for(int i=0;i<fd.arg_types.size();i++) {

			PropertyInfo pi;
			pi.type=fd.arg_types[i];
#ifdef DEBUG_ENABLED
			pi.name=fd.arg_names[i];
#endif
			mi.arguments.push_back(pi);
		}

		mi.default_arguments=fd.default_args;
		PropertyInfo ret;
#ifdef DEBUG_ENABLED
		ret.type=fd.return_type;
		if (fd.returns)
			ret.name="ret";
		mi.return_val=ret;
#endif		

		p_list->push_back(mi);
	}

}

void Variant::get_constructor_list(Variant::Type p_type, List<MethodInfo> *p_list) {

	ERR_FAIL_INDEX(p_type,VARIANT_MAX);

	//custom constructors
	for(const List<_VariantCall::ConstructData>::Element *E=_VariantCall::construct_funcs[p_type].constructors.front();E;E=E->next()) {

		const _VariantCall::ConstructData &cd = E->get();
		MethodInfo mi;
		mi.name=Variant::get_type_name(p_type);
		for(int i=0;i<cd.arg_count;i++) {

			PropertyInfo pi;
			pi.name=cd.arg_names[i];
			pi.type=cd.arg_types[i];
			mi.arguments.push_back(pi);
		}
		p_list->push_back(mi);
	}
	//default constructors
	for(int i=0;i<VARIANT_MAX;i++) {
		if (i==p_type)
			continue;
		if (!Variant::can_convert(Variant::Type(i),p_type))
			continue;

		MethodInfo mi;
		mi.name=Variant::get_type_name(p_type);
		PropertyInfo pi;
		pi.name="from";
		pi.type=Variant::Type(i);
		mi.arguments.push_back(pi);
		p_list->push_back(mi);
	}
}


void Variant::get_numeric_constants_for_type(Variant::Type p_type, List<StringName> *p_constants) {

	ERR_FAIL_INDEX(p_type,Variant::VARIANT_MAX);

	_VariantCall::ConstantData& cd = _VariantCall::constant_data[p_type];

	for(Map<StringName,int>::Element *E=cd.value.front();E;E=E->next()) {

		p_constants->push_back(E->key());
	}
}


bool Variant::has_numeric_constant(Variant::Type p_type, const StringName& p_value)  {

	ERR_FAIL_INDEX_V(p_type,Variant::VARIANT_MAX,false);
	_VariantCall::ConstantData& cd = _VariantCall::constant_data[p_type];
	return cd.value.has(p_value);
}

int Variant::get_numeric_constant_value(Variant::Type p_type, const StringName& p_value) {

	ERR_FAIL_INDEX_V(p_type,Variant::VARIANT_MAX,0);
	_VariantCall::ConstantData& cd = _VariantCall::constant_data[p_type];


	Map<StringName,int>::Element *E = cd.value.find(p_value);
	ERR_FAIL_COND_V(!E,0);
	return E->get();
}



void register_variant_methods() {

	_VariantCall::type_funcs = memnew_arr(_VariantCall::TypeFunc,Variant::VARIANT_MAX );

	_VariantCall::construct_funcs = memnew_arr(_VariantCall::ConstructFunc,Variant::VARIANT_MAX );
	_VariantCall::constant_data = memnew_arr(_VariantCall::ConstantData, Variant::VARIANT_MAX);

#define ADDFUNC0(m_vtype,m_ret,m_class,m_method,m_defarg)\
_VariantCall::addfunc(Variant::m_vtype,Variant::m_ret,_SCS(#m_method),VCALL(m_class,m_method),m_defarg);
#define ADDFUNC1(m_vtype,m_ret,m_class,m_method,m_arg1,m_argname1,m_defarg)\
_VariantCall::addfunc(Variant::m_vtype,Variant::m_ret,_SCS(#m_method),VCALL(m_class,m_method),m_defarg,_VariantCall::Arg(Variant::m_arg1,_SCS(m_argname1)) );
#define ADDFUNC2(m_vtype,m_ret,m_class,m_method,m_arg1,m_argname1,m_arg2,m_argname2,m_defarg)\
_VariantCall::addfunc(Variant::m_vtype,Variant::m_ret,_SCS(#m_method),VCALL(m_class,m_method),m_defarg,_VariantCall::Arg(Variant::m_arg1,_SCS(m_argname1)),_VariantCall::Arg(Variant::m_arg2,_SCS(m_argname2)));
#define ADDFUNC3(m_vtype,m_ret,m_class,m_method,m_arg1,m_argname1,m_arg2,m_argname2,m_arg3,m_argname3,m_defarg)\
_VariantCall::addfunc(Variant::m_vtype,Variant::m_ret,_SCS(#m_method),VCALL(m_class,m_method),m_defarg,_VariantCall::Arg(Variant::m_arg1,_SCS(m_argname1)),_VariantCall::Arg(Variant::m_arg2,_SCS(m_argname2)),_VariantCall::Arg(Variant::m_arg3,_SCS(m_argname3)));
#define ADDFUNC4(m_vtype,m_ret,m_class,m_method,m_arg1,m_argname1,m_arg2,m_argname2,m_arg3,m_argname3,m_arg4,m_argname4,m_defarg)\
_VariantCall::addfunc(Variant::m_vtype,Variant::m_ret,_SCS(#m_method),VCALL(m_class,m_method),m_defarg,_VariantCall::Arg(Variant::m_arg1,_SCS(m_argname1)),_VariantCall::Arg(Variant::m_arg2,_SCS(m_argname2)),_VariantCall::Arg(Variant::m_arg3,_SCS(m_argname3)),_VariantCall::Arg(Variant::m_arg4,_SCS(m_argname4)));


	/* STRING */
	ADDFUNC1(STRING,INT,String,casecmp_to,STRING,"to",varray());
	ADDFUNC1(STRING,INT,String,nocasecmp_to,STRING,"to",varray());
	ADDFUNC0(STRING,INT,String,length,varray());
	ADDFUNC2(STRING,STRING,String,substr,INT,"from",INT,"len",varray());

	ADDFUNC2(STRING,INT,String,find,STRING,"what",INT,"from",varray(0));

	ADDFUNC1(STRING,INT,String,find_last,STRING,"what",varray());
	ADDFUNC2(STRING,INT,String,findn,STRING,"what",INT,"from",varray(0));
	ADDFUNC2(STRING,INT,String,rfind,STRING,"what",INT,"from",varray(-1));
	ADDFUNC2(STRING,INT,String,rfindn,STRING,"what",INT,"from",varray(-1));
	ADDFUNC1(STRING,BOOL,String,match,STRING,"expr",varray());
	ADDFUNC1(STRING,BOOL,String,matchn,STRING,"expr",varray());
	ADDFUNC1(STRING,BOOL,String,begins_with,STRING,"text",varray());

	ADDFUNC2(STRING,STRING,String,replace,STRING,"what",STRING,"forwhat",varray());
	ADDFUNC2(STRING,STRING,String,replacen,STRING,"what",STRING,"forwhat",varray());
	ADDFUNC2(STRING,STRING,String,insert,INT,"pos",STRING,"what",varray());
	ADDFUNC0(STRING,STRING,String,capitalize,varray());
	ADDFUNC2(STRING,STRING_ARRAY,String,split,STRING,"divisor",BOOL,"allow_empty",varray(true));
	ADDFUNC2(STRING,REAL_ARRAY,String,split_floats,STRING,"divisor",BOOL,"allow_empty",varray(true));
	ADDFUNC0(STRING,STRING,String,to_upper,varray());

	ADDFUNC0(STRING,STRING,String,to_lower,varray());
	ADDFUNC1(STRING,STRING,String,left,INT,"pos",varray());
	ADDFUNC1(STRING,STRING,String,right,INT,"pos",varray());
	ADDFUNC0(STRING,STRING,String,strip_edges,varray());
	ADDFUNC0(STRING,STRING,String,extension,varray());
	ADDFUNC0(STRING,STRING,String,basename,varray());
	ADDFUNC1(STRING,STRING,String,plus_file,STRING,"file",varray());
	ADDFUNC1(STRING,STRING,String,ord_at,INT,"at",varray());
//	ADDFUNC2(STRING,String,erase,INT,INT,varray());
	ADDFUNC0(STRING,INT,String,hash,varray());
	ADDFUNC0(STRING,STRING,String,md5_text,varray());
	ADDFUNC0(STRING,RAW_ARRAY,String,md5_buffer,varray());
	ADDFUNC0(STRING,BOOL,String,empty,varray());
	ADDFUNC0(STRING,BOOL,String,is_abs_path,varray());
	ADDFUNC0(STRING,BOOL,String,is_rel_path,varray());
	ADDFUNC0(STRING,STRING,String,get_base_dir,varray());
	ADDFUNC0(STRING,STRING,String,get_file,varray());
	ADDFUNC0(STRING,STRING,String,xml_escape,varray());
	ADDFUNC0(STRING,STRING,String,xml_unescape,varray());
	ADDFUNC0(STRING,STRING,String,percent_encode,varray());
	ADDFUNC0(STRING,STRING,String,percent_decode,varray());
	ADDFUNC0(STRING,BOOL,String,is_valid_identifier,varray());
	ADDFUNC0(STRING,BOOL,String,is_valid_integer,varray());
	ADDFUNC0(STRING,BOOL,String,is_valid_float,varray());
	ADDFUNC0(STRING,BOOL,String,is_valid_html_color,varray());
	ADDFUNC0(STRING,BOOL,String,is_valid_ip_address,varray());
	ADDFUNC0(STRING,INT,String,to_int,varray());
	ADDFUNC0(STRING,REAL,String,to_float,varray());
	ADDFUNC0(STRING,INT,String,hex_to_int,varray());
	ADDFUNC1(STRING,STRING,String,pad_decimals,INT,"digits",varray());
	ADDFUNC1(STRING,STRING,String,pad_zeros,INT,"digits",varray());

	ADDFUNC0(VECTOR2,VECTOR2,Vector2,normalized,varray());
	ADDFUNC0(VECTOR2,REAL,Vector2,length,varray());
	ADDFUNC0(VECTOR2,REAL,Vector2,atan2,varray());
	ADDFUNC0(VECTOR2,REAL,Vector2,length_squared,varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,distance_to,VECTOR2,"to",varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,distance_squared_to,VECTOR2,"to",varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,angle_to,VECTOR2,"to",varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,angle_to_point,VECTOR2,"to",varray());
	ADDFUNC2(VECTOR2,VECTOR2,Vector2,linear_interpolate,VECTOR2,"b",REAL,"t",varray());
	ADDFUNC4(VECTOR2,VECTOR2,Vector2,cubic_interpolate,VECTOR2,"b",VECTOR2,"pre_a",VECTOR2,"post_b",REAL,"t",varray());
	ADDFUNC1(VECTOR2,VECTOR2,Vector2,rotated,REAL,"phi",varray());
	ADDFUNC0(VECTOR2,VECTOR2,Vector2,tangent,varray());
	ADDFUNC0(VECTOR2,VECTOR2,Vector2,floor,varray());
	ADDFUNC0(VECTOR2,VECTOR2,Vector2,floorf,varray());
	ADDFUNC1(VECTOR2,VECTOR2,Vector2,snapped,VECTOR2,"by",varray());
	ADDFUNC0(VECTOR2,REAL,Vector2,get_aspect,varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,dot,VECTOR2,"with",varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,slide,VECTOR2,"vec",varray());
	ADDFUNC1(VECTOR2,REAL,Vector2,reflect,VECTOR2,"vec",varray());
	//ADDFUNC1(VECTOR2,REAL,Vector2,cross,VECTOR2,"with",varray());

	ADDFUNC0(RECT2,REAL,Rect2,get_area,varray());
	ADDFUNC1(RECT2,BOOL,Rect2,intersects,RECT2,"b",varray());
	ADDFUNC1(RECT2,BOOL,Rect2,encloses,RECT2,"b",varray());
	ADDFUNC0(RECT2,BOOL,Rect2,has_no_area,varray());
	ADDFUNC1(RECT2,RECT2,Rect2,clip,RECT2,"b",varray());
	ADDFUNC1(RECT2,RECT2,Rect2,merge,RECT2,"b",varray());
	ADDFUNC1(RECT2,BOOL,Rect2,has_point,VECTOR2,"point",varray());
	ADDFUNC1(RECT2,RECT2,Rect2,grow,REAL,"by",varray());
	ADDFUNC1(RECT2,RECT2,Rect2,expand,VECTOR2,"to",varray());

	ADDFUNC0(VECTOR3,INT,Vector3,min_axis,varray());
	ADDFUNC0(VECTOR3,INT,Vector3,max_axis,varray());
	ADDFUNC0(VECTOR3,REAL,Vector3,length,varray());
	ADDFUNC0(VECTOR3,REAL,Vector3,length_squared,varray());
	ADDFUNC0(VECTOR3,VECTOR3,Vector3,normalized,varray());
	ADDFUNC0(VECTOR3,VECTOR3,Vector3,inverse,varray());
	ADDFUNC1(VECTOR3,VECTOR3,Vector3,snapped,REAL,"by",varray());
	ADDFUNC2(VECTOR3,VECTOR3,Vector3,rotated,VECTOR3,"axis",REAL,"phi",varray());
	ADDFUNC2(VECTOR3,VECTOR3,Vector3,linear_interpolate,VECTOR3,"b",REAL,"t",varray());
	ADDFUNC4(VECTOR3,VECTOR3,Vector3,cubic_interpolate,VECTOR3,"b",VECTOR3,"pre_a",VECTOR3,"post_b",REAL,"t",varray());
	ADDFUNC1(VECTOR3,REAL,Vector3,dot,VECTOR3,"b",varray());
	ADDFUNC1(VECTOR3,VECTOR3,Vector3,cross,VECTOR3,"b",varray());
	ADDFUNC0(VECTOR3,VECTOR3,Vector3,abs,varray());
	ADDFUNC1(VECTOR3,REAL,Vector3,distance_to,VECTOR3,"b",varray());
	ADDFUNC1(VECTOR3,REAL,Vector3,distance_squared_to,VECTOR3,"b",varray());
	ADDFUNC1(VECTOR3,VECTOR3,Vector3,slide,VECTOR3,"by",varray());
	ADDFUNC1(VECTOR3,VECTOR3,Vector3,reflect,VECTOR3,"by",varray());

	ADDFUNC0(PLANE,PLANE,Plane,normalized,varray());
	ADDFUNC0(PLANE,VECTOR3,Plane,center,varray());
	ADDFUNC0(PLANE,VECTOR3,Plane,get_any_point,varray());
	ADDFUNC1(PLANE,BOOL,Plane,is_point_over,VECTOR3,"point",varray());
	ADDFUNC1(PLANE,REAL,Plane,distance_to,VECTOR3,"point",varray());
	ADDFUNC2(PLANE,BOOL,Plane,has_point,VECTOR3,"point",REAL,"epsilon",varray(CMP_EPSILON));
	ADDFUNC1(PLANE,VECTOR3,Plane,project,VECTOR3,"point",varray());
	ADDFUNC2(PLANE,VECTOR3,Plane,intersect_3,PLANE,"b",PLANE,"c",varray());
	ADDFUNC2(PLANE,VECTOR3,Plane,intersects_ray,VECTOR3,"from",VECTOR3,"dir",varray());
	ADDFUNC2(PLANE,VECTOR3,Plane,intersects_segment,VECTOR3,"begin",VECTOR3,"end",varray());

	ADDFUNC0(QUAT,REAL,Quat,length,varray());
	ADDFUNC0(QUAT,REAL,Quat,length_squared,varray());
	ADDFUNC0(QUAT,QUAT,Quat,normalized,varray());
	ADDFUNC0(QUAT,QUAT,Quat,inverse,varray());
	ADDFUNC1(QUAT,REAL,Quat,dot,QUAT,"b",varray());
	ADDFUNC2(QUAT,QUAT,Quat,slerp,QUAT,"b",REAL,"t",varray());
	ADDFUNC2(QUAT,QUAT,Quat,slerpni,QUAT,"b",REAL,"t",varray());
	ADDFUNC4(QUAT,QUAT,Quat,cubic_slerp,QUAT,"b",QUAT,"pre_a",QUAT,"post_b",REAL,"t",varray());

	ADDFUNC0(COLOR,INT,Color,to_32,varray());
	ADDFUNC0(COLOR,INT,Color,to_ARGB32,varray());
	ADDFUNC0(COLOR,REAL,Color,gray,varray());
	ADDFUNC0(COLOR,COLOR,Color,inverted,varray());
	ADDFUNC0(COLOR,COLOR,Color,contrasted,varray());
	ADDFUNC2(COLOR,COLOR,Color,linear_interpolate,COLOR,"b",REAL,"t",varray());
	ADDFUNC1(COLOR,COLOR,Color,blend,COLOR,"over",varray());
	ADDFUNC1(COLOR,STRING,Color,to_html,BOOL,"with_alpha",varray(true));

	ADDFUNC0(IMAGE, INT, Image, get_format, varray());
	ADDFUNC0(IMAGE, INT, Image, get_width, varray());
	ADDFUNC0(IMAGE, INT, Image, get_height, varray());
	ADDFUNC0(IMAGE, BOOL, Image, empty, varray());
	ADDFUNC3(IMAGE, COLOR, Image, get_pixel, INT, "x", INT, "y", INT, "mipmap_level", varray(0));
	ADDFUNC4(IMAGE, NIL, Image, put_pixel, INT, "x", INT, "y", COLOR, "color", INT, "mipmap_level", varray(0));
	ADDFUNC3(IMAGE, IMAGE, Image, brushed, IMAGE, "src", IMAGE, "brush", VECTOR2, "pos", varray(0));
	ADDFUNC1(IMAGE, INT, Image, load, STRING, "path", varray(0));
	ADDFUNC1(IMAGE, INT, Image, save_png, STRING, "path", varray(0));
	ADDFUNC3(IMAGE, NIL, Image, brush_transfer, IMAGE, "src", IMAGE, "brush", VECTOR2, "pos", varray(0));
	ADDFUNC0(IMAGE, RECT2, Image, get_used_rect, varray(0));
	ADDFUNC1(IMAGE, IMAGE, Image, get_rect, RECT2, "area", varray(0));
	ADDFUNC1(IMAGE, IMAGE, Image, compressed, INT, "format", varray(0));
	ADDFUNC0(IMAGE, IMAGE, Image, decompressed, varray(0));
	ADDFUNC3(IMAGE, IMAGE, Image, resized, INT, "x", INT, "y", INT, "interpolation", varray(((int)Image::INTERPOLATE_BILINEAR)));
	ADDFUNC0(IMAGE, RAW_ARRAY, Image, get_data, varray());
	ADDFUNC3(IMAGE, NIL, Image, blit_rect, IMAGE, "src", RECT2, "src_rect", VECTOR2, "dest", varray(0));
	ADDFUNC1(IMAGE, IMAGE, Image, converted, INT, "format", varray(0));

	ADDFUNC0(_RID,INT,RID,get_id,varray());

	ADDFUNC0(NODE_PATH,BOOL,NodePath,is_absolute,varray());
	ADDFUNC0(NODE_PATH,INT,NodePath,get_name_count,varray());
	ADDFUNC1(NODE_PATH,STRING,NodePath,get_name,INT,"idx",varray());
	ADDFUNC0(NODE_PATH,INT,NodePath,get_subname_count,varray());
	ADDFUNC1(NODE_PATH,STRING,NodePath,get_subname,INT,"idx",varray());
	ADDFUNC0(NODE_PATH,STRING,NodePath,get_property,varray());
	ADDFUNC0(NODE_PATH,BOOL,NodePath,is_empty,varray());

	ADDFUNC0(DICTIONARY,INT,Dictionary,size,varray());
	ADDFUNC0(DICTIONARY,BOOL,Dictionary,empty,varray());
	ADDFUNC0(DICTIONARY,NIL,Dictionary,clear,varray());
	ADDFUNC1(DICTIONARY,BOOL,Dictionary,has,NIL,"value",varray());
	ADDFUNC1(DICTIONARY,NIL,Dictionary,erase,NIL,"value",varray());
	ADDFUNC0(DICTIONARY,INT,Dictionary,hash,varray());
	ADDFUNC0(DICTIONARY,ARRAY,Dictionary,keys,varray());

	ADDFUNC1(DICTIONARY,INT,Dictionary,parse_json,STRING,"json",varray());
	ADDFUNC0(DICTIONARY,STRING,Dictionary,to_json,varray());

	ADDFUNC0(ARRAY,INT,Array,size,varray());
	ADDFUNC0(ARRAY,BOOL,Array,empty,varray());
	ADDFUNC0(ARRAY,NIL,Array,clear,varray());
	ADDFUNC0(ARRAY,INT,Array,hash,varray());
	ADDFUNC1(ARRAY,NIL,Array,push_back,NIL,"value",varray());
	ADDFUNC1(ARRAY,NIL,Array,append,NIL,"value",varray());
	ADDFUNC1(ARRAY,NIL,Array,resize,INT,"pos",varray());
	ADDFUNC2(ARRAY,NIL,Array,insert,INT,"pos",NIL,"value",varray());
	ADDFUNC1(ARRAY,NIL,Array,remove,INT,"pos",varray());
	ADDFUNC1(ARRAY,NIL,Array,erase,NIL,"value",varray());
	ADDFUNC1(ARRAY,INT,Array,find,NIL,"value",varray());
	ADDFUNC0(ARRAY,NIL,Array,sort,varray());
	ADDFUNC2(ARRAY,NIL,Array,sort_custom,OBJECT,"obj",STRING,"func",varray());
	ADDFUNC0(ARRAY,NIL,Array,invert,varray());

	ADDFUNC0(RAW_ARRAY,INT,ByteArray,size,varray());
	ADDFUNC2(RAW_ARRAY,NIL,ByteArray,set,INT,"idx",INT,"byte",varray());
	ADDFUNC1(RAW_ARRAY,INT,ByteArray,get,INT,"idx",varray());
	ADDFUNC1(RAW_ARRAY,NIL,ByteArray,push_back,INT,"byte",varray());
	ADDFUNC1(RAW_ARRAY,NIL,ByteArray,resize,INT,"idx",varray());

	ADDFUNC0(RAW_ARRAY,STRING,ByteArray,get_string_from_ascii,varray());
	ADDFUNC0(RAW_ARRAY,STRING,ByteArray,get_string_from_utf8,varray());


	ADDFUNC0(INT_ARRAY,INT,PackedIntArray,size,varray());
	ADDFUNC2(INT_ARRAY,NIL,PackedIntArray,set,INT,"idx",INT,"integer",varray());
	ADDFUNC1(INT_ARRAY,INT,PackedIntArray,get,INT,"idx",varray());
	ADDFUNC1(INT_ARRAY,NIL,PackedIntArray,push_back,INT,"integer",varray());
	ADDFUNC1(INT_ARRAY,NIL,PackedIntArray,resize,INT,"idx",varray());

	ADDFUNC0(REAL_ARRAY,INT,PackedFloatArray,size,varray());
	ADDFUNC2(REAL_ARRAY,NIL,PackedFloatArray,set,INT,"idx",REAL,"value",varray());
	ADDFUNC1(REAL_ARRAY,REAL,PackedFloatArray,get,INT,"idx",varray());
	ADDFUNC1(REAL_ARRAY,NIL,PackedFloatArray,push_back,REAL,"value",varray());
	ADDFUNC1(REAL_ARRAY,NIL,PackedFloatArray,resize,INT,"idx",varray());

	ADDFUNC0(STRING_ARRAY,INT,PackedStringArray,size,varray());
	ADDFUNC2(STRING_ARRAY,NIL,PackedStringArray,set,INT,"idx",STRING,"string",varray());
	ADDFUNC1(STRING_ARRAY,STRING,PackedStringArray,get,INT,"idx",varray());
	ADDFUNC1(STRING_ARRAY,NIL,PackedStringArray,push_back,STRING,"string",varray());
	ADDFUNC1(STRING_ARRAY,NIL,PackedStringArray,resize,INT,"idx",varray());

	ADDFUNC0(VECTOR2_ARRAY,INT,PackedVector2Array,size,varray());
	ADDFUNC2(VECTOR2_ARRAY,NIL,PackedVector2Array,set,INT,"idx",VECTOR2,"vector2",varray());
	ADDFUNC1(VECTOR2_ARRAY,VECTOR2,PackedVector2Array,get,INT,"idx",varray());
	ADDFUNC1(VECTOR2_ARRAY,NIL,PackedVector2Array,push_back,VECTOR2,"vector2",varray());
	ADDFUNC1(VECTOR2_ARRAY,NIL,PackedVector2Array,resize,INT,"idx",varray());

	ADDFUNC0(VECTOR3_ARRAY,INT,PackedVector3Array,size,varray());
	ADDFUNC2(VECTOR3_ARRAY,NIL,PackedVector3Array,set,INT,"idx",VECTOR3,"vector3",varray());
	ADDFUNC1(VECTOR3_ARRAY,VECTOR3,PackedVector3Array,get,INT,"idx",varray());
	ADDFUNC1(VECTOR3_ARRAY,NIL,PackedVector3Array,push_back,VECTOR3,"vector3",varray());
	ADDFUNC1(VECTOR3_ARRAY,NIL,PackedVector3Array,resize,INT,"idx",varray());

	ADDFUNC0(COLOR_ARRAY,INT,PackedColorArray,size,varray());
	ADDFUNC2(COLOR_ARRAY,NIL,PackedColorArray,set,INT,"idx",COLOR,"color",varray());
	ADDFUNC1(COLOR_ARRAY,COLOR,PackedColorArray,get,INT,"idx",varray());
	ADDFUNC1(COLOR_ARRAY,NIL,PackedColorArray,push_back,COLOR,"color",varray());
	ADDFUNC1(COLOR_ARRAY,NIL,PackedColorArray,resize,INT,"idx",varray());

	//pointerbased

	ADDFUNC0(_AABB,REAL,AABB,get_area,varray());
	ADDFUNC0(_AABB,BOOL,AABB,has_no_area,varray());
	ADDFUNC0(_AABB,BOOL,AABB,has_no_surface,varray());
	ADDFUNC1(_AABB,BOOL,AABB,intersects,_AABB,"with",varray());
	ADDFUNC1(_AABB,BOOL,AABB,encloses,_AABB,"with",varray());
	ADDFUNC1(_AABB,_AABB,AABB,merge,_AABB,"with",varray());
	ADDFUNC1(_AABB,_AABB,AABB,intersection,_AABB,"with",varray());
	ADDFUNC1(_AABB,BOOL,AABB,intersects_plane,PLANE,"plane",varray());
	ADDFUNC2(_AABB,BOOL,AABB,intersects_segment,VECTOR3,"from",VECTOR3,"to",varray());
	ADDFUNC1(_AABB,BOOL,AABB,has_point,VECTOR3,"point",varray());
	ADDFUNC1(_AABB,VECTOR3,AABB,get_support,VECTOR3,"dir",varray());
	ADDFUNC0(_AABB,VECTOR3,AABB,get_longest_axis,varray());
	ADDFUNC0(_AABB,INT,AABB,get_longest_axis_index,varray());
	ADDFUNC0(_AABB,REAL,AABB,get_longest_axis_size,varray());
	ADDFUNC0(_AABB,VECTOR3,AABB,get_shortest_axis,varray());
	ADDFUNC0(_AABB,INT,AABB,get_shortest_axis_index,varray());
	ADDFUNC0(_AABB,REAL,AABB,get_shortest_axis_size,varray());
	ADDFUNC1(_AABB,_AABB,AABB,expand,VECTOR3,"to_point",varray());
	ADDFUNC1(_AABB,_AABB,AABB,grow,REAL,"by",varray());
	ADDFUNC1(_AABB,VECTOR3,AABB,get_endpoint,INT,"idx",varray());

	ADDFUNC0(MATRIX32,MATRIX32,Transform2D,inverse,varray());
	ADDFUNC0(MATRIX32,MATRIX32,Transform2D,affine_inverse,varray());
	ADDFUNC0(MATRIX32,REAL,Transform2D,get_rotation,varray());
	ADDFUNC0(MATRIX32,VECTOR2,Transform2D,get_origin,varray());
	ADDFUNC0(MATRIX32,VECTOR2,Transform2D,get_scale,varray());
	ADDFUNC0(MATRIX32,MATRIX32,Transform2D,orthonormalized,varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,rotated,REAL,"phi",varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,scaled,VECTOR2,"scale",varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,translated,VECTOR2,"offset",varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,xform,NIL,"v",varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,xform_inv,NIL,"v",varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,basis_xform,NIL,"v",varray());
	ADDFUNC1(MATRIX32,MATRIX32,Transform2D,basis_xform_inv,NIL,"v",varray());
	ADDFUNC2(MATRIX32,MATRIX32,Transform2D,interpolate_with,MATRIX32,"m",REAL,"c",varray());

	ADDFUNC0(MATRIX3,MATRIX3,Basis,inverse,varray());
	ADDFUNC0(MATRIX3,MATRIX3,Basis,transposed,varray());
	ADDFUNC0(MATRIX3,MATRIX3,Basis,orthonormalized,varray());
	ADDFUNC0(MATRIX3,REAL,Basis,determinant,varray());
	ADDFUNC2(MATRIX3,MATRIX3,Basis,rotated,VECTOR3,"axis",REAL,"phi",varray());
	ADDFUNC1(MATRIX3,MATRIX3,Basis,scaled,VECTOR3,"scale",varray());
	ADDFUNC0(MATRIX3,VECTOR3,Basis,get_scale,varray());
	ADDFUNC0(MATRIX3,VECTOR3,Basis,get_euler,varray());
	ADDFUNC1(MATRIX3,REAL,Basis,tdotx,VECTOR3,"with",varray());
	ADDFUNC1(MATRIX3,REAL,Basis,tdoty,VECTOR3,"with",varray());
	ADDFUNC1(MATRIX3,REAL,Basis,tdotz,VECTOR3,"with",varray());
	ADDFUNC1(MATRIX3,VECTOR3,Basis,xform,VECTOR3,"v",varray());
	ADDFUNC1(MATRIX3,VECTOR3,Basis,xform_inv,VECTOR3,"v",varray());
	ADDFUNC0(MATRIX3,INT,Basis,get_orthogonal_index,varray());

	ADDFUNC0(TRANSFORM,TRANSFORM,Transform3D,inverse,varray());
	ADDFUNC0(TRANSFORM,TRANSFORM,Transform3D,affine_inverse,varray());
	ADDFUNC0(TRANSFORM,TRANSFORM,Transform3D,orthonormalized,varray());
	ADDFUNC2(TRANSFORM,TRANSFORM,Transform3D,rotated,VECTOR3,"axis",REAL,"phi",varray());
	ADDFUNC1(TRANSFORM,TRANSFORM,Transform3D,scaled,VECTOR3,"scale",varray());
	ADDFUNC1(TRANSFORM,TRANSFORM,Transform3D,translated,VECTOR3,"ofs",varray());
	ADDFUNC2(TRANSFORM,TRANSFORM,Transform3D,looking_at,VECTOR3,"target",VECTOR3,"up",varray());
	ADDFUNC1(TRANSFORM,NIL,Transform3D,xform,NIL,"v",varray());
	ADDFUNC1(TRANSFORM,NIL,Transform3D,xform_inv,NIL,"v",varray());

#ifdef DEBUG_ENABLED	
	_VariantCall::type_funcs[Variant::TRANSFORM].functions["xform"].returns=true;
	_VariantCall::type_funcs[Variant::TRANSFORM].functions["xform_inv"].returns=true;
#endif	

	ADDFUNC0(INPUT_EVENT,BOOL,InputEvent,is_pressed,varray());
	ADDFUNC1(INPUT_EVENT,BOOL,InputEvent,is_action,STRING,"action",varray());
	ADDFUNC0(INPUT_EVENT,BOOL,InputEvent,is_echo,varray());
	//ADDFUNC2(INPUT_EVENT,NIL,InputEvent,set_as_action,STRING,"action",BOOL,"pressed",varray());

	/* REGISTER CONSTRUCTORS */

	_VariantCall::add_constructor(_VariantCall::Vector2_init1,Variant::VECTOR2,"x",Variant::REAL,"y",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Rect2_init1,Variant::RECT2,"pos",Variant::VECTOR2,"size",Variant::VECTOR2);
	_VariantCall::add_constructor(_VariantCall::Rect2_init2,Variant::RECT2,"x",Variant::REAL,"y",Variant::REAL,"width",Variant::REAL,"height",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Matrix32_init3,Variant::MATRIX32,"x_axis",Variant::VECTOR2,"y_axis",Variant::VECTOR2,"origin",Variant::VECTOR2);

	_VariantCall::add_constructor(_VariantCall::Vector3_init1,Variant::VECTOR3,"x",Variant::REAL,"y",Variant::REAL,"z",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Plane_init1,Variant::PLANE,"a",Variant::REAL,"b",Variant::REAL,"c",Variant::REAL,"d",Variant::REAL);
	_VariantCall::add_constructor(_VariantCall::Plane_init2,Variant::PLANE,"v1",Variant::VECTOR3,"v2",Variant::VECTOR3,"v3",Variant::VECTOR3);
	_VariantCall::add_constructor(_VariantCall::Plane_init3,Variant::PLANE,"normal",Variant::VECTOR3,"d",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Quat_init1,Variant::QUAT,"x",Variant::REAL,"y",Variant::REAL,"z",Variant::REAL,"w",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Color_init1,Variant::COLOR,"r",Variant::REAL,"g",Variant::REAL,"b",Variant::REAL,"a",Variant::REAL);
	_VariantCall::add_constructor(_VariantCall::Color_init2,Variant::COLOR,"r",Variant::REAL,"g",Variant::REAL,"b",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::AABB_init1,Variant::_AABB,"pos",Variant::VECTOR3,"size",Variant::VECTOR3);

	_VariantCall::add_constructor(_VariantCall::Matrix3_init1,Variant::MATRIX3,"x_axis",Variant::VECTOR3,"y_axis",Variant::VECTOR3,"z_axis",Variant::VECTOR3);
	_VariantCall::add_constructor(_VariantCall::Matrix3_init2,Variant::MATRIX3,"axis",Variant::VECTOR3,"phi",Variant::REAL);

	_VariantCall::add_constructor(_VariantCall::Transform_init1,Variant::TRANSFORM,"x_axis",Variant::VECTOR3,"y_axis",Variant::VECTOR3,"z_axis",Variant::VECTOR3,"origin",Variant::VECTOR3);
	_VariantCall::add_constructor(_VariantCall::Transform_init2,Variant::TRANSFORM,"basis",Variant::MATRIX3,"origin",Variant::VECTOR3);

	/* REGISTER CONSTANTS */

	_VariantCall::constant_data[Variant::VECTOR3].value["AXIS_X"]=Vector3::AXIS_X;
	_VariantCall::constant_data[Variant::VECTOR3].value["AXIS_Y"]=Vector3::AXIS_Y;
	_VariantCall::constant_data[Variant::VECTOR3].value["AXIS_Z"]=Vector3::AXIS_Z;

	_VariantCall::constant_data[Variant::INPUT_EVENT].value["NONE"]=InputEvent::NONE;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["KEY"]=InputEvent::KEY;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["MOUSE_MOTION"]=InputEvent::MOUSE_MOTION;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["MOUSE_BUTTON"]=InputEvent::MOUSE_BUTTON;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["JOYSTICK_MOTION"]=InputEvent::JOYSTICK_MOTION;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["JOYSTICK_BUTTON"]=InputEvent::JOYSTICK_BUTTON;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["SCREEN_TOUCH"]=InputEvent::SCREEN_TOUCH;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["SCREEN_DRAG"]=InputEvent::SCREEN_DRAG;
	_VariantCall::constant_data[Variant::INPUT_EVENT].value["ACTION"]=InputEvent::ACTION;

	_VariantCall::constant_data[Variant::IMAGE].value["COMPRESS_BC"]=Image::COMPRESS_BC;
	_VariantCall::constant_data[Variant::IMAGE].value["COMPRESS_PVRTC2"]=Image::COMPRESS_PVRTC2;
	_VariantCall::constant_data[Variant::IMAGE].value["COMPRESS_PVRTC4"]=Image::COMPRESS_PVRTC4;
	_VariantCall::constant_data[Variant::IMAGE].value["COMPRESS_ETC"]=Image::COMPRESS_ETC;



	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_GRAYSCALE"]=Image::FORMAT_GRAYSCALE;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_INTENSITY"]=Image::FORMAT_INTENSITY;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_GRAYSCALE_ALPHA"]=Image::FORMAT_GRAYSCALE_ALPHA;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_RGB"]=Image::FORMAT_RGB;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_RGBA"]=Image::FORMAT_RGBA;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_INDEXED"]=Image::FORMAT_INDEXED;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_INDEXED_ALPHA"]=Image::FORMAT_INDEXED_ALPHA;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_YUV_422"]=Image::FORMAT_YUV_422;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_YUV_444"]=Image::FORMAT_YUV_444;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_BC1"]=Image::FORMAT_BC1;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_BC2"]=Image::FORMAT_BC2;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_BC3"]=Image::FORMAT_BC3;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_BC4"]=Image::FORMAT_BC4;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_BC5"]=Image::FORMAT_BC5;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_PVRTC2"]=Image::FORMAT_PVRTC2;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_PVRTC2_ALPHA"]=Image::FORMAT_PVRTC2_ALPHA;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_PVRTC4"]=Image::FORMAT_PVRTC4;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_PVRTC4_ALPHA"]=Image::FORMAT_PVRTC4_ALPHA;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_ETC"]=Image::FORMAT_ETC;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_ATC"]=Image::FORMAT_ATC;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_ATC_ALPHA_EXPLICIT"]=Image::FORMAT_ATC_ALPHA_EXPLICIT;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_ATC_ALPHA_INTERPOLATED"]=Image::FORMAT_ATC_ALPHA_INTERPOLATED;
	_VariantCall::constant_data[Variant::IMAGE].value["FORMAT_CUSTOM"]=Image::FORMAT_CUSTOM;

}

void unregister_variant_methods() {


	memdelete_arr(_VariantCall::type_funcs);
	memdelete_arr(_VariantCall::construct_funcs);
	memdelete_arr( _VariantCall::constant_data );


}






