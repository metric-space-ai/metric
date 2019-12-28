#ifndef SERIALIZER_H_
#define SERIALIZER_H_


#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/polymorphic.hpp"
#include <fstream>

#include "../Layer.h"

// some cereal functionality had to be customized, in order to support
// more user-friendly json format
#ifdef _MSC_VER
#define CEREAL_SIMPLIFY_FOR_BASE_CLASS_HELPER savePolymorphicSharedPtr( ar, ptr, ::cereal::traits::has_shared_from_this<T>::type() );
#else                                                    
#define CEREAL_SIMPLIFY_FOR_BASE_CLASS_HELPER savePolymorphicSharedPtr( ar, ptr, typename ::cereal::traits::has_shared_from_this<T>::type() ); 
#endif                                                      

#define CEREAL_SIMPLIFY_FOR_BASE_CLASS(Base)                            \
namespace cereal {                                                      \
    namespace detail {                                                  \
        template<typename T, typename = std::enable_if_t<std::is_base_of<Base, T>::value>> \
            using enable_if_SerialProxyParent = T;                      \
                                                                        \
        template <class Archive, class T>                               \
        struct InputBindingCreator<Archive, enable_if_SerialProxyParent<T>> \
        {                                                               \
            InputBindingCreator()                                       \
            {                                                           \
                auto & map = StaticObject<InputBindingMap<Archive>>::getInstance().map; \
                auto lock = StaticObject<InputBindingMap<Archive>>::lock(); \
                auto key = std::string(binding_name<T>::name());        \
                auto lb = map.lower_bound(key);                         \
                                                                        \
                if (lb != map.end() && lb->first == key)                \
                    return;                                             \
                                                                        \
                typename InputBindingMap<Archive>::Serializers serializers; \
                                                                        \
                serializers.shared_ptr =                                \
                    [](void * arptr, std::shared_ptr<void> & dptr, std::type_info const & baseInfo) \
                    {                                                   \
                        Archive & ar = *static_cast<Archive*>(arptr);   \
                        std::shared_ptr<T> ptr = std::make_shared<T>(); \
                        ptr->load(ar);                                  \
                        dptr = PolymorphicCasters::template upcast<T>( ptr, baseInfo ); \
                    };                                                  \
                                                                        \
                map.insert( lb, { std::move(key), std::move(serializers) } ); \
            }                                                           \
        };                                                              \
                                                                        \
                                                                        \
        template <class Archive, class T>                               \
        struct OutputBindingCreator<Archive, enable_if_SerialProxyParent<T>> \
        {                                                               \
            static void writeMetadata(Archive & ar)                     \
            {                                                           \
                char const * name = binding_name<T>::name();            \
                std::uint32_t id = ar.registerPolymorphicType(name);    \
                if( id & detail::msb_32bit )                            \
                {                                                       \
                    std::string namestring(name);                       \
                    ar( CEREAL_NVP_("type", namestring) );              \
                }                                                       \
            }                                                           \
                                                                        \
            class PolymorphicSharedPointerWrapper                       \
            {                                                           \
            public:                                                     \
                PolymorphicSharedPointerWrapper( T const * dptr ) : refCount(), wrappedPtr( refCount, dptr ) \
                { }                                                     \
                                                                        \
                inline std::shared_ptr<T const> const & operator()() const { return wrappedPtr; } \
                                                                        \
            private:                                                    \
                std::shared_ptr<void> refCount;       \
                std::shared_ptr<T const> wrappedPtr;  \
            };                                                          \
                                                                        \
                                                                        \
            static inline void savePolymorphicSharedPtr( Archive & ar, T const * dptr, std::true_type  ) \
            {                                                           \
                ::cereal::memory_detail::EnableSharedStateHelper<T> state( const_cast<T *>(dptr) ); \
                PolymorphicSharedPointerWrapper psptr( dptr );          \
                ar( CEREAL_NVP_("ptr_wrapper1", memory_detail::make_ptr_wrapper( psptr() ) ) ); \
            }                                                           \
                                                                        \
            static inline void savePolymorphicSharedPtr( Archive & ar, T const * dptr, std::false_type  ) \
            {                                                           \
                dptr->save(ar);                                         \
            }                                                           \
                                                                        \
            OutputBindingCreator()                                      \
            {                                                           \
                auto & map = StaticObject<OutputBindingMap<Archive>>::getInstance().map; \
                auto key = std::type_index(typeid(T));                  \
                auto lb = map.lower_bound(key);                         \
                                                                        \
                if (lb != map.end() && lb->first == key)                \
                    return;                                             \
                                                                        \
                typename OutputBindingMap<Archive>::Serializers serializers; \
                                                                        \
                                                                        \
                serializers.shared_ptr =                                \
                    [&](void * arptr, void const * dptr, std::type_info const & baseInfo) \
                    {                                                   \
                        Archive & ar = *static_cast<Archive*>(arptr);   \
                        writeMetadata(ar);                              \
                                                                        \
                        auto ptr = PolymorphicCasters::template downcast<T>( dptr, baseInfo ); \
CEREAL_SIMPLIFY_FOR_BASE_CLASS_HELPER                                   \
                    };                                                  \
                                                                        \
                map.insert( { std::move(key), std::move(serializers) } ); \
            }                                                           \
        };                                                              \
    }                                                                   \
                                                                        \
                                                                        \
    template <>                                                         \
    inline void                                                         \
    load<cereal::JSONInputArchive, Base>( cereal::JSONInputArchive & ar, std::shared_ptr<Base> & ptr ) \
    {                                                                   \
        using Archive = cereal::JSONInputArchive;                       \
        std::string name;                                               \
        ar( CEREAL_NVP_("type", name) );                                \
                                                                        \
        auto const & bindingMap = detail::StaticObject<detail::InputBindingMap<Archive>>::getInstance().map; \
                                                                        \
        auto binding = bindingMap.find(name);                           \
        if (binding == bindingMap.end())                                \
            throw cereal::Exception("Trying to serialize an unregistered polymorphic type (" + name + ").\n"); \
                                                                        \
        std::shared_ptr<void> result;                                   \
        binding->second.shared_ptr(&ar, result, typeid(Base));          \
        ptr = std::static_pointer_cast<Base>(result);                   \
                                                                        \
    }                                                                   \
}

