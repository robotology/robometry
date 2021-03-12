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

/**
 * @brief A structure to represent a Record.
 *
 */
template<class T>
struct Record
{
    double m_ts;/**< timestamp */
    std::vector<T> m_datum;/**< the actual data of the record */

    /**
     * @brief Construct an empty Record object
     *
     */
    Record() = default;

    /**
     * @brief Construct a new Record object copying the _datum
     *
     * @param[in] _ts Timestamp to assign to the record.
     * @param[in] _datum Datum to be copied.
     */
    Record(const double& _ts,
           const std::vector<T>& _datum) : m_ts(_ts), m_datum(_datum) {
               m_payload = sizeof(m_ts) + sizeof(m_datum) + sizeof(T) * m_datum.capacity();
    }

    /**
     * @brief Construct a new Record object moving the _datum
     *
     * @param[in] _ts Timestamp to assign to the record.
     * @param[in] _datum Datum to be moved.
     */
    Record(const double& _ts,
           std::vector<T>&& _datum) : m_ts(_ts), m_datum(std::move(_datum)) {
               m_payload = sizeof(m_ts) + sizeof(m_datum) + sizeof(T) * m_datum.capacity();
    }
    /**
     * @brief Get the payload of the Record as nr of bytes
     *
     * @return size_t The payload of the Record.
     */
    size_t getPayload() const {
        return m_payload;
    }

    private:
    size_t m_payload{0};

    // Trying to apply the rule of zero

};


} // yarp::telemetry

#endif
