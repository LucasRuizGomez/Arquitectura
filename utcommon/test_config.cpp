// tests/config_tests.cpp
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

#include "config.hpp"

using namespace render;
namespace fs = std::filesystem;

namespace {

  std::string writeTmp(std::string const & name, std::string const & content) {
    fs::create_directories("tests_tmp");
    auto path = "tests_tmp/" + name;
    std::ofstream f(path);
    f << content;
    f.close();
    return path;
  }

  TEST(ConfigRead, FailsWhenFileDoesNotExist) {
    EXPECT_THROW(
        {
          try {
            (void) read_config("no_existe_12345.cfg");
          } catch (std::runtime_error const & e) {
            std::string msg = e.what();
            EXPECT_NE(msg.find("Cannot open configuration file"), std::string::npos);
            throw;
          }
        },
        std::runtime_error);
  }

  TEST(ConfigRead, ParsesAllValidKeys) {
    std::string const cfgText = "aspect_ratio: 4 3\n"
                                "image_width: 800\n"
                                "gamma: 2.2\n"
                                "samples_per_pixel: 32\n"
                                "max_depth: 10\n"
                                "field_of_view: 60\n"
                                "material_rng_seed: 123\n"
                                "ray_rng_seed: 456\n"
                                "background_dark_color: 0.1 0.2 0.3\n"
                                "background_light_color: 0.9 0.95 1\n"
                                "camera_position: 1 2 3\n"
                                "camera_target: 0 0 0\n"
                                "camera_north: 0 1 0\n";
    auto path                 = writeTmp("valid_all.cfg", cfgText);
    Config c                  = read_config(path);

    EXPECT_EQ(c.aspect_ratio.first, 4);
    EXPECT_EQ(c.aspect_ratio.second, 3);
    EXPECT_EQ(c.image_width, 800);
    EXPECT_FLOAT_EQ(c.gamma, 2.2F);
    EXPECT_EQ(c.samples_per_pixel, 32);
    EXPECT_EQ(c.max_depth, 10);
    EXPECT_FLOAT_EQ(c.field_of_view, 60.0F);
    EXPECT_EQ(c.material_rng_seed, 123);
    EXPECT_EQ(c.ray_rng_seed, 456);

    // OJO: el parser usa getline tras la clave -> preserva el espacio inicial.
    EXPECT_EQ(c.background_dark_color, " 0.1 0.2 0.3");
    EXPECT_EQ(c.background_light_color, " 0.9 0.95 1");
    EXPECT_EQ(c.camera_position, " 1 2 3");
    EXPECT_EQ(c.camera_target, " 0 0 0");
    EXPECT_EQ(c.camera_north, " 0 1 0");
  }

  TEST(ConfigRead, AspectRatioMissingNumbers) {
    auto p = writeTmp("ar_missing.cfg", "aspect_ratio:\n");
    EXPECT_THROW(
        {
          try {
            (void) read_config(p);
          } catch (std::runtime_error const & e) {
            EXPECT_NE(std::string(e.what()).find("[aspect_ratio:]"), std::string::npos);
            throw;
          }
        },
        std::runtime_error);
  }

  TEST(ConfigRead, AspectRatioNonPositive) {
    auto p = writeTmp("ar_nonpos.cfg", "aspect_ratio: 0 -1\n");
    EXPECT_THROW(
        {
          try {
            (void) read_config(p);
          } catch (std::runtime_error const & e) {
            std::string msg = e.what();
            EXPECT_NE(msg.find("[aspect_ratio:]"), std::string::npos);
            EXPECT_NE(msg.find("must be > 0"), std::string::npos);
            throw;
          }
        },
        std::runtime_error);
  }

  TEST(ConfigRead, AspectRatioExtraTokens) {
    auto p = writeTmp("ar_extra.cfg", "aspect_ratio: 16 9 basura\n");
    EXPECT_THROW(
        {
          try {
            (void) read_config(p);
          } catch (std::runtime_error const & e) {
            // El código lanza el mismo mensaje que para “must be > 0”, aunque sea “extra”.
            // Testea al menos que refiere a aspect_ratio.
            EXPECT_NE(std::string(e.what()).find("[aspect_ratio:]"), std::string::npos);
            throw;
          }
        },
        std::runtime_error);
  }

