// $Id: smartpointer.hh 5504 2009-04-08 13:35:31Z christi $

#ifndef OPM_SHARED_PTR_HH
#define OPM_SHARED_PTR_HH


#if defined HAVE_MEMORY
# include <memory>
#endif
#if defined HAVE_TR1_MEMORY
# include <tr1/memory>
#endif
#if defined HAVE_BOOST_SHARED_PTR_HPP
# include <boost/shared_ptr.hpp>
#endif

#include<opm/common/nullptr.hh>
#include<opm/common/typetraits.hh>
/**
 * @file 
 * @brief This file implements the class shared_ptr (a reference counting
 * pointer), for those systems that don't have it in the standard library.
 * @author Markus Blatt
 */
namespace Opm
{
    // A shared_ptr implementation has been found if SHARED_PTR_NAMESPACE is set at all
#ifdef SHARED_PTR_NAMESPACE
    using SHARED_PTR_NAMESPACE :: shared_ptr;
#else

    /** @addtogroup Common
     *
     * @{
     */
    /**
     * @brief A reference counting smart pointer.
     *
     * It is designed such that it is usable within a std::vector.
     * The contained object is destroyed only if there are no more
     * references to it.
     */
    template<class T>
    class shared_ptr
    {
    public:
        /**
         * @brief The data type we are a pointer for.
         *
         * This has to have a parameterless constructor.
         */
        typedef T element_type;

        /** 
         * @brief Constructs a new smart pointer and allocates the referenced Object.
         */
        inline shared_ptr();
        
        /** 
         * @brief Constructs a new smart pointer from a preallocated Object.
         *
         * \param pointer Raw pointer to the shared data
         *
         * note: the object must be allocated on the heap and after handing the pointer to
         * shared_ptr the ownership of the pointer is also handed to the shared_ptr.
         */
        inline shared_ptr(T * pointer);
        
        /** 
         * @brief Constructs a new smart pointer from a preallocated Object.
         *
         * \tparam Deleter This class must by copyconstructable, the copy constructor must not throw an exception
         * and it must implement void operator() (T*) const
         *
         * \param pointer Raw pointer to the shared data
         * \param deleter A copy of this deleter is stored
         *
         * note: the object must be allocated on the heap and after handing the pointer to
         * shared_ptr the ownership of the pointer is also handed to the shared_ptr.
         */
        template<class Deleter>
        inline shared_ptr(T * pointer, Deleter deleter);
        
        /**
         * @brief Copy constructor.
         * @param pointer The object to copy.
         */
        inline shared_ptr(const shared_ptr<T>& pointer);
        
        /**
         * @brief Destructor.
         */
        inline ~shared_ptr();

        /** \brief Assignment operator */
        inline shared_ptr& operator=(const shared_ptr<T>& pointer);
        
        /** \brief Dereference as object */
        inline element_type& operator*();
        
        /** \brief Dereference as pointer */
        inline element_type* operator->();
      
        /** \brief Dereference as const object */
        inline const element_type& operator*() const;
        
        /** \brief Dereference as const pointer */
        inline const element_type* operator->() const;

        /** \brief Access to the raw pointer, if you really want it */
        element_type* get() const {
            return rep_==0 ? 0 : rep_->rep_;
        }

        /** \brief Swap content of this shared_ptr and another */
        inline void swap(shared_ptr& other);

        /** \brief Decrease the reference count by one and free the memory if the
            reference count has reached 0
        */
        inline void reset();

        /** \brief Detach shared pointer and set it anew for the given pointer */
        inline void reset(T* pointer);

        //** \brief Same as shared_ptr(pointer,deleter).swap(*this)
        template<class Deleter>
        inline void reset(T* pointer, Deleter deleter);

        /** \brief The number of shared_ptrs pointing to the object we point to */
        int use_count() const;

    private:
        /** @brief The object we reference. */  
        class PointerRep
        {
            friend class shared_ptr<element_type>;
        protected:
            /** @brief The number of references. */
            int count_;
            /** @brief The representative. */
            element_type * rep_;
            /** @brief Constructor from existing Pointer. */
            PointerRep(element_type * p) : count_(1), rep_(p) {}
            /** @brief Destructor, deletes element_type* rep_. */
            virtual ~PointerRep() {};
        };

        /** @brief Adds call to deleter to PointerRep. */  
        template<class Deleter>
        class PointerRepImpl :
            public PointerRep
        {
            friend class shared_ptr<element_type>;

            /** @brief Constructor from existing Pointer with custom deleter. */
            PointerRepImpl(element_type * p, const Deleter& deleter) :
                PointerRep(p),
                deleter_(deleter)
            {}

            /** @brief Destructor, deletes element_type* rep_ using deleter. */
            ~PointerRepImpl()
            { deleter_(this->rep_); }

            // store a copy of the deleter
            Deleter deleter_;
        };

