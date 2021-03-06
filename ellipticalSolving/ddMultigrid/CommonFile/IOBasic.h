#ifndef IO_BASIC_H
#define IO_BASIC_H

#include "MathBasic.h"
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

PRJ_BEGIN

//io for basic type
struct IOData;
#define IO_BASIC_DECL(T)	\
ostream& writeBinaryData(const T& val,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(T& val,istream& is,IOData* dat=NULL);
IO_BASIC_DECL(char)
IO_BASIC_DECL(unsigned char)
IO_BASIC_DECL(short)
IO_BASIC_DECL(unsigned short)
IO_BASIC_DECL(int)
IO_BASIC_DECL(unsigned int)
//IO_BASIC_DECL(scalarD)
IO_BASIC_DECL(bool)
IO_BASIC_DECL(sizeType)
#undef IO_BASIC_DECL

//minimal serializable system
struct Serializable {
  Serializable(sizeType type):_type(type) {}
  virtual bool read(istream& is) {
    ASSERT_MSG(false,"Not Implemented!");
    return false;
  }
  virtual bool write(ostream& os) const {
    ASSERT_MSG(false,"Not Implemented!");
    return false;
  }
  virtual bool read(istream& is,IOData* dat) {
    return read(is);
  }
  virtual bool write(ostream& os,IOData* dat) const {
    return write(os);
  }
  virtual boost::shared_ptr<Serializable> copy() const {
    ASSERT_MSG(false,"Not Implemented!");
    return boost::shared_ptr<Serializable>(new Serializable(_type));
  }
  sizeType type() const {
    return _type;
  }
protected:
  void setSerializableType(sizeType type) {
    _type=type;
  }
  sizeType _type;
};
struct IOData {
public:
  typedef boost::shared_ptr<Serializable> SPTR;
  typedef boost::fast_pool_allocator<std::pair<const SPTR,sizeType> > ALLOC_MAP_WR;
  typedef boost::fast_pool_allocator<std::pair<const sizeType,SPTR> > ALLOC_MAP_RD;
  typedef boost::unordered_map<SPTR,sizeType,boost::hash<SPTR>,std::equal_to<SPTR>,ALLOC_MAP_WR> MAP_WR;
  typedef boost::unordered_map<sizeType,SPTR,boost::hash<sizeType>,std::equal_to<sizeType>,ALLOC_MAP_RD> MAP_RD;
  typedef boost::unordered_map<sizeType,SPTR,boost::hash<sizeType>,std::equal_to<sizeType>,ALLOC_MAP_RD> TYPE_SET;
public:
  IOData():_index(0) {}
  sizeType getIndex() {
    return _index++;
  }
  void registerType(boost::shared_ptr<Serializable> type) {
    ASSERT_MSG(type->type() >= 0,"Given type doesn't support shared_ptr serialization!");
    ASSERT_MSGV(_tSet.find(type->type()) == _tSet.end(),"Conflicit type id: %ld",type->type());
    _tSet[type->type()]=type;
  }
  template <typename T>
  void registerType() {
    registerType(boost::shared_ptr<Serializable>(new T));
  }
  template <typename T>void createNew(istream& is,boost::shared_ptr<T>& val) const {
    sizeType type;
    readBinaryData(type,is);
    ASSERT_MSG(type != -1,"Type not found!")
    for(TYPE_SET::const_iterator beg=_tSet.begin(),end=_tSet.end(); beg!=end; beg++)
      if(beg->first == type) {
        val=boost::dynamic_pointer_cast<T>(beg->second->copy());
        return;
      }
    ASSERT_MSG(false,"Cannot find compatible type!")
  }
  MAP_WR _ptrMapWr;
  MAP_RD _ptrMapRd;
private:
  sizeType _index;
  TYPE_SET _tSet;
};

//io for shared_ptr
template <typename T>FORCE_INLINE ostream& writeBinaryData(boost::shared_ptr<T> val,ostream& os,IOData* dat=NULL)
{
  ASSERT_MSG(dat,"You must provide pool for serialize shared_ptr!")
  if(val.get() == NULL) {
    writeBinaryData((sizeType)-1,os);
    return os;
  }
  boost::shared_ptr<Serializable> ptrS=boost::dynamic_pointer_cast<Serializable>(val);
  ASSERT_MSG(ptrS,"Not serializable type!")
  IOData::MAP_WR::const_iterator it=dat->_ptrMapWr.find(ptrS);
  if(it == dat->_ptrMapWr.end()) {
    sizeType id=dat->getIndex();
    writeBinaryData(id,os,dat);
    writeBinaryData(val->type(),os,dat);
    dat->_ptrMapWr[val]=id;
    writeBinaryData(*val,os,dat);
  } else {
    writeBinaryData(it->second,os,dat);
  }
  return os;
}
template <typename T>FORCE_INLINE istream& readBinaryData(boost::shared_ptr<T>& val,istream& is,IOData* dat=NULL)
{
  ASSERT_MSG(dat,"You must provide pool for serialize shared_ptr!")
  sizeType id;
  readBinaryData(id,is,dat);
  if(id == -1) {
    val.reset((T*)NULL);
    return is;
  }
  IOData::MAP_RD::const_iterator it=dat->_ptrMapRd.find(id);
  if(it == dat->_ptrMapRd.end()) {
    dat->createNew(is,val);
    dat->_ptrMapRd[id]=val;
    readBinaryData(*val,is,dat);
  } else {
    val=boost::dynamic_pointer_cast<T>(dat->_ptrMapRd[id]);
  }
  return is;
}
FORCE_INLINE ostream& writeBinaryData(const Serializable& val,ostream& os,IOData* dat=NULL)
{
  val.write(os,dat);
  return os;
}
FORCE_INLINE istream& readBinaryData(Serializable& val,istream& is,IOData* dat=NULL)
{
  val.read(is,dat);
  return is;
}

//io for float is double
ostream& writeBinaryData(scalarF val,ostream& os,IOData* dat=NULL);
istream& readBinaryData(scalarF& val,istream& is,IOData* dat=NULL);
ostream& writeBinaryData(scalarD val,ostream& os,IOData* dat=NULL);
istream& readBinaryData(scalarD& val,istream& is,IOData* dat=NULL);

//io for fixed matrix
#define IO_FIXED_DECL(NAME)	\
ostream& writeBinaryData(const NAME& v,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(NAME& v,istream& is,IOData* dat=NULL);
//double
IO_FIXED_DECL(Vec2d)
IO_FIXED_DECL(Vec3d)
IO_FIXED_DECL(Vec4d)
IO_FIXED_DECL(Vec6d)
IO_FIXED_DECL(Mat2d)
IO_FIXED_DECL(Mat3d)
IO_FIXED_DECL(Mat4d)
IO_FIXED_DECL(Mat6d)
//float is double
IO_FIXED_DECL(Vec2f)
IO_FIXED_DECL(Vec3f)
IO_FIXED_DECL(Vec4f)
IO_FIXED_DECL(Vec6f)
IO_FIXED_DECL(Mat2f)
IO_FIXED_DECL(Mat3f)
IO_FIXED_DECL(Mat4f)
IO_FIXED_DECL(Mat6f)
//sizeType
IO_FIXED_DECL(Vec2i)
IO_FIXED_DECL(Vec3i)
IO_FIXED_DECL(Vec4i)
//char
IO_FIXED_DECL(Vec2c)
IO_FIXED_DECL(Vec3c)
IO_FIXED_DECL(Vec4c)
#undef IO_FIXED_DECL

//io for non-fixed matrix
#define IO_NON_FIXED_DECL(NAME)	\
ostream& writeBinaryData(const NAME& v,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(NAME& v,istream& is,IOData* dat=NULL);
IO_NON_FIXED_DECL(Rowd)
IO_NON_FIXED_DECL(Cold)
IO_NON_FIXED_DECL(Matd)
IO_NON_FIXED_DECL(Rowf)
IO_NON_FIXED_DECL(Colf)
IO_NON_FIXED_DECL(Matf)
IO_NON_FIXED_DECL(Rowi)
IO_NON_FIXED_DECL(Coli)
IO_NON_FIXED_DECL(Mati)
#undef IO_NON_FIXED_DECL

//io for quaternion
#define IO_FIXED_QUAT_DECL(NAMEQ,NAMET,NAMEA)	\
ostream& writeBinaryData(const NAMEQ& v,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(NAMEQ& v,istream& is,IOData* dat=NULL);         \
ostream& writeBinaryData(const NAMET& v,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(NAMET& v,istream& is,IOData* dat=NULL);         \
ostream& writeBinaryData(const NAMEA& v,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(NAMEA& v,istream& is,IOData* dat=NULL);
IO_FIXED_QUAT_DECL(Quatd,Transd,Affined)
IO_FIXED_QUAT_DECL(Quatf,Transf,Affinef)
#undef IO_FIXED_QUAT_DECL

//io for bounding box
#define IO_BB_DECL(NAME)	\
ostream& writeBinaryData(const BBox<NAME>& b,ostream& os,IOData* dat=NULL);	\
istream& readBinaryData(BBox<NAME>& b,istream& is,IOData* dat=NULL);
IO_BB_DECL(scalarD)
IO_BB_DECL(scalarF)
#undef IO_BB_DECL

PRJ_END

#endif
