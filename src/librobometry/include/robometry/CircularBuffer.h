/*
 * Copyright (C) 2026 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_CIRCULAR_BUFFER_H
#define ROBOMETRY_CIRCULAR_BUFFER_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace robometry {

/**
 * Fixed-capacity storage, iterated from oldest to newest. Inserting into a full
 * buffer overwrites its oldest element; inserting at capacity zero does nothing.
 * Raw storage is allocated only on construction or capacity changes. Only live
 * elements are constructed and destroyed; unused capacity is left untouched.
 * Elements themselves may allocate memory.
 *
 * Copies own their elements independently. Structural modifications invalidate
 * iterators. Access from multiple threads requires external synchronization.
 */
template<class T>
class CircularBuffer
{
public:
    using value_type = T;
    using size_type = std::size_t;

    template<bool IsConst>
    class Iterator
    {
        friend class CircularBuffer;
        template<bool> friend class Iterator;

        using BufferType = std::conditional_t<IsConst, const CircularBuffer, CircularBuffer>;
        BufferType* m_buffer{nullptr};
        std::ptrdiff_t m_position{0};

        Iterator(BufferType* buffer, std::ptrdiff_t position) noexcept
            : m_buffer(buffer), m_position(position) {}

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;

        Iterator() noexcept = default;

        template<bool OtherConst, std::enable_if_t<IsConst && !OtherConst, int> = 0>
        Iterator(const Iterator<OtherConst>& other) noexcept
            : m_buffer(other.m_buffer), m_position(other.m_position) {}

        reference operator*() const { return (*m_buffer)[static_cast<size_type>(m_position)]; }
        pointer operator->() const { return std::addressof(operator*()); }
        reference operator[](difference_type n) const { return *(*this + n); }
        Iterator& operator++() noexcept { ++m_position; return *this; }
        Iterator operator++(int) noexcept { auto old = *this; ++*this; return old; }
        Iterator& operator--() noexcept { --m_position; return *this; }
        Iterator operator--(int) noexcept { auto old = *this; --*this; return old; }
        Iterator& operator+=(difference_type n) noexcept { m_position += n; return *this; }
        Iterator& operator-=(difference_type n) noexcept { m_position -= n; return *this; }
        Iterator operator+(difference_type n) const noexcept { auto it = *this; return it += n; }
        Iterator operator-(difference_type n) const noexcept { auto it = *this; return it -= n; }
        friend Iterator operator+(difference_type n, Iterator it) noexcept { return it += n; }

        template<bool OtherConst>
        difference_type operator-(const Iterator<OtherConst>& other) const noexcept
        {
            assert(m_buffer == other.m_buffer);
            return m_position - other.m_position;
        }

        template<bool OtherConst>
        bool operator==(const Iterator<OtherConst>& other) const noexcept
        { return m_buffer == other.m_buffer && m_position == other.m_position; }
        template<bool OtherConst>
        bool operator!=(const Iterator<OtherConst>& other) const noexcept { return !(*this == other); }
        template<bool OtherConst>
        bool operator<(const Iterator<OtherConst>& other) const noexcept { return (*this - other) < 0; }
        template<bool OtherConst>
        bool operator>(const Iterator<OtherConst>& other) const noexcept { return other < *this; }
        template<bool OtherConst>
        bool operator<=(const Iterator<OtherConst>& other) const noexcept { return !(other < *this); }
        template<bool OtherConst>
        bool operator>=(const Iterator<OtherConst>& other) const noexcept { return !(*this < other); }
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    explicit CircularBuffer(size_type capacity = 0)
    {
        allocator_type allocator;
        if (capacity > allocator_traits::max_size(allocator))
            throw std::length_error("CircularBuffer capacity is too large");
        if (capacity != 0)
            m_data = allocator_traits::allocate(allocator, capacity);
        m_capacity = capacity;
    }

    ~CircularBuffer()
    {
        clear();
        if (m_data)
        {
            allocator_type allocator;
            allocator_traits::deallocate(allocator, m_data, m_capacity);
        }
    }

    CircularBuffer(const CircularBuffer& other)
    {
        // The temporary owns every successfully constructed prefix if copying
        // throws, including when the source wraps across the end of storage.
        CircularBuffer copy(other.capacity());
        if (!other.empty())
        {
            const auto first = (std::min)(other.m_size, other.m_capacity - other.m_head);
            copy.appendCopies(other.m_data + other.m_head, first);
            copy.appendCopies(other.m_data, other.m_size - first);
        }
        swap(copy);
    }
    CircularBuffer& operator=(const CircularBuffer& other)
    {
        if (this != &other)
        {
            CircularBuffer copy(other);
            swap(copy);
        }
        return *this;
    }

