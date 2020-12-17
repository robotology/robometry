/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <boost/circular_buffer.hpp>
#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/telemetry/Record.h>

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

    std::cout<<"XXXXXXXX CIRCULAR BUFFER OF INT XXXXXXXX"<<std::endl;
    // Create a circular buffer with a capacity for 3 Record<int32_t> structures.
    boost::circular_buffer<yarp::telemetry::Record<int32_t>> cb_i(3);

    auto count = 0;
    auto total_payload = 0;
    cout<<"The capacity is: "<<cb_i.capacity()<<" and the size is: "<<cb_i.size()<<std::endl;
    // Insert threee elements into the buffer.
    cb_i.push_back(Record(Stamp(count++, yarp::os::Time::now()), 1));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_i.push_back(Record(Stamp(count++, yarp::os::Time::now()), 2));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_i.push_back(Record(Stamp(count++, yarp::os::Time::now()), 3));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cout<<"The capacity is: "<<cb_i.capacity()<<" and the size is: "<<cb_i.size()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_i) {
        cout<<c_el.m_ts.getCount() << " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts.getTime() << " | " << c_el.m_datum<<std::endl;
    }

    cb_i.push_back(Record(Stamp(count++, yarp::os::Time::now()), 4));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_i.push_back(Record(Stamp(count++, yarp::os::Time::now()), 5));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    cout<<"The capacity is: "<<cb_i.capacity()<<" and the size is: "<<cb_i.size()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_i) {
        total_payload += c_el.getPayload();
        cout<<c_el.m_ts.getCount() << " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts.getTime() << " | " << c_el.m_datum<<std::endl;
    }

    std::cout<<"The circular buffer payload is "<<total_payload<<" bytes "<<std::endl;

    std::cout<<"XXXXXXXX CIRCULAR BUFFER OF DOUBLE XXXXXXXX"<<std::endl;

    // Create a circular buffer with a capacity for 3 Record<double> structures.
    boost::circular_buffer<yarp::telemetry::Record<double>> cb_d(3);

    count = 0;
    total_payload = 0;
    cout<<"The capacity is: "<<cb_d.capacity()<<" and the size is: "<<cb_d.size()<<std::endl;
    // Insert threee elements into the buffer.
    cb_d.push_back(Record(Stamp(count++, yarp::os::Time::now()), 0.1));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_d.push_back(Record(Stamp(count++, yarp::os::Time::now()), 0.2));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_d.push_back(Record(Stamp(count++, yarp::os::Time::now()), 0.3));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cout<<"The capacity is: "<<cb_d.capacity()<<" and the size is: "<<cb_d.size()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_i) {
        cout<<c_el.m_ts.getCount() << " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts.getTime() << " | " << c_el.m_datum<<std::endl;
    }

    cb_d.push_back(Record(Stamp(count++, yarp::os::Time::now()), 0.4));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_d.push_back(Record(Stamp(count++, yarp::os::Time::now()), 0.5));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    cout<<"The capacity is: "<<cb_d.capacity()<<" and the size is: "<<cb_d.size()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_i) {
        total_payload += c_el.getPayload();
        cout<<c_el.m_ts.getCount() << " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts.getTime() << " | " << c_el.m_datum<<std::endl;
    }

    std::cout<<"The circular buffer payload is "<<total_payload<<" bytes "<<std::endl;
    std::cout<<"XXXXXXXX CIRCULAR BUFFER OF VECTOR OF DOUBLE XXXXXXXX"<<std::endl;

    // Create a circular buffer with a capacity for 3 Record<vector<double>> structures.
    boost::circular_buffer<yarp::telemetry::Record<vector<double>>> cb_v(3);

    count = 0;
    total_payload = 0;
    cout<<"The capacity is: "<<cb_v.capacity()<<" and the size is: "<<cb_v.size()<<std::endl;
    // Insert threee elements into the buffer.
    cb_v.push_back(Record(Stamp(count++, yarp::os::Time::now()), vector<double>{0.1, 0.2, 0.3}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_v.push_back(Record(Stamp(count++, yarp::os::Time::now()), vector<double>{0.3, 0.4, 0.5}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_v.push_back(Record(Stamp(count++, yarp::os::Time::now()), vector<double>{0.6, 0.7, 0.8}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cout<<"The capacity is: "<<cb_v.capacity()<<" and the size is: "<<cb_v.size()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_v) {
        cout<<c_el.m_ts.getCount() << " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts.getTime() << " | " ;
        for(const auto& v_el : c_el.m_datum)
        {
             cout<<v_el<<" ";
        }
        cout<<std::endl;
    }

    cb_v.push_back(Record(Stamp(count++, yarp::os::Time::now()), vector<double>{0.9, 1.0, 1.1}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_v.push_back(Record(Stamp(count++, yarp::os::Time::now()), vector<double>{1.2, 1.3, 1.4}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    cout<<"The capacity is: "<<cb_v.capacity()<<" and the size is: "<<cb_v.size()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_v) {
        total_payload += c_el.getPayload();
        cout<<c_el.m_ts.getCount() << " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts.getTime() << " | " ;
        for(const auto& v_el : c_el.m_datum)
        {
             cout<<v_el<<" ";
        }
        cout<<std::endl;
    }

    std::cout<<"The circular buffer payload is "<<total_payload<<" bytes "<<std::endl;

    return 0;
 }