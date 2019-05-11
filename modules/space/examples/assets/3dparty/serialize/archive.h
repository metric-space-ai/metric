/* -*- mode: c++ -*- */
// archive.h
//
// Dmitry Vinokurov <vinick@omskproject.sotline.ru>
// Last change: Time-stamp: <22-Sep-2005 15:03:55 vinick>
//
//

#ifndef ARCHIVE_H_GUARD
#define ARCHIVE_H_GUARD
//#include "meta/type_traits.h"
//#include "meta/eval_if.h"
// #include "stl_saver.h"
// #include "stl_loader.h"
// #include "string_serialize.h"
// #include "metaserialize.h"
// #include "splitmember.h"
#include <vector>
#include <set>
#include <map>
#include <list>
#include <string>
#include <deque>
#include <cstring>
namespace serialize
{

class access
{
 public:
  template <typename Archive, typename T>
  static void member_load( Archive & ar, T &t)
  {
    t.load(ar,0);
  }

  template <typename Archive, typename T>
  static void member_save( Archive & ar, const T &t)
  {
    t.save(ar,0);
  }
};
template<class Archive, class T>
struct member_saver
{
  static void invoke(Archive & ar, const T & t	)
  {
    access::member_save(ar, t);
  }
};

template<class Archive, class T>
struct member_loader
{
  static void invoke(Archive & ar, T & t	)
  {
    access::member_load(ar, t);
  }
};
// detail
template<class Archive, class T>
inline void split_member(	Archive & ar, T & t)
{
  using typex = typename std::conditional<Archive::is_saving, serialize::member_saver<Archive,T>, serialize::member_loader<Archive, T>>::type;
  typex::invoke(ar, t);
}



// split member function serialize funcition into save/load
#define SERIALIZE_SPLIT_MEMBERS()                 \
  friend class access;                            \
  template<class Archive>                         \
  void serialize(Archive &ar, const unsigned int) \
  {                                               \
    serialize::split_member(ar, *this);           \
  }

template <typename Archive, typename Container>
void save_collection(Archive & ar, const Container & c)
{
  unsigned int count = c.size();
  ar & count;
  typename Container::const_iterator p = c.begin();
  while(count-- > 0 )
  {
    ar & *p;
    ++p;
  }
}
template<typename Archive,
         typename T, typename Allocator>
inline void save(Archive & ar, const std::vector<T,Allocator> & v)
{
  save_collection<Archive,std::vector<T,Allocator> >(ar,v);
}

//! outer wrapper for save compound types which may serialize themself
template<typename Archive,typename T>
inline 	void save(Archive & ar, const T & t)
{
  const_cast<T&>(t).serialize(ar, 0);
}
		

template<typename Archive,
         typename T, typename Allocator>
inline void save(Archive & ar, const std::list<T,Allocator> & v)
{
  save_collection<Archive,std::list<T,Allocator> >(ar,v);
}

template<typename Archive,
         typename T, typename Allocator>
inline void save(Archive & ar, const std::deque<T,Allocator> & v)
{
  save_collection<Archive,std::deque<T,Allocator> >(ar,v);
}

template<typename Archive,
         typename Key,typename Type,
         typename Compare, typename Allocator>
inline void save(Archive & ar, const std::map<Key,Type,Compare,Allocator> & v)
{
  save_collection<Archive,std::map<Key,Type,Compare,Allocator> >(ar,v);
}
template<typename Archive,
         typename Key,
         typename Compare, typename Allocator>
inline void save(Archive & ar, const std::set<Key,Compare,Allocator> & v)
{
  save_collection<Archive,std::set<Key,Compare,Allocator> >(ar,v);
}

template <typename Archive,
          typename First, typename Second>
inline void save(Archive & ar, const std::pair<First,Second> & v)
{
  ar & v.first & v.second;
}

template<class Container>
class reserve_imp
{
 public:
  void operator()(Container &s, unsigned int count) const {
    s.reserve(count);
  }
};

template<class Container>
class no_reserve_imp
{
 public:
  void operator()(Container & /* s */, unsigned int /* count */) const{}
};
// sequential container input
template<class Archive, class Container>
struct archive_input_seq
{
  inline void operator()(Archive &ar, Container &s)
  {
    typedef  typename Container::value_type type;
    type t;
    ar.load(t);
    s.push_back(t);
  }
};
// map input
template<class Archive, class Container>
struct archive_input_map
{
  inline void operator()(Archive &ar, Container &s)
  {
    typedef  std::pair<typename Container::key_type,	typename Container::mapped_type	> type;
    type t;
    ar.load(t);
    s.insert(t);
  }
};

// set input
template<class Archive, class Container>
struct archive_input_set
{
  inline void operator()(Archive &ar, Container &s)
  {
    typedef typename Container::value_type type;
    type t;
    ar.load(t);
    s.insert(t);
  }
};

template <typename Archive, typename Container,typename InputFunction, class R>
void load_collection(Archive & ar,  Container & s)
{
  s.clear();
  // retrieve number of elements
  unsigned int count=0;
  ar & count;
  R rx;
  rx(s, count);
  InputFunction ifunc;
  for(;count > 0;--count)
  {
    ifunc(ar, s);
  }
				
}
//! outer wrapper for load compound types which may serialize themself
template<typename Archive,typename T>
inline 	void load(Archive & ar, T & t)
{
  t.serialize(ar, 0);
}
		
		
template<typename Archive,typename T, typename Allocator>
inline void load(Archive & ar,  std::vector<T,Allocator> & v)
{
  serialize::load_collection<Archive,
                             std::vector<T,Allocator>,
                             archive_input_seq<Archive, std::vector<T, Allocator> >,
                             reserve_imp<std::vector<T, Allocator> >
                             >(ar,v);
}

template<typename Archive,typename T, typename Allocator>
inline void load(Archive & ar, std::list<T,Allocator> & v)
{
  load_collection<Archive,
                  std::list<T,Allocator>,
                  archive_input_seq<Archive, std::list<T, Allocator> >,
                  no_reserve_imp<std::list<T, Allocator> >
                  >(ar,v);
}

template<typename Archive,typename T, typename Allocator>
inline void load(Archive & ar,  std::deque<T,Allocator> & v)
{
  load_collection<Archive,
                  std::deque<T,Allocator>,
                  archive_input_seq<Archive, std::deque<T, Allocator> >,
                  no_reserve_imp<std::deque<T, Allocator> >
                  >(ar,v);
}

template<typename Archive,typename Key,typename Type,typename Compare, typename Allocator>
inline void load(Archive & ar,  std::map<Key,Type,Compare,Allocator> & v)
{
  load_collection<Archive,
                  std::map<Key,Type,Compare,Allocator>,
                  archive_input_map<Archive, std::map<Key, Type, Compare, Allocator> >,
                  no_reserve_imp<std::map<Key, Type, Compare, Allocator	> >
                  >(ar,v);
}

template<typename Archive,typename Key,typename Compare, typename Allocator>
inline void load(Archive & ar, std::set<Key,Compare,Allocator> & v)
{
  load_collection<Archive,
                  std::set<Key,Compare,Allocator>,
                  archive_input_set<Archive, std::set<Key, Compare, Allocator> >,
                  no_reserve_imp<std::set<Key, Compare, Allocator> >
                  >(ar,v);
}

template <typename Archive, typename First, typename Second>
inline void load(Archive & ar, std::pair<First,Second> & v)
{
  ar & v.first & v.second;
}
//! outer wrapper for saving char strings
template<typename Archive>
inline 	void save(Archive & ar, const char * s)
{
  std::size_t l = strlen(s);
  ar & l;
  ar.binary_save(s,l);
}

//! outer wrapper for saving  std::string
template<typename Archive>
inline 	void save(Archive & ar, const std::string & s)
{
  std::size_t l = s.size();
  ar.save(l);
  ar.binary_save(s.data(),l);
}

//! outer wrapper for loading char strings
//! buffer must be inirialized and has enough size;
template<typename Archive>
inline 	void load(Archive & ar, char * s)
{
  std::size_t l=0;
  ar.load(l);
  ar.binary_load(s,l);
  s[l]='\0';
}

//! outer wrapper for loading std::string
template<typename Archive>
inline 	void load(Archive & ar, std::string &s)
{
  std::size_t l=0;
  ar.load(l);
  s.resize(l);
  ar.binary_load(const_cast<char*>(s.data()),l);
}

/** \class saver saver.h
 *  \brief metastructure for different ways of saving fundamental and compound types
 *
 */
//! partial specialization of struct saver for compound types
template <typename T,typename Archive>
struct compound_saver
{
  static void invoke(Archive & ar,const T &t)
  {
    serialize::save(ar,t);
  }
};

//! partial specialization of struct saver for fundamental types
template <typename T,typename Archive>
struct binary_saver
{
  static void invoke(Archive & ar,const T &t)
  {
    ar.binary_save(&t,sizeof(T));
  }
};


/** \class loader 
 *  \brief metastructure for different ways of loading fundamental and compound types
 *
 */
template <typename T,typename Archive>
struct compound_loader
{
  static void invoke(Archive & ar,T &t)
  {
    serialize::load(ar,t);
  }
};
//! partial specialization of struct loader for fundumental types
template <typename T,typename Archive>
struct binary_loader
{
  static void invoke(Archive & ar,T &t)
  {
    ar.binary_load(&t,sizeof(T));
  }
};


/** \class oarchive archive.h "serialize/archive.h"
 *
 *
 */
template<typename OStream>
class oarchive
{
  OStream & os;
  typedef typename OStream::char_type char_type;
  typedef oarchive<OStream> self_type;
 public:
  constexpr static bool is_saving = true;
  oarchive(OStream & ostr):os(ostr) {}

