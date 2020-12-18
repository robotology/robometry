In the following table we have a list of the frequencies and sizes of the data we are working with. This is important to calculate how much time we have between two consecutive readings, to test the performance of the system (how fast it can write to the circular buffer)

| Data type                             | Unitary dimension | Occurrences | Frequency |
|---------------------------------------|-------------------|-------------|-----------|
| desired joint positions               | 23x1              | 1           | 100Hz     |
| measured joint positions              | 23x1              | 1           | 100Hz     |
| desired joint velocities              | 23x1              | 1           | 100Hz     |
| measured joint velocities             | 23x1              | 1           | 100Hz     |
| desired joint accelerations           | 23x1              | 1           | 100Hz     |
| measured joint accelerations          | 23x1              | 1           | 100Hz     |
| desired joint torques                 | 23x1              | 1           | 100Hz     |
| measured joint torques                | 23x1              | 1           | 100Hz     |
| measured joint currents               | 23x1              | 1           | 100Hz     |
| measured joint PWM                    | 23x1              | 1           | 100Hz     |
| desired CoM position                  | 3x1               | 1           | 100Hz     |
| measured CoM position                 | 3x1               | 1           | 100Hz     |
| desired momentum                      | 6x1               | 1           | 100Hz     |
| measured momentum                     | 6x1               | 1           | 100Hz     |
| estimated base transform              | 4x4               | 1           | 100Hz     |
| estimated base velocity                 | 6x1               | 2           | 100Hz     |
| desired feet transform                | 4x4               | 2           | 100Hz     |
| measured feet transform               | 4x4               | 2           | 100Hz     |
| desired feet velocity                 | 6x1               | 2           | 100Hz     |
| measured feet velocity               | 6x1               | 2           | 100Hz     |
| desired orientation of other frames   | 4x4               | 4           | 100Hz     |
| desired contact wrenches              | 6x1               | 4           | 100Hz     |
| measured contact wrenches             | 6x1               | 4           | 100Hz     |
| IMU readings (orientation)            | 3x1               | 5           | 100Hz     |
| IMU readings (acceleration)           | 3x1               | 5           | 100Hz     |
| IMU readings (gyros)                  | 3x1               | 5           | 100Hz     |
| Mass matrix                           | 29x29             | 1           | 100Hz     |
| Corilis matrix                        | 29x1              | 1           | 100Hz     |
| Jacobians                             | 29x6              | 5           | 100Hz     |
| Messages coming from the robot boards | ?                 | 1           | ?         |
| controller settings                   | ?                 | 1           | Only once |

Let's assume to have around 5ms of free time every 10ms.

