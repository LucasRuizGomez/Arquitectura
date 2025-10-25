#include "hittable.hpp"
#include "vector.hpp"  // <-- Asegúrate de que está incluido
#include <limits>

namespace render {

  std::optional<HitRecord> hit_sphere(Sphere const & s, Ray const & r, float lambda_min,
                                      float lambda_max) {
    // --- CORREGIDO ---
    // Creamos un 'render::vector' directamente desde los 'float' de la esfera
    vector const center(s.cx, s.cy, s.cz);

    // Ahora usamos los operadores de 'render::vector'
    vector rc = r.origin() - center;
    float A   = r.direction().length_squared();  // Es 1.0F
    float B   = 2.0F * dot(rc, r.direction());
    float C   = rc.length_squared() - s.r * s.r;

    float discriminant = B * B - 4.F * A * C;

    if (discriminant < 0.F) {
      return std::nullopt;
    }

    float sqrt_discriminant = std::sqrtf(discriminant);  // Usar sqrtf

    float lambda = (-B - sqrt_discriminant) / (2.0F * A);
    if (lambda < lambda_min || lambda > lambda_max) {
      lambda = (-B + sqrt_discriminant) / (2.0F * A);
      if (lambda < lambda_min || lambda > lambda_max) {
        return std::nullopt;
      }
    }

    HitRecord rec;
    rec.lambda        = lambda;
    rec.point         = r.at(lambda);
    rec.normal        = (rec.point - center).normalized();
    rec.material_name = s.material;

    return rec;
  }

  // (Esto sigue igual, con el 'maybe_unused' para que compile)
  std::optional<HitRecord> hit_cylinder([[maybe_unused]] Cylinder const & c,
                                        [[maybe_unused]] Ray const & r,
                                        [[maybe_unused]] float lambda_min,
                                        [[maybe_unused]] float lambda_max) {
    return std::nullopt;
  }

  std::optional<HitRecord> hit_scene(Scene const & scene, Ray const & r, float lambda_min,
                                     float lambda_max) {
    std::optional<HitRecord> closest_hit = std::nullopt;
    float closest_so_far                 = lambda_max;

    for (auto const & sphere : scene.spheres) {
      if (auto hit = hit_sphere(sphere, r, lambda_min, closest_so_far)) {
        closest_so_far = hit->lambda;
        closest_hit    = hit;
      }
    }

    for (auto const & cylinder : scene.cylinders) {
      if (auto hit = hit_cylinder(cylinder, r, lambda_min, closest_so_far)) {
        closest_so_far = hit->lambda;
        closest_hit    = hit;
      }
    }

    return closest_hit;
  }

}  // namespace render
