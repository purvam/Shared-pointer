/*
 * SharedPtr.hpp
 *
 *  Created on: May 8, 2016
 *      Author: Smurfette
 */

#ifndef SHAREDPTR_HPP_
#define SHAREDPTR_HPP_

#include<iostream>
#include<pthread.h>
#include <typeinfo>

namespace cs540{

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;

class RCount{
public:
	RCount(int i=0):ref_count(i){};
	void addRef(){
   pthread_mutex_lock(&mut);
		++this->ref_count;
   pthread_mutex_unlock(&mut);
	}
	int removeRef(){
    int a;
    pthread_mutex_lock(&mut);
    a=--this->ref_count;
    pthread_mutex_unlock(&mut);
    return a;
	}
  int getRefCount(){
    int a;
    pthread_mutex_lock(&mut);
    a=this->ref_count;
    pthread_mutex_unlock(&mut);
    return a;
  }
	virtual ~RCount(){}
protected:
	std::size_t ref_count;
};

template <typename T>
class proxy: public RCount{
public:
	proxy(T* ptr=0){
    this->dmbPointer=ptr;
    pthread_mutex_lock(&mut);
    this->ref_count=(ptr==nullptr)? 0:1;
    pthread_mutex_unlock(&mut);
	}
	proxy(const proxy &rhs){
		this->dmbPointer=rhs.dmbPointer;
    pthread_mutex_lock(&mut);
    this->ref_count=rhs.getRefCount();
    pthread_mutex_unlock(&mut);
		this->addRef();
	}
	~proxy(){
		if(this->getRefCount()==0){
			delete this->dmbPointer;
      this->dmbPointer=nullptr;
   }
	}
private:
	T* dmbPointer;
};

template <typename T>
class SharedPtr{

public:
	SharedPtr(){
	  this->rcount=nullptr;
    this->dmbPtr=nullptr;
  }

	template <typename U>
	explicit SharedPtr(U*);

	SharedPtr(const SharedPtr<T> &p);
	template <typename U>
	SharedPtr(const SharedPtr<U> &p);

	SharedPtr(SharedPtr<T> &&p);
	template<typename U>
	SharedPtr(SharedPtr<U> &&p);
  
  SharedPtr<T>  &operator=(const SharedPtr<T> &p);
  template <typename U>
  SharedPtr<T> &operator=(const SharedPtr<U> &p);

  SharedPtr<T>  &operator=(SharedPtr<T> &&p);
  template <typename U>
  SharedPtr<T> &operator=(SharedPtr<U> &&p);

	~SharedPtr(){
    if(this->rcount!=nullptr && this->rcount->removeRef() == 0){
      delete this->rcount;
    }
    this->dmbPtr=nullptr;
  }

  void reset();
  template <typename U>
  void reset(U *p);
  
  T *get() const;
  T &operator*() const;
  T *operator->() const;
  explicit operator bool() const;

	RCount *rcount;
  T *dmbPtr;
};

template <typename T>
SharedPtr<T>::operator bool() const{
  if(this->dmbPtr!=nullptr)
    return true;
    
  return false;
}

template <typename T>
T* SharedPtr<T>::get() const{
  if(this->rcount==nullptr)
    return nullptr;
    
  return this->dmbPtr;
}

template <typename T>
T& SharedPtr<T>::operator*() const{
  return *(this->dmbPtr);
}

template <typename T>
T* SharedPtr<T>::operator->() const{
  return (this->dmbPtr);
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(U *obj){
  try{
    this->rcount=new proxy<U>(obj);
  }
  catch(const std::bad_alloc &e){
    std::cout<<e.what()<<"\n";
    this->rcount=nullptr;
  }
  
  if(this->rcount!=nullptr)
    this->dmbPtr=static_cast<T *> (obj);
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr<T> &p){
  this->rcount=p.rcount;
  
  if(p.dmbPtr!=nullptr && p.rcount!=nullptr)
    this->rcount->addRef();
    
  this->dmbPtr=static_cast<T *> (p.dmbPtr);
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(const SharedPtr<U> &p){
  this->rcount=p.rcount;
  
  if(p.dmbPtr!=nullptr && p.rcount!=nullptr)
    this->rcount->addRef();
    
  this->dmbPtr=static_cast<T *> (p.dmbPtr);
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr<T> &&p){
  try{
    this->dmbPtr=dynamic_cast<T *> (p.dmbPtr);
  }
  catch(const std::bad_cast &e){
    std::cout<<e.what()<<"\n";
    this->dmbPtr=nullptr;
  }
  if(this->dmbPtr!=nullptr){
    this->rcount=p.rcount;
    p.rcount=nullptr;
    p.dmbPtr=nullptr;
  }
}

template <typename T>
template <typename U>
SharedPtr<T>::SharedPtr(SharedPtr<U> &&p){
  try{
    this->dmbPtr=dynamic_cast<T *> (p.dmbPtr);
  }
  catch(const std::bad_cast &e){
    std::cout<<e.what()<<"\n";
    this->dmbPtr=nullptr;
  }
  if(this->dmbPtr1=nullptr){
    this->rcount=p.rcount;
    p.rcount=nullptr;
    p.dmbPtr=nullptr;
  }
}

template <typename T1, typename T2>
bool operator==(const SharedPtr<T1> &p1, const SharedPtr<T2> &p2){
  if(p1.dmbPtr==nullptr && p2.dmbPtr==nullptr)
    return true;
    
  if(p1.dmbPtr==p2.dmbPtr)
    return true;
    
  return false;
}

template <typename T1, typename T2>
bool operator!=(const SharedPtr<T1> &p1, const SharedPtr<T2> &p2){
  if(p1.dmbPtr!=p2.dmbPtr)
    return true;
  
  if((p1.dmbPtr==nullptr && p2.dmbPtr!=nullptr) || (p1.dmbPtr!=nullptr && p2.dmbPtr==nullptr))
    return true;
    
  return false;
}

template <typename T>
bool operator==(const SharedPtr<T> &p1, std::nullptr_t p2){
  if(p1.dmbPtr==p2)
    return true;
    
  return false;
}

template <typename T>
bool operator==(std::nullptr_t p1, const SharedPtr<T> &p2){
  if(p1==p2.dmbPtr)
    return true;
    
  return false;
}

template <typename T>
bool operator!=(const SharedPtr<T> &p1, std::nullptr_t p2){
  if(p1.dmbPtr!=p2)
    return true;
    
  return false;
}

template <typename T>
bool operator!=(std::nullptr_t p1, const SharedPtr<T> &p2){
  if(p1!=p2.dmbPtr)
    return true;
    
  return false;
}

template <typename T, typename U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U> &p){
  SharedPtr<T> obj;
  obj.dmbPtr=static_cast<T*> (p.dmbPtr);
  obj.rcount=p.rcount;
  obj.rcount->addRef();
  return obj;
}

template <typename T, typename U>
SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &p){
  SharedPtr<T> obj;
  try{
    obj.dmbPtr=dynamic_cast<T*> (p.dmbPtr);
  }
  catch(const std::bad_cast &e){
    std::cout<<e.what()<<"\n";
    obj.dmbPtr=nullptr;
    obj.rcount=nullptr;
    return obj;
  }
  if(p.dmbPtr!=nullptr){
    obj.rcount=p.rcount;
    obj.rcount->addRef();
  }
  return obj;
}


template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T> &p){
  if(*this==p)
    return *this;
  
  if(this->rcount!=nullptr){
    if(this->dmbPtr==nullptr || (this->rcount!=nullptr && this->rcount->removeRef() == 0))
      delete this->rcount;
      
    this->dmbPtr=nullptr;
  }
  this->rcount=p.rcount;
  
  if(p.dmbPtr!=nullptr && this->rcount != nullptr)
    this->rcount->addRef();
    
  this->dmbPtr=static_cast<T *> (p.dmbPtr);
  return *this;
}

