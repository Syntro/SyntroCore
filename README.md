# SyntroCore

The core Syntro libraries and applications for running a Syntro cloud.

#### Applications

* SyntroCFS
* SyntroControl
* SyntroExec
* SyntroLog
* SyntroStore 

#### Libraries

* SyntroControlLib
* SyntroLib
* SyntroGUI

#### C++ Headers

* /usr/include/syntro

#### Package Config

* /usr/lib/pkgconfig/syntro.pc


### Build Dependencies

* Qt4 or Qt5 development libraries and headers
* pkgconfig

### Fetch

        git clone git://github.com/Syntro/SyntroCore.git


### Build (Linux)

        qmake 
        make 
        sudo make install


After the install step on Linux you will be able to build and run [SyntroLCam][1]
and [SyntroView][2].

There are VS2010 solution files for building SyntroCore binaries on Windows.
They are configured for use with Qt5 libraries, but the code is Qt4 compatible.

### Run

Instructions for running the various SyntroCore applications can be found on
the [Pansenti website][3].

[1]: https://github.com/Syntro/SyntroLCam
[2]: https://github.com/Syntro/SyntroView
[3]: http://www.pansenti.com/wordpress/?page_id=547

