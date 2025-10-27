#pragma once

// Incluimos todas las dependencias
#include "config.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "rng.hpp"  // <-- ¡Incluimos el RNG que acabamos de crear!
#include "scene.hpp"
#include "vector.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <sstream>

namespace render {

  // --- Namespace anónimo para helpers internos ---
  namespace {

    // --- HELPER PARA PARSEAR VECTORES ---
    static vector parse_vector_from_string(std::string s) {
      std::istringstream iss(s);
      float x, y, z;
      if (!(iss >> x >> y >> z)) {
        std::cerr << "Error parseando vector desde string: [" << s << "]\n";
        return {1.F, 0.F, 1.F};
      }
      return {x, y, z};
    }

    // --- HELPER PARA REFLEXIÓN (PDF Ec. 35) ---
    static vector reflect(vector const & v_in, vector const & normal) {
      return v_in - 2.F * dot(v_in, normal) * normal;
    }

    // --- HELPER PARA REFRACCIÓN (PDF Ec. 37 - Ley de Snell) ---
    // Devuelve std::nullopt si hay Reflexión Interna Total
    static std::optional<vector> refract(vector const & v_in_unit, vector const & normal,
                                         float etai_over_etat) {
      float const cos_theta = std::min(dot(-v_in_unit, normal), 1.0F);

      vector r_out_perp = etai_over_etat * (v_in_unit + cos_theta * normal);
      float desc_sq     = 1.0F - r_out_perp.length_squared();

      // Reflexión interna total
      if (desc_sq < 0.0F) {
        return std::nullopt;
      }

      vector r_out_para = -std::sqrtf(desc_sq) * normal;
      return r_out_perp + r_out_para;
    }

    // --- HELPER PARA APROXIMACIÓN DE SCHLICK (PDF Ec. 38) ---
    static float schlick(float cosine, float ref_idx) {
      float r0 = (1.F - ref_idx) / (1.F + ref_idx);
      r0       = r0 * r0;
      return r0 + (1.F - r0) * std::powf(1.F - cosine, 5.F);
    }

    // --- CLASE DE CÁMARA ---
    class Camera {
      // ... (Tu clase Camera. Sin cambios) ...

    public:
      // VENTANA DE PROYECCION
      Camera(vector lookfrom, vector lookat, vector vup, float vfov, float aspect_ratio) {
        auto const theta           = vfov * (std::numbers::pi_v<float> / 180.0F);
        auto const h               = std::tan(theta / 2.0F);
        auto const viewport_height = 2.0F * h;
        auto const viewport_width  = aspect_ratio * viewport_height;
        w                          = (lookfrom - lookat).normalized();
        u                          = cross(vup, w).normalized();
        v                          = cross(w, u);
        origin                     = lookfrom;
        horizontal                 = viewport_width * u;
        vertical                   = viewport_height * v;
        lower_left_corner          = origin - horizontal / 2.0F - vertical / 2.0F - w;
      }

      [[nodiscard]] Ray get_ray(float s, float t) const {
        return {origin, lower_left_corner + s * horizontal + t * vertical - origin};
      }

    private:
      vector origin, lower_left_corner, horizontal, vertical, u, v, w;
    };

    // --- CONTEXTO DE RENDERIZADO (Actualizado) ---
    struct RenderContext {
      vector bg_dark;
      vector bg_light;
      float inv_gamma;
      int max_depth;
      RNG material_rng;  // <-- ¡Añadido!
      RNG ray_rng;       // Optimizacion de Antialiasing para eliminar ruido en la imagen
    };

    // BORRADO DE ESTOS VECTORES DE FISICA DUPLICADOS
    /*
        // (PDF Ec. 35) Calcula la reflexión de un vector 'v' en una normal 'n'
        inline vector reflect(vector const & v, vector const & n) {
          return v - 2.F * dot(v, n) * n;
        }

        // (PDF Ec. 37) Calcula la refracción (Ley de Snell)
        inline vector refract(vector const & uv, vector const & n, float etai_over_etat) {
          auto cos_theta        = std::min(dot(-uv, n), 1.0F);
          vector r_out_perp     = etai_over_etat * (uv + cos_theta * n);
          vector r_out_parallel = -std::sqrtf(std::fabs(1.0F - r_out_perp.length_squared())) * n;
          return r_out_perp + r_out_parallel;
        }

        // (PDF Ec. 38) Aproximación de Schlick para reflectividad
        inline float schlick(float cosine, float ref_idx) {
          auto r0 = (1.F - ref_idx) / (1.F + ref_idx);
          r0      = r0 * r0;
          return r0 + (1.F - r0) * std::pow(1.F - cosine, 5.F);
        } */