template <typename T>
template <typename U>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<U> &p){
  if(*this==p)
    return *this;
  
  if(this->rcount!=nullptr){
    if(this->dmbPtr==nullptr || (this->rcount!=nullptr && this->rcount->removeRef() == 0))
      delete this->rcount;
      
    this->dmbPtr=nullptr;
  }
  this->rcount=p.rcount;
  
  if(p.dmbPtr!=nullptr && this->rcount != nullptr)
    this->rcount->addRef();
    
  this->dmbPtr=static_cast<T *> (p.dmbPtr);
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<T> &&p){
  if(*this==p)
    return *this;
  
  if(this->rcount!=nullptr){
    if(this->dmbPtr==nullptr || (this->rcount!=nullptr && this->rcount->removeRef() == 0))
      delete this->rcount;
      
    this->dmbPtr=nullptr;
  }
  this->rcount=p.rcount;
  try{
    this->dmbPtr=dynamic_cast<T *> (p.dmbPtr);
  }
  catch(const std::bad_cast &e){
    std::cout<<e.what()<<"\n";
    this->dmbPtr=nullptr;
    this->rcount=nullptr;
    return *this;
  }
  p.rcount=nullptr;
  p.dmbPtr=nullptr;
  return *this;
}

template <typename T>
template <typename U>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<U> &&p){
  if(*this==p)
    return *this;
  
  if(this->rcount!=nullptr){
    if(this->dmbPtr==nullptr || (this->rcount!=nullptr && this->rcount->removeRef() == 0))
      delete this->rcount;
    this->dmbPtr=nullptr;
  }
  this->rcount=p.rcount;
  try{
    this->dmbPtr=dynamic_cast<T *> (p.dmbPtr);
  }
  catch(const std::bad_cast &e){
    std::cout<<e.what()<<"\n";
    this->dmbPtr=nullptr;
    this->rcount=nullptr;
    return *this;
  }
  p.rcount=nullptr;
  p.dmbPtr=nullptr;
  return *this;
}

template <typename T>
void SharedPtr<T>::reset(){
  if(this->rcount!=nullptr)
    if(this->dmbPtr==nullptr || (this->rcount!=nullptr && this->rcount->removeRef() == 0)) 
      delete this->rcount;
      
  this->rcount=nullptr;
  this->dmbPtr=nullptr;
}

template <typename T>
template <typename U>
void SharedPtr<T>::reset(U *p){
  if(this->rcount!=nullptr)
    if(this->dmbPtr==nullptr || (this->rcount!=nullptr && this->rcount->removeRef() == 0)) 
      delete this->rcount;
  
  try{    
    this->rcount=new proxy<U>(p);
  }
  catch(const std::bad_alloc &e){
    std::cout<<e.what()<<"\n";
    this->rcount=nullptr;
    this->dmbPtr=nullptr;
  }
  if(this->rcount!=nullptr)
    this->dmbPtr=static_cast<T *> (p);
}


}	//name space
#endif /* SHAREDPTR_HPP_ */
