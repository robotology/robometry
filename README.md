
Robometry
=========

![Continuous Integration](https://github.com/robotology/robometry/workflows/CI%20Workflow/badge.svg)

[![Anaconda-Server Badge](https://anaconda.org/robotology/robometry/badges/downloads.svg)](https://anaconda.org/conda-forge/librobometry)
[![Anaconda-Server Badge](https://anaconda.org/robotology/robometry/badges/installer/conda.svg)](https://anaconda.org/conda-forge/librobometry)
[![Anaconda-Server Badge](https://anaconda.org/robotology/robometry/badges/platforms.svg)](https://anaconda.org/conda-forge/librobometry)

Telemetry suite for logging data from your robot 🤖.

## Tested OSes
- Windows 10
- Ubuntu 20.04, 22.04
- macOS >= 10.15

## Installation from binaries

### Conda packages

It is possible to install on `linux`, `macOS` and `Windows` via [conda](https://anaconda.org/conda-forge/librobometry), just running:
```bash
conda install -c conda-forge librobometry
```

## Installation from sources

### Dependencies

The dependencies are:
- [CMake](https://cmake.org/install/) (minimum version 3.12)
- [Boost](https://www.boost.org/)
- [matio-cpp](https://github.com/ami-iit/matio-cpp#installation) (minimum version 0.1.1)
- [nlohmann_json](https://github.com/nlohmann/json#integration) (minimum version 3.10.0)
- [Catch2](https://github.com/catchorg/Catch2.git) (v3.7.1, for the unit tests)

The optional dependencies are:
- [YARP](https://www.yarp.it/git-master/install.html) (minimum version 3.5.0)
- [icub-main](https://robotology.github.io/robotology-documentation/doc/html/index.html) (minimum version 2.7.0)

### Linux/macOS
```
git clone https://github.com/robotology/robometry
cd robometry
mkdir build && cd build
cmake ../
make
[sudo] make install
```
Notice: sudo is not necessary if you specify the CMAKE_INSTALL_PREFIX. In this case it is necessary to add in the .bashrc or .bash_profile the following lines:

`export robometry_DIR=/path/where/you/installed/`

### Windows

With IDE build tool facilities, such as Visual Studio:
```
git clone https://github.com/robotology/robometry
cd robometry
mkdir build && cd build
cmake ..
cmake --build . --target ALL_BUILD --config Release
cmake --build . --target INSTALL --config Release
```
In order to allow CMake finding robometry, you have to specify the path where you installed in the `CMAKE_PREFIX_PATH` or exporting the `robometry_DIR` env variable pointing to the same path.

## librobometry
In order to use this library in your own application, add this lines in your `CMakeLists.txt`
```cmake
find_package(robometry)

add_executable(myApp)
target_link_libraries(myApp robometry::robometry)
```

### Example scalar variable

Here is the code snippet for dumping in a `.mat` file 3 samples of the scalar variables `"one"` and `"two"`. The type of the channel is inferred when pushing the first time

```c++
    robometry::BufferConfig bufferConfig;

    // We use the default config, setting only the number of samples (no auto/periodic saving)
    bufferConfig.n_samples = n_samples;

    robometry::BufferManager bm(bufferConfig);
    bm.setFileName("buffer_manager_test");
    robometry::ChannelInfo var_one{ "one", {1} };
    robometry::ChannelInfo var_two{ "two", {1} };

    bool ok = bm.addChannel(var_one);
    ok = ok && bm.addChannel(var_two);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        bm.push_back(i , "one");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bm.push_back(i + 1.0, "two");
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

    description_list: {[1×0 char]}
                 two: [1×1 struct]
                 one: [1×1 struct]


buffer_manager_test.one =

  struct with fields:

              data: [1×3 int32]
        dimensions: [1 3]
    elements_names: {'element_0'}
  units_of_measure: {'n.d.'}
              name: 'one'
        timestamps: [1.6481e+09 1.6481e+09 1.6481e+09]
```

### Example vector variable

It is possible to save and dump also vector variables.
Here is the code snippet for dumping in a `.mat` file 3 samples of the 4x1 vector variables `"one"` and `"two"`.

```c++
    robometry::BufferConfig bufferConfig;
    bufferConfig.auto_save = true; // It will save when invoking the destructor
    bufferConfig.channels = { {"one", {4,1}, {}, {"meters"}}, {"two", {4,1}, {}, {"degrees"}} };
    bufferConfig.filename = "buffer_manager_test_vector";
    bufferConfig.n_samples = 3;

    robometry::BufferManager bm_v(bufferConfig); //Only vectors of doubles are accepted
    for (int i = 0; i < 3; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "one");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "two");
    }

```

```
buffer_manager_test_vector =

  struct with fields:

    description_list: {[1×0 char]}
                 two: [1×1 struct]
                 one: [1×1 struct]


>> buffer_manager_test_vector.one

ans =

  struct with fields:

              data: [4×1×3 double]
        dimensions: [4 1 3]
    elements_names: {4×1 cell}
  units_of_measure: {4×1 cell}
              name: 'one'
        timestamps: [1.6481e+09 1.6481e+09 1.6481e+09]


>> buffer_manager_test_vector.one.elements_names

ans =

  4×1 cell array

    {'element_0'}
    {'element_1'}
    {'element_2'}
    {'element_3'}

>> buffer_manager_test_vector.one.units_of_measure

ans =

  4×1 cell array

    {'m'}
    {'m'}
    {'m'}
    {'m'}
```

It is also possible to specify the name of the elements of each variable with
```c++
robometry::ChannelInfo var_one{ "one", {4,1}, {"A", "B", "C", "D"}, {"m", "cm", "mm", "nm"}};
```


### Example matrix variable

Here is the code snippet for dumping in a `.mat` file 3 samples of the 2x3 matrix variable`"one"` and of the 3x2 matrix variable `"two"`.
``BufferManager``  expects all the inputs to be of vector types, but then input is remapped into a matrix of the specified type.

```c++
    robometry::BufferManager bm_m;
    bm_m.resize(3);
    bm_m.setFileName("buffer_manager_test_matrix");
    bm_m.enablePeriodicSave(0.1); // This will try to save a file each 0.1 sec
    bm_m.setDefaultPath("/my/preferred/path");
    bm_m.setDescriptionList({"head", "left_arm"});
    std::vector<robometry::ChannelInfo> vars{ { "one",{2,3} },
                                                    { "two",{3,2} } };

    bool ok = bm_m.addChannels(vars);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
### Example nested struct

It is possible to save and dump vectors and matrices into nested `mat` structures. To add an element into the matlab struct the you should use the separator `::`. For instance the to store a vector in `A.B.C.my_vector` you should define the channel name as `A::B::C::my_vector`
Here is the code snippet for dumping in a `.mat` file 3 samples of the 4x1 vector variables `"one"` and `"two"` into `struct1` and `struct2`.

```c++
    robometry::BufferConfig bufferConfig;
    bufferConfig.auto_save = true; // It will save when invoking the destructor
    bufferConfig.channels = { {"struct1::one",{4,1}}, {"struct1::two",{4,1}}, {"struct2::one",{4,1}} }; // Definition of the elements into substruct
    bufferConfig.filename = "buffer_manager_test_nested_vector";
    bufferConfig.n_samples = 3;

    robometry::BufferManager bm_v(bufferConfig);
    for (int i = 0; i < 3; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "struct1::one");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "struct1::two");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bm_v.push_back({ (double)i, i/2.0, i/3.0, i/4.0 }, "struct2::one");
    }

```

```
buffer_manager_test_nested_vector =

  struct with fields:

    description_list: {[1×0 char]}
             struct2: [1×1 struct]
             struct1: [1×1 struct]

>> buffer_manager_test_nested_vector.struct1

ans =

  struct with fields:

    two: [1×1 struct]
    one: [1×1 struct]

>> buffer_manager_test_nested_vector.struct1.one

ans =

  struct with fields:

          data: [4×1×3 double]
    dimensions: [4 1 3]
          name: 'one'
    timestamps: [1.6415e+09 1.6415e+09 1.6415e+09]
```

### Example multiple types

``BufferManager`` can be used to store channels of different types, including ``struct``s. In order to store a ``struct``, it is necessary to use the ``VISITABLE_STRUCT`` macro (see https://github.com/garbageslam/visit_struct). The available conversions depend on [``matio-cpp``](https://github.com/ami-iit/matio-cpp).
```c++
struct testStruct
{
    int a;
    double b;
};
VISITABLE_STRUCT(testStruct, a, b);

...

    robometry::BufferManager bm;
    robometry::BufferConfig bufferConfig;

    robometry::ChannelInfo var_int{ "int_channel", {1}};
    robometry::ChannelInfo var_double{ "double_channel", {1}};
    robometry::ChannelInfo var_string{ "string_channel", {1}};
    robometry::ChannelInfo var_vector{ "vector_channel", {4, 1}};
    robometry::ChannelInfo var_struct{ "struct_channel", {1}};

    bm.addChannel(var_int);
    bm.addChannel(var_double);
    bm.addChannel(var_string);
    bm.addChannel(var_vector);
    bm.addChannel(var_struct);

    bufferConfig.n_samples = 3;
    bufferConfig.filename = "buffer_manager_test_multiple_types";
    bufferConfig.auto_save = true;

    bm.configure(bufferConfig);

    testStruct item;

    for (int i = 0; i < 3; i++) {
        bm.push_back(i, "int_channel");
        bm.push_back(i * 1.0, "double_channel");
        bm.push_back("iter" + std::to_string(i), "string_channel");
        bm.push_back({i + 0.0, i + 1.0, i + 2.0, i + 3.0}, "vector_channel");
        item.a = i;
        item.b = i;
        bm.push_back(item, "struct_channel");

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

```
The above snippet of code generates channels of different types. It produces the following output.
```
>> buffer_manager_test_multiple_types

buffer_manager_test_multiple_types =

  struct with fields:

    description_list: {[1×0 char]}
     yarp_robot_name: [1×0 char]
      struct_channel: [1×1 struct]
      vector_channel: [1×1 struct]
      string_channel: [1×1 struct]
      double_channel: [1×1 struct]
         int_channel: [1×1 struct]

>> buffer_manager_test_multiple_types.string_channel

ans =

  struct with fields:

              data: {1×3 cell}
        dimensions: [1 3]
    elements_names: {'element_0'}
  units_of_measure: {'n.d.'}
              name: 'string_channel'
        timestamps: [1.6512e+09 1.6512e+09 1.6512e+09]

>> buffer_manager_test_multiple_types.vector_channel

ans =

  struct with fields:

              data: [4×1×3 double]
        dimensions: [4 1 3]
    elements_names: {4×1 cell}
  units_of_measure: {'n.d.'}
              name: 'vector_channel'
        timestamps: [1.6512e+09 1.6512e+09 1.6512e+09]

```

### Example additional callback

``BufferManager`` can call an additional callback every time the save function is called. The
following example define a custom callback that saves a dummy `txt` file along with the `mat` saved
by the telemetry
```c++
bool myCallback(const std::string& file_name, const SaveCallbackSaveMethod& method) {
  std::string file_name_with_extension = file_name + ".txt";
  std::ofstream my_file(file_name_with_extension.c_str());

  // Write to the file
  my_file << "Dummy file!";

  // Close the file
  my_file.close();

  return true;
};


robometry::BufferManager bm;
bm.setSaveCallback(myCallback);
```

### Example configuration file

It is possible to load the configuration of a BufferManager **from a json file**
```c++
   robometry::BufferManager bm;
   robometry::BufferConfig bufferConfig;
   bool ok = bufferConfigFromJson(bufferConfig,"test_json.json");
   ok = ok && bm.configure(bufferConfig);
```

Where the file has to have this format:
```json
{
  "yarp_robot_name": "robot",
  "description_list": [
    "This is a test",
    "Or it isn't?"
  ],
  "path":"/my/preferred/path",
  "filename": "buffer_manager_test_conf_file",
  "n_samples": 20,
  "save_period": 1.0,
  "data_threshold": 10,
  "auto_save": true,
  "save_periodically": true,
  "channels": [
    {
      "dimensions": [1,1],
      "elements_names": ["element_0"],
      "name": "one",
      "units_of_measure": ["meters"]
    },
    {
      "dimensions": [1,1],
      "elements_names": ["element_0"],
      "name": "two",
      "units_of_measure": ["degrees"]
    }
  ],
  "enable_compression": true,
  "file_indexing": "%Y_%m_%d_%H_%M_%S",
  "mat_file_version": "v7_3"
}
```
The configuration can be saved **to a json file**
```c++
    robometry::BufferConfig bufferConfig;
    bufferConfig.n_samples = 10;
    bufferConfig.save_period = 0.1; //seconds
    bufferConfig.data_threshold = 5;
    bufferConfig.save_periodically = true;
    std::vector<robometry::ChannelInfo> vars{ { "one",{2,3} },
                                                    { "two",{3,2} } };
    bufferConfig.channels = vars;

    auto ok = bufferConfigToJson(bufferConfig, "test_json_write.json");
```


## TelemetryDeviceDumper

The `telemetryDeviceDumper` is a [yarp device](http://yarp.it/git-master/note_devices.html) that has to be launched through the [`yarprobotinterface`](http://yarp.it/git-master/yarprobotinterface.html) for dumping quantities from your robot(e.g. encoders, velocities etc.) in base of what specified in the configuration. It currently needs icub-main version equal or higher than `2.7.0` Specificially this is needed when enabling the parameter `logIRawValuesPublisher`, which is used for dumping any type of raw data values coming from the low level, e.g. raw encoder data. 

### Export the env variables
* Add `${CMAKE_INSTALL_PREFIX}/share/yarp` (where `${CMAKE_INSTALL_PREFIX}` needs to be substituted to the directory that you choose as the `CMAKE_INSTALL_PREFIX`) to your `YARP_DATA_DIRS` environment variable (for more on the `YARP_DATA_DIRS` env variable, see [YARP documentation on data directories](http://www.yarp.it/yarp_data_dirs.html) ).
* Once you do that, you should be able to find the `telemetryDeviceDumper` device compiled by this repo using the command `yarp plugin telemetryDeviceDumper`, which should have an output similar to:
~~~
Yes, this is a YARP plugin
  * library:        CMAKE_INSTALL_PREFIX/lib/yarp/yarp_telemetryDeviceDumper.dll
  * system version: 5
  * class name:     robometry::TelemetryDeviceDumper
  * base class:     yarp::dev::DeviceDriver
~~~
If this is not the case, there could be some problems in finding the plugin. In that case, just move yourself to the `${CMAKE_INSTALL_PREFIX}/share/yarp` directory and launch the device from there.

Further documentation about the configuration parameters and the mapping of the variables inside the .mat file can be browsed [here](https://robotology.github.io/robometry/classrobometry_1_1TelemetryDeviceDumper.html)

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.


## License
[See License](https://github.com/robotology/robometry/blob/master/LICENSE)
