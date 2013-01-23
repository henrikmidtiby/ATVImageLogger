if(USE_DATALOGGER) #If logging data
  set(DATALOGGER_SRCS datalogger.cpp)
  add_definitions(-DUSE_DATALOGGER)
endif(USE_DATALOGGER)

set(VisionSpray_SRCS ${DATALOGGER_SRCS} makerelativecoordinatesworker.cpp calculategaussianfeaturesworker.cpp algoritm.cpp VisionSpray.cpp main.cpp cqtopencvviewergl.cpp spraytimekeeper.cpp)
