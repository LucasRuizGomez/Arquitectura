#pragma once
#include <string>

namespace render {

  struct Config {
    int image_width{1'920};
    float gamma{2.2F};
    int samples_per_pixel{20};
    int max_depth{5};

    std::string camera_position{"0 0 -10"};
    std::string camera_target{"0 0 0"};
    std::string camera_north{"0 1 0"};

    float field_of_view{90.0F};
    int material_rng_seed{13};
    int ray_rng_seed{19};

    std::string background_dark_color{"0.25 0.5 1"};
    std::string background_light_color{"1 1 1"};
  };

  Config read_config(std::string const & filename);

}  // namespace render
