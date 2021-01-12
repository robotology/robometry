/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/telemetry/BufferManager.h>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace yarp::os;
using namespace yarp::telemetry;

 int main()
 {
    Network yarp;

    yarp::telemetry::BufferManager<int32_t> bm({ {"one",{1,1}},
                                                 {"two",{1,1}} }, 3);

    for (int i = 0; i < 10; i++) {
        bm.push_back(i, "one");
        yarp::os::Time::delay(0.2);
        bm.push_back(i + 1, "two");
    }

    if (bm.saveToFile("buffer_manager_test.mat"))
        std::cout << "File saved correctly!" << std::endl;
    else
        std::cout << "Something went wrong..." << std::endl;



    return 0;
 }