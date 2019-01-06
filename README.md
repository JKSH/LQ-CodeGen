LQ Code Generator
=================
This project generates code for [LQ](https://github.com/JKSH/LQ-Bindings/)
(pronounced "Luke"), LabVIEW bindings for Qt.


Project Folders
---------------
* [data](data): Contains the specifications used to generate the binding code.
* [src](src): Contains the source code for the code generator itself.
* [templates](templates): Contains the template files that get populated by the
  binding code.
    - The templates also contains custom-written code (both LabVIEW and C++)
      that make the bindings work. Although these are not technically templates,
      they need to evolve closely with the code generator, which is why they are
      in this repo.


Usage
-----
The code generator software is intended to be run from their respective IDEs.

* C++: Open the [project file](src/Cpp/LQ-CodeGen-Cpp.pro) in [Qt Creator](https://doc.qt.io/qtcreator/)
  and build and run the program in-place.
* LabVIEW: Open the [project file](src/LabVIEW/LQ-CodeGen-LabVIEW.lvproj) in
  the LabVIEW development environment and run _Main.vi_.


System Requirements
-------------------
* NI LabVIEW 2014 or newer
* A C++11 compliant compiler (tested with Microsoft Visual C++ 2017)
* Qt 5.10 or newer


Copyright
---------
Copyright (c) 2019 Sze Howe Koh <<szehowe.koh@gmail.com>>

The LQ Code Generator is published under the [Mozilla Public License v2.0](LICENSE.MPLv2)