        /** \brief A default deleter that just calls delete */
        struct DefaultDeleter
        {
            void operator() (element_type* p) const
            { delete p; }
        };


        PointerRep *rep_;

        // Needed for the implicit conversion to "bool"
        typedef T* shared_ptr::PointerRep::*__unspecified_bool_type;
        
    public:
        /** \brief Implicit conversion to "bool" */
        operator __unspecified_bool_type() const // never throws
        { 
            return rep_ == 0 ? 0 : &shared_ptr::PointerRep::rep_; 
        }


    };

    template<class T>
    inline shared_ptr<T>::shared_ptr(T * p)
    {
        rep_ = new PointerRepImpl<DefaultDeleter>(p, DefaultDeleter());
    }

    template<class T>
    template<class Deleter>
    inline shared_ptr<T>::shared_ptr(T * p, Deleter deleter)
    {
        rep_ = new PointerRepImpl<Deleter>(p, deleter);
    }

    template<class T>
    inline shared_ptr<T>::shared_ptr()
    {
        rep_ = nullptr;
    }

    template<class T>
    inline shared_ptr<T>::shared_ptr(const shared_ptr<T>& other) : rep_(other.rep_)
    {
        if (rep_)
            ++(rep_->count_);
    }

    template<class T>
    inline shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& other)
    {
        if (other.rep_)
            (other.rep_->count_)++;
        
        if(rep_!=0 && --(rep_->count_)<=0) 
            delete rep_;

        rep_ = other.rep_;
        return *this;
    }

    template<class T>
    inline shared_ptr<T>::~shared_ptr()
    {
        if(rep_!=0 && --(rep_->count_)==0){
            delete rep_;
            rep_=0;
        }
    }

    template<class T>
    inline T& shared_ptr<T>::operator*()
    {
        return *(rep_->rep_);
    }

    template<class T>
    inline T *shared_ptr<T>::operator->()
    {
        return rep_->rep_;
    }

    template<class T>
    inline const T& shared_ptr<T>::operator*() const
    {
        return *(rep_->rep_);
    }

    template<class T>
    inline const T *shared_ptr<T>::operator->() const
    {
        return rep_->rep_;
    }
    
    template<class T>
    inline int shared_ptr<T>::use_count() const
    {
        return rep_->count_;
    }

    template<class T>
    inline void shared_ptr<T>::swap(shared_ptr<T>& other)
    {
        PointerRep* dummy = rep_;
        rep_ = other.rep_;
        other.rep_ = dummy;
    }

    template<class T>
    inline void shared_ptr<T>::reset()
    {
        shared_ptr<T>().swap(*this);
    }

    template<class T>
    inline void shared_ptr<T>::reset(T* pointer)
    {
        shared_ptr<T>(pointer).swap(*this);
    }

    template<class T>
    template<class Deleter>
    inline void shared_ptr<T>::reset(T* pointer, Deleter deleter)
    {
        shared_ptr<T>(pointer, deleter).swap(*this);
    }

    /** @} */
#endif  // #ifdef SHARED_PTR_NAMESPACE

    /**
       @brief implements the Deleter concept of shared_ptr without deleting anything
       @relates shared_ptr

       If you allocate an object on the stack, but want to pass it to a class or function as a shared_ptr,
       you can use this deleter to avoid accidential deletion of the stack-allocated object.

       For convinience we provide two free functions to create a shared_ptr from a stack-allocated object
       (\see stackobject_to_shared_ptr):

       1) Convert a stack-allocated object to a shared_ptr:
       @code
            int i = 10;
            shared_ptr<int> pi = stackobject_to_shared_ptr(i);
       @endcode
       2) Convert a stack-allocated object to a shared_ptr of a base class
       @code
            class A {};
            class B : public A {};

            ...
            
            B b;
            shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b);
       @endcode
       
       @tparam T type of the stack-allocated object
     */
    template<class T>
    struct null_deleter
    {
        void operator() (T* p) const {}
    };

    /**
       @brief Convert a stack-allocated object to a shared_ptr:
       @relates shared_ptr
       @code
            int i = 10;
            shared_ptr<int> pi = stackobject_to_shared_ptr(i);
       @endcode
     */
    template<typename T>
    inline shared_ptr<T> stackobject_to_shared_ptr(T & t)
    {
        return shared_ptr<T>(&t, null_deleter<T>());
    }

    /**
       @brief Convert a stack object to a shared_ptr of a base class
       @relates shared_ptr
       @code
            class A {};
            class B : public A {};

            ...
            
            B b;
            shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b);
       @endcode
     */
    template<typename T, typename T2>
    inline shared_ptr<T2> stackobject_to_shared_ptr(T & t)
    {
        return shared_ptr<T2>(dynamic_cast<T2*>(&t), null_deleter<T2>());
    }

}
#endif