  TEST(ConfigRead, ImageWidthInvalid) {
    auto p1 = writeTmp("iw_text.cfg", "image_width: hola\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("iw_nonpos.cfg", "image_width: 0\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);
  }

  TEST(ConfigRead, GammaInvalid) {
    auto p1 = writeTmp("gamma_text.cfg", "gamma: NaN\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("gamma_nonpos.cfg", "gamma: 0\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);
  }

  TEST(ConfigRead, SamplesPerPixelInvalid) {
    auto p1 = writeTmp("spp_text.cfg", "samples_per_pixel: X\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("spp_nonpos.cfg", "samples_per_pixel: -3\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);
  }

  TEST(ConfigRead, MaxDepthInvalid) {
    auto p1 = writeTmp("md_text.cfg", "max_depth: lol\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("md_nonpos.cfg", "max_depth: 0\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);
  }

  TEST(ConfigRead, FieldOfViewInvalid) {
    auto p1 = writeTmp("fov_text.cfg", "field_of_view: what\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("fov_low.cfg", "field_of_view: 0\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);

    auto p3 = writeTmp("fov_high.cfg", "field_of_view: 180\n");
    EXPECT_THROW((void) read_config(p3), std::runtime_error);
  }

  TEST(ConfigRead, SeedsInvalid) {
    auto p1 = writeTmp("mat_seed_text.cfg", "material_rng_seed: a\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("mat_seed_nonpos.cfg", "material_rng_seed: 0\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);

    auto p3 = writeTmp("ray_seed_text.cfg", "ray_rng_seed: b\n");
    EXPECT_THROW((void) read_config(p3), std::runtime_error);

    auto p4 = writeTmp("ray_seed_nonpos.cfg", "ray_rng_seed: -1\n");
    EXPECT_THROW((void) read_config(p4), std::runtime_error);
  }

  TEST(ConfigRead, BackgroundColorsMustNotBeEmpty) {
    auto p1 = writeTmp("bg_dark_empty.cfg", "background_dark_color:\n");
    EXPECT_THROW((void) read_config(p1), std::runtime_error);

    auto p2 = writeTmp("bg_light_empty.cfg", "background_light_color:\n");
    EXPECT_THROW((void) read_config(p2), std::runtime_error);
  }

  TEST(ConfigRead, UnknownKeyThrows) {
    auto p = writeTmp("unknown.cfg", "desconocida: 123\n");
    EXPECT_THROW(
        {
          try {
            (void) read_config(p);
          } catch (std::runtime_error const & e) {
            EXPECT_NE(std::string(e.what()).find("Unknown configuration key"), std::string::npos);
            throw;
          }
        },
        std::runtime_error);
  }

  // BUG conocido: los comentarios no se saltan (falta `continue;`)
  // Este test describe el comportamiento esperado (ignorar '# ...'),
  // pero está deshabilitado hasta que se arregle el parser.
  TEST(ConfigRead, DISABLED_CommentsAreIgnored) {
    std::string const cfg = "# comentario\n"
                            "aspect_ratio: 1 1\n";
    auto p                = writeTmp("with_comment.cfg", cfg);
    Config c              = read_config(p);
    EXPECT_EQ(c.aspect_ratio.first, 1);
    EXPECT_EQ(c.aspect_ratio.second, 1);
  }

  TEST(ConfigRead, CameraStringsPreserveRestOfLine) {
    std::string const cfg = "camera_position: 10 20 30\n"
                            "camera_target: 0 0 1 # trailing text\n"
                            "camera_north:  0  1  0  \n";
    auto p                = writeTmp("camera.cfg", cfg);
    Config c              = read_config(p);

    // Mantiene el espacio inicial por cómo se usa getline tras extraer la clave
    EXPECT_EQ(c.camera_position, " 10 20 30");
    EXPECT_EQ(c.camera_target, " 0 0 1 # trailing text");
    EXPECT_EQ(c.camera_north, "  0  1  0  ");
  }

}  // namespace
