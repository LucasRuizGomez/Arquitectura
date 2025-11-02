#pragma once

#include "ray.hpp"     // Para Ray
#include "scene.hpp"   // Para Sphere y Cylinder
#include "vector.hpp"  // <-- Incluimos vector
#include <optional>
#include <string>

namespace render {

  // Contiene la información de una intersección
  struct HitRecord {
    float lambda{};  // Distancia 't' o 'λ' a lo largo del rayo
    vector point;    // <-- Ahora es un render::vector
    vector normal;   // <-- Ahora es un render::vector
    std::string material_name;
  };

  // ... (Las declaraciones de funciones siguen igual) ...

  std::optional<HitRecord> hit_sphere(Sphere const & s, Ray const & r, float lambda_min,
                                      float lambda_max);

  std::optional<HitRecord> hit_cylinder(Cylinder const & c, Ray const & r, float lambda_min,
                                        float lambda_max);

  std::optional<HitRecord> hit_scene(Scene const & scene, Ray const & r, float lambda_min,
                                     float lambda_max);

}  // namespace render
