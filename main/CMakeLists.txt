#configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/../config.h" @ONLY)

set(OpenCV_STATIC ON)
set(BUILD_SHARED_LIBS OFF)
find_package(OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(
    H-alfaSSed 
    	main.cpp
	ser.cpp
)
target_link_libraries(H-alfaSSed ${OpenCV_LIBS})

SET_TARGET_PROPERTIES(H-alfaSSed PROPERTIES
  LINK_FLAGS "-lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_stitching -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core"
)

#SET (CMAKE_EXE_LINKER_FLAGS "-static")

install(
	TARGETS H-alfaSSed
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
)
