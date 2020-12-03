/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <boost/circular_buffer.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

struct Data {
    std::chrono::time_point<std::chrono::system_clock>  ts;
    int datum;
    Data(const std::chrono::time_point<std::chrono::system_clock>& _ts,
         const int& _datum) : ts(_ts), datum(_datum) {
        std::cout<<"Data(_ts, _datum)"<<std::endl;
    }
    Data(const Data& _rhs) : ts(_rhs.ts), datum(_rhs.datum) {
        std::cout<<"Data(const Data&)"<<std::endl;
    }

    Data(Data&& _lhs) noexcept : ts(std::move(_lhs.ts)), datum(std::move(_lhs.datum)) {
        std::cout<<"Data(Data&&)"<<std::endl;
    }

    Data& operator=(const Data& _rhs) {
        std::cout<<"operator=(Data&)"<<std::endl;
        if (this == &_rhs)
            return *this;
        ts = _rhs.ts;
        datum = _rhs.datum;
        return *this;
    }

    Data& operator=(Data&& _lhs) noexcept {
        std::cout<<"operator=(Data&&)"<<std::endl;
        if (this == &_lhs)
            return *this;
        ts = std::move(_lhs.ts);
        datum = std::move(_lhs.datum); // not needed, the move of an int is == to a copy
        return *this;
    }


    ~Data(){
        std::cout<<"~Data"<<std::endl;
    }

};

 int main()
 {
    // Create a circular buffer with a capacity for 3 Data structures.
    boost::circular_buffer<Data> cb(3);

    // Insert threee elements into the buffer.
    cb.push_back(Data(std::chrono::system_clock::now(), 1));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    cb.push_back(Data(std::chrono::system_clock::now(), 2));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    cb.push_back(Data(std::chrono::system_clock::now(), 3));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb) {
        cout<<std::chrono::system_clock::to_time_t(c_el.ts)<<" "<<c_el.datum<<endl;
    }


    cb.push_back(Data(std::chrono::system_clock::now(), 4));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    cb.push_back(Data(std::chrono::system_clock::now(), 5));


    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb) {
        cout<<std::chrono::system_clock::to_time_t(c_el.ts)<<" "<<c_el.datum<<endl;
    }
    // Elements can be popped from either the front or the back.
    cb.pop_back();  // 5 is removed.
    cb.pop_front(); // 3 is removed.

    cout<<"Removing 3 and 5"<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb) {
        cout<<std::chrono::system_clock::to_time_t(c_el.ts)<<" "<<c_el.datum<<endl;
    }

    return 0;
 }