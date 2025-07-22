# CMakeList.txt: proyecto de CMake para Transpiler, incluya el origen y defina
# la lógica específica del proyecto aquí.
#

# Agregue un origen al ejecutable de este proyecto.
add_executable (Transpiler "Transpiler.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Transpiler PROPERTY CXX_STANDARD 20)
endif()

# TODO: Agregue pruebas y destinos de instalación si es necesario.
