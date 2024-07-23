cmake_minimum_required(VERSION 3.28)
project(fluid_sim)

set(OpenGlLinkers -lglut -lGLU -lGL) # setting all the Glut libraries as one variable.

set(CMAKE_CXX_STANDARD 17)

add_executable(fluid_sim main.cpp)

target_link_libraries(fluid_sim ${OpenGlLinkers}) # linking opengl libraries to the project