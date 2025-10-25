#include "config.hpp"
#include "hittable.hpp"
#include "image_soa.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "vector.hpp"  // <-- ¡Ahora es el principal!

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <print>
#include <sstream>
#include <string>

// --- Lógica de renderizado (idéntica a AOS) ---
namespace {

  // ¡Ya no necesitamos 'using render::Vec3f'
  using render::Ray;
  using render::vector;

  // --- HELPER PARA PARSEAR VECTORES ---
  static vector parse_vector_from_string(std::string s) {
    std::istringstream iss(s);
    float x, y, z;
    if (!(iss >> x >> y >> z)) {
      std::cerr << "Error parseando vector desde string: [" << s << "]\n";
      return {1.F, 0.F, 1.F};  // Error: Color Rosa
    }
    return {x, y, z};
  }

  // --- HELPER PARA ESCRIBIR COLOR (adaptado para ImageSOA) ---
  static void write_color(render::ImageSOA & image, int x, int y, vector color) {
    auto const r = std::clamp(color.x(), 0.0F, 1.0F);
    auto const g = std::clamp(color.y(), 0.0F, 1.0F);
    auto const b = std::clamp(color.z(), 0.0F, 1.0F);

    auto const r_byte = static_cast<uint8_t>(r * 255.999F);
    auto const g_byte = static_cast<uint8_t>(g * 255.999F);
    auto const b_byte = static_cast<uint8_t>(b * 255.999F);

    image.set_pixel(x, y, r_byte, g_byte, b_byte);
  }

  // --- CLASE DE CÁMARA ---
  class Camera {
  public:
    Camera(vector lookfrom, vector lookat, vector vup, float vfov, float aspect_ratio) {
      auto const theta           = vfov * (std::numbers::pi_v<float> / 180.0F);
      auto const h               = std::tan(theta / 2.0F);
      auto const viewport_height = 2.0F * h;
      auto const viewport_width  = aspect_ratio * viewport_height;

      w = (lookfrom - lookat).normalized();
      u = cross(vup, w).normalized();
      v = cross(w, u);

      origin            = lookfrom;
      horizontal        = viewport_width * u;
      vertical          = viewport_height * v;
      lower_left_corner = origin - horizontal / 2.0F - vertical / 2.0F - w;
    }

    [[nodiscard]] Ray get_ray(float s, float t) const {
      return {origin, lower_left_corner + s * horizontal + t * vertical - origin};
    }

  private:
    vector origin, lower_left_corner, horizontal, vertical;
    vector u, v, w;
  };

  // --- CONTEXTO DE RENDERIZADO ---
  struct RenderContext {
    vector bg_dark;
    vector bg_light;
  };

  // --- FUNCIÓN PRINCIPAL DE COLOR ---
  vector ray_color(Ray const & r, render::Scene const & scene, RenderContext const & ctx) {
    float const infinity = std::numeric_limits<float>::infinity();

    if (auto hit = render::hit_scene(scene, r, 0.001F, infinity)) {
      try {
        render::Material const & mat = scene.materials.at(hit->material_name);
        if (mat.type == "matte" && mat.params.size() >= 3) {
          return {mat.params[0], mat.params[1], mat.params[2]};
        }
      } catch (std::out_of_range const &) {
        std::cerr << "Error: Material no encontrado: " << hit->material_name << '\n';
      }
      return {1.F, 0.F, 1.F};  // Error: Rosa
    }

    float m = (r.direction().y() + 1.0F) * 0.5F;
    return (1.0F - m) * ctx.bg_light + m * ctx.bg_dark;
  }

}  // namespace

// --- MAIN (SOA) ---
int main(int argc, char * argv[]) {
  try {
    std::span<char *> args(argv, static_cast<size_t>(argc));

    if (argc != 4) {
      std::cerr << "Error: Invalid number of arguments: " << (argc - 1) << '\n';
      return 1;
    }

    std::string const config_file{args[1]};
    std::string const scene_file{args[2]};
    std::string const output_file{args[3]};

    render::Config const cfg  = render::read_config(config_file);
    render::Scene const scene = render::read_scene(scene_file);

    std::cout << "Loaded " << scene.materials.size() << " materials\n";
    std::cout << "Loaded " << scene.spheres.size() << " spheres\n";
    std::cout << "Loaded " << scene.cylinders.size() << " cylinders\n";

    int const width         = cfg.image_width;
    auto const aspect_w     = static_cast<float>(cfg.aspect_ratio.first);
    auto const aspect_h     = static_cast<float>(cfg.aspect_ratio.second);
    auto const aspect_ratio = aspect_w / aspect_h;
    auto const height       = static_cast<int>(static_cast<float>(width) / aspect_ratio);

    render::ImageSOA image(width, height);

    vector cam_pos    = parse_vector_from_string(cfg.camera_position);
    vector cam_target = parse_vector_from_string(cfg.camera_target);
    vector cam_north  = parse_vector_from_string(cfg.camera_north);

    RenderContext ctx;
    ctx.bg_dark  = parse_vector_from_string(cfg.background_dark_color);
    ctx.bg_light = parse_vector_from_string(cfg.background_light_color);

    Camera camera(cam_pos, cam_target, cam_north, cfg.field_of_view, aspect_ratio);

    std::println(std::cout, "Starting SOA rendering ({}x{})...", width, height);

    for (int y = 0; y < height; ++y) {
      if (y % (height / 20) == 0 || y == height - 1) {
        std::print(std::cerr, "\rScanlines remaining: {:<4}", height - 1 - y);
      }

      for (int x = 0; x < width; ++x) {
        float u = static_cast<float>(x) / static_cast<float>(width - 1);
        float v =
            static_cast<float>(height - 1 - y) / static_cast<float>(height - 1);  // Y invertida

        Ray r              = camera.get_ray(u, v);
        vector pixel_color = ray_color(r, scene, ctx);
        write_color(image, x, y, pixel_color);
      }
    }

    std::println(std::cerr, "\nRender complete.");
    std::println(std::cout, "Saving to {}", output_file);
    image.save_to_ppm(output_file);

  } catch (std::exception const & e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
