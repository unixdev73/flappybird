set(STAGING_DIR ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME})
set(EXECUTABLE_NAME run)

set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Adding optimization at level 3")
	add_compile_options(-O3)
else()
	message(STATUS "Adding debug symbols")
	add_compile_options(-O0 -g)
endif()

add_executable(${EXECUTABLE_NAME} main.cpp
	application.cpp resource.cpp button.cpp animation.cpp)
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE SFML::Graphics SFML::Audio SFML::Network)
target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${STAGING_DIR})

file(MAKE_DIRECTORY ${STAGING_DIR}/font)
file(MAKE_DIRECTORY ${STAGING_DIR}/img)

file(COPY ${CMAKE_SOURCE_DIR}/asset/font/ExoRegular.ttf DESTINATION ${STAGING_DIR}/font)
file(COPY ${CMAKE_SOURCE_DIR}/asset/img/BirdSprite.png DESTINATION ${STAGING_DIR}/img)

install(DIRECTORY ${STAGING_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX} USE_SOURCE_PERMISSIONS)
