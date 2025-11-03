#include "../include/renderer.hpp"  // Include the header file

// Add other necessary includes that were used by the function bodies
#include "../include/config.hpp"
#include "../include/hittable.hpp"
#include "../include/ray.hpp"
#include "../include/rng.hpp"
#include "../include/scene.hpp"
#include "../include/vector.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>
#include <sstream>
#include <stdexcept>  // For std::out_of_range
#include <string>

namespace render {

  // --- DEFINITIONS MOVED HERE ---

  vector parse_vector_from_string(std::string s) {
    std::istringstream iss(s);
    float x{}, y{}, z{};  // inicializamos
    if (!(iss >> x >> y >> z)) {
      std::cerr << "Error parseando vector desde string: [" << s << "]\n";
      return {1.F, 0.F, 1.F};  // Return default on error
    }
    return {x, y, z};
  }

  vector reflect(vector const & v_in, vector const & normal) {
    return v_in - 2.F * dot(v_in, normal) * normal;
  }

  std::optional<vector> refract(vector const & v_in_unit, vector const & normal,
                                float etai_over_etat) {
    float const cos_theta = std::min(dot(-v_in_unit, normal), 1.0F);
    vector r_out_perp     = etai_over_etat * (v_in_unit + cos_theta * normal);
    float desc_sq         = 1.0F - r_out_perp.length_squared();

    if (desc_sq < 0.0F) {
      return std::nullopt;  // Total internal reflection
    }
    vector r_out_para = -std::sqrtf(desc_sq) * normal;
    return r_out_perp + r_out_para;
  }

  // VENTANA DE PROYECCION  --> COMPROBAR SI EL CFG EN LOS MAIN ESTÁ BIEN
  Camera::Camera(Config const & cfg) {
    vector const P = parse_vector_from_string(cfg.camera_position);
    vector const D = parse_vector_from_string(cfg.camera_target);
    vector const N = parse_vector_from_string(cfg.camera_north);

    float const fov_vertical_grados = cfg.field_of_view;
    float const r_aspecto =
        static_cast<float>(cfg.aspect_ratio.first) / static_cast<float>(cfg.aspect_ratio.second);
    int const img_width_px  = cfg.image_width;
    int const img_height_px = static_cast<int>(static_cast<float>(img_width_px) / r_aspecto);

    m_camera_origin = P;

    vector const v_focal = P - D;                // VECTOR FOCAL
    float const d_focal  = v_focal.magnitude();  // DISTANCIA FOCAL
    float const alpha_rad =
        fov_vertical_grados *
        (std::numbers::pi_v<float> / 180.F);  // ANGULO DEL CAMPO DE VISION EN RADIANTES
    float const h_p =
        2.0F * std::tan(alpha_rad / 2.0F) * d_focal;  // ALTURA DE LA VENTANA DE PROYECCION
    float const w_p = h_p * r_aspecto;                // ANCHURA VENTANA DE PROYECCION

    // VECTORES DIRECTORES
    vector const v_focal_unit = v_focal.normalized();          // Dividir un vector por su magnitud
    vector const u_vec = cross(N, v_focal_unit).normalized();  // Normaliza un producto vectorial
    vector const v_vec = cross(v_focal_unit, u_vec);           // Producto Vectorial

    vector const p_h = w_p * u_vec;     // Vector Horizontal
    vector const p_v = h_p * (-v_vec);  // Vector Vertical

    m_delta_x = p_h / static_cast<float>(img_width_px);   // Vector Horizontal entre el ancho en px
    m_delta_y = p_v / static_cast<float>(img_height_px);  // Vector Vertical entre el alto en px

    m_origin_ventana = P - v_focal - 0.5F * (p_h + p_v) + 0.5F * (m_delta_x + m_delta_y);  // ORIGEN
  }