  ~oarchive() { os.flush();}

  template <typename T>
  oarchive & operator & (const T & t)
  {
    save(t);
    return *this;
  }

  template<typename T>
  oarchive & operator << (const T & t) {
    *this & t;
    return *this;
  }
  template<typename T>
  void save(const T &t)
  {
    using typex = typename std::conditional<std::is_fundamental<T>::value, serialize::binary_saver<T,oarchive<OStream>>, serialize::compound_saver<T, oarchive<OStream>>>::type;
    typex::invoke(*this,t);
  }

  void binary_save(const void * address, std::size_t count)
  {
    count = ( count + sizeof(typename OStream::char_type) - 1)
            / sizeof(typename OStream::char_type);
    os.write(static_cast<const typename OStream::char_type *>(address), count  );
  }

};


/** \class iarchive archive.h "serialize/archive.h"
 *
 *
 */
template <typename IStream>
class iarchive
{
 private:
  IStream & is;
 public:
  //enum { is_saving = 1 };
  constexpr static bool is_saving = false;
  iarchive(IStream & istr):is(istr) {}

  ~iarchive() {}

  bool operator !()const  { return is.fail();}
  operator void *() const {return is.operator void*();}

  template <typename T>
  iarchive & operator >> (T & t) {
    return *this & t;
  }
  template <typename T>
  iarchive & operator & (T & t)
  {
    load(t);
    return *this;
  }

  template<typename T>
  void load(T &t)
  {
    using typex = typename std::conditional<std::is_fundamental<T>::value, serialize::binary_loader<T,iarchive<IStream>>, serialize::compound_loader<T, iarchive<IStream>>>::type;
    typex::invoke(*this,t);
  }

  void binary_load(void * address, std::size_t count)
  {
    std::size_t s = count / sizeof(typename IStream::char_type);
    is.read(static_cast<typename IStream::char_type *>(address),s);
  }
};
};
#endif // ARCHIVE_H_GUARD
