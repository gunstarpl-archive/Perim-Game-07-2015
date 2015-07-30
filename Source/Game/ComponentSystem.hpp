#pragma once

#include "Precompiled.hpp"
#include "ComponentPool.hpp"

//
// Component System
//

namespace Game
{    
    // Component system class.
    class ComponentSystem
    {
    public:
        // Type declarations.
        typedef std::unique_ptr<ComponentPoolInterface>               ComponentPoolPtr;
        typedef std::unordered_map<std::type_index, ComponentPoolPtr> ComponentPoolList;
        typedef ComponentPoolList::value_type                         ComponentPoolPair;

    public:
        ComponentSystem() :
            m_initialized(false)
        {
        }

        ~ComponentSystem()
        {
        }

        // Initializes the component system.
        bool Initialize(Context& context)
        {
            BOOST_ASSERT(!m_initialized);

            // Add instance to the context.
            BOOST_ASSERT(context.Set(this));

            // Success!
            return m_initialized = true;
        }

        // Declares a component type.
        template<typename Type>
        void Declare()
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            static_assert(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Check if component type was already declared.
            auto it = m_pools.find(typeid(Type));

            if(it != m_pools.end())
                return;

            // Create and add pool to the collection.
            auto pool = std::make_unique<ComponentPool<Type>>();
            auto pair = ComponentPoolPair(typeid(Type), std::move(pool));
            auto result = m_pools.insert(std::move(pair));

            BOOST_ASSERT(result.second == true);
        }

        // Creates a component.
        template<typename Type>
        Type* Create(EntityHandle handle)
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            static_assert(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Get the component pool.
            ComponentPool<Type>* pool = GetComponentPool<Type>();

            if(pool == nullptr)
                return nullptr;

            // Create and return the component.
            return pool->Create(handle);
        }

        // Lookups a component.
        template<typename Type>
        Type* Lookup(EntityHandle handle)
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            static_assert(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Get the component pool.
            ComponentPool<Type>* pool = GetComponentPool<Type>();

            if(pool == nullptr)
                return nullptr;

            // Lookup and return the component.
            return pool->Lookup(handle);
        }

        // Removes a component.
        template<typename Type>
        bool Remove(EntityHandle handle)
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            static_assert(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Get the component pool.
            ComponentPool<Type>* pool = GetComponentPool<Type>();

            if(pool == nullptr)
                return false;

            // Remove a component.
            return pool->Remove(handle);
        }

        // Gets the begin iterator.
        template<typename Type>
        typename ComponentPool<Type>::ComponentIterator Begin()
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            BOOST_STATIC_ASSERT_MSG(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Get the component pool.
            ComponentPool<Type>* pool = GetComponentPool<Type>();

            if(pool == nullptr)
                return ComponentPool<Type>::ComponentIterator();

            // Return the iterator.
            return pool->Begin();
        }

        // Gets the end iterator.
        template<typename Type>
        typename ComponentPool<Type>::ComponentIterator End()
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            BOOST_STATIC_ASSERT_MSG(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Get the component pool.
            ComponentPool<Type>* pool = GetComponentPool<Type>();

            if(pool == nullptr)
                return ComponentPool<Type>::ComponentIterator();

            // Return the iterator.
            return pool->End();
        }

        // Gets a component pool.
        template<typename Type>
        ComponentPool<Type>* GetComponentPool()
        {
            BOOST_ASSERT(m_initialized);

            // Validate component type.
            BOOST_STATIC_ASSERT_MSG(std::is_base_of<Component, Type>::value, "Not a component type.");

            // Find pool by component type.
            auto it = m_pools.find(typeid(Type));

            if(it == m_pools.end())
                return nullptr;

            // Cast the pointer that we already know is a component pool.
            ComponentPool<Type>* pool = reinterpret_cast<ComponentPool<Type>*>(it->second.get());

            // Return the pool.
            return pool;
        }

    public:
        // Connects to entity destroyed event signal.
        void ConnectEntityDestroyed(boost::signals2::signal<void(EntityHandle)>& signal)
        {
            BOOST_ASSERT(m_initialized);

            m_entityDestroyed = signal.connect(boost::bind(&ComponentSystem::OnEntityDestroyed, this, _1));
        }

    private:
        // Called when an entity gets destroyed.
        void OnEntityDestroyed(EntityHandle handle)
        {
            BOOST_ASSERT(m_initialized);

            // Remove entity components from every pool.
            for(auto& pair : m_pools)
            {
                auto& pool = pair.second;
                pool->Remove(handle);
            }
        }

    private:
        // Component pools.
        ComponentPoolList m_pools;

        // Signal connections.
        boost::signals2::connection m_entityDestroyed;

        // Initialization state.
        bool m_initialized;
    };
}