# mdam (Monotoring Dynamically Allocated Memory)

## About

'mdam' is tool monitoring dynamically allocated memory. If you load 'mdam.so' with LD_PRELOAD, you can find memory leak in C language program compiled with gcc.

## Build
Please clone this repository and execute the following command.
```
$ cd mdam
$ make
```
If make is succeed, a shared library nemed  'mdam.so'  is created.

## Usage
### 1. Execute program with mdam.so

Please configure mdam.so to LD_PRELOAD and execute program which you want to monitor memory leak.
```
$ LD_PRELOAD=mdam.so <executable file>
```
### 2. Finish program and display result
When the program finishes, the result memory leak detection result is displayed.

If memory leak is not detected, like following message is displayed.
```
Any memory leak is not detected !
```
Otherwise, like following message is displayed.
```
----------------------------------------------------------
Address           Size (byte)   Return address    Caller
----------------------------------------------------------
0x5596930c3260    15            0x5596916d38fc    main
0x5596930c32d0    30            0x5596916d390a    main
0x5596930c34b0    512           0x5596916d3932    main
0x5596930c36e0    552           0x7efe7f956e4a    fopen
```
The meaning of each item is follwing.

| Item | Description |
----|----
| Address        | Address of leaked memory. |
| Size (byte)    | Byte size of leaked memory. | 
| Return address | Address of instruction which occured memory leak.|
| Caller         | Name of function which allocate the leaked memory. |

**Notice : 'Caller' is displayed correctly only when target program is compiled with '-rdynamic' option. Otherwise displayed '(none)'**

### 3. Change the output destination
By default, the memory leak detection result is displayed console. But if you set file path to 'MDAM_LOG', mdam is writes the result to the specified file.

For example, set "leak.log" to MDAM_LOG.
```
$ MDAM_LOG=leak.log LD_PRELOAD=mdam.so <executable file>
```
The memory leak detection result is written in 'leak.log'.
```
$ cat leak.log
----------------------------------------------------------
Address           Size (byte)   Return address    Caller
----------------------------------------------------------
0x5596930c3260    15            0x5596916d38fc    main
0x5596930c32d0    30            0x5596916d390a    main
0x5596930c34b0    512           0x5596916d3932    main
0x5596930c36e0    552           0x7efe7f956e4a    fopen
```
