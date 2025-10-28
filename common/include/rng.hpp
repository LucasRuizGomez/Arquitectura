#pragma once

#include "vector.hpp"
#include <random>

namespace render {

  // Un generador simple basado en el estándar de C++
  class RNG {
  public:
    // Se inicializa con la semilla del config.txt
    RNG(uint64_t seed) : gen(seed) { }

    // Devuelve un float aleatorio en [0, 1)
    float random_float() { return dist(gen); }

    // Devuelve un vector aleatorio en [0, 1)
    render::vector random_vector() {
      return {random_float(), random_float(), random_float()};  // He cambiado esto por si no va
    }

    // Devuelve un vector aleatorio dentro de una esfera unitaria
    // (PDF Sec 3.5.1, Ec. 34 - el 'v_alea')
    render::vector random_in_unit_sphere() {
      while (true) {
        // Genera un punto aleatorio en un cubo de [-1, +1]
        auto p = render::vector(random_float() * 2.F - 1.F, random_float() * 2.F - 1.F,
                                random_float() * 2.F - 1.F);

        // Si el punto está fuera de la esfera, lo descartamos y probamos de nuevo
        if (p.length_squared() >= 1.F) {
          continue;
        }

        // Devolvemos el vector normalizado
        return p;  // MATERIAL MATE --> ANTES ERA return p.normalized();
      }
    }

  private:
    std::mt19937_64 gen;  // Generador Mersenne Twister
    std::uniform_real_distribution<float> dist{0.0F, 1.0F};
  };

}  // namespace render
