#pragma once

// Keep all includes
#include "config.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "rng.hpp"
#include "scene.hpp"
#include "vector.hpp"

#include <algorithm>  // Needed for std::clamp in write_color template
#include <cmath>      // Needed for std::pow in write_color template
#include <fstream>    // Needed for ImageT::save_to_ppm potentially
#include <iostream>   // Needed for std::cerr, std::println
#include <limits>     // Needed for infinity
#include <numbers>    // Needed for pi
#include <optional>   // Needed for refract
#include <sstream>    // Needed for parse_vector_from_string (in .cpp now)
#include <string>     // Needed for parse_vector_from_string (in .cpp now)

namespace render {

  // --- DECLARATIONS ONLY ---

  // Helper function declarations (definitions moved to .cpp)
  vector parse_vector_from_string(std::string s);
  vector reflect(vector const & v_in, vector const & normal);
  std::optional<vector> refract(vector const & v_in_unit, vector const & normal,
                                float etai_over_etat);

  // --- Class Definitions (Methods defined in .cpp) ---
  class Camera {
  public:
    // Constructor Declaration
    Camera(Config const & cfg);

    // Method Declaration
    [[nodiscard]] Ray get_ray(float x_jit, float y_jit) const;

  private:
    vector m_camera_origin;
    vector m_origin_ventana;
    vector m_delta_x;
    vector m_delta_y;
  };

  // Struct Definition (simple data structure)
  struct RenderContext {
    vector bg_dark;
    vector bg_light;
    float inv_gamma{};
    int max_depth{};
    RNG material_rng;
    RNG ray_rng;
  };

  // --- Main Color Function Declaration ---
  vector ray_color(Ray const & r, Scene const & scene, RenderContext & ctx, int depth);

  // --- TEMPLATE DEFINITIONS (Must stay in header) ---

  // Helper for writing color (Template)
  template <typename ImageT>

  // ESTO NO DEBERIA ESTAR EN EL RENDERER.CPP?¿?¿¿
  static void write_color(ImageT & image, int x, int y, render::vector color, float inv_gamma) {
    //
    // CORRECION GAMMA
    //
    // COLOR = COLOR^(1/gamma))
    //
    // IMPORTANTE --> REVISAR QUE LA INVERSA DE GAMMA SE ESTA PASANDO BIEN
    color = render::vector(std::pow(color.x(), inv_gamma), std::pow(color.y(), inv_gamma),
                           std::pow(color.z(), inv_gamma));

    // TRUNCADO DE VALORES
    auto const r = std::clamp(color.x(), 0.0F, 1.0F);
    auto const g = std::clamp(color.y(), 0.0F, 1.0F);
    auto const b = std::clamp(color.z(), 0.0F, 1.0F);

    // ESCALADO DE RANGO [0, 255]
    auto const r_byte = static_cast<uint8_t>(r * 255.999F);
    auto const g_byte = static_cast<uint8_t>(g * 255.999F);
    auto const b_byte = static_cast<uint8_t>(b * 255.999F);
    image.set_pixel(x, y, r_byte, g_byte, b_byte);
  }

  // Recorre la imagen y va lanzando rayos
  template <typename ImageT>
  void run_render_loop(ImageT & image, render::Config const & cfg, render::Scene const & scene) {
    int const width  = image.width;
    int const height = image.height;

    Camera camera(cfg);

    RenderContext ctx{.bg_dark      = parse_vector_from_string(cfg.background_dark_color),
                      .bg_light     = parse_vector_from_string(cfg.background_light_color),
                      .inv_gamma    = 1.0F / cfg.gamma,
                      .max_depth    = cfg.max_depth,
                      .material_rng = RNG(static_cast<uint64_t>(cfg.material_rng_seed)),
                      .ray_rng      = RNG(static_cast<uint64_t>(cfg.ray_rng_seed))};

    // Recorre los pixeles
    for (int y = 0; y < height; ++y) {
      if (y % (height / 20) == 0 or y == height - 1) {
        // Use std::cerr for progress output as before
        std::cerr << "\rScanlines remaining: " << (height - 1 - y) << "    ";
      }

      // Recorre los pixeles
      for (int x = 0; x < width; ++x) {
        vector accumulated_color(0, 0, 0);

        // Esto es el bucle para el ANTIALIASING --> random_float() funcion generadora de numeros
        // random rng.hpp
        for (int s = 0; s < cfg.samples_per_pixel; ++s) {
          float const delta_x =
              ctx.ray_rng.random_float() - 0.5F;  // Numero random x entre el intervalo [-0,5;0,5]
          float const delta_y =
              ctx.ray_rng.random_float() - 0.5F;  // Numero random y entre el intervalo [-0,5;0,5]
          float const x_jit = static_cast<float>(x) + delta_x;  // Suma Numero random x y columna
          float const y_jit = static_cast<float>(y) + delta_y;  // Suma Numero random y y fila
          Ray r             = camera.get_ray(x_jit, y_jit);

          // SUMA CONTRIBUCIONES DE COLOR
          accumulated_color += ray_color(r, scene, ctx, cfg.max_depth);
        }

        // PROMEDIO DE CONTRIBUCIONES DE COLOR
        auto final_color = accumulated_color / static_cast<float>(cfg.samples_per_pixel);
        write_color(image, x, y, final_color, ctx.inv_gamma);
      }
    }
    std::cerr << "\nRender complete.\n";
  }

}  // namespace render
