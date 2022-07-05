/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <robometry/Buffer.h>

#include <iostream>
#include <functional>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace robometry;

 int main()
 {

    std::cout<<"XXXXXXXX ROBOMETRY BUFFER OF INT XXXXXXXX"<<std::endl;
    robometry::Buffer cb_i(3);

    cout<<"The space available is: "<<cb_i.getBufferFreeSpace()<<std::endl;
    // Insert threee elements into the buffer.
    cb_i.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<int32_t>{ 1 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_i.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<int32_t>{ 2 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_i.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<int32_t>{ 3 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cout<<"The space available is: "<<cb_i.getBufferFreeSpace()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_i) {
        cout<<std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts<< " | " << any_cast<vector<int32_t>>(c_el.m_datum)[0]<<std::endl;
    }

    cb_i.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<int32_t>{ 4 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_i.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<int32_t>{ 5 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    cout<<"The space available is: "<<cb_i.getBufferFreeSpace()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_i) {
        cout<< " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts<< " | " << any_cast<vector<int32_t>>(c_el.m_datum)[0]<<std::endl;
    }


    std::cout<<"XXXXXXXX ROBOMETRY BUFFER OF DOUBLE XXXXXXXX"<<std::endl;

    robometry::Buffer cb_d(3);

    cout<<"The space available is: "<<cb_d.getBufferFreeSpace()<<std::endl;
    // Insert threee elements into the buffer.
    cb_d.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{ 0.1 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_d.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{ 0.2 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_d.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{ 0.3 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cout<<"The space available is: "<<cb_d.getBufferFreeSpace()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_d) {
        cout<< " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts<< " | " << any_cast<vector<double>>(c_el.m_datum)[0]<<std::endl;
    }

    cb_d.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{ 0.4 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_d.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{ 0.5 }}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    cout<<"The space available is: "<<cb_d.getBufferFreeSpace()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_d) {
        cout<< std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts << " | " << any_cast<vector<double>>(c_el.m_datum)[0]<<std::endl;
    }

    std::cout<<"XXXXXXXX ROBOMETRY BUFFER OF VECTOR OF DOUBLE XXXXXXXX"<<std::endl;

    robometry::Buffer cb_v(3);

    cout<<"The space available is: "<<cb_v.getBufferFreeSpace()<<std::endl;
    // Insert threee elements into the buffer.
    cb_v.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{0.1, 0.2, 0.3}}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_v.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{0.3, 0.4, 0.5}}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_v.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{0.6, 0.7, 0.8}}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cout<<"The space available is: "<<cb_v.getBufferFreeSpace()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_v) {
        cout<< " " << std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts << " | " ;
        for(const auto& v_el : any_cast<vector<double>>(c_el.m_datum))
        {
             cout<<v_el<<" ";
        }
        cout<<std::endl;
    }

    cb_v.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{0.9, 1.0, 1.1}}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cb_v.push_back(Record({std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), vector<double>{1.2, 1.3, 1.4}}));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    cout<<"The space available is: "<<cb_v.getBufferFreeSpace()<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb_v) {
        cout<< std::setw( 14 ) << std::setprecision( 14 ) << c_el.m_ts << " | " ;
        for(const auto& v_el : any_cast<vector<double>>(c_el.m_datum))
        {
             cout<<v_el<<" ";
        }
        cout<<std::endl;
    }


    return 0;
 }
