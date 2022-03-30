/*
 * Copyright (C) 2006-2022 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <yarp/telemetry/experimental/Buffer.h>

yarp::telemetry::experimental::Buffer::Buffer(size_t num_elements): m_buffer_ptr(std::make_shared<boost::circular_buffer<Record>>(num_elements))
{

}

void yarp::telemetry::experimental::Buffer::push_back(const Record &elem)
{
    m_buffer_ptr->push_back(elem);
}

void yarp::telemetry::experimental::Buffer::push_back(Record &&elem)
{
    m_buffer_ptr->push_back(std::move(elem));
}

size_t yarp::telemetry::experimental::Buffer::getBufferFreeSpace() const {
    return m_buffer_ptr->capacity() - m_buffer_ptr->size();
}

size_t yarp::telemetry::experimental::Buffer::size() const {
    return m_buffer_ptr->size();
}

size_t yarp::telemetry::experimental::Buffer::capacity() const {
    return m_buffer_ptr->capacity();

}

bool yarp::telemetry::experimental::Buffer::empty() const {
    return m_buffer_ptr->empty();
}

void yarp::telemetry::experimental::Buffer::resize(size_t new_size) {
    return m_buffer_ptr->resize(new_size);
}

void yarp::telemetry::experimental::Buffer::set_capacity(size_t new_size) {
    return m_buffer_ptr->set_capacity(new_size);
}

bool yarp::telemetry::experimental::Buffer::full() const {
    return m_buffer_ptr->full();
}

yarp::telemetry::experimental::Buffer::const_iterator yarp::telemetry::experimental::Buffer::begin() const noexcept {
    return m_buffer_ptr->begin();
}

yarp::telemetry::experimental::Buffer::iterator yarp::telemetry::experimental::Buffer::end() noexcept {
    return m_buffer_ptr->end();
}

yarp::telemetry::experimental::Buffer::iterator yarp::telemetry::experimental::Buffer::begin() noexcept {
    return m_buffer_ptr->begin();
}

yarp::telemetry::experimental::Buffer::const_iterator yarp::telemetry::experimental::Buffer::end() const noexcept {
    return m_buffer_ptr->end();
}

void yarp::telemetry::experimental::Buffer::clear() noexcept {
    return m_buffer_ptr->clear();

}

std::shared_ptr<boost::circular_buffer<yarp::telemetry::experimental::Record> > yarp::telemetry::experimental::Buffer::getBufferSharedPtr() const {
    return m_buffer_ptr;
}
