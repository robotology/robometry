/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_BUFFER_H
#define YARP_TELEMETRY_BUFFER_H

#include <yarp/telemetry/Record.h>
#include <boost/circular_buffer.hpp>
#include <cstring>
#include <vector>

namespace yarp::telemetry {

template<class T>
class Buffer {
public:

using iterator       =  typename boost::circular_buffer<Record<T>>::iterator;
using const_iterator =  typename boost::circular_buffer<Record<T>>::const_iterator;

Buffer() = delete;

Buffer(const Buffer&) = default;

Buffer(Buffer&&) noexcept = default;

Buffer<T>& operator=(const Buffer<T>&) = default;

Buffer<T>& operator=(Buffer<T>&&) noexcept = default;

virtual ~Buffer() = default;

Buffer(size_t num_elements,  const std::vector<size_t>& dimensions,
       const std::string& name): m_buffer(num_elements), m_dimensions(dimensions), m_variable_name(name) {

}

Buffer(size_t num_elements, const std::string& name): m_buffer(num_elements), m_variable_name(name) {

}

// TODO: enforced Record<T> ??
// TODO: check if I am pushing a vector with the right dimensions
inline void push_back(const Record<T> &elem)
{
    m_buffer.push_back(elem);
}

// TODO: enforced Record<T> ??
// TODO: check if I am pushing a vector with the right dimensions
inline void push_back(Record<T>&& elem)
{
    m_buffer.push_back(std::move(elem));
}

size_t getBufferFreeSpace() const {
    return m_buffer.capacity() - m_buffer.size();
}

bool empty() const {
    return m_buffer.empty();
}

bool full() const {
    return m_buffer.full();
}

iterator begin() noexcept {
    return m_buffer.begin();
}

iterator end() noexcept {
    return m_buffer.end();
}

const_iterator begin() const noexcept {
    return m_buffer.begin();
}

const_iterator end() const noexcept {
    return m_buffer.end();
}


private:
boost::circular_buffer<Record<T>> m_buffer;
std::vector<size_t> m_dimensions{1,1};
std::string m_variable_name;

};


} // yarp::telemetry

#endif
