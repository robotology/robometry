/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_BUFFER_H
#define YARP_TELEMETRY_BUFFER_H

#include <yarp/telemetry/experimental/Record.h>
#include <boost/circular_buffer.hpp>
#include <cstring>
#include <vector>
#include <memory>

namespace yarp::telemetry::experimental {

/**
 * @brief A class to represent the buffer of yarp::telemetry::experimental::Record<T>.
 *
 */
template<class T>
class Buffer {
public:

    using iterator       =  typename boost::circular_buffer<Record<T>>::iterator;
    using const_iterator =  typename boost::circular_buffer<Record<T>>::const_iterator;

    Buffer() = default;

    /**
     * @brief Construct a new Buffer object copying from another Buffer
     *
     * @param[in] _other Buffer to be copied.
     */
    Buffer(const Buffer& _other) = default;

    /**
     * @brief Construct a new Buffer object moving from another Buffer
     *
     * @param[in] _other Buffer to be moved.
     */
    Buffer(Buffer&& _other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     *
     * @param[in] _other Buffer to be copied.
     * @return Buffer<T>& Resulting Buffer.
     */
    Buffer<T>& operator=(const Buffer<T>& _other) = default;

    /**
     * @brief Move assignment operator.
     *
     * @param[in] _other Buffer to be moved.

     * @return Buffer<T>& Resulting Buffer.
     */
    Buffer<T>& operator=(Buffer<T>&& _other) noexcept = default;

    /**
     * @brief Destroy the Buffer object
     *
     */
    virtual ~Buffer() = default;
    /**
     * @brief Construct a new Buffer object.
     *
     * @param[in] num_elements Number of elements of the Buffer to be constructed.
     */
    explicit Buffer(size_t num_elements): m_buffer_ptr(std::make_shared<boost::circular_buffer<Record<T>>>(num_elements))
    {

    }

    /**
     * @brief Push back copying the new Record.
     *
     * @param[in] elem Record to be copied
     */
    inline void push_back(const Record<T> &elem)
    {
        m_buffer_ptr->push_back(elem);
    }

   /**
     * @brief Push back moving the new Record.
     *
     * @param[in] elem Record to be moved.
     */
    inline void push_back(Record<T>&& elem)
    {
        m_buffer_ptr->push_back(std::move(elem));
    }

    /**
     * @brief Get the Buffer free space.
     *
     * @return size_t The free space expressed in bytes.
     */
    size_t getBufferFreeSpace() const {
        return m_buffer_ptr->capacity() - m_buffer_ptr->size();
    }

    /**
     * @brief Get the size of the Buffer.
     *
     * @return size_t The size of the buffer
     */
    size_t size() const {
        return m_buffer_ptr->size();
    }

    /**
     * @brief Get the capacity of Buffer
     *
     * @return size_t The capacity of the buffer.
     */
    size_t capacity() const {
        return m_buffer_ptr->capacity();

    }

    /**
     * @brief Return true if the Buffer is empty, false otherwise.

     *
     */
    bool empty() const {
        return m_buffer_ptr->empty();
    }

    /**
     * @brief Resize the Buffer.
     *
     * @param[in] new_size The new size to be resized to.
     */
    void resize(size_t new_size) {
        return m_buffer_ptr->resize(new_size);
    }

    /**
     * @brief Change the capacity of the Buffer.
     *
     * @param[in] new_size The new size.
     */
    void set_capacity(size_t new_size) {
        return m_buffer_ptr->set_capacity(new_size);
    }

    /**
     * @brief Return true if the Buffer is full, false otherwise.
     *
     */
    bool full() const {
        return m_buffer_ptr->full();
    }

    /**
     * @brief Return the iterator referred to the begin of the Buffer.
     *
     * @return iterator
     */
    iterator begin() noexcept {
        return m_buffer_ptr->begin();
    }

   /**
     * @brief Return the iterator referred to the end of the Buffer.
     *
     * @return iterator
     */
    iterator end() noexcept {
        return m_buffer_ptr->end();
    }

    /**
     * @brief Return the const iterator referred to the begin of the Buffer.
     *
     * @return const_iterator
     */
    const_iterator begin() const noexcept {
        return m_buffer_ptr->begin();
    }

    /**
     * @brief Return the const iterator referred to the end of the Buffer.
     *
     * @return const_iterator
     */
    const_iterator end() const noexcept {
        return m_buffer_ptr->end();
    }

    /**
     * @brief Clear the content of the buffer.
     *
     */
    void clear() noexcept {
        return m_buffer_ptr->clear();

    }

    /**
     * @brief Get the Buffer shared_ptr object.
     *
     * @return std::shared_ptr<boost::circular_buffer<Record<T>>>
     */
    std::shared_ptr<boost::circular_buffer<Record<T>>> getBufferSharedPtr() const {
        return m_buffer_ptr;
    }

private:
    std::shared_ptr<boost::circular_buffer<Record<T>>> m_buffer_ptr;


};


} // yarp::telemetry::experimental

#endif
