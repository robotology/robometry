/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_RECORD_H
#define YARP_TELEMETRY_RECORD_H

#include <vector>

namespace yarp::telemetry {

template<class T>
struct Record
{
    double m_ts;
    std::vector<T> m_datum;


    Record(const double& _ts,
           const std::vector<T>& _datum) : m_ts(_ts), m_datum(_datum) {
               m_payload = sizeof(m_ts) + sizeof(m_datum) + sizeof(T) * m_datum.capacity();
    }

    Record(const double& _ts,
           std::vector<T>&& _datum) : m_ts(_ts), m_datum(std::move(_datum)) {
               m_payload = sizeof(m_ts) + sizeof(m_datum) + sizeof(T) * m_datum.capacity();
    }

    size_t getPayload() const {
        return m_payload;
    }

    private:
    size_t m_payload{0};

    // Trying to apply the rule of zero

};


} // yarp::telemetry

#endif
