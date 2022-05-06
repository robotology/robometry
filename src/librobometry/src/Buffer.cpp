/*
 * Copyright (C) 2006-2022 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <robometry/Buffer.h>

robometry::Buffer::Buffer(size_t num_elements): m_buffer_ptr(std::make_shared<boost::circular_buffer<Record>>(num_elements))
{

}

void robometry::Buffer::push_back(const Record &elem)
{
    m_buffer_ptr->push_back(elem);
}

void robometry::Buffer::push_back(Record &&elem)
{
    m_buffer_ptr->push_back(std::move(elem));
}

size_t robometry::Buffer::getBufferFreeSpace() const {
    return m_buffer_ptr->capacity() - m_buffer_ptr->size();
}

size_t robometry::Buffer::size() const {
    return m_buffer_ptr->size();
}

size_t robometry::Buffer::capacity() const {
    return m_buffer_ptr->capacity();

}

bool robometry::Buffer::empty() const {
    return m_buffer_ptr->empty();
}

void robometry::Buffer::resize(size_t new_size) {
    return m_buffer_ptr->resize(new_size);
}

void robometry::Buffer::set_capacity(size_t new_size) {
    return m_buffer_ptr->set_capacity(new_size);
}

bool robometry::Buffer::full() const {
    return m_buffer_ptr->full();
}

robometry::Buffer::const_iterator robometry::Buffer::begin() const noexcept {
    return m_buffer_ptr->begin();
}

robometry::Buffer::iterator robometry::Buffer::end() noexcept {
    return m_buffer_ptr->end();
}

robometry::Buffer::iterator robometry::Buffer::begin() noexcept {
    return m_buffer_ptr->begin();
}

robometry::Buffer::const_iterator robometry::Buffer::end() const noexcept {
    return m_buffer_ptr->end();
}

void robometry::Buffer::clear() noexcept {
    return m_buffer_ptr->clear();

}

std::shared_ptr<boost::circular_buffer<robometry::Record> > robometry::Buffer::getBufferSharedPtr() const {
    return m_buffer_ptr;
}
