#pragma once
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace render {

  // === Tipos de materiales ===
  struct Material {
    std::string name;
    std::string type;           // matte | metal | refractive
    std::vector<float> params;  // reflectancia o índice de refracción
  };

  // === Tipos de objetos ===
  struct Sphere {
    float cx{}, cy{}, cz{}, r{};
    std::string material;
  };

  struct Cylinder {
    float cx{}, cy{}, cz{}, r{};
    float ax{}, ay{}, az{};
    std::string material;
  };

  // === Escena completa ===
  struct Scene {
    std::unordered_map<std::string, Material> materials;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;
  };

  // === Función de lectura ===
  // ¡Asegúrate de que esta línea es una DECLARACIÓN (termina en ';')!
  Scene read_scene(std::string const & filename);

}  // namespace render
