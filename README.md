#### Version 1.0

- Ported from KE old Keil project.

#### Version 1.1

- Add comment to code.

#### Version 1.2

- Add basic comment to all code.

#### Version 1.3

- Add Uart comments to code.

#### Version 1.4

- Add Record module original files.

#### Version1.5

- Add reentrant solutions.

#### Version 1.6

- Simplify Record functions

#### Version 1.7

- Base Simplify 
- Tested

#### Version 1.8 

- New project file

#### Version 1.9

- Reorganized part of Main function 

#### Version 2.0

- Uart is completed

#### Version 2.1

- Flash read & write completed

#### Version 2.2

- Flash read & write record

#### Version 2.3

- 0x08 device time OK

#### Version 2.4

- Flash record new method

#### Version 2.5

- Flash record write and read is OK

#### Version 3.0

> 2020/2/16 3:24

- Device function is all complete. 

#### Version 3.1

> 2020/2/17 10:21

- Set Baud rate to 4800bps 8 1 Even
- Set uart send interval to 10ms in case of data loss(a better way to wait send complete)
- Add blink yellow led for sensor expiration
- Add a switch for sensor control 

#### Version 3.2

> 2020/2/17 16:09

- Fix sensor expired led problem

#### Version 3.3

> 2020/2/18 16:21

- Fix read records checksum error

#### Version 3.4

> 2020/2/19 10:43

- Fix query record index.  No.01 means the oldest record and the maximum one means the newest.  
- Add sensor high value fault. When adc sample value is too high, the device will trigger a fault alarm.