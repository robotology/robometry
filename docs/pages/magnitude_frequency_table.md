In the following table we have a list of the frequencies and sizes of the data we are working with. This is important to calculate how much time we have between two consecutive readings, to test the performance of the system (how fast it can write to the circular buffer)

|               Data type               | Unitary dimension | Occurrences | Frequency | Type                                               |
|:-------------------------------------:|:-----------------:|:-----------:|:---------:|----------------------------------------------------|
| desired joint positions               | 23x1              | 1           | 100Hz     | ``double``                                         |
| measured joint positions              | 23x1              | 1           | 100Hz     | ``double``                                         |
| desired joint velocities              | 23x1              | 1           | 100Hz     | ``double``                                         |
| measured joint velocities             | 23x1              | 1           | 100Hz     | ``double``                                         |
| desired joint accelerations           | 23x1              | 1           | 100Hz     | ``double``                                         |
| measured joint accelerations          | 23x1              | 1           | 100Hz     | ``double``                                         |
| desired joint torques                 | 23x1              | 1           | 100Hz     | ``double``                                         |
| measured joint torques                | 23x1              | 1           | 100Hz     | ``double``                                         |
| measured joint currents               | 23x1              | 1           | 100Hz     | ``double``                                         |
| measured joint PWM                    | 23x1              | 1           | 100Hz     | ``double``                                         |
| desired CoM position                  | 3x1               | 1           | 100Hz     | ``double``                                         |
| measured CoM position                 | 3x1               | 1           | 100Hz     | ``double``                                         |
| desired momentum                      | 6x1               | 1           | 100Hz     | ``double``                                         |
| measured momentum                     | 6x1               | 1           | 100Hz     | ``double``                                         |
| estimated base transform              | 4x4               | 1           | 100Hz     | ``double``                                         |
| estimated base velocity               | 6x1               | 2           | 100Hz     | ``double``                                         |
| desired feet transform                | 4x4               | 2           | 100Hz     | ``double``                                         |
| measured feet transform               | 4x4               | 2           | 100Hz     | ``double``                                         |
| desired feet velocity                 | 6x1               | 2           | 100Hz     | ``double``                                         |
| measured feet velocity                | 6x1               | 2           | 100Hz     | ``double``                                         |
| desired orientation of other frames   | 4x4               | 4           | 100Hz     | ``double``                                         |
| desired contact wrenches              | 6x1               | 4           | 100Hz     | ``double``                                         |
| measured contact wrenches             | 6x1               | 4           | 100Hz     | ``double``                                         |
| IMU readings (orientation)            | 3x1               | 5           | 100Hz     | ``double``                                         |
| IMU readings (acceleration)           | 3x1               | 5           | 100Hz     | ``double``                                         |
| IMU readings (gyros)                  | 3x1               | 5           | 100Hz     | ``double``                                         |
| Mass matrix                           | 29x29             | 1           | 100Hz     | ``double``                                         |
| Corilis matrix                        | 29x1              | 1           | 100Hz     | ``double``                                         |
| Jacobians                             | 29x6              | 5           | 100Hz     | ``double``                                         |
| Messages coming from the robot boards | ?                 | 1           | ?         | Text, separated by type (Error, Warning, Info, ..) |
| controller settings                   | ?                 | 1           | Only once | Struct                                             |

Let's assume to have around 5ms of free time every 10ms.

