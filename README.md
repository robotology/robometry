![YARP logo](https://raw.githubusercontent.com/robotology/yarp/master/doc/images/yarp-robot-24.png "YARP")
YARP telemetry
==============

:warning: LIBRARY UNDER DEVELOPMENT :warning:

**Since it is under development, we cannot guarantee that the API of `libYARP_telemetry` and the user interface of `telemetryDeviceDumper`(the configuration parameters) will not implement breaking changes. Be aware of this if you start using the code contained in this repository, sorry for the unconvenience.**

![Continuous Integration](https://github.com/robotology/yarp-telemetry/workflows/Continuous%20Integration/badge.svg)

[![YARP homepage](https://img.shields.io/badge/YARP-Yet_Another_Robot_Platform-19c2d8.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABYAAAAWCAYAAADEtGw7AAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAB3RJTUUH4QEDEQMztwAfSwAAAhRJREFUOMvVlD9ME2EYxn931x4crVDbajTRwbQSg8ZuDm6NuombBgcTZxMXY4gLySV2YDFMupKwAtESRhIxEQdcIFEjqK1/SsBaWkr/3ZXenUPTUpprA7Ed/Lbvfb7n+d7vyfN+AgCvV89gSMt0YknGFcKhhEiXlsOuOHbuhDvkVurYk29bua/FsgEQ7JOl8cCpYzVsNV+qPI3/yTdr2HYc9rrkSHROGZ2eUbTkhuJzSvVzPqckaskNZXR6RolE55Sw1yUfumOA+M4uWV0nq+nAQW5W04llsgz09LS0QiQyMcnS4nzHzF1anCcyMenAwotg+Zvxl7dvsmeaDPl9TK0nD2C3BgMEvR6cop2Tlh8Lr60Vwys/M7IoVDeJBFnDsGrY+1xp7/JKYqu2L3/PHz4VBcO0Cob9S00TMub+Ra09toghimsd81gU17CICa0m78WF0/1XB/rkdhrvssXyg8+bu3aT1zJuwV7ZEXL3OtsJJ/WKeaTJA4hu57QvWrnSTvhTQa8cWfj5r3Txn6zu7idkaCVwvUEQ+huCrmGRassWOA6CqyGM6aoWCPXawsdHYD1uYL3l+sU7bYUXPjwD7u73wkNuXJrtqhX/n3DVY1UVOTn4ClG6Vkcqxiap9SFUtWzLVFUZX2AZp3y+Xitrs6Tj91FVs5oKh/ss27+Hm6gBRM8IMGX/Vs8IO6lQU/UeDvcY8OMv7HG7CnjlFeQAAAAASUVORK5CYII=)](http://www.yarp.it/)

[![Anaconda-Server Badge](https://anaconda.org/robotology/yarp-telemetry/badges/downloads.svg)](https://anaconda.org/robotology/yarp-telemetry)
[![Anaconda-Server Badge](https://anaconda.org/robotology/yarp-telemetry/badges/installer/conda.svg)](https://conda.anaconda.org/robotology)
[![Anaconda-Server Badge](https://anaconda.org/robotology/yarp-telemetry/badges/platforms.svg)](https://anaconda.org/robotology/yarp-telemetry)

This is the telemetry component for YARP.

## Tested OSes
- Windows 10
- Ubuntu 20.04
- macOS >= 10.15

## Installation from binaries

### Conda packages

It is possible to install on `linux`, `macOS` and `Windows` via [conda](https://anaconda.org/robotology/yarp-telemetry), just running:
```bash
conda install -c robotology yarp-telemetry
```

## Installation from sources

### Dependencies

The depencies are:
- [CMake](https://cmake.org/install/) (minimum version 3.12)
- [Boost](https://www.boost.org/)
- [YARP](https://www.yarp.it/git-master/install.html) (minimum version 3.4.0)
- [matio-cpp](https://github.com/dic-iit/matio-cpp#installation) (minimum version 0.1.1)
- [nlohmann_json](https://github.com/nlohmann/json#integration) (minimum version 3.10.0)
- [Catch2](https://github.com/catchorg/Catch2.git) (v2.13.1, for the unit tests)

### Linux/macOS
```
git clone https://github.com/robotology/yarp-telemetry
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
git clone https://github.com/robotology/yarp-telemetry
cd yarp-telemetry
mkdir build && cd build
cmake ..
cmake --build . --target ALL_BUILD --config Release
cmake --build . --target INSTALL --config Release
```
In order to allow CMake finding yarp-telemetry, you have to specify the path where you installed in the `CMAKE_PREFIX_PATH` or exporting the `YARP_telemetry_DIR` env variable pointing to the same path.

## Export the env variables
* Add `${CMAKE_INSTALL_PREFIX}/share/yarp` (where `${CMAKE_INSTALL_PREFIX}` needs to be substituted to the directory that you choose as the `CMAKE_INSTALL_PREFIX`) to your `YARP_DATA_DIRS` enviromental variable (for more on the `YARP_DATA_DIRS` env variable, see [YARP documentation on data directories](http://www.yarp.it/yarp_data_dirs.html) ).
* Once you do that, you should be able to find the `telemetryDeviceDumper` device compiled by this repo using the command `yarp plugin telemetryDeviceDumper`, which should have an output similar to:
~~~
Yes, this is a YARP plugin
  * library:        CMAKE_INSTALL_PREFIX/lib/yarp/yarp_telemetryDeviceDumper.dll
  * system version: 5
  * class name:     yarp::telemetry::experimental::TelemetryDeviceDumper
  * base class:     yarp::dev::DeviceDriver
~~~
If this is not the case, there could be some problems in finding the plugin. In that case, just move yourself to the `${CMAKE_INSTALL_PREFIX}/share/yarp` directory and launch the device from there.

## libYARP_telemetry
In order to use this library in your own appliction add this lines in your `CMakeLists.txt`
```cmake
find_package(YARP COMPONENTS telemetry)

add_executable(myApp)
target_link_libraries(myApp YARP::YARP_telemetry)
```

### Example scalar variable

Here is the code snippet for dumping in a `.mat` file 3 samples of the scalar varibles `"one"` and `"two"`.

```c++
    yarp::telemetry::experimental::BufferConfig bufferConfig;

    // We use the default config, setting only the number of samples (no auto/periodic saving)
    bufferConfig.n_samples = n_samples;

    yarp::telemetry::experimental::BufferManager<int32_t> bm(bufferConfig);
    bm.setFileName("buffer_manager_test");
    yarp::telemetry::experimental::ChannelInfo var_one{ "one", {1,1} };
    yarp::telemetry::experimental::ChannelInfo var_two{ "two", {1,1} };

    bool ok = bm.addChannel(var_one);
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

    description_list: {[1×0 char]}
                 two: [1×1 struct]
                 one: [1×1 struct]


buffer_manager_test.one =

  struct with fields:

              data: [1×1×3 int32]
        dimensions: [1 1 3]
    elements_names: {'element_0'}
              name: 'one'
        timestamps: [1.6481e+09 1.6481e+09 1.6481e+09]
```

### Example vector variable

It is possible to save and dump also vector variables.
Here is the code snippet for dumping in a `.mat` file 3 samples of the 4x1 vector variables `"one"` and `"two"`.

```c++
    yarp::telemetry::experimental::BufferConfig bufferConfig;
    bufferConfig.auto_save = true; // It will save when invoking the destructor
    bufferConfig.channels = { {"one",{4,1}}, {"two",{4,1}} };
    bufferConfig.filename = "buffer_manager_test_vector";
    bufferConfig.n_samples = 3;

    yarp::telemetry::experimental::BufferManager<double> bm_v(bufferConfig);
    for (int i = 0; i < 10; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "one");
        yarp::os::Time::delay(0.2);
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
              name: 'one'
        timestamps: [1.6481e+09 1.6481e+09 1.6481e+09]


>> buffer_manager_test_vector.one.elements_names

ans =

  4×1 cell array

    {'element_0'}
    {'element_1'}
    {'element_2'}
    {'element_3'}
```

It is also possible to specify the name of the elements of each variable with
```c++
yarp::telemetry::experimental::ChannelInfo var_one{ "one", {4,1}, {"A", "B", "C", "D"}};
```


### Example matrix variable

Here is the code snippet for dumping in a `.mat` file 3 samples of the 2x3 matrix variable`"one"` and of the 3x2 matrix variable `"two"`.

```c++
    yarp::telemetry::experimental::BufferManager<int32_t> bm_m;
    bm_m.resize(3);
    bm_m.setFileName("buffer_manager_test_matrix");
    bm_m.enablePeriodicSave(0.1); // This will try to save a file each 0.1 sec
    bm_m.setDefaultPath("/my/preferred/path");
    bm_m.setDescriptionList({"head", "left_arm"});
    std::vector<yarp::telemetry::experimental::ChannelInfo> vars{ { "one",{2,3} },
                                                    { "two",{3,2} } };

    bool ok = bm_m.addChannels(vars);
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
### Example nested struct

It is possible to save and dump vectors and matrices into nested `mat` structures. To add an element into the matlab struct the you should use the separator `::`. For instance the to store a vector in `A.B.C.my_vector` you should define the channel name as `A::B::C::my_vector`
Here is the code snippet for dumping in a `.mat` file 3 samples of the 4x1 vector variables `"one"` and `"two"` into `struct1` and `struct2`.

```c++
    yarp::telemetry::experimental::BufferConfig bufferConfig;
    bufferConfig.auto_save = true; // It will save when invoking the destructor
    bufferConfig.channels = { {"struct1::one",{4,1}}, {"struct1::two",{4,1}}, {"struct2::one",{4,1}} }; // Definition of the elements into substruct
    bufferConfig.filename = "buffer_manager_test_nested_vector";
    bufferConfig.n_samples = 3;

    yarp::telemetry::experimental::BufferManager<double> bm_v(bufferConfig);
    for (int i = 0; i < 10; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "struct1::one");
        yarp::os::Time::delay(0.2);
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "struct1::two");
        yarp::os::Time::delay(0.2);
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

### Example configuration file

It is possible to load the configuration of a BufferManager **from a json file**
```c++
   yarp::telemetry::experimental::BufferManager<int32_t> bm;
   yarp::telemetry::experimental::BufferConfig bufferConfig;
   bool ok = bufferConfigFromJson(bufferConfig,"test_json.json");
   ok = ok && bm.configure(bufferConfig);
```

Where the file has to have this format:
```json
{
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
      "name": "one"
    },
    {
      "dimensions": [1,1],
      "elements_names": ["element_0"],
      "name": "two"
    }
  ],
  "enable_compression": true,
  "file_indexing": "%Y_%m_%d_%H_%M_%S",
  "mat_file_version": "v7_3"
}
```
The configuration can be saved **to a json file**
```c++
    yarp::telemetry::experimental::BufferConfig bufferConfig;
    bufferConfig.n_samples = 10;
    bufferConfig.save_period = 0.1; //seconds
    bufferConfig.data_threshold = 5;
    bufferConfig.save_periodically = true;
    std::vector<yarp::telemetry::experimental::ChannelInfo> vars{ { "one",{2,3} },
                                                    { "two",{3,2} } };
    bufferConfig.channels = vars;

    auto ok = bufferConfigToJson(bufferConfig, "test_json_write.json");
```


## TelemetryDeviceDumper

The `telemetryDeviceDumper` is a [yarp device](http://yarp.it/git-master/note_devices.html) that has to be launched through the [`yarprobotinterface`](http://yarp.it/git-master/yarprobotinterface.html) for dumping quantities from your robot(e.g. encoders, velocities etc.) in base of what specified in the configuration.

### Parameters


| Parameter name | Type | Units | Default | Required | Description |
| -------- | -------- | -------- | -------- | -------- | -------- |
| `axesNames`     | List of strings     | -  | -     | Yes     | The axes contained in the axesNames parameter are then mapped to the wrapped controlboard in the attachAll method, using controlBoardRemapper class. |
| `logJointVelocity`  (DEPRECATED)   | bool     | -     | false     | No     | Enable the log of joint velocities.     |
| `logJointAcceleration` (DEPRECATED)   | bool     | -     | false     | No     | Enable the log of joint accelerations.     |
| `logIEncoders`     | bool     | -     |  true | No     | Enable the log of `encoders`, `velocity` and `acceleration` (http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html)     |
| `logITorqueControl`     | bool     | -     | false     | No     | Enable the log of `torque`(http://yarp.it/git-master/classyarp_1_1dev_1_1ITorqueControl.html).     |
| `logIMotorEncoders`     | bool     | -     | false     | No     | Enable the log of `motor_encoders`, `motor_velocity` and `motor_acceleration` (http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html).     |
| `logIControlMode`     | bool     | -     | false     | No     | Enable the log of `control_mode` (http://yarp.it/git-master/classyarp_1_1dev_1_1IControlMode.html.     |
| `logIInteractionMode`     | bool     | -     | false     | No     | Enable the log of `interaction_modes` (http://yarp.it/git-master/classyarp_1_1dev_1_1IInteractionMode.html.     |
| `logIPidControl`     | bool     | -     | false     | No     | Enable the log of `position_error`, `position_reference`, `torque_error`, `torque_reference`(http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html).|
| `logIAmplifierControl`     | bool     | -     | false     | No     | Enable the log of `pwm` and `current` (http://yarp.it/git-master/classyarp_1_1dev_1_1IAmplifierControl.html).     |
| `logAllQuantities` (DEPRECATED)    | bool     | -     | false     | No     | Enable the log all quantities described above. |
| `logControlBoardQuantities` | bool     | -     | false     | No     | Enable the log of all the quantities that requires the attach to a control board (`logIEncoders`, `logITorqueControl`, `logIMotorEncoders`, `logIControlMode`, `logIInteractionMode`, `logIPidControl`, `logIAmplifierControl`). |
| `logILocalization2D` | bool     | -     | false     | No     | Enable the log of `odometry_data` (http://yarp.it/git-master/classyarp_1_1dev_1_1Nav2D_1_1ILocalization2D.html). For properly logging the odometry data [this patch](https://github.com/robotology/yarp/pull/2602) is needed on YARP side |
| `saveBufferManagerConfiguration`     | bool     | -    | false     | No     | Enable the save of the configuration of the BufferManager into `path`+ `"bufferConfig"` + `experimentName` + `".json"`     |
| `json_file`     | string     | -     | -     | No     | Configure the `yarp::telemetry::experimental::BufferManager`s reading from a json file like [in Example configuration file](#example-configuration-file). Note that this configuration will overwrite the parameter-by-parameter configuration   |
| `experimentName`     | string     | -     | -     | Yes     | Prefix of the files that will be saved. The files will be named: `experimentName`+`timestamp`+ `".mat"`.     |
| `path`     | string     | -     | -     | No     | Path of the folder where the data will be saved.     |
| `n_samples`     | size_t     | -     | -     | Yes     | The max number of samples contained in the circular buffer/s     |
| `save_periodically`     | bool     | -     | false     | No(but it has to be set to true if `auto_save` is set to false)     | The flag for enabling the periodic save thread.     |
| `save_period`     | double     | seconds     | -     | Yes(if `save_periodically` is set to true)     | The period in seconds of the save thread     |
| `data_threshold`     | size_t     | -     | 0     | No     | The save thread saves to a file if there are at least `data_threshold` samples     |
| `auto_save`     | bool     | -     | false     | No(but it has to be set to true if `save_periodically` is set to false)     | the flag for enabling the save in the destructor of the `yarp::telemetry::experimental::BufferManager`     |

### Mapping .mat variables -> YARP interfaces

| Variable name        | YARP interface |
| -------------------- | -------------- |
| `encoders`           | [`yarp::dev::IEncoders::getEncoders`](http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html#abcfe10041280b99c7c4384c4fd93a9dd) |
| `velocity`           | [`yarp::dev::IEncoders::getEncoderSpeeds`](http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html#ac84ae2f65f4a93b66827a3a424d1f743) |
| `acceleration`       | [`yarp::dev::IEncoders::getEncoderAccelerations`](http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html#a3bcb5fe5c6a5e15e57f4723bbe12c57a) |
| `motor_encoders`     | [`yarp::dev::IMotorEncoders::getMotorEncoders`](http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html#ac02fb05bb3e9ac9a381d41b59c38c412) |
| `motor_velocity`     | [`yarp::dev::IMotorEncoders::getMotorEncoderSpeeds`](http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html#a4624348cc129bfeb12ad0e6d2892b76c) |
| `motor_acceleration` | [`yarp::dev::IMotorEncoders::getMotorEncoderAccelerations`](http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html#a9394d8b5cc4f3d58aeaa07c3fb9a6e6a) |
| `control_mode`       | [`yarp::dev::IControlMode::getControlModes`](http://yarp.it/git-master/classyarp_1_1dev_1_1IControlMode.html#a32f04715873a8099ec40671f65faff8d) |
| `interaction_mode`   | [`yarp::dev::IInteractionMode::getInteractionModes`](http://yarp.it/git-master/classyarp_1_1dev_1_1IInteractionMode.html#a6055ce20216f479da6c63807a4d11f54) |
| `position_error`     | [`yarp::dev::IPidControl::getPidErrors`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#aea29e0fdf34f819ac69a3b940556ba28) |
| `position_reference` | [`yarp::dev::IPidControl::getPidReferences`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#a29e8f684a15d859229a9ae2902f886da) |
| `torque_error`       | [`yarp::dev::IPidControl::getPidErrors`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#aea29e0fdf34f819ac69a3b940556ba28) |
| `torque_reference`   | [`yarp::dev::IPidControl::getPidReferences`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#a29e8f684a15d859229a9ae2902f886da) |
| `odometry_data   `   | [`yarp::dev::Nav2D::ILocalization2D::getEstimatedOdometry`](http://yarp.it/git-master/classyarp_1_1dev_1_1Nav2D_1_1ILocalization2D.html#a02bff57282777ce7511b671abd4c95f0) |

### Example of xml

Example of xml file for using it on the `iCub` robot:

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE devices PUBLIC "-//YARP//DTD yarprobotinterface 3.0//EN" "http://www.yarp.it/DTD/yarprobotinterfaceV3.0.dtd">


    <device xmlns:xi="http://www.w3.org/2001/XInclude" name="telemetryDeviceDumper" type="telemetryDeviceDumper">
        <param name="axesNames">(torso_pitch,torso_roll,torso_yaw,neck_pitch, neck_roll,neck_yaw,l_shoulder_pitch,l_shoulder_roll,l_shoulder_yaw,l_elbow,l_wrist_prosup,l_wrist_pitch,l_wrist_yaw,r_shoulder_pitch,r_shoulder_roll,r_shoulder_yaw,r_elbow,r_wrist_prosup,r_wrist_pitch,r_wrist_yaw,l_hip_pitch,l_hip_roll,l_hip_yaw,l_knee,l_ankle_pitch,l_ankle_roll,r_hip_pitch,r_hip_roll,r_hip_yaw,r_knee,r_ankle_pitch,r_ankle_roll)</param>
        <param name="logIEncoders">true</param>
        <param name="logITorqueControl">true</param>
        <param name="logIMotorEncoders">true</param>
        <param name="logIControlMode">true</param>
        <param name="logIInteractionMode">true</param>
        <param name="logIPidControl">false</param>
        <param name="logIAmplifierControl">true</param>
        <param name="saveBufferManagerConfiguration">true</param>
        <param name="experimentName">test_telemetry</param>
        <param name="path">/home/icub/test_telemetry/</param>
        <param name="n_samples">100000</param>
        <param name="save_periodically">true</param>
        <param name="save_period">120.0</param>
        <param name="data_threshold">300</param>
        <param name="auto_save">true</param>

        <action phase="startup" level="15" type="attach">
            <paramlist name="networks">
                <!-- motorcontrol and virtual torque sensors -->
                <elem name="left_lower_leg">left_leg-eb7-j4_5-mc</elem>
                <elem name="right_lower_leg">right_leg-eb9-j4_5-mc</elem>
                <elem name="left_upper_leg">left_leg-eb6-j0_3-mc</elem>
                <elem name="right_upper_leg">right_leg-eb8-j0_3-mc</elem>
                <elem name="torso">torso-eb5-j0_2-mc</elem>
                <elem name="right_lower_arm">right_arm-eb27-j4_7-mc</elem>
                <elem name="left_lower_arm">left_arm-eb24-j4_7-mc</elem>
                <elem name="right_upper_arm">right_arm-eb3-j0_3-mc</elem>
                <elem name="left_upper_arm">left_arm-eb1-j0_3-mc</elem>
                <elem name="head-j0">head-eb20-j0_1-mc</elem>
                <elem name="head-j2">head-eb21-j2_5-mc</elem>
                <!-- ft -->
            </paramlist>
        </action>

        <action phase="shutdown" level="2" type="detach" />

    </device>
```



## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.


## License
[See License](https://github.com/robotology/yarp-telemetry/blob/master/LICENSE)
