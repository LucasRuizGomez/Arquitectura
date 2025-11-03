#include "../include/vector.hpp"

// La mayoría de las implementaciones se han movido a 'vector.hpp'
// como 'inline' para un mejor rendimiento.
// Dejamos este archivo .cpp para que el enlazador
// y CMake estén contentos.

namespace render {

  // Ya no usamos 'magnitude()', usamos 'length()',
  // pero podemos dejar esto aquí.
  // Opcionalmente, puedes borrar este archivo
  // y quitarlo de 'common/CMakeLists.txt'.

}  // namespace render
