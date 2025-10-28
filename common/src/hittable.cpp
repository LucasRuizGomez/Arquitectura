#include "../include/hittable.hpp"
#include "../include/vector.hpp"
#include <algorithm>  // Para std::min, std::max
#include <cmath>      // Para std::sqrtf, std::fabs
#include <iostream>
#include <limits>
#include <optional>  // <-- Add this line
#include <vector>
using std::vector;  // Now you can just write 'vector'

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
    return rec;
  }

  // --- INTERSECCIÓN CON CILINDRO (Versión de 10 Unidades con Guardia) ---
  // En common/hittable.cpp

  // En common/hittable.cpp

  // --- INTERSECCIÓN CON CILINDRO (¡CORREGIDA!) ---
  std::optional<HitRecord> hit_cylinder(Cylinder const & c, Ray const & r, float lambda_min,
                                        float lambda_max) {
    vector const C = vector(c.cx, c.cy, c.cz);
    vector const axis_vec(c.ax, c.ay, c.az);          // 1. El vector de eje (del archivo)
    vector const axis       = axis_vec.normalized();  // 2. El eje NORMALIZADO (para cálculos)
    float const height      = axis_vec.magnitude();   // 3. La altura es la magnitud (del PDF)
    float const half_height = height / 2.0F;          // 4. La media altura
    float const radius_sq   = c.r * c.r;

    /* float const half_height = 5.0F;  // PDF Sec 3.4.2 */
    /* float const half_height = c.height / 2.0F;  // Usamos la altura del cilindro */
    // Centros de las tapas inferior (Cb) y superior (Ct)
    /* vector const Cb = C - half_height * axis; */
    /* vector const Ct = C + half_height * axis; */

    vector const OC = r.origin() - C;
    vector const DR = r.direction();

    float const dr_dot_axis = dot(DR, axis);
    float const oc_dot_axis = dot(OC, axis);

    // --- 1. CÁLCULO DEL CUERPO (Infinito) ---
    float A      = dot(DR, DR) - dr_dot_axis * dr_dot_axis;
    float B      = 2.0F * (dot(DR, OC) - dr_dot_axis * oc_dot_axis);
    float C_body = dot(OC, OC) - oc_dot_axis * oc_dot_axis - radius_sq;

    float discriminant = B * B - 4.F * A * C_body;

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
      if (lambda < lambda_min || lambda > min_lambda) {
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

      //
      //
      //
      //
      // Invertimos la normal en caso especifico
      //

      if (dot(r.direction(), rec.normal) > 0.F) {
        // Si el producto punto es positivo, la normal está apuntando
        // en la misma dirección que el rayo, lo cual está MAL.
        rec.normal = -rec.normal;
      }

      //
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
      /* vector Q = r.at(lambda); */
      // Comprueba si el punto de impacto está DENTRO del radio
      if ((Q - cap_center).length_squared() <= radius_sq) {
        min_lambda = lambda;
        HitRecord rec;
        rec.lambda = lambda;
        rec.point  = Q;

        //
        //
        //
        rec.normal = normal;

        // SOLUCION PROBLEMA TAPAS OSCURAS
        //
        //
        if (dot(r.direction(), rec.normal) > 0.F) {
          rec.normal = -rec.normal;
        }
        //
        //
        rec.material_name = c.material;
        closest_hit       = rec;
      }
    };

    /* check_cap_hit(Cb, -axis);  // Tapa inferior (normal hacia -axis)
    check_cap_hit(Ct, axis);   // Tapa superior (normal hacia +axis) */

    /* float const half_height = c.height / 2.0F;  // ESTO PUEDE CAUSAR PROBLEMAS */

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
    // 'closest_so_far' guardará el lambda máximo para las siguientes
    // comprobaciones, como pide el PDF.
    float closest_so_far = lambda_max;

    // 1. Comprobar todas las esferas
    // Iteramos sobre el vector de esferas de la escena
    for (auto const & sphere : scene.spheres) {
      // Llamamos a hit_sphere, pero usamos 'closest_so_far' como límite superior.
      auto hit_record = hit_sphere(sphere, r, lambda_min, closest_so_far);

      if (hit_record) {
        // Si golpeamos una esfera, actualizamos nuestro límite superior.
        // Cualquier objeto futuro debe estar MÁS CERCA que este.
        closest_so_far = hit_record->lambda;
        closest_hit    = hit_record;  // Guardamos este golpe como el mejor
      }
    }

    // 2. Comprobar todos los cilindros
    // Iteramos sobre el vector de cilindros de la escena
    for (auto const & cylinder : scene.cylinders) {
      // Pasamos 'closest_so_far', que puede haber sido acortado por una esfera.
      auto hit_record = hit_cylinder(cylinder, r, lambda_min, closest_so_far);

      if (hit_record) {
        // Si golpeamos un cilindro que está aún más cerca,
        // actualizamos el límite y lo guardamos.
        closest_so_far = hit_record->lambda;
        closest_hit    = hit_record;
      }
    }

    // Devolvemos el golpe más cercano de todos (o nullopt si no hubo ninguno)
    return closest_hit;
  }

}  // namespace render
