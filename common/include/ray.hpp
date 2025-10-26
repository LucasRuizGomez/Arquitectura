#pragma once
#include "vector.hpp"

namespace render {

  class Ray {
  public:
    Ray() = default;

    // El constructor se asegura de que la dirección esté normalizada
    Ray(vector const & origin, vector const & direction)
        : m_origin(origin), m_direction(direction.normalized()) { }

    [[nodiscard]] vector origin() const { return m_origin; }

    [[nodiscard]] vector direction() const { return m_direction; }

    // Calcula el punto en el rayo: P(λ) = Origen + λ * Dirección
    [[nodiscard]] vector at(float lambda) const { return m_origin + lambda * m_direction; }

  private:
    vector m_origin;     // <-- Ahora es un render::vector
    vector m_direction;  // <-- Ahora es un render::vector
  };

}  // namespace render
