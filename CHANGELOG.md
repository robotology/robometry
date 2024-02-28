# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

## [1.2.3] - 2024-02-28

- Added `log_period` parameter

## [1.2.2] - 2023-09-06

- Ported unit tests to catch v3.

## [1.2.0] - 2022-08-31

- Added `units_of_measure` parameter.

## [1.1.0] - 2022-06-09

- Added doxygen documentation available at https://robotology.github.io/robometry/index.html.
- Removed deprecated options `logAllQuantities`, `logJointAcceleration`, `logJointVelocity`
  from `telemetryDeviceDumper` device.

## [1.0.0] - 2022-05-23
- Renamed the repo robometry instead of yarp-telemetry.
- Renamed `YARP_telemetry` in `robometry`.
- ``BufferManager`` is no more a templated class.

## [0.5.1] - 2022-05-06

- Added the method `setSaveCallback` for adding an additiona function invoked with
  `saveToFile`, for saving additional data (e.g. videos).

## [0.5.0] - 2022-05-04

- Add `yarp_robot_name` variable in the saved mat file
- Added the possibility to specify the names of the each element of a channel.
- BufferInfo is now a struct that contains the name, the dimension and the elements_names.
- Added the possibility to have channels with different types (including custom structs) in a single ``BufferManager``.
- Fixed a bug preventing a log file to be saved when multiple channels are empty.
- YARP is now an optional dependency
- Added `yarp_robot_name` in `telemetryDeviceDumper`.
- Refactored variables names in `telemetryDeviceDumper` in a more hierarchical structure.

## [0.4.0] - 2022-02-22

- Added the possibility to pass `matioCpp::Span` objects to `BufferManager::push_back` and `Record` constructors
- Added the creation of the dir in the `BufferManager` if the path specified does not exist.
- Added the `file_indexing` parameter in the `BufferConfig` struct.
- Add the possibility to specify the saved matfile version in the `BufferManager` class.
- Added the possibility to have multilayer structures in the `BufferManager`.

## [0.3.0] - 2021-10-18

- Added the log of the estimated odometry from `yarp::dev::Nav2D::ILocalization2D`.
- Deprecated the `logAllQuantities` option in favour of `logControlBoardQuantities`.

## [0.2.0] - 2021-05-19

- Added the possibility to specify the time of a ``Record`` with ``push_back``.
- Added the possibility to enable the zlib compression.
- Fixed yarp-telemetry.ini generation.
- Fixed load of `telemetryDeviceDumper` plugin.
- Added the log from these interfaces in the `telemetryDeviceDumper`:
  - `yarp::dev::IMotorEncoders`
  - `yarp::dev::IPidControl`
  - `yarp::dev::IAmplifierControl`
  - `yarp::dev::IControlMode`
  - `yarp::dev::IInteractionMode`
  - `yarp::dev::ITorqueControl`
- Added check of the existence of the path specified in the configuration.

## [0.1.0] - 2021-04-02

First release of `yarp-telemetry`, compatible with YARP 3.4.

