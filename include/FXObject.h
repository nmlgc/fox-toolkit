/********************************************************************************
*                                                                               *
*                         T o p l e v el   O b j e c t                          *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: FXObject.h,v 1.14 1998/09/18 13:17:47 jvz Exp $                       *
********************************************************************************/
#ifndef FXOBJECT_H
#define FXOBJECT_H


// Minimum and maximum keys
#define MINKEY    0
#define MAXKEY    65535


// Minimum and maximum types
#define MINTYPE   0
#define MAXTYPE   65535


// Association key
typedef FXuint FXSelector;


// Forward
class FXObject;

// Selector mapped function
typedef long (FXObject::* FXSelFunction)(FXObject*,FXSelector,void*);


// Describes a FOX object
struct FXMetaClass {
  const FXchar       *className;
  FXObject*         (*manufacture)();
  const FXMetaClass  *baseClass;
  const void         *assoc;
  FXuint              nassocs;
  FXuint              namelen;
  
  // Check if metaclass is subclass of some other metaclass
  int isSubClassOf(const FXMetaClass* metaclass) const;

  // Make instance of some object
  FXObject* makeInstance() const;

  // Ask class name
  const char* getClassName() const { return className; }

  // Ask base class
  const FXMetaClass* getBaseClass() const { return baseClass; }

  // Find metaclass object
  static const FXMetaClass* getMetaClassFromName(const FXchar* name);
  
  };


// Pre-runtime initializer
class __FXMETACLASSINITIALIZER__ {
public:
  __FXMETACLASSINITIALIZER__(const FXMetaClass* meta);
  };
  
  
// Macro to set up class declaration
#define FXDECLARE(className) \
  public: \
   virtual const FXMetaClass* getMetaClass() const; \
   static FXObject* manufacture(); \
   static const FXMetaClass metaClass; \
  private:


// Macro to set up class implementation
#define FXIMPLEMENT(className,baseClassName,mapping,nmappings) \
  __FXMETACLASSINITIALIZER__ className##Initializer(&className::metaClass); \
  const FXMetaClass className::metaClass={#className,className::manufacture,&baseClassName::metaClass,mapping,nmappings,sizeof(#className)}; \
  const FXMetaClass* className::getMetaClass() const {return &className::metaClass;} \
  FXObject* className::manufacture(){return new className;} 


// MetaClass of a class
#define FXMETACLASS(className) (&className::metaClass)


// Set up map type
#define FXDEFMAP(className) static const struct { FXSelector keylo; FXSelector keyhi; long (className::* func)(FXObject*,FXSelector,void*); } 

// Define range of function types
#define FXMAPTYPES(typelo,typehi,func) {MKUINT(MINKEY,typelo),MKUINT(MAXKEY,typehi),&func}

// Define range of function types
#define FXMAPTYPE(type,func) {MKUINT(MINKEY,type),MKUINT(MAXKEY,type),&func}

// Define range of functions
#define FXMAPFUNCS(type,keylo,keyhi,func) {MKUINT(keylo,type),MKUINT(keyhi,type),&func}

// Define one function
#define FXMAPFUNC(type,key,func) {MKUINT(key,type),MKUINT(key,type),&func}


// Base of all FOX object
class FXObject {
  FXDECLARE(FXObject)
public:

  // Get class name of some object
  const char* getClassName() const;

  // Check if object is member of metaclass
  int isMemberOf(const FXMetaClass* metaclass) const;

  // Associate function with key
  FXSelFunction assoc(FXSelector key) const;
  
  // Handle message
  virtual long handle(FXObject* sender,FXSelector key,void* data);
  
  // Save object to stream
  virtual void save(FXStream& store) const;
  
  // Load object from stream
  virtual void load(FXStream& store);

  // No association found
  static FXSelFunction null;
  
  // Unhandled function
  long onDefault(FXObject*,FXSelector,void*);

  // Virtual destructor
  virtual ~FXObject();
  };


// Saving of FXObject derived classes
template<class TYPE>
inline FXStream& operator<<(FXStream& store,const TYPE *const& obj){return store.saveObject((FXObjectPtr&)(obj));}


// Loading of FXObject derived classes
template<class TYPE>
inline FXStream& operator>>(FXStream& store,TYPE*& obj){return store.loadObject((FXObjectPtr&)(obj));}


#endif
