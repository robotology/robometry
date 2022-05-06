/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_RECORD_H
#define ROBOMETRY_RECORD_H

#include <vector>
#include <any>

#include <matioCpp/Span.h>

namespace robometry {

/**
 * @brief A structure to represent a Record.
 *
 */
struct Record
{
    double m_ts;/**< timestamp */
    std::any m_datum;/**< the actual data of the record */
};

} // robometry

#endif // ROBOMETRY_RECORD_H