    // --- FUNCIÓN PRINCIPAL DE COLOR (¡Implementación completa!) ---
    vector ray_color(Ray const & r, render::Scene const & scene,
                     RenderContext & ctx,  // Pasa por referencia para usar el RNG
                     int depth             // Profundidad actual
    ) {
      // Condición de parada (PDF Sec 3.5.1)
      if (depth <= 0) {
        return {0.F, 0.F, 0.F};  // Negro
      }

      float const infinity = std::numeric_limits<float>::infinity();

      if (auto hit = render::hit_scene(scene, r, 0.001F, infinity)) {
        try {
          Material const & mat = scene.materials.at(hit->material_name);
          vector albedo;  // Color base del material

          // --- LÓGICA DE MATERIAL 'MATTE' (PDF Sec 3.5.1) ---
          if (mat.type == "matte" and mat.params.size() >= 3) {
            albedo = {mat.params[0], mat.params[1], mat.params[2]};
            vector bounce_direction =
                (hit->normal + ctx.material_rng.random_in_unit_sphere()).normalized();
            Ray bounced_ray(hit->point, bounce_direction);
            return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);
          }

          // --- LÓGICA DE MATERIAL 'METAL' (PDF Sec 3.5.2) ---
          if (mat.type == "metal" and mat.params.size() >= 4) {
            albedo           = {mat.params[0], mat.params[1], mat.params[2]};
            float roughness  = mat.params[3];
            vector reflected = reflect(r.direction(), hit->normal);
            // Añadir "rugosidad" (fuzz)
            vector bounce_direction =
                (reflected + roughness * ctx.material_rng.random_in_unit_sphere()).normalized();
            Ray bounced_ray(hit->point, bounce_direction);

            // Si el rebote se mete "dentro" del objeto, se absorbe
            if (dot(bounced_ray.direction(), hit->normal) <= 0.F) {
              return {0.F, 0.F, 0.F};
            }
            return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);
          }

          // NUEVA LOGICA REGRATIVE

          if (mat.type == "refractive" and mat.params.size() >= 1) {
            albedo    = {1.0F, 1.0F, 1.0F};  // Dieléctricos son transparentes
            float ior = mat.params[0];       // Índice de Refracción

            // Determina si el rayo entra o sale
            bool const front_face      = dot(r.direction(), hit->normal) < 0.F;
            vector const normal        = front_face ? hit->normal : -hit->normal;
            float const etai           = front_face ? 1.0F : ior;
            float const etat           = front_face ? ior : 1.0F;
            float const etai_over_etat = etai / etat;

            float const cos_theta = std::min(dot(-r.direction(), normal), 1.0F);

            // Probabilidad de reflejar (Schlick)
            float reflect_prob = schlick(cos_theta, ior);

            // Decide aleatoriamente si reflejar o refractar
            if (reflect_prob > ctx.material_rng.random_float()) {
              vector reflected = reflect(r.direction(), normal);
              Ray bounced_ray(hit->point, reflected.normalized());
              return ray_color(bounced_ray, scene, ctx, depth - 1);
            }

            // --- INICIO DE LA SECCIÓN CORREGIDA ---

            // REFRACCIÓN
            // Llama a la versión que devuelve std::optional
            auto refracted_opt = refract(r.direction(), normal, etai_over_etat);

            // Comprueba si hay Reflexión Interna Total (si refract() devuelve nullopt)
            if (!refracted_opt) {
              // Si hay reflexión interna, actúa como un espejo
              vector reflected = reflect(r.direction(), normal);
              Ray bounced_ray(hit->point, reflected.normalized());
              return ray_color(bounced_ray, scene, ctx, depth - 1);
            }

            // Si llegamos aquí, SÍ hay refracción.
            // Desempaquetamos el valor de forma segura con *
            Ray bounced_ray(hit->point, (*refracted_opt).normalized());
            return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);

            // --- FIN DE LA SECCIÓN CORREGIDA ---
          }

