#include "hittable.hpp"
#include "vector.hpp"
#include <algorithm>  // Para std::min, std::max
#include <cmath>      // Para std::sqrtf, std::fabs
#include <limits>

namespace render {

  // Constante de precisión para evitar problemas con floats
  constexpr float epsilon = 0.00001F;  // Un poco más pequeño

  // --- INTERSECCIÓN CON ESFERA (Sin cambios) ---
  std::optional<HitRecord> hit_sphere(Sphere const & s, Ray const & r, float lambda_min,
                                      float lambda_max) {
    vector const center(s.cx, s.cy, s.cz);
    vector rc          = r.origin() - center;
    float A            = r.direction().length_squared();  // 1.0F
    float B            = 2.0F * dot(rc, r.direction());
    float C            = rc.length_squared() - s.r * s.r;
    float discriminant = B * B - 4.F * A * C;

    if (discriminant < 0.F) {
      return std::nullopt;
    }

    float sqrt_discriminant = std::sqrtf(discriminant);
    float lambda            = (-B - sqrt_discriminant) / (2.0F * A);
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

  // --- INTERSECCIÓN CON CILINDRO (Versión de 10 Unidades con Guardia) ---
  std::optional<HitRecord> hit_cylinder(Cylinder const & c, Ray const & r, float lambda_min,
                                        float lambda_max) {
    // Parámetros del PDF (C, r, â, h)
    vector const center(c.cx, c.cy, c.cz);
    vector const axis_vec(c.ax, c.ay, c.az);
    float const radius      = c.r;
    float const radius_sq   = radius * radius;
    float const height      = 10.0F;  // Constante del PDF (Sec 3.4.2)
    float const half_height = height / 2.0F;

    if (axis_vec.length_squared() < epsilon) {
      return std::nullopt;
    }                                           // Eje inválido
    vector const axis = axis_vec.normalized();  // â

    vector const Cb = center - half_height * axis;  // Centro base
    vector const Ct = center + half_height * axis;  // Centro tapa superior

    std::optional<HitRecord> closest_hit = std::nullopt;
    float min_lambda                     = lambda_max;

    // --- 1. Cálculo de intersección con superficie curva (Tubo) ---
    vector const Or_minus_C  = r.origin() - center;
    float const dr_dot_axis  = dot(r.direction(), axis);
    float const OrC_dot_axis = dot(Or_minus_C, axis);

    float A = r.direction().length_squared() - dr_dot_axis * dr_dot_axis;
    float B = 2.0F * (dot(r.direction(), Or_minus_C) - dr_dot_axis * OrC_dot_axis);
    float C = Or_minus_C.length_squared() - OrC_dot_axis * OrC_dot_axis - radius_sq;

    // --- ¡¡NUEVA GUARDIA!! ---
    // Comprobar si el rayo es paralelo al eje del cilindro
    if (std::fabs(A) > epsilon) {
      // --- El rayo NO es paralelo, resolver ecuación cuadrática ---
      float discriminant = B * B - 4.F * A * C;
      if (discriminant >= 0.F) {  // Si hay intersección con el tubo infinito...
        float sqrt_disc = std::sqrtf(discriminant);

        auto check_body_hit = [&](float lam) {
          if (lam < lambda_min || lam > min_lambda) {
            return;
          }
          vector Q     = r.at(lam);
          float h_proj = dot(Q - center, axis);
          if (std::fabs(h_proj) <= half_height) {
            min_lambda = lam;
            HitRecord rec;
            rec.lambda        = lam;
            rec.point         = Q;
            rec.normal        = (Q - center - h_proj * axis).normalized();
            rec.material_name = c.material;
            closest_hit       = rec;
          }
        };
        check_body_hit((-B - sqrt_disc) / (2.0F * A));  // Raíz 1
        check_body_hit((-B + sqrt_disc) / (2.0F * A));  // Raíz 2
      }
    }
    // (Si A es cero, el rayo es paralelo. No puede golpear el cuerpo a menos
    // que C también sea 0, lo que es muy raro. Solo comprobamos las tapas).
    // --- FIN DE LA GUARDIA ---

    // --- 2. Cálculo de intersección con las Tapas (Planos) ---
    auto check_cap_hit = [&](vector const & cap_center, vector const & normal) {
      float dr_dot_normal = dot(r.direction(), normal);
      if (std::fabs(dr_dot_normal) < epsilon) {
        return;
      }  // Rayo paralelo a la tapa

      float lambda = dot(cap_center - r.origin(), normal) / dr_dot_normal;
      if (lambda < lambda_min || lambda > min_lambda) {
        return;
      }  // Fuera de rango

      vector Q = r.at(lambda);
      if ((Q - cap_center).length_squared() <= radius_sq) {
        min_lambda = lambda;
        HitRecord rec;
        rec.lambda        = lambda;
        rec.point         = Q;
        rec.normal        = normal;
        rec.material_name = c.material;
        closest_hit       = rec;
      }
    };

    check_cap_hit(Cb, -axis);  // Tapa inferior
    check_cap_hit(Ct, axis);   // Tapa superior

    return closest_hit;
  }

  // --- hit_scene (Sin cambios) ---
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
