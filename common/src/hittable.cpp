#include "../include/hittable.hpp"
#include "../include/vector.hpp"
#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>
using std::vector;
#include "../include/scene.hpp"

namespace render {

  // Constante de precisión para evitar problemas con floats
  constexpr float epsilon = 0.00001F;  // Un poco más pequeño

  // --- INTERSECCIÓN CON ESFERA (Sin cambios) ---
  std::optional<HitRecord> hit_sphere(Sphere const & s, Ray const & r, float lambda_min,
                                      float lambda_max) {
    // M: comprobaciones básicas de validez del objeto
    if (s.r <= 0.0F) {
      throw std::runtime_error("Error: Invalid sphere radius (must be > 0)");
    }
    if (s.material.empty()) {
      throw std::runtime_error("Error: Sphere material name is empty");
    }

    vector const center(s.cx, s.cy, s.cz);
    vector const rc          = r.origin() - center;
    float const A            = r.direction().length_squared();  // 1.0F
    float const B            = 2.0F * dot(rc, r.direction());
    float const C            = rc.length_squared() - s.r * s.r;
    float const discriminant = B * B - 4.F * A * C;

    // M: comprobación del discriminante
    if (std::isnan(discriminant) or std::isinf(discriminant)) {
      throw std::runtime_error("Error: Sphere discriminant produced NaN or INF");
    }

    if (discriminant < 0.F) {
      return std::nullopt;  // No hay intersección
    }

    float sqrt_discriminant = std::sqrtf(discriminant);
    float lambda            = (-B - sqrt_discriminant) / (2.0F * A);
    if (lambda < lambda_min or lambda > lambda_max) {
      lambda = (-B + sqrt_discriminant) / (2.0F * A);
      if (lambda < lambda_min or lambda > lambda_max) {
        return std::nullopt;
      }
    }

    HitRecord rec;
    rec.lambda        = lambda;
    rec.point         = r.at(lambda);
    rec.normal        = (rec.point - center).normalized();
    rec.material_name = s.material;

    // M: comprobación final de normal
    if (std::isnan(rec.normal.x()) or std::isnan(rec.normal.y()) or std::isnan(rec.normal.z())) {
      throw std::runtime_error("Error: Sphere normal computed as NaN");
    }

    return rec;
  }