          /*           // --- LÓGICA DE MATERIAL 'REFRACTIVE' (PDF Sec 3.5.3) ---
                    if (mat.type == "refractive" && mat.params.size() >= 1) {
                      albedo    = {1.0F, 1.0F, 1.0F};  // Los dieléctricos puros no tiñen la luz
                      float ior = mat.params[0];       // Índice de Refracción

                      // Determinamos si el rayo entra o sale del objeto
                      bool front_face      = dot(r.direction(), hit->normal) < 0.F;
                      vector normal        = front_face ? hit->normal : -hit->normal;
                      float etai           = front_face ? 1.0F : ior;  // Aire = 1.0
                      float etat           = front_face ? ior : 1.0F;
                      float etai_over_etat = etai / etat;

                      float cos_theta = std::min(dot(-r.direction(), normal), 1.0F);

                      // Reflectividad (Schlick)
                      float reflect_prob = schlick(cos_theta, ior);

                      // Reflexión (siempre ocurre un poco)
                      if (reflect_prob > ctx.material_rng.random_float()) {
                        vector reflected = reflect(r.direction(), normal);
                        Ray bounced_ray(hit->point, reflected.normalized());
                        return ray_color(bounced_ray, scene, ctx, depth - 1);
                      }

                      // Refracción (si no hay reflexión total interna)
                      float sin_theta = std::sqrtf(1.0F - cos_theta * cos_theta);
                      if (etai_over_etat * sin_theta > 1.0F) {
                        // Reflexión interna total (actúa como un espejo)
                        vector reflected = reflect(r.direction(), normal);
                        Ray bounced_ray(hit->point, reflected.normalized());
                        return ray_color(bounced_ray, scene, ctx, depth - 1);
                      }

                      vector refracted = refract(r.direction(), normal, etai_over_etat);
                      Ray bounced_ray(hit->point, refracted.normalized());
                      return albedo * ray_color(bounced_ray, scene, ctx, depth - 1);
                    } */

        } catch (std::out_of_range const &) {
          std::cerr << "Error: Material no encontrado: " << hit->material_name << '\n';
        }
        return {1.F, 0.F, 1.F};  // Error: Rosa
      }

      // Si no golpea nada, devuelve el color de fondo (cielo)
      float m = (r.direction().y() + 1.0F) * 0.5F;
      return (1.0F - m) * ctx.bg_light + m * ctx.bg_dark;
    }

    // --- HELPER DE ESCRITURA (Plantilla) ---
    template <typename ImageT>
    static void write_color(ImageT & image, int x, int y, render::vector color, float inv_gamma) {
      // Corrección Gamma (que ya tenías)
      color = render::vector(std::pow(color.x(), inv_gamma), std::pow(color.y(), inv_gamma),
                             std::pow(color.z(), inv_gamma));

      auto const r      = std::clamp(color.x(), 0.0F, 1.0F);
      auto const g      = std::clamp(color.y(), 0.0F, 1.0F);
      auto const b      = std::clamp(color.z(), 0.0F, 1.0F);
      auto const r_byte = static_cast<uint8_t>(r * 255.999F);
      auto const g_byte = static_cast<uint8_t>(g * 255.999F);
      auto const b_byte = static_cast<uint8_t>(b * 255.999F);
      image.set_pixel(x, y, r_byte, g_byte, b_byte);
    }

  }  // namespace

  // --- FUNCIÓN DEL BUCLE PRINCIPAL (Plantilla) ---
  template <typename ImageT>
  void run_render_loop(ImageT & image, render::Config const & cfg, render::Scene const & scene) {
    int const width         = image.width;
    int const height        = image.height;
    auto const aspect_w     = static_cast<float>(cfg.aspect_ratio.first);
    auto const aspect_h     = static_cast<float>(cfg.aspect_ratio.second);
    auto const aspect_ratio = aspect_w / aspect_h;

    Camera camera(parse_vector_from_string(cfg.camera_position),
                  parse_vector_from_string(cfg.camera_target),
                  parse_vector_from_string(cfg.camera_north), cfg.field_of_view, aspect_ratio);

    // --- CONTEXTO DE RENDERIZADO (Actualizado) ---
    RenderContext ctx{
      .bg_dark      = parse_vector_from_string(cfg.background_dark_color),
      .bg_light     = parse_vector_from_string(cfg.background_light_color),
      .inv_gamma    = 1.0F / cfg.gamma,  // Usamos el gamma del config
      .max_depth    = cfg.max_depth,
      .material_rng = RNG(cfg.material_rng_seed),  // ¡Inicializa el RNG!
      .ray_rng      = RNG(cfg.ray_rng_seed)        // Inicializamos ray_rng
    };

    // Bucle de Renderizado Actualizado

    for (int y = 0; y < height; ++y) {
      if (y % (height / 20) == 0 or y == height - 1) {
        std::print(std::cerr, "\rScanlines remaining: {:<4}", height - 1 - y);
      }

      for (int x = 0; x < width; ++x) {
        // Acumula el color para este píxel
        vector accumulated_color(0, 0, 0);

        // Bucle para Antialiasing (Multisampling)
        for (int s = 0; s < cfg.samples_per_pixel; ++s) {
          // Coordenadas UV con offset aleatorio
          float u =
              (static_cast<float>(x) + ctx.ray_rng.random_float()) / static_cast<float>(width - 1);
          float v = (static_cast<float>(height - 1 - y) + ctx.ray_rng.random_float()) /
                    static_cast<float>(height - 1);

          // Obtiene el rayo y calcula su color
          Ray r = camera.get_ray(u, v);
          accumulated_color += ray_color(r, scene, ctx, cfg.max_depth);
        }

        // Promedia el color acumulado
        auto final_color = accumulated_color / static_cast<float>(cfg.samples_per_pixel);

        // Escribe el color final en la imagen
        write_color(image, x, y, final_color, ctx.inv_gamma);
      }
    }

    /*     for (int y = 0; y < height; ++y) {
          if (y % (height / 20) == 0 || y == height - 1) {
            std::print(std::cerr, "\rScanlines remaining: {:<4}", height - 1 - y);
          }

          for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / static_cast<float>(width - 1);
            float v = static_cast<float>(height - 1 - y) / static_cast<float>(height - 1);

            Ray r = camera.get_ray(u, v);
            // ¡Llamada a la nueva función recursiva!
            vector pixel_color = ray_color(r, scene, ctx, ctx.max_depth);
            write_color(image, x, y, pixel_color, ctx.inv_gamma);
          }
        }
     */

    std::println(std::cerr, "\nRender complete.");
  }

}  // namespace render