  Ray Camera::get_ray(float x_jit, float y_jit) const {
    // Aquí se estan creando los valores aleatorios? --> x_jit y y_jit --> Aqui se supone que se
    // está sumando ya la columna y la fila
    vector const pixel_center = m_origin_ventana + (x_jit * m_delta_x) + (y_jit * m_delta_y);

    // ESTA LOGICA NO ME QUEDA NADA CLARA --> PAGINA 12 PDF
    //"Crea un nuevo rayo de luz que comience en la posición de mi cámara (m_camera_origin)
    // y apunte en la dirección que va desde la cámara hacia el punto pixel_center."
    return {m_camera_origin, (pixel_center - m_camera_origin).normalized()};
  }

  vector ray_color(Ray const & r, render::Scene const & scene, RenderContext & ctx, int depth) {
    // PROFUNDIDAD <= 0, NO HAY CONTRIBUCION AL COLOR
    if (depth <= 0) {
      return {0.F, 0.F, 0.F};
    }

    float const infinity = std::numeric_limits<float>::infinity();

    // CONTRIBUCION AL COLOR CORRESPONDIENTE CON LA INTERSECCION
    if (auto hit = render::hit_scene(scene, r, 0.001F, infinity)) {
      try {
        Material const & mat = scene.materials.at(hit->material_name);
        vector albedo;

        // --- LÓGICA DE MATERIAL 'MATTE' ---
        if (mat.type == "matte" and mat.params.size() >= 3) {
          albedo                  = {mat.params[0], mat.params[1], mat.params[2]};
          vector bounce_direction = hit->normal + ctx.material_rng.random_in_unit_sphere();

          if (std::fabs(bounce_direction.x()) < 1e-8F and
              std::fabs(bounce_direction.y()) < 1e-8F and
              std::fabs(bounce_direction.z()) < 1e-8F)
          {
            bounce_direction = hit->normal;
          }
          Ray bounced_ray(hit->point, bounce_direction.normalized());
          return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);
        }

        // --- LÓGICA DE MATERIAL 'METAL' ---
        if (mat.type == "metal" and mat.params.size() >= 4) {
          albedo           = {mat.params[0], mat.params[1], mat.params[2]};
          float roughness  = mat.params[3];
          vector reflected = reflect(r.direction(), hit->normal);
          vector bounce_direction =
              (reflected + roughness * ctx.material_rng.random_in_unit_sphere());
          Ray bounced_ray(hit->point, bounce_direction);

          /*           if (dot(bounced_ray.direction(), hit->normal) <= 0.F) {
                      return {0.F, 0.F, 0.F};
                    }  //////// ESTE IF ESTABA COMENTADO */
          return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);
        }

        // --- LÓGICA DE MATERIAL 'REFRACTIVE' ---
        if (mat.type == "refractive" and mat.params.size() >= 1) {
          albedo                       = {1.0F, 1.0F, 1.0F};
          float ior                    = mat.params[0];
          bool const front_face        = dot(r.direction(), hit->normal) < 0.F;
          vector const normal          = front_face ? hit->normal : -hit->normal;
          float const refraction_ratio = front_face ? (1.0F / ior) : ior;
          vector const unit_direction  = r.direction();

          auto refracted_opt = refract(unit_direction, normal, refraction_ratio);

          vector direction;
          if (!refracted_opt) {
            direction = reflect(unit_direction, normal);
          } else {
            direction = *refracted_opt;
          }
          Ray bounced_ray(
              hit->point,
              direction);  // (normal * ... ) lo he añadido para ver si arregla refractive
          return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);
        }
      } catch (std::out_of_range const &) {
        std::cerr << "Error: Material no encontrado: " << hit->material_name << '\n';
      }
      return {1.F, 0.F, 1.F};  // Error: Pink
    }

    // --- Background Color ---
    float m = (r.direction().y() + 1.0F) * 0.5F;

    return (1.0F - m) * ctx.bg_light + m * ctx.bg_dark;

    /* return (1.0F - m) * ctx.bg_dark +
           m * ctx.bg_light;  // NO SE TIO ESTO CREO Q ESTA BIEN ASI PERO SINO VOLVER A INVERTIR */
  }

}  // namespace render
