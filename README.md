VisionSpray
===========

VisionSpray is a video based spray decision framework, developed for evaluating the performance of machine vision based decision systems used at the [University of Southern Denmark](http://www.sdu.dk). The framework glues together camera interfacing, nozzle control and timing as well as automated datalogging annotated by timestamps as well as optional gps.
It is the plan to include the algorithms in this repository as soon as they have been published in the scientific litterature.
The system is developed and tested on Linux (currently Ubuntu 12.04 x64), using a Basler [acA2000-50gc](http://www.baslerweb.com/products/ace.html?model=173) controlled through [Aravis](https://live.gnome.org/Aravis). The spray nozzles is controlled using a Advantech Adam [6052](http://www.advantech.com/products/ADAM-6052/mod_8E8D8D87-F30B-4F82-B77E-4E3822DFCA5D.aspx) I/O adapter, and a custom solenoid amplifier developed by [EKTOS](http://ektos.net/).
