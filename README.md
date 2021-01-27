![YARP logo](https://raw.githubusercontent.com/robotology/yarp/master/doc/images/yarp-robot-24.png "YARP")
YARP telemetry
==============
:warning: LIBRARY UNDER DEVELOPMENT :warning:

![Continuous Integration](https://github.com/robotology-playground/yarp-telemetry/workflows/Continuous%20Integration/badge.svg)

[![YARP homepage](https://img.shields.io/badge/YARP-Yet_Another_Robot_Platform-19c2d8.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABYAAAAWCAYAAADEtGw7AAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAB3RJTUUH4QEDEQMztwAfSwAAAhRJREFUOMvVlD9ME2EYxn931x4crVDbajTRwbQSg8ZuDm6NuombBgcTZxMXY4gLySV2YDFMupKwAtESRhIxEQdcIFEjqK1/SsBaWkr/3ZXenUPTUpprA7Ed/Lbvfb7n+d7vyfN+AgCvV89gSMt0YknGFcKhhEiXlsOuOHbuhDvkVurYk29bua/FsgEQ7JOl8cCpYzVsNV+qPI3/yTdr2HYc9rrkSHROGZ2eUbTkhuJzSvVzPqckaskNZXR6RolE55Sw1yUfumOA+M4uWV0nq+nAQW5W04llsgz09LS0QiQyMcnS4nzHzF1anCcyMenAwotg+Zvxl7dvsmeaDPl9TK0nD2C3BgMEvR6cop2Tlh8Lr60Vwys/M7IoVDeJBFnDsGrY+1xp7/JKYqu2L3/PHz4VBcO0Cob9S00TMub+Ra09toghimsd81gU17CICa0m78WF0/1XB/rkdhrvssXyg8+bu3aT1zJuwV7ZEXL3OtsJJ/WKeaTJA4hu57QvWrnSTvhTQa8cWfj5r3Txn6zu7idkaCVwvUEQ+huCrmGRassWOA6CqyGM6aoWCPXawsdHYD1uYL3l+sU7bYUXPjwD7u73wkNuXJrtqhX/n3DVY1UVOTn4ClG6Vkcqxiap9SFUtWzLVFUZX2AZp3y+Xitrs6Tj91FVs5oKh/ss27+Hm6gBRM8IMGX/Vs8IO6lQU/UeDvcY8OMv7HG7CnjlFeQAAAAASUVORK5CYII=)](http://www.yarp.it/)

This is the telemetry component for YARP.

## Installation
### Dependencies

The depencies are:
- [CMake](https://cmake.org/install/) (minimum version 3.12)
- [Boost](https://www.boost.org/)
- [YARP](https://www.yarp.it/git-master/install.html) (minimum version 3.4.0)
- [matio-cpp](https://github.com/dic-iit/matio-cpp#installation) 

### Linux/macOS
```
git clone https://github.com/robotology-playground/yarp-telemetry
cd yarp-telemetry
mkdir build && cd build
cmake ../
make
[sudo] make install
```
Notice: sudo is not necessary if you specify the CMAKE_INSTALL_PREFIX. In this case it is necessary to add in the .bashrc or .bash_profile the following lines:

`export YARP_telemetry_DIR=/path/where/you/installed/`

### Windows

With IDE build tool facilities, such as Visual Studio:
```
git clone https://github.com/robotology-playground/yarp-telemetry
cd yarp-telemetry
mkdir build && cd build
cmake ..
cmake --build . --target ALL_BUILD --config Release
cmake --build . --target INSTALL --config Release
```
In order to allow CMake finding yarp-telemetry, you have to specify the path where you installed in the `CMAKE_PREFIX_PATH` or exporting the `YARP_telemetry_DIR` env variable pointing to the same path.

## Usage
In order to use this library in your own appliction add this lines in your `CMakeLists.txt`
```cmake
find_package(YARP COMPONENTS telemetry)

add_executable(myApp)
target_link_libraries(myApp YARP::YARP_telemetry)
```

### Example scalar variable

Here is the code snippet for dumping in a `.mat` file 3 samples of the scalar varibles `"one"` and `"two"`.

```c++
    yarp::telemetry::BufferManager<int32_t> bm(n_samples);
    bm.setFileName("buffer_manager_test.mat");
    ChannelInfo var_one{ "one", {1,1} };
    ChannelInfo var_two{ "two", {1,1} };

    auto ok = bm.addChannel(var_one);
    ok = ok && bm.addChannel(var_two);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        bm.push_back({ i }, "one");
        yarp::os::Time::delay(0.2);
        bm.push_back({ i + 1 }, "two");
    }

    if (bm.saveToFile())
        std::cout << "File saved correctly!" << std::endl;
    else
        std::cout << "Something went wrong..." << std::endl;

```
And here is the resulting .mat file:

```
buffer_manager_test = 

  struct with fields:

    one: [1×1 struct]
    two: [1×1 struct]


buffer_manager_test.one

ans = 

  struct with fields:

          data: [1×1×3 int32]
    dimensions: [1 1 3]
          name: 'one'
    timestamps: [523132.9969457 523133.1979436 523133.3988861]
```


### Example vector variable

It is possible to save and dump also vector variables.
Here is the code snippet for dumping in a `.mat` file 3 samples of the 4x1 vector variables `"one"` and `"two"`.

```c++
    yarp::telemetry::BufferManager<double> bm_v({ {"one",{4,1}},
                                                  {"two",{4,1}} }, 3);

    for (int i = 0; i < 10; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "one");
        yarp::os::Time::delay(0.2);
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "two");
    }

    if (bm_v.saveToFile("buffer_manager_test_vector.mat"))
        std::cout << "File saved correctly!" << std::endl;
    else
        std::cout << "Something went wrong..." << std::endl;
```

```
buffer_manager_test_vector = 

  struct with fields:

    one: [1×1 struct]
    two: [1×1 struct]

>> buffer_manager_test_vector.one

ans = 

  struct with fields:

          data: [4×1×3 double]
    dimensions: [4 1 3]
          name: 'one'
    timestamps: [523135.0186688 523135.219639 523135.4203739]
```

### Example matrix variable

Here is the code snippet for dumping in a `.mat` file 3 samples of the 2x3 matrix variable`"one"` and of the 3x2 matrix variable `"two"`.

```c++
    yarp::telemetry::BufferManager<int32_t> bm_m(n_samples, true);
    bm_m.setFileName("buffer_manager_test_matrix.mat");
    std::vector<ChannelInfo> vars{ { "one",{2,3} },
                                   { "two",{3,2} } };

    ok = bm_m.addChannels(vars);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
        yarp::os::Time::delay(0.2);
        bm_m.push_back({ i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 }, "two");
    }

```

```
>> buffer_manager_test_matrix.one

ans = 

  struct with fields:

          data: [2×3×3 int32]
    dimensions: [2 3 3]
          name: 'one'
    timestamps: [112104.7605783 112104.9608881 112105.1611651]
    
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.


## License
[See License](https://github.com/robotology-playground/yarp-telemetry/blob/master/LICENSE)
