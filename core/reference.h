/*************************************************************************/
/*  reference.h                                                          */
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
#ifndef REFERENCE_H
#define REFERENCE_H

#include "object.h"
#include "safe_refcount.h"
#include "ref_ptr.h"
#include "object_type_db.h"

/**
	@author Juan Linietsky <reduzio@gmail.com>
*/
class RefCounted : public Object{

	OBJ_TYPE( RefCounted, Object );
friend class RefBase;
	SafeRefCount refcount;
	SafeRefCount refcount_init;
protected:

	static void _bind_methods();
public:


	_FORCE_INLINE_ bool is_referenced() const { return refcount_init.get()<1; }
	bool init_ref();
	void reference();
	bool unreference();

	RefCounted();
	~RefCounted();
};

#if 0
class RefBase {
protected:
	void ref_inc(RefCounted *p_reference);
	bool ref_dec(RefCounted *p_reference);
	RefCounted *first_ref(RefCounted *p_reference);
	RefCounted * get_reference_from_ref(const RefBase &p_base);
	virtual RefCounted * get_reference() const=0;
	char * get_refptr_data(const RefPtr &p_refptr) const;
public:

	virtual ~RefBase() {}
};
#endif

template<class T>
class Ref {

	T *reference;

	void ref( const Ref& p_from ) {

		if (p_from.reference==reference)
			return;

		unref();

		reference=p_from.reference;
		if (reference)
			reference->reference();
	}

	void ref_pointer( T* p_ref ) {

		ERR_FAIL_COND(!p_ref);

		if (p_ref->init_ref())
			reference=p_ref;
	}

	//virtual RefCounted * get_reference() const { return reference; }
public:


	_FORCE_INLINE_ bool operator<(const Ref<T>& p_r) const {

		return reference<p_r.reference;
	}
	_FORCE_INLINE_ bool operator==(const Ref<T>& p_r) const {

		return reference==p_r.reference;
	}
	_FORCE_INLINE_ bool operator!=(const Ref<T>& p_r) const {

		return reference!=p_r.reference;
	}

	_FORCE_INLINE_ T* operator->() {

		return reference;
	}

	_FORCE_INLINE_ T* operator*() {

		return reference;
	}

	_FORCE_INLINE_ const T* operator->() const {

		return reference;
	}

	_FORCE_INLINE_ const T* ptr() const {

		return reference;
	}
	_FORCE_INLINE_ T* ptr() {

		return reference;
	}

	_FORCE_INLINE_ const T* operator*() const  {

		return reference;
	}

	RefPtr get_ref_ptr() const {

		RefPtr refptr;
		Ref<RefCounted> * irr = reinterpret_cast<Ref<RefCounted>*>( refptr.get_data() );
		*irr = *this;
		return refptr;
	};

#if 0
	// go to RefPtr
	operator RefPtr() const {

		return get_ref_ptr();
	}
#endif

#if 1
	operator Variant() const {

		return Variant( get_ref_ptr() );
	}
#endif

	void operator=( const Ref& p_from ) {

		ref(p_from);
	}

	template<class T_Other>
	void operator=( const Ref<T_Other>& p_from ) {

		RefCounted *refb = const_cast<RefCounted*>(static_cast<const RefCounted*>(p_from.ptr()));
		if (!refb) {
			unref();
			return;
		}
		Ref r;
		r.reference=refb->cast_to<T>();
		ref(r);
		r.reference=NULL;
	}

	void operator=( const RefPtr& p_refptr ) {

		Ref<RefCounted> * irr = reinterpret_cast<Ref<RefCounted>*>( p_refptr.get_data() );
		RefCounted *refb = irr->ptr();
		if (!refb) {
			unref();
			return;
		}
		Ref r;
		r.reference=refb->cast_to<T>();
		ref(r);
		r.reference=NULL;
	}


	void operator=( const Variant& p_variant ) {

		RefPtr refptr=p_variant;
		Ref<RefCounted> * irr = reinterpret_cast<Ref<RefCounted>*>( refptr.get_data() );
		RefCounted *refb = irr->ptr();
		if (!refb) {
			unref();
			return;
		}
		Ref r;
		r.reference=refb->cast_to<T>();
		ref(r);
		r.reference=NULL;
	}

	Ref( const Ref& p_from ) {

		reference=NULL;
		ref(p_from);
	}

	template<class T_Other>
	Ref( const Ref<T_Other>& p_from ) {

		reference=NULL;
		RefCounted *refb = const_cast<RefCounted*>(static_cast<const RefCounted*>(p_from.ptr()));
		if (!refb) {
			unref();
			return;
		}
		Ref r;
		r.reference=refb->cast_to<T>();
		ref(r);
		r.reference=NULL;
	}

	Ref( T* p_reference ) {

		if (p_reference)
			ref_pointer(p_reference);
		else
			reference=NULL;
	}

	Ref( const Variant& p_variant) {

		RefPtr refptr=p_variant;
		Ref<RefCounted> * irr = reinterpret_cast<Ref<RefCounted>*>( refptr.get_data() );
		reference=NULL;
		RefCounted *refb = irr->ptr();
		if (!refb) {
			unref();
			return;
		}
		Ref r;
		r.reference=refb->cast_to<T>();
		ref(r);
		r.reference=NULL;
	}



	Ref( const RefPtr& p_refptr) {

		Ref<RefCounted> * irr = reinterpret_cast<Ref<RefCounted>*>( p_refptr.get_data() );
		reference=NULL;
		RefCounted *refb = irr->ptr();
		if (!refb) {
			unref();
			return;
		}
		Ref r;
		r.reference=refb->cast_to<T>();
		ref(r);
		r.reference=NULL;
	}

	inline bool is_valid() const { return reference!=NULL; }
	inline bool is_null() const { return reference==NULL; }

	void unref() {
		//TODO this should be moved to mutexes, since this engine does not really
		// do a lot of referencing on references and stuff
		// mutexes will avoid more crashes?

		if (reference && reference->unreference()) {

			memdelete(reference);
		}
		reference=NULL;
	}


	Ref() {

		reference=NULL;
	}

	~Ref() {

		unref();
	}

};

typedef Ref<RefCounted> REF;


class WeakRef : public RefCounted {

	OBJ_TYPE(WeakRef,RefCounted);

	ObjectID ref;
protected:
	static void _bind_methods();

public:
	Variant get_ref() const;
	void set_obj(Object *p_object);
	void set_ref(const REF& p_ref);

	WeakRef();
};



#endif // REFERENCE_H
