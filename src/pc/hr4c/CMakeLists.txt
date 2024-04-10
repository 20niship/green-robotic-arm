find_package(Threads REQUIRED)
find_package(GLEW REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

set(HR4C_INCLUDES 
  /opt/homebrew/include
  ${EIGEN3_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}
  ${GLFW_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}

  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cpptoml/include
  ${CMAKE_CURRENT_SOURCE_DIR}/implot
  ${CMAKE_CURRENT_SOURCE_DIR}/simpleson
  ${CMAKE_CURRENT_SOURCE_DIR}/cpptoml/include
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/
  ${CMAKE_CURRENT_SOURCE_DIR}/IconFontCppHeaders
  ${PROJECT_SOURCE_DIR}
)

link_directories(
  /opt/homebrew/lib
)

set(ALL_LINKS 
  Threads::Threads
  modbus
  ${GLFW_LIBRARIES}
  glfw
  glew
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

set(IMPLOT_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/implot)

file(GLOB hr4c_CORE_SOURCE 
  ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/fsensor/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/loguru/loguru.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphics/*.cpp
  ${IMPLOT_PREFIX}/implot.cpp
  ${IMPLOT_PREFIX}/implot_demo.cpp
  ${IMPLOT_PREFIX}/implot_items.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/simpleson/json.cpp 

  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(hr4c-core ${hr4c_CORE_SOURCE})
target_link_libraries(hr4c-core PUBLIC ${ALL_LINKS})
target_include_directories(hr4c-core PUBLIC ${HR4C_INCLUDES})

