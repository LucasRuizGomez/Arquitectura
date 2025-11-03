/**
 * @file renderer.cpp
 * @brief Implementa las funciones principales del motor de renderizado.
 *
 * Contiene las rutinas de cálculo de rayos, reflexión, refracción y color,
 * así como la configuración de la cámara y su proyección sobre el plano de imagen.
 */

#include "../include/renderer.hpp"

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

  /**
   * @brief Convierte una cadena con tres componentes numéricas en un vector 3D.
   *
   * Ejemplo de formato válido: `"1.0 0.5 -2.3"`.
   * Si el formato es incorrecto, se devuelve un vector magenta por defecto (1, 0, 1)
   * y se informa por consola estándar.
   *
   * @param s Cadena con los valores del vector separados por espacios.
   * @return Vector con las componentes x, y, z extraídas.
   */

  vector parse_vector_from_string(std::string s) {
    std::istringstream iss(s);
    float x{}, y{}, z{};  // inicializamos
    if (!(iss >> x >> y >> z)) {
      std::cerr << "Error parseando vector desde string: [" << s << "]\n";
      return {1.F, 0.F, 1.F};  // Return default on error
    }
    return {x, y, z};
  }

  /**
   * @brief Calcula el vector reflejado de una dirección incidente respecto a una normal.
   * @param v_in Vector incidente.
   * @param normal Normal de la superficie.
   * @return Vector reflejado.
   */

  vector reflect(vector const & v_in, vector const & normal) {
    return v_in - 2.F * dot(v_in, normal) * normal;
  }

  /**
   * @brief Calcula la refracción de un rayo en la frontera entre dos medios.
   *
   * Usa la ley de Snell para determinar el nuevo vector de dirección. Si no hay
   * refracción posible (reflexión total interna), devuelve std::nullopt.
   *
   * @param v_in_unit Vector incidente unitario.
   * @param normal Normal de la superficie.
   * @param etai_over_etat Relación de índices de refracción entre ambos medios.
   * @return Vector refractado o std::nullopt si no hay transmisión.
   */

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

  /**
   * @brief Inicializa la cámara de renderizado a partir de los parámetros de configuración.
   *
   * Calcula la posición de la cámara, la orientación y la ventana de proyección
   * en función del campo de visión y la relación de aspecto definidos en el archivo de
   * configuración.
   *
   * @param cfg Estructura Config con los parámetros de cámara e imagen.
   */

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

  /**
   * @brief Genera un rayo desde la cámara hacia el píxel indicado en el plano de imagen.
   *
   * Los parámetros x_jit y y_jit incluyen un desplazamiento aleatorio (jitter)
   * para aplicar antialiasing.
   *
   * @param x_jit Coordenada X del píxel con desplazamiento aleatorio.
   * @param y_jit Coordenada Y del píxel con desplazamiento aleatorio.
   * @return Rayo normalizado que parte de la cámara y atraviesa el píxel correspondiente.
   */

  Ray Camera::get_ray(float x_jit, float y_jit) const {
    vector const pixel_center = m_origin_ventana + (x_jit * m_delta_x) + (y_jit * m_delta_y);

    //"Crea un nuevo rayo de luz que comience en la posición de mi cámara (m_camera_origin)
    // y apunte en la dirección que va desde la cámara hacia el punto pixel_center."
    return {m_camera_origin, (pixel_center - m_camera_origin).normalized()};
  }

  /**
   * @brief Calcula el color resultante de un rayo al interactuar con la escena.
   *
   * El color se obtiene de forma recursiva aplicando los modelos de material:
   *  - **Matte:** reflexión difusa aleatoria.
   *  - **Metal:** reflexión especular con rugosidad.
   *  - **Refractive:** transmisión según índice de refracción.
   *
   * Si no hay intersección, devuelve el color de fondo interpolado.
   *
   * @param r Rayo lanzado desde la cámara o rebote previo.
   * @param scene Escena con objetos y materiales.
   * @param ctx Contexto de render con parámetros de iluminación y RNG.
   * @param depth Profundidad máxima de recursión para los rebotes.
   * @return Vector RGB con el color resultante.
   */

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
          Ray bounced_ray(hit->point, direction);
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
  }

}  // namespace render