  // --- INTERSECCIÓN CON CILINDRO (¡CORREGIDA!) ---
  std::optional<HitRecord> hit_cylinder(Cylinder const & c, Ray const & r, float lambda_min,
                                        float lambda_max) {
    // M: comprobaciones de entrada
    if (c.r <= 0.0F) {
      throw std::runtime_error("Error: Invalid cylinder radius (must be > 0)");
    }
    if (c.material.empty()) {
      throw std::runtime_error("Error: Cylinder material name is empty");
    }
    if (c.ax == 0 and c.ay == 0 and c.az == 0) {
      throw std::runtime_error("Error: Cylinder axis vector cannot be zero-length");
    }

    vector const C = vector(c.cx, c.cy, c.cz);
    vector const axis_vec(c.ax, c.ay, c.az);          // 1. El vector de eje (del archivo)
    vector const axis       = axis_vec.normalized();  // 2. El eje NORMALIZADO (para cálculos)
    float const height      = axis_vec.magnitude();   // 3. La altura es la magnitud (del PDF)
    float const half_height = height / 2.0F;          // 4. La media altura
    float const radius_sq   = c.r * c.r;

    // M: comprobación de NaN o altura
    if (std::isnan(height) or height <= 0.0F) {
      throw std::runtime_error("Error: Cylinder height is invalid or zero");
    }

    vector const OC = r.origin() - C;
    vector const DR = r.direction();

    float const dr_dot_axis = dot(DR, axis);
    float const oc_dot_axis = dot(OC, axis);

    // --- 1. CÁLCULO DEL CUERPO (Infinito) ---
    float A      = dot(DR, DR) - dr_dot_axis * dr_dot_axis;
    float B      = 2.0F * (dot(DR, OC) - dr_dot_axis * oc_dot_axis);
    float C_body = dot(OC, OC) - oc_dot_axis * oc_dot_axis - radius_sq;

    float discriminant = B * B - 4.F * A * C_body;

    // M: comprobación de discriminante
    if (std::isnan(discriminant) or std::isinf(discriminant)) {
      throw std::runtime_error("Error: Cylinder discriminant produced NaN or INF");
    }

    if (discriminant < 0.F) {
      return std::nullopt;  // No golpea el cilindro infinito
    }

    float sqrt_discriminant              = std::sqrtf(discriminant);
    float min_lambda                     = lambda_max;
    std::optional<HitRecord> closest_hit = std::nullopt;

    // --- 2. LÓGICA DE COMPROBACIÓN (El Bug) ---
    // Esta lambda comprueba si un 'hit' (en 'lambda')
    // está dentro de la altura finita del cilindro.

    auto check_body_hit = [&](float lambda) {
      if (lambda < lambda_min or lambda > min_lambda) {
        return;  // Fuera del rango visible
      }

      // 1. Calcula el punto de impacto
      vector const Q = r.at(lambda);

      // 2. Calcula la altura del impacto usando el 'axis' normalizado
      float const hit_height = dot(Q - C, axis);

      // 3. Comprueba si está dentro de la altura (usando 'half_height' de la línea 60)
      if (std::fabs(hit_height) > half_height) {
        return;  // Golpeó el tubo infinito, pero fuera de la altura finita
      }

      // Si hemos llegado aquí, el 'hit' es válido
      min_lambda = lambda;
      HitRecord rec;
      rec.lambda = lambda;
      rec.point  = Q;

      // 4. Calcula la normal (simplificado)
      rec.normal = (Q - C - hit_height * axis).normalized();

      // M: comprobación de normal inválida
      if (std::isnan(rec.normal.x()) or std::isnan(rec.normal.y()) or std::isnan(rec.normal.z())) {
        throw std::runtime_error("Error: Cylinder normal computed as NaN");
      }

      // Invertimos la normal en caso especifico
      if (dot(r.direction(), rec.normal) > 0.F) {
        // Si el producto punto es positivo, la normal está apuntando
        // en la misma dirección que el rayo, lo cual está MAL.
        rec.normal = -rec.normal;
      }

      rec.material_name = c.material;
      closest_hit       = rec;
    };

    // Comprobamos las dos soluciones de la cuadrática
    check_body_hit((-B - sqrt_discriminant) / (2.0F * A));
    check_body_hit((-B + sqrt_discriminant) / (2.0F * A));

    // --- 3. COMPROBACIÓN DE LAS TAPAS ---
    auto check_cap_hit = [&](vector const & cap_center, vector const & normal) {
      float dr_dot_normal = dot(r.direction(), normal);

      // Evita división por cero (rayo paralelo a la tapa)
      if (std::fabs(dr_dot_normal) < 0.0001F) {
        return;
      }

      float lambda = dot(cap_center - r.origin(), normal) / dr_dot_normal;
      if (lambda < lambda_min or lambda > min_lambda) {
        return;  // Fuera de rango
      }

      vector const Q = r.at(lambda);

      // Comprueba si el punto de impacto está DENTRO del radio
      if ((Q - cap_center).length_squared() <= radius_sq) {
        min_lambda = lambda;
        HitRecord rec;
        rec.lambda = lambda;
        rec.point  = Q;
        rec.normal = normal;

        // M: validación de normal numérica
        if (std::isnan(rec.normal.x()) or std::isnan(rec.normal.y()) or std::isnan(rec.normal.z()))
        {
          throw std::runtime_error("Error: Cap normal computed as NaN");
        }

        // SOLUCION PROBLEMA TAPAS OSCURAS
        if (dot(r.direction(), rec.normal) > 0.F) {
          rec.normal = -rec.normal;
        }

        rec.material_name = c.material;
        closest_hit       = rec;
      }
    };

    // Calcula los centros de las tapas superior e inferior
    vector const cap_top_center    = C + axis * half_height;
    vector const cap_bottom_center = C - axis * half_height;

    // Comprueba la tapa superior (normal = eje)
    check_cap_hit(cap_top_center, axis);
    // Comprueba la tapa inferior (normal = -eje)
    check_cap_hit(cap_bottom_center, -axis);

    return closest_hit;
  }

  // --- INTERSECCIÓN CON LA ESCENA (PDF Sec 3.3) ---
  std::optional<HitRecord> hit_scene(Scene const & scene, Ray const & r, float lambda_min,
                                     float lambda_max) {
    std::optional<HitRecord> closest_hit = std::nullopt;
    float closest_so_far                 = lambda_max;

    // 1. Comprobar todas las esferas
    for (auto const & sphere : scene.spheres) {
      auto hit_record = hit_sphere(sphere, r, lambda_min, closest_so_far);

      if (hit_record) {
        closest_so_far = hit_record->lambda;
        closest_hit    = hit_record;
      }
    }

    // 2. Comprobar todos los cilindros
    for (auto const & cylinder : scene.cylinders) {
      auto hit_record = hit_cylinder(cylinder, r, lambda_min, closest_so_far);

      if (hit_record) {
        closest_so_far = hit_record->lambda;
        closest_hit    = hit_record;
      }
    }

    return closest_hit;
  }

}  // namespace render
