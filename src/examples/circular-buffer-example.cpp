/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <boost/circular_buffer.hpp>
#include <iostream>

using namespace std;

 int main()
 {
    // Create a circular buffer with a capacity for 3 integers.
    boost::circular_buffer<int> cb(3);

    // Insert threee elements into the buffer.
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);

    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb) {
        cout<<c_el<<endl;
    }


    cout<<"Pushing 4 and 5"<<endl;
    cb.push_back(4);  // Overwrite 1 with 4.
    cb.push_back(5);  // Overwrite 2 with 5.

    
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb) {
        cout<<c_el<<endl;
    }
    // Elements can be popped from either the front or the back.
    cb.pop_back();  // 5 is removed.
    cb.pop_front(); // 3 is removed.

    cout<<"Removing 3 and 5"<<std::endl;
    cout<<"The circular buffer contains:"<<endl;
    for (auto& c_el : cb) {
        cout<<c_el<<endl;
    }

    return 0;
 }