    CircularBuffer(CircularBuffer&& other) noexcept { swap(other); }
    CircularBuffer& operator=(CircularBuffer&& other) noexcept
    {
        if (this != &other)
        {
            CircularBuffer moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    size_type size() const noexcept { return m_size; }
    size_type capacity() const noexcept { return m_capacity; }
    bool empty() const noexcept { return m_size == 0; }
    bool full() const noexcept { return m_size == capacity(); }

    T& operator[](size_type index) noexcept
    {
        assert(index < m_size);
        return m_data[physicalIndex(index)];
    }
    const T& operator[](size_type index) const noexcept
    {
        assert(index < m_size);
        return m_data[physicalIndex(index)];
    }
    T& front() noexcept { return (*this)[0]; }
    const T& front() const noexcept { return (*this)[0]; }
    T& back() noexcept { return (*this)[m_size - 1]; }
    const T& back() const noexcept { return (*this)[m_size - 1]; }

    void push_back(const T& value) { pushBack(value); }
    void push_back(T&& value) { pushBack(std::move(value)); }

    void pop_back() noexcept
    {
        assert(!empty());
        std::destroy_at(m_data + physicalIndex(m_size - 1));
        --m_size;
    }
    void pop_front() noexcept
    {
        assert(!empty());
        std::destroy_at(m_data + m_head);
        advanceHead();
        --m_size;
    }
    void clear() noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            if (!empty())
            {
                const auto first = (std::min)(m_size, m_capacity - m_head);
                std::destroy_n(m_data + m_head, first);
                std::destroy_n(m_data, m_size - first);
            }
        }
        m_size = 0;
        m_head = 0;
    }

    /** Change capacity, preserving the oldest elements when shrinking. */
    void set_capacity(size_type new_capacity)
    {
        if (new_capacity == capacity())
            return;
        CircularBuffer replacement(new_capacity);
        const auto retained = (std::min)(m_size, new_capacity);
        if (retained != 0)
        {
            const auto first = (std::min)(retained, m_capacity - m_head);
            replacement.appendTransferred(m_data + m_head, first);
            replacement.appendTransferred(m_data, retained - first);
        }
        swap(replacement);
    }

    /** Remove newest elements or append copies of value; grow capacity if needed. */
    void resize(size_type new_size, const T& value = T())
    {
        // value may refer to an element of this buffer, invalidated by reallocation.
        if (new_size > capacity())
        {
            T saved_value(value);
            set_capacity(new_size);
            while (m_size < new_size)
                push_back(saved_value);
        }
        else
        {
            while (m_size < new_size)
                push_back(value);
            while (m_size > new_size)
                pop_back();
        }
    }

    iterator begin() noexcept { return iterator(this, 0); }
    iterator end() noexcept { return iterator(this, static_cast<std::ptrdiff_t>(m_size)); }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator end() const noexcept { return cend(); }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0); }
    const_iterator cend() const noexcept { return const_iterator(this, static_cast<std::ptrdiff_t>(m_size)); }

    void swap(CircularBuffer& other) noexcept
    {
        std::swap(m_data, other.m_data);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_head, other.m_head);
        std::swap(m_size, other.m_size);
    }

private:
    using allocator_type = std::allocator<T>;
    using allocator_traits = std::allocator_traits<allocator_type>;

    // Each uninitialized algorithm destroys its own partial output on failure.
    // m_size accounts for completed spans, which the temporary then owns.
    void appendCopies(const T* source, size_type count)
    {
        if (count == 0)
            return;
        std::uninitialized_copy_n(source, count, m_data + m_size);
        m_size += count;
    }

    void appendTransferred(T* source, size_type count)
    {
        if (count == 0)
            return;
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
            std::uninitialized_move_n(source, count, m_data + m_size);
        else
            std::uninitialized_copy_n(source, count, m_data + m_size);
        m_size += count;
    }

    // Conditional wrap avoids an integer division for every full-buffer push.
    void advanceHead() noexcept
    {
        ++m_head;
        if (m_head == capacity())
            m_head = 0;
    }

    // Avoid overflowing m_head + index for very large capacities.
    size_type physicalIndex(size_type index) const noexcept
    {
        const auto tail_size = capacity() - m_head;
        return index < tail_size ? m_head + index : index - tail_size;
    }

    template<class U>
    void pushBack(U&& value)
    {
        if (capacity() == 0)
            return;
        if (full())
        {
            // Assign to a live object; a failed assignment leaves metadata intact.
            m_data[m_head] = std::forward<U>(value);
            advanceHead();
        }
        else
        {
            allocator_type allocator;
            allocator_traits::construct(allocator, m_data + physicalIndex(m_size),
                                        std::forward<U>(value));
            ++m_size;
        }
    }

    T* m_data{nullptr};
    size_type m_capacity{0};
    size_type m_head{0};
    size_type m_size{0};
};

} // namespace robometry

#endif // ROBOMETRY_CIRCULAR_BUFFER_H