namespace MiniDNN
{

    template<typename Scalar>
    std::vector<std::shared_ptr<Layer<Scalar>>> loadFrom(const std::string& filename) {
        std::ifstream is(filename);

        // TODO: consider exception type
        if (is.fail())
            throw std::runtime_error("file doesn't exist, " + filename);
        cereal::JSONInputArchive iarchive(is);

        std::vector<std::shared_ptr<Layer<Scalar>>> res;

        size_t i;
        try {
            for (i = 0;; i++) {
                std::shared_ptr<typename Layer<Scalar>::LayerSerialProxy> lsp;
                iarchive(::cereal::make_nvp(std::to_string(i), lsp));
                res.push_back(lsp->getLoadLayer());
            }
        } catch (cereal::Exception& e) {
            // this is sad
            if (std::string(e.what()).compare("XML Parsing failed - provided NVP (" + std::to_string(i) + ") not found") &&
                std::string(e.what()).compare("JSON Parsing failed - provided NVP (" + std::to_string(i) + ") not found"))
                throw;
        } 

        return res;
    }

    template<typename Scalar>
    void saveTo(const std::string& filename, const std::vector<std::shared_ptr<Layer<Scalar>>>& layers) {
        std::ofstream os(filename);

        // TODO: consider exception type
        if (os.fail())
            throw std::runtime_error("couldn't open file for writing, " + filename);
        cereal::JSONOutputArchive oarchive(os);

        // freeing an object while there are other objects to store with
        // an archive is problematic for cereal

        std::vector<std::shared_ptr<typename Layer<Scalar>::LayerSerialProxy>> tmp;
        for (auto& i : layers)
            tmp.push_back(i->getSerial());

        size_t k = 0;
        for (auto& i : tmp) {
            oarchive(::cereal::make_nvp(std::to_string(k), i));
            k++;
        }
    }
}

#endif
