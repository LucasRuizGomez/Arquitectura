#include "../include/hittable.hpp"
#include "../include/vector.hpp"
// #include <vector>
// using std::vector;
#include "../include/scene.hpp"

#include <cmath>

// #include <iomanip>
// #include <iostream>

namespace render {

  // Constante de precisión para evitar problemas con floats
  constexpr float epsilon = 0.00001F;  // Un poco más pequeño

  // INTERSECCIÓN CON ESFERA
  std::optional<HitRecord> hit_sphere(Sphere const & s, Ray const & r, float lambda_min,
                                      float lambda_max) {
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
    if (std::isnan(discriminant) or std::isinf(discriminant)) {
      throw std::runtime_error("Error: Sphere discriminant produced NaN or INF");
    }
    if (discriminant < 0.F) {
      return std::nullopt;  // No hay intersección
    }
    float const sqrt_discriminant = std::sqrtf(discriminant);
    float lambda                  = (-B - sqrt_discriminant) / (2.0F * A);
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
    if (std::isnan(rec.normal.x()) or std::isnan(rec.normal.y()) or std::isnan(rec.normal.z())) {
      throw std::runtime_error("Error: Sphere normal computed as NaN");
    }
    if (dot(r.direction(), rec.normal) > 0.F) {
      rec.normal = -rec.normal;
    }
    return rec;
  }

  namespace {  // Namespace anonimo

    struct CylinderHitTest {
      Ray r;
      vector C;
      vector axis;
      float height;
      float half_height;
      float radius_sq;
      float lambda_min;
      std::string material_name;
      float min_lambda;
      std::optional<HitRecord> closest_hit;

      // CONSTRUCTOR
      CylinderHitTest(Cylinder const & c, Ray const & ray, float l_min, float l_max)
          : r(ray),  // Copia el rayo
            C(c.cx, c.cy, c.cz), axis(vector(c.ax, c.ay, c.az).normalized()),
            height(vector(c.ax, c.ay, c.az).magnitude()), half_height(height / 2.0F),
            radius_sq(c.r * c.r), lambda_min(l_min), material_name(c.material), min_lambda(l_max),
            closest_hit(std::nullopt)  // <-- Inicializa el 'optional'
      {
        // El cuerpo del constructor SOLO valida
        if (std::isnan(height) or height <= 0.0F) {
          throw std::runtime_error("Error: Cylinder height is invalid or zero");
        }
      }

      void check_body_hit(float lambda) {
        if (lambda < lambda_min or lambda > min_lambda) {
          return;
        }
        vector const Q         = r.at(lambda);
        float const hit_height = dot(Q - C, axis);

        if (std::fabs(hit_height) > half_height) {
          return;
        }

        min_lambda = lambda;
        HitRecord rec;
        rec.lambda = lambda;
        rec.point  = Q;
        rec.normal = (Q - C - hit_height * axis);

        if (std::isnan(rec.normal.x()) or std::isnan(rec.normal.y()) or std::isnan(rec.normal.z()))
        {
          throw std::runtime_error("Error: Cylinder Normal computed as Nan");
        }
        if (dot(r.direction(), rec.normal) > 0.F) {
          rec.normal = -rec.normal;
        }
        rec.material_name = material_name;
        closest_hit       = rec;
      }

      void check_cap_hit(vector const & cap_center, vector const & normal) {
        float dr_dot_normal = dot(r.direction(), normal);

        if (std::fabs(dr_dot_normal) < 0.0001F) {
          return;
        }
        float lambda = dot(cap_center - r.origin(), normal) / dr_dot_normal;
        if (lambda < lambda_min or lambda > min_lambda) {
          return;
        }
        vector const Q = r.at(lambda);
        if ((Q - cap_center).length_squared() <= radius_sq) {
          min_lambda = lambda;
          HitRecord rec;
          rec.lambda = lambda;
          rec.point  = Q;
          rec.normal = normal;

          if (std::isnan(rec.normal.x()) or
              std::isnan(rec.normal.y()) or
              std::isnan(rec.normal.z()))
          {
            throw std::runtime_error("Error: Cap normal computed as NaN");
          }
          if (dot(r.direction(), rec.normal) > 0.F) {
            rec.normal = -rec.normal;
          }
          rec.material_name = material_name;
          closest_hit       = rec;
        }
      }
    };

  }  // namespace

  //  FUNCIÓN hit_cylinder
  std::optional<HitRecord> hit_cylinder(Cylinder const & c, Ray const & r, float lambda_min,
                                        float lambda_max) {
    // Validaciones de entrada
    if (c.r <= 0.0F) {
      throw std::runtime_error("Error: Invalid cylinder radius (must be > 0)");
    }
    if (c.material.empty()) {
      throw std::runtime_error("Error: Cylinder material name is empty");
    }
    if (c.ax == 0 and c.ay == 0 and c.az == 0) {
      throw std::runtime_error("Error: Cylinder axis vector cannot be zero-length");
    }
    CylinderHitTest test_ctx(c, r, lambda_min, lambda_max);
    vector const OC         = r.origin() - test_ctx.C;
    vector const DR         = r.direction();
    float const dr_dot_axis = dot(DR, test_ctx.axis);
    float const oc_dot_axis = dot(OC, test_ctx.axis);
    float A                 = dot(DR, DR) - dr_dot_axis * dr_dot_axis;
    float B                 = 2.0F * (dot(DR, OC) - dr_dot_axis * oc_dot_axis);
    float C_body            = dot(OC, OC) - oc_dot_axis * oc_dot_axis - test_ctx.radius_sq;
    float discriminant      = B * B - 4.F * A * C_body;
    if (std::isnan(discriminant) or std::isinf(discriminant)) {
      throw std::runtime_error("Error: Cylinder discriminant produced NaN or INF");
    }
    if (discriminant >= 0.F) {
      float sqrt_discriminant = std::sqrtf(discriminant);

      if (std::fabs(A) > epsilon) {
        test_ctx.check_body_hit((-B - sqrt_discriminant) / (2.0F * A));
        test_ctx.check_body_hit((-B + sqrt_discriminant) / (2.0F * A));
      }
    }
    vector const cap_top_center    = test_ctx.C + test_ctx.axis * test_ctx.half_height;
    vector const cap_bottom_center = test_ctx.C - test_ctx.axis * test_ctx.half_height;

    test_ctx.check_cap_hit(cap_top_center, test_ctx.axis);
    test_ctx.check_cap_hit(cap_bottom_center, -test_ctx.axis);
    return test_ctx.closest_hit;
  }

  std::optional<HitRecord> hit_scene(Scene const & scene, Ray const & r, float lambda_min,
                                     float lambda_max) {
    std::optional<HitRecord> closest_hit = std::nullopt;
    float closest_so_far                 = lambda_max;

    // Comprobar todas las esferas
    for (auto const & sphere : scene.spheres) {
      auto hit_record = hit_sphere(sphere, r, lambda_min, closest_so_far);

      if (hit_record) {
        closest_so_far = hit_record->lambda;
        closest_hit    = hit_record;
      }
    }

    // Comprobar todos los cilindros
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
