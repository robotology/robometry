/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_BUFFER_H
#define ROBOMETRY_BUFFER_H

#include <robometry/api.h>
#include <robometry/Record.h>
#include <boost/circular_buffer.hpp>
#include <cstring>
#include <vector>
#include <memory>

namespace robometry {

/**
 * @brief A class to represent the buffer of robometry::Record.
 *
 */
class ROBOMETRY_API Buffer {
public:

    using iterator       =  typename boost::circular_buffer<Record>::iterator;
    using const_iterator =  typename boost::circular_buffer<Record>::const_iterator;

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
     * @return Buffer& Resulting Buffer.
     */
    Buffer& operator=(const Buffer& _other) = default;

    /**
     * @brief Move assignment operator.
     *
     * @param[in] _other Buffer to be moved.

     * @return Buffer& Resulting Buffer.
     */
    Buffer& operator=(Buffer&& _other) noexcept = default;

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
    Buffer(size_t num_elements);

    /**
     * @brief Push back copying the new Record.
     *
     * @param[in] elem Record to be copied
     */
    void push_back(const Record &elem);

   /**
     * @brief Push back moving the new Record.
     *
     * @param[in] elem Record to be moved.
     */
    void push_back(Record&& elem);

    /**
     * @brief Get the Buffer free space.
     *
     * @return size_t The free space expressed in bytes.
     */
    size_t getBufferFreeSpace() const;

    /**
     * @brief Get the size of the Buffer.
     *
     * @return size_t The size of the buffer
     */
    size_t size() const;

    /**
     * @brief Get the capacity of Buffer
     *
     * @return size_t The capacity of the buffer.
     */
    size_t capacity() const;

    /**
     * @brief Return true if the Buffer is empty, false otherwise.

     *
     */
    bool empty() const;

    /**
     * @brief Resize the Buffer.
     *
     * @param[in] new_size The new size to be resized to.
     */
    void resize(size_t new_size);

    /**
     * @brief Change the capacity of the Buffer.
     *
     * @param[in] new_size The new size.
     */
    void set_capacity(size_t new_size);

    /**
     * @brief Return true if the Buffer is full, false otherwise.
     *
     */
    bool full() const;

    /**
     * @brief Return the iterator referred to the begin of the Buffer.
     *
     * @return iterator
     */
    iterator begin() noexcept;

   /**
     * @brief Return the iterator referred to the end of the Buffer.
     *
     * @return iterator
     */
    iterator end() noexcept;

    /**
     * @brief Return the const iterator referred to the begin of the Buffer.
     *
     * @return const_iterator
     */
    const_iterator begin() const noexcept;

    /**
     * @brief Return the const iterator referred to the end of the Buffer.
     *
     * @return const_iterator
     */
    const_iterator end() const noexcept;

    /**
     * @brief Clear the content of the buffer.
     *
     */
    void clear() noexcept;

    /**
     * @brief Get the Buffer shared_ptr object.
     *
     * @return std::shared_ptr<boost::circular_buffer<Record>>
     */
    std::shared_ptr<boost::circular_buffer<Record>> getBufferSharedPtr() const;

private:
    std::shared_ptr<boost::circular_buffer<Record>> m_buffer_ptr;


};


} // robometry

#endif
